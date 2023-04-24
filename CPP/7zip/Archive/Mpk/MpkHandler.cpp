// MpkHandler.cpp

#include "StdAfx.h"

#include "../../../../C/CpuArch.h"

#include "../../../Common/ComTry.h"
#include "../../../Common/IntToString.h"
#include "../../../Common/StringConvert.h"

#include "../../../Windows/PropVariant.h"

#include "../../Common/ProgressUtils.h"
#include "../../Common/StreamUtils.h"

#include "../Common/ItemNameUtils.h"

#include "MpkHandler.h"

#define Get32(p) GetUi32(p)

using namespace NWindows;

static void Get_AString_From_UString(const UString &src, AString &dst)
{
  UnicodeStringToMultiByte2(dst, src, CP_UTF8);
}

namespace NArchive {
namespace Mpk {

static const Byte kProps[] =
{
  kpidPath,
  kpidSize,
  kpidPackSize,
  kpidCTime,
  kpidCRC,
};

static const Byte kArcProps[] =
{
  kpidName,
  kpidMethod,
  kpidCRC,
};

IMP_IInArchive_Props
IMP_IInArchive_ArcProps

STDMETHODIMP CHandler::GetArchiveProperty(PROPID propID, PROPVARIANT *value)
{
  RINOK(NWindows::NCOM::PropVariant_Clear(value));

  COM_TRY_BEGIN
  NCOM::CPropVariant prop;
  switch (propID)
  {
    case kpidMethod: prop = "Deflate"; break;
    case kpidCRC: prop = _archive.CRC; break;

    case kpidErrorFlags:
    {
      UInt32 v = 0;
      prop = v;
      break;
    }
    
    case kpidName:
    {
      prop = _archive.GetName();
      break;
    }
  }
  return prop.Detach(value);
  COM_TRY_END
}


STDMETHODIMP CHandler::Open(IInStream *stream, const UInt64 *maxCheckStartPosition, IArchiveOpenCallback * openArchiveCallback)
{
  DEBUG_PRINT("Open\n");
  COM_TRY_BEGIN
  RINOK(Close());
  RINOK(_archive.Open(stream, maxCheckStartPosition, openArchiveCallback))
  _stream = stream;
  return S_OK;
  COM_TRY_END
}

STDMETHODIMP CHandler::Close()
{
  DEBUG_PRINT("Close\n");
  _archive.Clear();
  _stream = nullptr;
  return S_OK;
}

STDMETHODIMP CHandler::GetNumberOfItems(UInt32 *numItems)
{
  DEBUG_PRINT("GetNumberOfItems\n");
  *numItems = _archive.Items.Size();
  return S_OK;
}

bool CHandler::GetUncompressedSize(unsigned index, UInt32 &size) const
{
  DEBUG_PRINT("GetUncompressedSize\n");
  CItem const& item = _archive.Items[index];
  size = item.DataSize;
  return true;
}

bool CHandler::GetCompressedSize(unsigned index, UInt32 &size) const
{
  DEBUG_PRINT("GetCompressedSize\n");
  const CItem &item = _archive.Items[index];
  size = item.CompressedSize;
  return true;
}


STDMETHODIMP CHandler::GetProperty(UInt32 index, PROPID propID, PROPVARIANT *value)
{
  DEBUG_PRINT("GetProperty(%d, %d)\n", index, propID);
  RINOK(NWindows::NCOM::PropVariant_Clear(value));

  COM_TRY_BEGIN
  NCOM::CPropVariant prop;

  CItem const& item = _archive.Items[index];
  switch (propID)
  {
    case kpidPath: prop = item.Name; break;
    case kpidSize: prop = item.DataSize; break;
    case kpidPackSize: prop = item.CompressedSize; break;
    case kpidCRC: prop = item.CompressedCRC; break;
    case kpidCTime:
      auto timestamp = (item.Timestamp + 11644473600ULL) * 10000000ULL; // unix to windows filetime
      FILETIME ft = {
        (UInt32)timestamp,
        (UInt32)(timestamp >> 32)
      };
      prop = ft;
      break;
  }
  prop.Detach(value);
  return S_OK;
  COM_TRY_END
}

STDMETHODIMP CHandler::Extract(const UInt32 *indices, UInt32 numItems, Int32 testMode, IArchiveExtractCallback *extractCallback)
{
  DEBUG_PRINT("Extract %d, %d\n", numItems, numItems > 0 ? indices[0] : -1);
  (void)indices;
  (void)numItems;
  (void)testMode;
  (void)extractCallback;

  COM_TRY_BEGIN
  bool allFilesMode = numItems == ~(0u);
  if (allFilesMode)
    numItems = (UInt32)_archive.NumFiles;

  UInt64 totalSize = 0;
  for (UInt32 i = 0; i < numItems; i++)
    totalSize += _archive.Items[allFilesMode ? i : indices[i]].DataSize;
  extractCallback->SetTotal(totalSize);

  RINOK(extractCallback->PrepareOperation(testMode));

  for (UInt32 i = 0; i < numItems; ++i)
  {
    auto index = allFilesMode ? i : indices[i];
    auto item = _archive.Items[index];
    CMyComPtr<ISequentialOutStream> realOutStream;
    DEBUG_PRINT("GetStream...");
    RINOK(extractCallback->GetStream(index, &realOutStream, testMode));

    DEBUG_PRINT("Seek...");
    RINOK(_archive.InStream->Seek(item.Offset, STREAM_SEEK_SET, nullptr));
    DEBUG_PRINT("Decompress...");
    auto inStream = CMyComPtr<ISequentialInStream>(_archive.InStream);
    RINOK(Decompress(inStream, realOutStream));

    RINOK(extractCallback->SetOperationResult(S_OK));
  }
  return S_OK;
  COM_TRY_END
}

STDMETHODIMP CHandler::UpdateItems(ISequentialOutStream *outStream, UInt32 numItems, IArchiveUpdateCallback *callback)
{
  DEBUG_PRINT("UpdateItems %d\n", numItems);
  COM_TRY_BEGIN
  if (!callback)
      return E_FAIL;

  auto name = _archive.GetName();
  if (!name.Len())
    name = "new.mpk";
  COutArchive outArchive(name);

  CObjectVector<CItem> items;
  for (UInt32 i = 0; i < numItems; ++i)
  {
    Int32 newData;
    Int32 newProps;
    UInt32 indexInArchive;
    RINOK(callback->GetUpdateItemInfo(i, &newData, &newProps, &indexInArchive));
    CItem item;
    if (indexInArchive != (UInt32)(Int32)-1)
      item = _archive.Items[indexInArchive];
    if (IntToBool(newProps))
    {
      {
        NCOM::CPropVariant prop;
        RINOK(callback->GetProperty(i, kpidPath, &prop));
        DEBUG_PRINT("kpidPath %d", prop.vt);
        if (prop.vt == VT_EMPTY || prop.vt != VT_BSTR)
          return E_INVALIDARG;
        Get_AString_From_UString(prop.bstrVal, item.Name);
      }
      {
        NCOM::CPropVariant prop;
        RINOK(callback->GetProperty(i, kpidSize, &prop));
        DEBUG_PRINT("kpidSize %d", prop.vt);
        if (prop.vt == VT_EMPTY || prop.vt != VT_UI8)
          return E_INVALIDARG;
        item.DataSize = prop.uintVal;
      }
      {
        NCOM::CPropVariant prop;
        RINOK(callback->GetProperty(i, kpidCTime, &prop));
        DEBUG_PRINT("kpidCTime %d", prop.vt);
        if (prop.vt == VT_EMPTY || prop.vt != VT_FILETIME)
          return E_INVALIDARG;
        UInt64 timestamp = (UInt64(prop.filetime.dwHighDateTime) << 32) | prop.filetime.dwLowDateTime;
        item.Timestamp = UInt32(timestamp / 10000000ULL - 11644473600ULL);
      }
    }
    if (IntToBool(newData))
    {
      CMyComPtr<ISequentialInStream> itemData;
      RINOK(callback->GetStream(i, &itemData));
      outArchive.AddItemToCompress(item, *itemData);
    }
    else
    {
      _archive.InStream->Seek(item.Offset, STREAM_SEEK_SET, nullptr);
      outArchive.AddItem(item, *_archive.InStream);
    }
  }

  auto res = outArchive.Save(outStream);
  callback->SetOperationResult(S_OK);
  return res;
  COM_TRY_END
}

STDMETHODIMP CHandler::GetFileTimeType(UInt32 *type)
{
  DEBUG_PRINT("GetFileTimeType\n");
  COM_TRY_BEGIN

  UInt32 t = NFileTimeType::kUnix;
  *type = t;

  return S_OK;
  COM_TRY_END
}

}}
