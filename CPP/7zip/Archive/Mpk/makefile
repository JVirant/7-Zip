PROG = mpk.dll
DEF_FILE = ../Archive.def
CFLAGS = $(CFLAGS) \
  -DEXTERNAL_CODECS \

AR_OBJS = \
  $O\ArchiveExports.obj \
  $O\DllExports.obj \

7Z_OBJS = \
  $O\MpkHandler.obj \
  $O\MpkIn.obj \
  $O\MpkRegister.obj \

COMMON_OBJS = \
  $O\CRC.obj \
  $O\IntToString.obj \
  $O\MyString.obj \
  $O\StringConvert.obj \
  $O\StringToInt.obj \
  $O\MyVector.obj \
  $O\Wildcard.obj \

WIN_OBJS = \
  $O\DLL.obj \
  $O\FileDir.obj \
  $O\FileFind.obj \
  $O\FileIO.obj \
  $O\FileName.obj \
  $O\PropVariant.obj \
  $O\Synchronization.obj \
  $O\System.obj \

7ZIP_COMMON_OBJS = \
  $O\CreateCoder.obj \
  $O\InBuffer.obj \
  $O\InOutTempBuffer.obj \
  $O\FilterCoder.obj \
  $O\LimitedStreams.obj \
  $O\LockedStream.obj \
  $O\MethodId.obj \
  $O\MethodProps.obj \
  $O\OutBuffer.obj \
  $O\ProgressUtils.obj \
  $O\PropId.obj \
  $O\StreamBinder.obj \
  $O\StreamObjects.obj \
  $O\StreamUtils.obj \
  $O\VirtThread.obj \

COMPRESS_OBJS = \
  $O\CopyCoder.obj \
  $O\DeflateDecoder.obj \
  $O\ZlibDecoder.obj \

AR_COMMON_OBJS = \
  $O\BitlDecoder.obj \
  $O\CoderMixer2.obj \
  $O\HandlerOut.obj \
  $O\InStreamWithCRC.obj \
  $O\ItemNameUtils.obj \
  $O\LzOutWindow.obj \
  $O\MultiStream.obj \
  $O\OutStreamWithCRC.obj \
  $O\ParseProperties.obj \

C_OBJS = \
  $O\Alloc.obj \
  $O\CpuArch.obj \
  $O\Threads.obj \

!include "../../Crc.mak"

!include "../../7zip.mak"
