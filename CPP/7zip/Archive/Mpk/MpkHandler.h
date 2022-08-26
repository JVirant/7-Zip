// MpkHandler.h

#ifndef __MPAK_HANDLER_H
#define __MPAK_HANDLER_H

#include "../../Common/CreateCoder.h"

#include "MpkIn.h"

namespace NArchive {
namespace Mpk {

class CHandler:
  public IInArchive,
  public CMyUnknownImp
{
  CArchive _archive;
  IInStream* _stream;

  bool GetUncompressedSize(unsigned index, UInt32 &size) const;
  bool GetCompressedSize(unsigned index, UInt32 &size) const;

public:
  MY_UNKNOWN_IMP1(IInArchive)

  INTERFACE_IInArchive(;)
};

}}

#endif
