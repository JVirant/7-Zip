// MpkRegister.cpp

#include "StdAfx.h"

#include "../../Common/RegisterArc.h"

#include "MpkHandler.h"

namespace NArchive {
namespace Mpk {

REGISTER_ARC_IO(
  "MPAK", "mpk", 0, 0xFF,
  kSignature,
  0,
  0,
  TIME_PREC_TO_ARC_FLAGS_MASK (NFileTimeType::kWindows)
  | TIME_PREC_TO_ARC_FLAGS_MASK (NFileTimeType::kUnix)
  | TIME_PREC_TO_ARC_FLAGS_MASK (NFileTimeType::k1ns)
  | TIME_PREC_TO_ARC_FLAGS_TIME_DEFAULT (NFileTimeType::kUnix),
  NULL)

}}
