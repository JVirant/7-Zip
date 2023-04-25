// MpkIn.h

#ifndef __ARCHIVE_MPAK_IN_H
#define __ARCHIVE_MPAK_IN_H

#include "./MpkItem.h"

#include "../../../Common/DynLimBuf.h"
#include "../../../Common/MyBuffer.h"
#include "../../../Common/MyCom.h"
#include "../../../Common/MyVector.h"

#include "../../Common/StreamObjects.h"
#include "../../Common/StreamUtils.h"

#include "../IArchive.h"

namespace NArchive {
namespace Mpk {

CByteBuffer Decompress(Byte const* src, size_t srcSize) throw();
CByteBuffer Decompress(CMyComPtr<ISequentialInStream> &src) throw();
HRESULT Decompress(Byte const* src, size_t srcSize, CMyComPtr<ISequentialOutStream> &outStream) throw();
HRESULT Decompress(CMyComPtr<ISequentialInStream> &src, CMyComPtr<ISequentialOutStream> &outStream) throw();

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
    InStream = nullptr;
    OpenCallback = nullptr;
    NumFiles = 0;
    Name.Wipe_and_Empty();
    Items.Clear();
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
