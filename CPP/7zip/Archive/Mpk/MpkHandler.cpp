// MpkHandler.cpp

#include "StdAfx.h"

#include "../../../../C/CpuArch.h"

#include "../../../Common/ComTry.h"
#include "../../../Common/IntToString.h"

#include "../../../Windows/PropVariant.h"

#include "../../Common/ProgressUtils.h"
#include "../../Common/StreamUtils.h"

#include "../Common/ItemNameUtils.h"

#include "MpkHandler.h"

#define Get32(p) GetUi32(p)

using namespace NWindows;

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
  Close();
  if (_archive.Open(stream, maxCheckStartPosition, openArchiveCallback) != S_OK)
    return S_FALSE;
  _stream = stream;
  return S_OK;
  COM_TRY_END
}

STDMETHODIMP CHandler::Close()
{
  DEBUG_PRINT("Close\n");
  _archive.Clear();
  if (_stream)
    _stream->Release();
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
    RINOK(extractCallback->GetStream(index, &realOutStream, testMode));

    _archive.InStream->Seek(item.Offset, STREAM_SEEK_SET, nullptr);
    RINOK(Decompress(*_archive.InStream, *realOutStream));

    RINOK(extractCallback->SetOperationResult(S_OK));
    realOutStream.Release();
  }
  return S_OK;
  COM_TRY_END
}

}}
