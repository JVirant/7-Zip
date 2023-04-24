// MpkItem.h

#ifndef __ARCHIVE_MPAK_ITEM_H
#define __ARCHIVE_MPAK_ITEM_H

#if !1
#include <cstdio>
#if _WIN32
#define DEBUG_PRINT(...) {char cad[512]; sprintf(cad, "[7Z] " __VA_ARGS__); OutputDebugStringA(cad);}
#else
#define DEBUG_PRINT(...) {printf(__VA_ARGS__); printf("\n");}
#endif
#else
static inline void __UNUSED__(...) {}
#define DEBUG_PRINT(...) __UNUSED__(__VA_ARGS__)
#endif

#include "../../../../C/CpuArch.h"
#include "../../../Common/MyString.h"

namespace NArchive {
namespace Mpk {

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

}}

#endif
