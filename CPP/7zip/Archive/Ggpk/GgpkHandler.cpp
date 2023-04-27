// GgpkHandler.cpp

#include "StdAfx.h"

#include "../../../../C/CpuArch.h"

#include "../../../Common/ComTry.h"
#include "../../../Common/StringConvert.h"

#include "../../../Windows/PropVariant.h"

#include "../../Common/LimitedStreams.h"
#include "../../Common/ProgressUtils.h"
#include "../../Common/StreamUtils.h"
#include "../../Compress/CopyCoder.h"

#include "../Common/ItemNameUtils.h"

#include "GgpkHandler.h"

#define Get32(p) GetUi32(p)

using namespace NWindows;

namespace NArchive {
namespace Ggpk {

static const Byte kProps[] =
{
  kpidPath,
  kpidIsDir,
  kpidSize,
  kpidSha256,
};

static const Byte kArcProps[] =
{
  kpidName,
  kpidMethod,
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
    case kpidMethod: prop = "Store"; break;

    case kpidErrorFlags:
    {
      UInt32 v = 0;
      prop = v;
      break;
    }

    case kpidName:
    {
      prop = "Content.ggpk";
      break;
    }
  }
  return prop.Detach(value);
  COM_TRY_END
}


STDMETHODIMP CHandler::Open(IInStream *stream, const UInt64 *maxCheckStartPosition, IArchiveOpenCallback * openArchiveCallback)
{
  DEBUG_PRINT("Open");
  COM_TRY_BEGIN
  RINOK(Close());
  RINOK(_archive.Open(stream, maxCheckStartPosition, openArchiveCallback));
  _stream = stream;
  return S_OK;
  COM_TRY_END
}

STDMETHODIMP CHandler::Close()
{
  DEBUG_PRINT("Close");
  _archive.Clear();
  _stream = nullptr;
  return S_OK;
}

STDMETHODIMP CHandler::GetNumberOfItems(UInt32 *numItems)
{
  DEBUG_PRINT("GetNumberOfItems");
  *numItems = _archive.Items.Size();
  return S_OK;
}

STDMETHODIMP CHandler::GetProperty(UInt32 index, PROPID propID, PROPVARIANT *value)
{
  RINOK(NWindows::NCOM::PropVariant_Clear(value));

  COM_TRY_BEGIN
  NCOM::CPropVariant prop;

  CItem const& item = _archive.Items[index];
  switch (propID)
  {
    case kpidPath: prop = item.Path + L'/' + item.Name; break;
    case kpidIsDir: prop = item.IsDirectory; break;
    case kpidSize: prop = item.Size; break;
    case kpidSha256:
    {
      #define GetHex(v) (char)(((v) < 10) ? ('0' + (v)) : ('a' + ((v) - 10)))
      char buffer[65];
      for (auto i = 0u; i < sizeof(item.Hash); ++i)
      {
        buffer[i * 2] = GetHex((item.Hash[i] >> 4) & 0xF);
        buffer[i * 2 + 1] = GetHex(item.Hash[i] & 0x0F);
      }
      buffer[64] = 0;
      prop = AString(buffer);
      break;
    }
  }

  prop.Detach(value);
  return S_OK;
  COM_TRY_END
}

STDMETHODIMP CHandler::Extract(const UInt32 *indices, UInt32 numItems, Int32 testMode, IArchiveExtractCallback *extractCallback)
{
  DEBUG_PRINT("Extract %d, %d", numItems, numItems > 0 ? indices[0] : -1);
  (void)indices;
  (void)numItems;
  (void)testMode;
  (void)extractCallback;

  COM_TRY_BEGIN
  bool allFilesMode = numItems == ~(0u);
  if (allFilesMode)
    numItems = (UInt32)_archive.Items.Size();

  UInt64 totalSize = 0;
  for (UInt32 i = 0; i < numItems; i++)
    totalSize += _archive.Items[allFilesMode ? i : indices[i]].Size;
  extractCallback->SetTotal(totalSize);

  RINOK(extractCallback->PrepareOperation(testMode));

  CMyComPtr<CLocalProgress> progress = new CLocalProgress();
  progress->Init(extractCallback, false);
  CMyComPtr<ICompressCoder> copyCoder = new NCompress::CCopyCoder();
  CMyComPtr<CLimitedSequentialInStream> inStream = new CLimitedSequentialInStream();
  inStream->SetStream(_archive.InStream);

  for (UInt32 i = 0; i < numItems; ++i)
  {
    auto index = allFilesMode ? i : indices[i];
    auto &item = _archive.Items[index];

    CMyComPtr<ISequentialOutStream> outStream;
    RINOK(extractCallback->GetStream(index, &outStream, testMode));
    RINOK(_archive.InStream->Seek(item.Offset, STREAM_SEEK_SET, nullptr));
    inStream->Init(item.Size);

    DEBUG_PRINT("Extract %ls/%ls %llu, %lluB isDir:%d", item.Path.Ptr(), item.Name.Ptr(), item.Offset, item.Size, item.IsDirectory);

    progress->InSize = item.Size;
    progress->OutSize = item.Size;

    RINOK(copyCoder->Code(inStream, outStream, nullptr, nullptr, progress));

    RINOK(extractCallback->SetOperationResult(NExtract::NOperationResult::kOK));
  }
  return S_OK;
  COM_TRY_END
}

}}
