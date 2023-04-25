// StreamUtils.cpp

#include "StdAfx.h"

#include "../../../C/CpuArch.h"

#include "StreamUtils.h"

static const UInt32 kBlockSize = ((UInt32)1 << 31);

HRESULT ReadStream(ISequentialInStream *stream, void *data, size_t *processedSize) throw()
{
  size_t size = *processedSize;
  *processedSize = 0;
  while (size != 0)
  {
    UInt32 curSize = (size < kBlockSize) ? (UInt32)size : kBlockSize;
    UInt32 processedSizeLoc;
    HRESULT res = stream->Read(data, curSize, &processedSizeLoc);
    *processedSize += processedSizeLoc;
    data = (void *)((Byte *)data + processedSizeLoc);
    size -= processedSizeLoc;
    RINOK(res);
    if (processedSizeLoc == 0)
      return S_OK;
  }
  return S_OK;
}

HRESULT ReadStream_FALSE(ISequentialInStream *stream, void *data, size_t size) throw()
{
  size_t processedSize = size;
  RINOK(ReadStream(stream, data, &processedSize));
  return (size == processedSize) ? S_OK : S_FALSE;
}

HRESULT ReadStream_FAIL(ISequentialInStream *stream, void *data, size_t size) throw()
{
  size_t processedSize = size;
  RINOK(ReadStream(stream, data, &processedSize));
  return (size == processedSize) ? S_OK : E_FAIL;
}

HRESULT WriteStream(ISequentialOutStream *stream, const void *data, size_t size) throw()
{
  while (size != 0)
  {
    UInt32 curSize = (size < kBlockSize) ? (UInt32)size : kBlockSize;
    UInt32 processedSizeLoc;
    HRESULT res = stream->Write(data, curSize, &processedSizeLoc);
    data = (const void *)((const Byte *)data + processedSizeLoc);
    size -= processedSizeLoc;
    RINOK(res);
    if (processedSizeLoc == 0)
      return E_FAIL;
  }
  return S_OK;
}


HRESULT ReadByte(ISequentialInStream *stream, Byte &data) throw()
{
  size_t size = 1;
  auto res = ReadStream(stream, &data, &size);
  if (res || size != 1)
    return S_FALSE;
  return S_OK;
}
HRESULT ReadUInt32LE(ISequentialInStream *stream, UInt32 &data) throw()
{
  size_t size = 4;
  auto res = ReadStream(stream, &data, &size);
  if (res || size != 4)
    return S_FALSE;
  data = GetUi32(&data);
  return S_OK;
}
HRESULT ReadUInt64LE(ISequentialInStream *stream, UInt64 &data) throw()
{
  size_t size = 8;
  auto res = ReadStream(stream, &data, &size);
  if (res || size != 8)
    return S_FALSE;
  data = GetUi64(&data);
  return S_OK;
}
HRESULT ReadBytes(ISequentialInStream *stream, Byte *array, size_t size) throw()
{
  size_t read = size;
  auto res = ReadStream(stream, array, &read);
  if (res || read != size)
    return S_FALSE;
  return S_OK;
}
HRESULT ReadCString(ISequentialInStream *stream, AString &string) throw()
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
