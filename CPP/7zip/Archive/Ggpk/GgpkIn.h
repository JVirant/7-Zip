// GgpkIn.h

#ifndef __ARCHIVE_GGPK_IN_H
#define __ARCHIVE_GGPK_IN_H

#include "./GgpkItem.h"

#include "../../../Common/MyCom.h"
#include "../../../Common/MyVector.h"

#include "../../Common/StreamObjects.h"
#include "../../Common/StreamUtils.h"

#include "../IArchive.h"

namespace NArchive {
namespace Ggpk {

class CArchive
{
public:
  CMyComPtr<IInStream> InStream;
  IArchiveOpenCallback *OpenCallback;
  CObjectVector<CItem> Items;
  UInt32 Version;
  UInt64 Completed;

  HRESULT Open(IInStream *stream, const UInt64 *maxCheckStartPosition, IArchiveOpenCallback *openCallback);

  void Clear()
  {
    InStream = nullptr;
    OpenCallback = nullptr;
    Items.Clear();
  }
};

}}
  
#endif
