// MpkOut.cpp

#include "StdAfx.h"

#include "../../../Windows/PropVariant.h"

#include "../../Common/StreamObjects.h"
#include "../../Common/StreamUtils.h"
#include "../../Compress/DeflateEncoder.h"
#include "../../Compress/ZlibEncoder.h"

#include "MpkOut.h"

namespace NArchive {
namespace Mpk {

static CByteBuffer Compress(ISequentialInStream &src, UInt64 srcSize, UInt32 &size, UInt32 &crc, int level = 5, ICompressProgressInfo *progress = nullptr)
{
  CDynBufSeqOutStream outStream;

  {
    NCompress::NZlib::CEncoder encoder;
    encoder.Create();

    NCompress::NDeflate::NEncoder::CEncProps props;
    props.Level = level;
    encoder.DeflateEncoderSpec->SetProps(&props);

    CMyComPtr<ICompressProgressInfo> compressProgress;
    if (progress)
      RINOK(progress->QueryInterface(IID_ICompressProgressInfo, (void**)&compressProgress))
    auto res = (*(ICompressCoder*)&encoder).Code(&src, &outStream, &srcSize, nullptr, compressProgress);
    if (res != 0)
      return CByteBuffer();
  }

  CByteBuffer buffer(outStream.GetSize());
  outStream.CopyToBuffer(buffer);
  size = (UInt32)buffer.Size();
  crc = CrcCalc(buffer, buffer.Size());
  DEBUG_PRINT("Compress... %llu %d %x", srcSize, size, crc);
  return buffer;
}

static CMyComPtr<ISequentialInStream> GetInStreamFromBuffer(CByteBuffer const &buffer)
{
  auto inStream = new CBufInStream();
  inStream->Init(buffer, buffer.Size());
  return inStream;
}

static HRESULT WriteUInt32LE(ISequentialOutStream &stream, UInt32 val)
{
  char buf[4];
  SetUi32(buf, val);
  return stream.Write(buf, sizeof(buf), nullptr);
}

HRESULT COutArchive::AddItemToCompress(CItem const &item, ISequentialInStream &data, int level, ICompressProgressInfo *progress)
{
  auto out = item;
  auto compressedBuffer = Compress(data, out.DataSize, out.CompressedSize, out.CompressedCRC, level, progress);
  DEBUG_PRINT("%s: compressed csize:%d crc:%08x", item.Name.Ptr(), out.CompressedSize, out.CompressedCRC);
  auto compressedStream = GetInStreamFromBuffer(compressedBuffer);
  auto res = this->AddItem(out, *compressedStream, nullptr);
  return res;
}

HRESULT COutArchive::AddItem(CItem const &item, ISequentialInStream &data, ICompressProgressInfo *progress)
{
  COutItem out;
  out.Name = item.Name;
  out.Timestamp = item.Timestamp;
  out.DataSize = item.DataSize;
  out.CompressedCRC = item.CompressedCRC;
  out.CompressedData = CByteBuffer(item.CompressedSize);
  RINOK(data.Read(out.CompressedData, item.CompressedSize, nullptr));

  if (progress)
  {
    UInt64 size = item.CompressedSize;
    progress->SetRatioInfo(&size, &size);
  }

  this->Items.Add(out);
  return S_OK;
}

HRESULT COutArchive::Save(ISequentialOutStream *outStream)
{
  RINOK(outStream->Write(kSignature, kSignatureSize, nullptr)); // Magic
  RINOK(outStream->Write("\2", 1, nullptr)); // version ?

  // Create index of files in MPK format
  CDynBufSeqOutStream bufferStream;
  ISequentialOutStream& bufferStreamRef = bufferStream;
  UInt32 offset = 0;
  UInt32 compressedOffset = 0;
  for (unsigned i = 0; i < this->Items.Size(); ++i)
  {
    auto item = this->Items[i];

    {
      char itemName[256];
      memset(itemName, 0, sizeof(itemName));
      memcpy(itemName, item.Name.Ptr(), MyMin(item.Name.Len(), 255u));
      RINOK(bufferStreamRef.Write(itemName, sizeof(itemName), nullptr));
    }

    RINOK(WriteUInt32LE(bufferStream, item.Timestamp));
    RINOK(WriteUInt32LE(bufferStream, 4)); // unknown, always 4?
    RINOK(WriteUInt32LE(bufferStream, offset));
    RINOK(WriteUInt32LE(bufferStream, item.DataSize));
    RINOK(WriteUInt32LE(bufferStream, compressedOffset));
    RINOK(WriteUInt32LE(bufferStream, (UInt32)item.CompressedData.Size()));
    RINOK(WriteUInt32LE(bufferStream, item.CompressedCRC));

    offset += item.DataSize;
    compressedOffset += (UInt32)item.CompressedData.Size();
  }
  CByteBuffer headerIndexData;
  UInt32 headerCompressedSize;
  UInt32 headerCRC;
  {
    bufferStream.CopyToBuffer(headerIndexData);
    auto headerInStream = GetInStreamFromBuffer(headerIndexData);
    headerIndexData = Compress(*headerInStream, headerIndexData.Size(), headerCompressedSize, headerCRC);
  }

  CByteBuffer name;
  UInt32 nameCompressedSize;
  UInt32 nameCRC;
  {
    bufferStream.Init();
    RINOK(bufferStreamRef.Write(this->ArchiveName.Ptr(), this->ArchiveName.Len() + 1, nullptr));
    bufferStream.CopyToBuffer(name);
    auto nameInStream = GetInStreamFromBuffer(name);
    name = Compress(*nameInStream, name.Size(), nameCompressedSize, nameCRC);
  }

  // Write header
  RINOK(WriteUInt32LE(*outStream, headerCRC ^ 0x03020100));
  RINOK(WriteUInt32LE(*outStream, headerCompressedSize ^ 0x07060504));
  RINOK(WriteUInt32LE(*outStream, nameCompressedSize ^ 0x0B0A0908));
  RINOK(WriteUInt32LE(*outStream, ((UInt32)this->Items.Size()) ^ 0x0F0E0D0C));
  RINOK(outStream->Write(name, nameCompressedSize, nullptr));
  RINOK(outStream->Write(headerIndexData, headerCompressedSize, nullptr));

  for (unsigned i = 0; i < this->Items.Size(); ++i)
  {
    auto item = this->Items[i];
    RINOK(outStream->Write(item.CompressedData, (UInt32)item.CompressedData.Size(), nullptr));
  }

  return S_OK;
}

}}
