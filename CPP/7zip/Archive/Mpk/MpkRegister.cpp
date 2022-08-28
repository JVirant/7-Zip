// MpkRegister.cpp

#include "StdAfx.h"

#include "../../Common/RegisterArc.h"

#include "MpkHandler.h"

namespace NArchive {
namespace Mpk {

REGISTER_ARC_I(
  "MPAK", "mpk", 0, 0xFF,
  kSignature,
  0,
  0,
  NULL)

}}
