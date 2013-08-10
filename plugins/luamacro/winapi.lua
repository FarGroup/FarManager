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

typedef struct _SECURITY_ATTRIBUTES {
  DWORD nLength;
  VOID* lpSecurityDescriptor;
  BOOL bInheritHandle;
} SECURITY_ATTRIBUTES;

enum { MAX_PATH=260 };

typedef struct _WIN32_FIND_DATAW {
  DWORD dwFileAttributes;
  FILETIME ftCreationTime;
  FILETIME ftLastAccessTime;
  FILETIME ftLastWriteTime;
  DWORD nFileSizeHigh;
  DWORD nFileSizeLow;
  DWORD dwReserved0;
  DWORD dwReserved1;
  WCHAR cFileName[MAX_PATH];
  WCHAR cAlternateFileName[14];
} WIN32_FIND_DATAW;

HANDLE FindFirstFileW (const wchar_t*, WIN32_FIND_DATAW*);
BOOL FindNextFileW (HANDLE, WIN32_FIND_DATAW*);
BOOL FindClose (HANDLE);

size_t wcslen(const wchar_t*);
int wcscmp(const wchar_t*, const wchar_t*);
int _wcsicmp(const wchar_t*, const wchar_t*);
int StrCmpLogicalW(const wchar_t*, const wchar_t*);

int MessageBoxW(void*, const wchar_t*, const wchar_t*, int);
]]
