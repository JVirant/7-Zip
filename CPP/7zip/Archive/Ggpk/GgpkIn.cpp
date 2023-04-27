// GgpkIn.cpp

#include "StdAfx.h"

#include <vector>

#include "GgpkIn.h"

#ifdef _MSC_VER
#pragma warning(disable: 4127)
#endif

namespace NArchive {
namespace Ggpk {
  static HRESULT ReadUString(IInStream *stream, UString &value, UInt32 length, UInt32 version)
  {
    if (version == 4) // UTF 32 (Little endian)
    {
      if (sizeof(wchar_t) != 4)
        return E_NOTIMPL; // TODO convert
    }
    else if (version == 3) // UTF 16 (Little endian)
    {
      if (sizeof(wchar_t) == 4)
      {
        auto data = (char16_t *)malloc(length * sizeof(char16_t));
        RINOK(stream->Read(data, length * 2, nullptr));
        value = UString();
        auto end = data + length;
        for (auto input = data; input < end;)
        {
          wchar_t uc = *input++;
          if ((uc - 0xd800u) >= 2048u)
            value += (wchar_t)uc;
          else if ((uc & 0xfffffc00u) == 0xd800u && input < end && (*input & 0xfffffc00u) == 0xdc00u)
            value += (wchar_t)((uc << 10) + wchar_t(*input++) - 0x35fdc00);
        }
        free(data);
        return S_OK;
      }
      if (sizeof(wchar_t) != 2)
        return E_NOTIMPL; // TODO convert
    }
    else
      return S_FALSE;

    auto data = SysAllocStringLen(nullptr, length);
    RINOK(stream->Read(data, length * sizeof(*data), nullptr));
    value = UString(data);
    SysFreeString(data);
    return S_OK;
  }

  struct CBaseRecord
  {
    UInt64 Offset;
    UInt32 Length;
    char Type[4];
  };
  struct CFileRecord
  {
    CBaseRecord Base;
    UString Name;
    UString Path;
    Byte Hash[32];
    UInt64 OffsetData;
    UInt64 Size;
  };
  struct CPdirRecord
  {
    CBaseRecord Base;
    Byte Hash[32];
    UString Name;
    UString Path;
  };

  static HRESULT ReadBaseRecord(CArchive *self, IInStream *stream, CBaseRecord &record)
  {
    stream->Seek(0, STREAM_SEEK_CUR, &record.Offset);
    RINOK(ReadUInt32LE(stream, record.Length));
    RINOK(stream->Read(record.Type, sizeof(record.Type), nullptr));

    self->Completed += record.Length;
    self->OpenCallback->SetCompleted(nullptr, &self->Completed);

    return S_OK;
  }
  static HRESULT ReadFileRecord(CArchive *self, IInStream *stream, CBaseRecord base, UString const &path)
  {
    UInt32 nameLength;

    CItem item;
    RINOK(ReadUInt32LE(stream, nameLength));
    RINOK(stream->Read(item.Hash, sizeof(item.Hash), nullptr));
    RINOK(ReadUString(stream, item.Name, nameLength, self->Version));
    item.Path = path;
    stream->Seek(0, STREAM_SEEK_CUR, &item.Offset);
    item.Size = base.Length - (item.Offset - base.Offset);

    item.IsDirectory = false;
    self->Items.Add(item);
    return S_OK;
  }
  static HRESULT ReadPdirRecord(CArchive *self, IInStream *stream, CBaseRecord base, UString const &path)
  {
    UInt32 nameLength;
    UInt32 entryCount;

    CItem item;
    stream->Seek(0, STREAM_SEEK_CUR, &item.Offset);
    item.Offset -= 8;
    item.Size = 0;
    RINOK(ReadUInt32LE(stream, nameLength));
    RINOK(ReadUInt32LE(stream, entryCount));
    RINOK(stream->Read(item.Hash, sizeof(item.Hash), nullptr));
    RINOK(ReadUString(stream, item.Name, nameLength, self->Version));
    item.Path = path;
    item.IsDirectory = true;
    self->Items.Add(item);

    struct CEntry
    {
      UInt32 Hash; // murmur2
      UInt64 Offset;
    };
    CRecordVector<CEntry> entries;
    entries.Reserve(entryCount);
    for (UInt32 i = 0; i < entryCount; ++i)
    {
      CEntry entry;
      RINOK(ReadUInt32LE(stream, entry.Hash));
      RINOK(ReadUInt64LE(stream, entry.Offset));
      entries.Add(entry);
    }

    auto recordPath = item.Path.Len() > 0 ? (item.Path + L'/' + item.Name) : item.Name;
    for (auto &entry: entries)
    {
      RINOK(stream->Seek(entry.Offset, STREAM_SEEK_SET, nullptr));
      RINOK(ReadBaseRecord(self, stream, base));

      if (memcmp(base.Type, "PDIR", 4) == 0)
      {
        RINOK(ReadPdirRecord(self, stream, base, recordPath));
      }
      else if (memcmp(base.Type, "FILE", 4) == 0)
      {
        RINOK(ReadFileRecord(self, stream, base, recordPath));
      }
      else
        DEBUG_PRINT("skip record %c%c%c%c (%uB)", base.Type[0], base.Type[1], base.Type[2], base.Type[3], base.Length);
    }

    UInt64 fileCount = self->Items.Size();
    self->OpenCallback->SetCompleted(&fileCount, nullptr);
    return S_OK;
  }


  HRESULT CArchive::Open(IInStream *stream, const UInt64 *maxCheckStartPosition, IArchiveOpenCallback *openCallback)
  {
    (void)maxCheckStartPosition;

    this->InStream = stream;
    this->OpenCallback = openCallback;

    UInt64 total;
    this->InStream->Seek(0, STREAM_SEEK_END, &total);
    this->InStream->Seek(0, STREAM_SEEK_SET, nullptr);
    this->OpenCallback->SetTotal(nullptr, &total);

    CBaseRecord ggpk;
    CPdirRecord root;
    RINOK(ReadBaseRecord(this, stream, ggpk));
    if (memcmp(ggpk.Type, "GGPK", 4) != 0)
      return E_FAIL;
    RINOK(ReadUInt32LE(stream, Version));
    UInt64 offsetRoot;
    UInt64 offsetFree;
    RINOK(ReadUInt64LE(stream, offsetRoot));
    RINOK(ReadUInt64LE(stream, offsetFree));

    RINOK(stream->Seek(offsetRoot, STREAM_SEEK_SET, nullptr));
    CBaseRecord base;
    RINOK(ReadBaseRecord(this, stream, base));
    RINOK(ReadPdirRecord(this, stream, base, UString("")));

    return S_OK;
  }

}}
