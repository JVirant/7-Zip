// MpkHandler.h

#ifndef __MPAK_HANDLER_H
#define __MPAK_HANDLER_H

#include "../../Common/CreateCoder.h"

#include "MpkIn.h"
#include "MpkOut.h"

namespace NArchive {
namespace Mpk {

class CHandler:
  public IInArchive,
  public IOutArchive,
  public CMyUnknownImp
{
  CArchive _archive;
  CMyComPtr<IInStream> _stream;

  bool GetUncompressedSize(unsigned index, UInt32 &size) const;
  bool GetCompressedSize(unsigned index, UInt32 &size) const;

public:
  MY_UNKNOWN_IMP2(IInArchive, IOutArchive)

  INTERFACE_IInArchive(;)
  INTERFACE_IOutArchive(;)
};

}}

#endif
