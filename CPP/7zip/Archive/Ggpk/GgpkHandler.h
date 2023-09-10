// GgpkHandler.h

#ifndef __ARCHIVE_GGPK_HANDLER_H
#define __ARCHIVE_GGPK_HANDLER_H

#include "../../Common/CreateCoder.h"

#include "GgpkIn.h"

namespace NArchive {
namespace Ggpk {

class CHandler Z7_final:
  public IInArchive,
  public CMyUnknownImp
{
  CArchive _archive;
  CMyComPtr<IInStream> _stream;

public:
  Z7_COM_UNKNOWN_IMP_1(IInArchive)

  Z7_IFACE_COM7_IMP(IInArchive)
};

}}

#endif
