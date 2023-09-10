// MpkHandler.h

#ifndef __MPAK_HANDLER_H
#define __MPAK_HANDLER_H

#include "../../Common/CreateCoder.h"

#include "MpkIn.h"
#include "MpkOut.h"

namespace NArchive {
namespace Mpk {

class CHandler Z7_final:
  public IInArchive,
  public IOutArchive,
  public ISetProperties,
  public CMyUnknownImp
{
  CArchive _archive;
  CMyComPtr<IInStream> _stream;
  int _compressionLevel = 5;

public:
  Z7_COM_QI_BEGIN2(IInArchive)
  Z7_COM_QI_ENTRY(IOutArchive)
  Z7_COM_QI_ENTRY(ISetProperties)
  Z7_COM_QI_END
  Z7_COM_ADDREF_RELEASE

  Z7_IFACE_COM7_IMP(IInArchive)
  Z7_IFACE_COM7_IMP(IOutArchive)
  STDMETHOD(SetProperties)(const wchar_t * const *names, const PROPVARIANT *values, UInt32 numProps);
};

}}

#endif
