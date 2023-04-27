// GgpkRegister.cpp

#include "StdAfx.h"

#include "../../Common/RegisterArc.h"

#include "GgpkHandler.h"

namespace NArchive {
namespace Ggpk {

static Byte const kSignature[] = { 'G', 'G', 'P', 'K' };

REGISTER_ARC_I(
  "GGPK", "ggpk", 0, 0xFE,
  kSignature,
  4,
  0,
  NULL)

}}
