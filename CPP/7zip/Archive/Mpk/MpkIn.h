// MpkIn.h

#ifndef __ARCHIVE_MPAK_IN_H
#define __ARCHIVE_MPAK_IN_H

#if !1
#include <cstdio>
#if _WIN32
#define DEBUG_PRINT(...) {char cad[512]; sprintf(cad, "[7Z] " __VA_ARGS__); OutputDebugStringA(cad);}
#else
#define DEBUG_PRINT(...) printf(__VA_ARGS__)
#endif
#else
static inline void __UNUSED__(...) {}
#define DEBUG_PRINT(...) __UNUSED__(__VA_ARGS__)
#endif

#include "../../../../C/CpuArch.h"

#include "../../../Common/DynLimBuf.h"
#include "../../../Common/MyBuffer.h"
#include "../../../Common/MyCom.h"
#include "../../../Common/MyVector.h"

#include "../../Common/StreamObjects.h"
#include "../../Common/StreamUtils.h"

#include "../IArchive.h"

namespace NArchive {
namespace Mpk {

HRESULT ReadByte(ISequentialInStream *stream, Byte& data) throw();
HRESULT ReadUInt32LE(ISequentialInStream *stream, UInt32& data) throw();
HRESULT ReadBytes(ISequentialInStream *stream, Byte* array, size_t size) throw();
HRESULT ReadCString(ISequentialInStream *stream, AString& string) throw();

CByteBuffer* Decompress(Byte const* src, size_t srcSize) throw();
CByteBuffer* Decompress(IInStream& src) throw();
HRESULT Decompress(Byte const* src, size_t srcSize, ISequentialOutStream& outStream) throw();
HRESULT Decompress(IInStream& src, ISequentialOutStream& outStream) throw();

const unsigned kSignatureSize = 4;
extern const Byte kSignature[kSignatureSize];
#define MPAK_SIGNATURE { 'M', 'P', 'A', 'K' }

struct CItem
{
  AString Name;
  UInt32 Timestamp;
  UInt32 DataSize;
  UInt32 Offset;
  UInt32 CompressedSize;
  UInt32 CompressedCRC;
};

class CArchive
{
public:
  CMyComPtr<IInStream> InStream;
  IArchiveOpenCallback *OpenCallback;
  UInt64 NumFiles;
  UInt32 CRC;
  CObjectVector<CItem> Items;

  void Clear()
  {
    InStream.Release();
    OpenCallback = NULL;
    NumFiles = 0;
    Name.Wipe_and_Empty();
  }

private:
  AString Name;

public:
  CArchive()
  {
  }
  HRESULT Open(IInStream *stream, const UInt64 *maxCheckStartPosition, IArchiveOpenCallback *openCallback);
  AString const& GetName() const { return Name; }
};

}}
  
#endif
