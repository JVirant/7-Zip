// MpkIn.cpp

#include "StdAfx.h"

#include "../../../Common/IntToString.h"
#include "../../../Common/StringToInt.h"

#include "../../Common/LimitedStreams.h"
#include "../../Common/StreamUtils.h"

#include "../../Compress/ZlibDecoder.h"

#include "MpkIn.h"

#define Get16(p) GetUi16(p)
#define Get32(p) GetUi32(p)

// #define NUM_SPEED_TESTS 1000

namespace NArchive {
namespace Mpk {

const Byte kSignature[kSignatureSize] = MPAK_SIGNATURE;

HRESULT CArchive::Open(IInStream *inStream, const UInt64 * /* maxCheckStartPosition */, IArchiveOpenCallback *openCallback)
{
  Clear();
  CrcGenerateTable();

  Byte sig[kSignatureSize];
  size_t sigSize = kSignatureSize;
  RINOK(ReadStream(inStream, sig, &sigSize));
  if (memcmp(sig, kSignature, kSignatureSize) != 0)
    return S_FALSE;
  RINOK(ReadByte(inStream, sig[0]));

  UInt32 checksum, dirsize, namesize, filecount;
  RINOK(ReadUInt32LE(inStream, checksum));
  RINOK(ReadUInt32LE(inStream, dirsize));
  RINOK(ReadUInt32LE(inStream, namesize));
  RINOK(ReadUInt32LE(inStream, filecount));
  checksum ^= 0x03020100;
  dirsize ^= 0x07060504;
  namesize ^= 0x0B0A0908;
  filecount ^= 0x0F0E0D0C;
  CRC = checksum;

  DEBUG_PRINT("checksum=%d", checksum);
  DEBUG_PRINT("dirsize=%d", dirsize);
  DEBUG_PRINT("namesize=%d", namesize);
  DEBUG_PRINT("filecount=%d", filecount);

  CByteBuffer buffer(max(dirsize, namesize));
  RINOK(ReadBytes(inStream, buffer, namesize));
  auto input = Decompress(buffer, namesize);
  Name.SetFrom((char const *)(unsigned char const *)input, (unsigned)input.Size());
  DEBUG_PRINT("archive name='%s' (%zub)", (char const*)Name, input.Size());

  RINOK(ReadBytes(inStream, buffer, dirsize));
  auto calculatedCRC = CrcCalc(buffer, dirsize);
  DEBUG_PRINT("CRC in file=%08x, calculated=%08x", CRC, calculatedCRC);
  if (calculatedCRC != CRC)
    return S_FALSE;
  input = Decompress(buffer, dirsize);

  CMyComPtr<CBufInStream> data = new CBufInStream();
  data->Init(input, input.Size());

  UInt64 currentOffset;
  inStream->Seek(0, STREAM_SEEK_CUR, &currentOffset);
  Byte buf[256];
  for (UInt32 i = 0; i < filecount; ++i) {
    CItem item;
    RINOK(ReadBytes(data, buf, 256));
    item.Name = (char const*)buf;
    RINOK(ReadUInt32LE(data, item.Timestamp));
    UInt32 tmp1;
    RINOK(ReadUInt32LE(data, tmp1));
    UInt32 offset;
    RINOK(ReadUInt32LE(data, offset));
    RINOK(ReadUInt32LE(data, item.DataSize));
    RINOK(ReadUInt32LE(data, item.Offset));
    item.Offset += (UInt32)currentOffset;
    RINOK(ReadUInt32LE(data, item.CompressedSize));
    RINOK(ReadUInt32LE(data, item.CompressedCRC));

    DEBUG_PRINT("%s: ts:%d u:%d off:%d size:%d coffset:%d csize:%d crc:%08x", item.Name.Ptr(), item.Timestamp, tmp1, offset, item.DataSize, item.Offset, item.CompressedSize, item.CompressedCRC);

    Items.Add(item);
  }

  InStream = inStream;
  OpenCallback = openCallback;
  return S_OK;
}

CByteBuffer Decompress(Byte const* src, size_t srcSize) throw()
{
  auto outStream = new CDynBufSeqOutStream();
  auto outPtr = CMyComPtr<ISequentialOutStream>(outStream);

  auto res = Decompress(src, srcSize, outPtr);
  if (res)
    return CByteBuffer();

  CByteBuffer buffer;
  outStream->CopyToBuffer(buffer);
  return buffer;
}

CByteBuffer Decompress(CMyComPtr<ISequentialInStream> &src) throw()
{
  auto outStream = new CDynBufSeqOutStream();
  auto outPtr = CMyComPtr<ISequentialOutStream>(outStream);

  auto res = Decompress(src, outPtr);
  if (res)
    return CByteBuffer();

  CByteBuffer buffer;
  outStream->CopyToBuffer(buffer);
  return buffer;
}

HRESULT Decompress(Byte const* src, size_t srcSize, CMyComPtr<ISequentialOutStream> &outStream) throw()
{
  auto inStream = new CBufInStream();
  auto inPtr = CMyComPtr<ISequentialInStream>(inStream);
  inStream->Init(src, srcSize);

  auto res = Decompress(inPtr, outStream);

  DEBUG_PRINT("Decompress() = %d ref=%d\n", res, inStream->__m_RefCount);
  return res;
}

/* Zlib decoder doesn't work with Mpk created by Mythic...
HRESULT Decompress(CMyComPtr<ISequentialInStream> &src, CMyComPtr<ISequentialOutStream> &outStream) throw()
{
  CMyComPtr<NCompress::NZlib::CDecoder> decoder = new NCompress::NZlib::CDecoder();
  return decoder->Code(src, outStream, nullptr, nullptr, nullptr);
}
*/
HRESULT Decompress(CMyComPtr<ISequentialInStream> &src, CMyComPtr<ISequentialOutStream> &outStream) throw()
{
  CMyComPtr<NCompress::NZlib::CDecoder> decoder = new NCompress::NZlib::CDecoder();
  return decoder->Code(src, outStream, nullptr, nullptr, nullptr);
}

}}
