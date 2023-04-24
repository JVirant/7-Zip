// MpkOut.h

#ifndef __ARCHIVE_MPAK_OUT_H
#define __ARCHIVE_MPAK_OUT_H

#include "./MpkItem.h"

#include "../../../Common/MyCom.h"
#include "../../../Common/MyBuffer.h"

#include "../../Common/OutBuffer.h"


namespace NArchive {
namespace Mpk {

struct COutItem
{
  AString Name;
  UInt32 Timestamp;
  UInt32 DataSize;
  UInt32 CompressedCRC;
  CByteBuffer CompressedData;
};

class COutArchive
{
  AString ArchiveName;
  CObjectVector<COutItem> Items;

public:
  HRESULT AddItemToCompress(CItem const &item, ISequentialInStream &data);
  HRESULT AddItem(CItem const &item, ISequentialInStream &data);
  HRESULT Save(ISequentialOutStream *outStream);

  COutArchive(AString const &name):
    ArchiveName(name)
  {}
};

}}

#endif
