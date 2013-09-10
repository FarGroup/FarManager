-- farapi.lua

local ffi = require "ffi"

ffi.cdef [[
typedef unsigned __int64 PLUGINPANELITEMFLAGS;

struct FarPanelItemFreeInfo
{
  size_t StructSize;
  HANDLE hPlugin;
};

typedef void (__stdcall *FARPANELITEMFREECALLBACK)(void* UserData, const struct FarPanelItemFreeInfo* Info);

struct UserDataItem
{
  void* Data;
  FARPANELITEMFREECALLBACK FreeData;
};

struct PluginPanelItemEx
{
  FILETIME CreationTime;
  FILETIME LastAccessTime;
  FILETIME LastWriteTime;
  FILETIME ChangeTime;
  unsigned __int64 FileSize;
  unsigned __int64 AllocationSize;
  const wchar_t *FileName;
  const wchar_t *AlternateFileName;
  const wchar_t *Description;
  const wchar_t *Owner;
  const wchar_t * const *CustomColumnData;
  size_t CustomColumnNumber;
  PLUGINPANELITEMFLAGS Flags;
  struct UserDataItem UserData;
  uintptr_t FileAttributes;
  uintptr_t NumberOfLinks;
  uintptr_t CRC32;
  intptr_t Position;
  intptr_t SortGroup;
  uintptr_t NumberOfStreams;
  unsigned __int64 StreamsSize;
};
]]
