// GgpkHandler.h

#ifndef __ARCHIVE_GGPK_HANDLER_H
#define __ARCHIVE_GGPK_HANDLER_H

#include "../../Common/CreateCoder.h"

#include "GgpkIn.h"

namespace NArchive {
namespace Ggpk {

class CHandler:
  public IInArchive,
  public CMyUnknownImp
{
  CArchive _archive;
  CMyComPtr<IInStream> _stream;

public:
  MY_UNKNOWN_IMP1(IInArchive)

  INTERFACE_IInArchive(;)
};

}}

#endif
