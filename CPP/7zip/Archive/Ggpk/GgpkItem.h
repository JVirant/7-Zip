// GgpkItem.h

#ifndef __ARCHIVE_GGPK_ITEM_H
#define __ARCHIVE_GGPK_ITEM_H

#if !1
#include <cstdio>
#if defined(_WIN32) && _WIN32
#define DEBUG_PRINT(...) {char cad[512]; sprintf(cad, "[7Z] " __VA_ARGS__); OutputDebugStringA(cad);}
#else
#define DEBUG_PRINT(...) {fprintf(stderr, __VA_ARGS__); printf("\n");}
#endif
#else
static inline void __UNUSED__(...) {}
#define DEBUG_PRINT(...) __UNUSED__(__VA_ARGS__)
#endif

#include "../../../../C/CpuArch.h"
#include "../../../Common/MyBuffer.h"
#include "../../../Common/MyString.h"

namespace NArchive {
namespace Ggpk {

struct CItem
{
  UString Path;
  UString Name;
  UInt64 Size;
  UInt64 Offset;
  Byte Hash[32];
  bool IsDirectory;
};

}}

#endif
