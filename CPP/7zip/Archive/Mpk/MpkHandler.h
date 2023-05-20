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
  public ISetProperties,
  public CMyUnknownImp
{
  CArchive _archive;
  CMyComPtr<IInStream> _stream;
  int _compressionLevel = 5;

public:
  MY_QUERYINTERFACE_BEGIN2(IInArchive)
  MY_QUERYINTERFACE_ENTRY(IOutArchive)
  MY_QUERYINTERFACE_ENTRY(ISetProperties)
  MY_QUERYINTERFACE_END
  MY_ADDREF_RELEASE

  INTERFACE_IInArchive(;)
  INTERFACE_IOutArchive(;)
  STDMETHOD(SetProperties)(const wchar_t * const *names, const PROPVARIANT *values, UInt32 numProps);
};

}}

#endif
