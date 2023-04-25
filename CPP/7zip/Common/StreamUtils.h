// StreamUtils.h

#ifndef __STREAM_UTILS_H
#define __STREAM_UTILS_H

#include "../IStream.h"
#include "../../Common/MyString.h"

HRESULT ReadStream(ISequentialInStream *stream, void *data, size_t *size) throw();
HRESULT ReadStream_FALSE(ISequentialInStream *stream, void *data, size_t size) throw();
HRESULT ReadStream_FAIL(ISequentialInStream *stream, void *data, size_t size) throw();
HRESULT WriteStream(ISequentialOutStream *stream, const void *data, size_t size) throw();


HRESULT ReadByte(ISequentialInStream *stream, Byte &data) throw();
HRESULT ReadUInt32LE(ISequentialInStream *stream, UInt32 &data) throw();
HRESULT ReadUInt64LE(ISequentialInStream *stream, UInt64 &data) throw();
HRESULT ReadBytes(ISequentialInStream *stream, Byte *array, size_t size) throw();
HRESULT ReadCString(ISequentialInStream *stream, AString &string) throw();

#endif
