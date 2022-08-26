// MpkIn.cpp

#include "StdAfx.h"

#include "../../../Common/IntToString.h"
#include "../../../Common/StringToInt.h"

#include "../../Common/LimitedStreams.h"
#include "../../Common/StreamUtils.h"

#include "../../Compress/DeflateDecoder.h"
#include "../../Compress/ZlibDecoder.h"

#include "MpkIn.h"

#define Get16(p) GetUi16(p)
#define Get32(p) GetUi32(p)

// #define NUM_SPEED_TESTS 1000

namespace NArchive {
namespace Mpk {

static const size_t kInputBufSize = 1 << 20;

const Byte kSignature[kSignatureSize] = MPAK_SIGNATURE;
static const UInt32 kMask_IsCompressed = (UInt32)1 << 31;

static const unsigned kNumCommandParams = 6;
static const unsigned kCmdSize = 4 + kNumCommandParams * 4;

static const char * const kErrorStr = "$_ERROR_STR_";

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

  DEBUG_PRINT("checksum=%d\n", checksum);
  DEBUG_PRINT("dirsize=%d\n", dirsize);
  DEBUG_PRINT("namesize=%d\n", namesize);
  DEBUG_PRINT("filecount=%d\n", filecount);

  Byte* buffer = new Byte[max(dirsize, namesize)];
  RINOK(ReadBytes(inStream, buffer, namesize));
  auto input = Decompress(buffer, namesize);
  input[namesize] = 0;
  Name = (char const*)input->operator const unsigned char *();
  DEBUG_PRINT("archive name=%s\n", (char const*)Name);


  RINOK(ReadBytes(inStream, buffer, dirsize));
  if (CrcCalc(buffer, dirsize) != CRC)
    return S_FALSE;
  input = Decompress(buffer, dirsize);

  CBufInStream* data = new CBufInStream();
  data->Init(*input, input->Size());

  UInt64 currentOffset;
  inStream->Seek(0, STREAM_SEEK_CUR, &currentOffset);
  Byte buf[256];
  for (UInt32 i = 0; i < filecount; ++i) {
    CItem item;
    RINOK(ReadBytes(data, buf, 256));
    item.Name = (char const*)buf;
    RINOK(ReadUInt32LE(data, item.Timestamp));
    UInt32 tmp;
    RINOK(ReadUInt32LE(data, tmp));
    RINOK(ReadUInt32LE(data, tmp));
    RINOK(ReadUInt32LE(data, item.DataSize));
    RINOK(ReadUInt32LE(data, item.Offset));
    item.Offset += (UInt32)currentOffset;
    RINOK(ReadUInt32LE(data, item.CompressedSize));
    RINOK(ReadUInt32LE(data, item.CompressedCRC));

    Items.Add(item);
  }
  delete [] buffer;
  delete input;
  data->Release();

  InStream = inStream;
  OpenCallback = openCallback;
  return S_OK;
}

HRESULT ReadByte(ISequentialInStream *stream, Byte& data) throw()
{
  size_t size = 1;
  auto res = ReadStream(stream, &data, &size);
  if (res || size != 1)
    return S_FALSE;
  return S_OK;
}
HRESULT ReadUInt32LE(ISequentialInStream *stream, UInt32& data) throw()
{
  size_t size = 4;
  auto res = ReadStream(stream, &data, &size);
  if (res || size != 4)
    return S_FALSE;
#ifdef MY_CPU_BE
  data = GetUi32(data);
#endif
  return S_OK;
}
HRESULT ReadBytes(ISequentialInStream *stream, Byte* array, size_t size) throw()
{
  size_t read = size;
  auto res = ReadStream(stream, array, &read);
  if (res || read != size)
    return S_FALSE;
  return S_OK;
}
HRESULT ReadCString(ISequentialInStream *stream, AString& string) throw()
{
  Byte c;
  string = "";
  do {
    RINOK(ReadByte(stream, c));
    if (c)
      string += (char)c;
  } while (c);
  return S_OK;
}

CByteBuffer* Decompress(Byte const* src, size_t srcSize) throw()
{
  CDynBufSeqOutStream outStream;

  auto res = Decompress(src, srcSize, outStream);
  if (res)
    return nullptr;

  CByteBuffer* buffer = new CByteBuffer(outStream.GetSize());
  outStream.CopyToBuffer(*buffer);
  return buffer;
}

CByteBuffer* Decompress(IInStream& src) throw()
{
  CDynBufSeqOutStream outStream;
  auto res = Decompress(src, outStream);
  if (res)
    return nullptr;

  CByteBuffer* buffer = new CByteBuffer(outStream.GetSize());
  outStream.CopyToBuffer(*buffer);
  return buffer;
}

HRESULT Decompress(Byte const* src, size_t srcSize, ISequentialOutStream& outStream) throw()
{
  auto inStream = new CBufInStream();
  inStream->Init(src, srcSize);

  auto res = Decompress(*inStream, outStream);

  DEBUG_PRINT("Decompress() = %d ref=%d\n", res, inStream->__m_RefCount);
  inStream->Release();
  return res;
}

HRESULT Decompress(IInStream& src, ISequentialOutStream& outStream) throw()
{
  UInt16 _tmp;
  ReadBytes(&src, (Byte *)&_tmp, 2);

  auto decoder = new NCompress::NDeflate::NDecoder::CCoder(false);
  src.AddRef(); // the decoder will release one ref
  auto res = decoder->Code(&src, &outStream, nullptr, nullptr, nullptr);
  decoder->Release();

  return res;
}

}}
