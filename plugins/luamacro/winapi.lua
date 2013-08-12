-- Windows API - types, constants and functions - via LuaJIT FFI.
-- Started 2011-10-01 by Shmuel Zeigerman

local ffi = require "ffi"

if pcall(ffi.sizeof, "FFI_WINAPI_DEFINED") then return end
ffi.cdef "typedef int FFI_WINAPI_DEFINED"

if jit.arch == "x64" then
ffi.cdef[[
typedef __int64 INT_PTR;
typedef unsigned __int64 UINT_PTR;
typedef __int64 LONG_PTR;
typedef unsigned __int64 ULONG_PTR;
typedef unsigned __int64 HANDLE_PTR;
]]
else -- "x86"
ffi.cdef[[
typedef  int INT_PTR;
typedef  unsigned int UINT_PTR;
typedef  long LONG_PTR;
typedef  unsigned long ULONG_PTR;
typedef unsigned long HANDLE_PTR;
]]
end

ffi.cdef[[
typedef void VOID;
typedef char CHAR;
typedef short SHORT;
typedef long LONG;
typedef unsigned short WORD;
typedef unsigned int UINT;
typedef unsigned long DWORD;
typedef DWORD COLORREF;
typedef int BOOL;
typedef ULONG_PTR DWORD_PTR;
typedef wchar_t WCHAR;
typedef void *HANDLE;

typedef struct _GUID
{
    unsigned long Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char Data4[8];
} GUID;

typedef struct _FILETIME {
  DWORD dwLowDateTime;
  DWORD dwHighDateTime;
} FILETIME;

typedef struct tagRECT {
  LONG left;
  LONG top;
  LONG right;
  LONG bottom;
} RECT;

enum {
  FILE_ATTRIBUTE_READONLY            = 0x00000001,
  FILE_ATTRIBUTE_HIDDEN              = 0x00000002,
  FILE_ATTRIBUTE_SYSTEM              = 0x00000004,
  FILE_ATTRIBUTE_DIRECTORY           = 0x00000010,
  FILE_ATTRIBUTE_ARCHIVE             = 0x00000020,
  FILE_ATTRIBUTE_DEVICE              = 0x00000040,
  FILE_ATTRIBUTE_NORMAL              = 0x00000080,
  FILE_ATTRIBUTE_TEMPORARY           = 0x00000100,
  FILE_ATTRIBUTE_SPARSE_FILE         = 0x00000200,
  FILE_ATTRIBUTE_REPARSE_POINT       = 0x00000400,
  FILE_ATTRIBUTE_COMPRESSED          = 0x00000800,
  FILE_ATTRIBUTE_OFFLINE             = 0x00001000,
  FILE_ATTRIBUTE_NOT_CONTENT_INDEXED = 0x00002000,
  FILE_ATTRIBUTE_ENCRYPTED           = 0x00004000,
  FILE_ATTRIBUTE_VIRTUAL             = 0x00010000,
  FILE_ATTRIBUTE_VALID_FLAGS         = 0x00017fb7,
  FILE_ATTRIBUTE_VALID_SET_FLAGS     = 0x000031a7,
};

size_t wcslen(const wchar_t*);
int wcscmp(const wchar_t*, const wchar_t*);
int _wcsicmp(const wchar_t*, const wchar_t*);
int StrCmpLogicalW(const wchar_t*, const wchar_t*);

int MessageBoxW(void*, const wchar_t*, const wchar_t*, int);
]]
