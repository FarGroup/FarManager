-- Windows API - types, constants and functions - via LuaJIT FFI.
-- Started 2011-10-01 by Shmuel Zeigerman

local ffi = require "ffi"

if jit.arch == "x64" then
ffi.cdef[[
typedef __int64 INT_PTR;
typedef unsigned __int64 UINT_PTR;
typedef __int64 LONG_PTR;
typedef unsigned __int64 ULONG_PTR;
typedef unsigned __int64 HANDLE_PTR;
typedef int HALF_PTR;
typedef unsigned int UHALF_PTR;
]]
else -- "x86"
ffi.cdef[[
typedef int INT_PTR;
typedef unsigned int UINT_PTR;
typedef long LONG_PTR;
typedef unsigned long ULONG_PTR;
typedef unsigned long HANDLE_PTR;
typedef short HALF_PTR;
typedef unsigned short UHALF_PTR;
]]
end

ffi.cdef[[
typedef INT_PTR *PINT_PTR;
typedef UINT_PTR *PUINT_PTR;
typedef LONG_PTR *PLONG_PTR;
typedef ULONG_PTR *PULONG_PTR;
typedef HALF_PTR *PHALF_PTR;
typedef UHALF_PTR *PUHALF_PTR;

typedef LONG_PTR SSIZE_T, *PSSIZE_T;
typedef ULONG_PTR DWORD_PTR, *PDWORD_PTR;
typedef ULONG_PTR SIZE_T, *PSIZE_T;
typedef signed char INT8, *PINT8;
typedef signed short INT16, *PINT16;
typedef signed int INT32, *PINT32;
typedef signed __int64 INT64, *PINT64;
typedef unsigned char UINT8, *PUINT8;
typedef unsigned short UINT16, *PUINT16;
typedef unsigned int UINT32, *PUINT32;
typedef unsigned __int64 UINT64, *PUINT64;
typedef signed int LONG32, *PLONG32;
typedef __int64 LONG64, *PLONG64;
typedef unsigned int ULONG32, *PULONG32;
typedef unsigned __int64 ULONG64, *PULONG64;
typedef unsigned int DWORD32, *PDWORD32;
typedef unsigned __int64 DWORD64, *PDWORD64;
typedef unsigned __int64 QWORD;

typedef unsigned long ULONG, *PULONG;
typedef unsigned short USHORT, *PUSHORT;
typedef unsigned char UCHAR, *PUCHAR;
typedef unsigned long DWORD, *PDWORD, *LPDWORD;
typedef int BOOL, *PBOOL, *LPBOOL;
typedef unsigned char BYTE, *PBYTE, *LPBYTE;
typedef unsigned short WORD, *PWORD, *LPWORD;
typedef float FLOAT, *PFLOAT;
typedef int INT,*PINT, *LPINT;
typedef void VOID, *PVOID, *LPVOID;
typedef const void *LPCVOID;
typedef unsigned int UINT, *PUINT;
typedef short SHORT, *PSHORT;
typedef long LONG, *PLONG, *LPLONG;
typedef char CCHAR;
typedef char CHAR, *PCHAR, *LPSTR, *PSTR;
typedef const CHAR *PCSTR, *LPCSTR;
typedef wchar_t WCHAR, *PWCHAR, *LPWSTR, *PWSTR;
typedef const WCHAR *LPCWSTR, *PCWSTR;
//#ifdef UNICODE
typedef LPWSTR PTSTR, LPTSTR;
typedef LPCWSTR PCTSTR, LPCTSTR;
typedef WCHAR TCHAR, *PTCHAR;
typedef WCHAR TBYTE, *PTBYTE;

typedef struct _UNICODE_STRING {
  USHORT  Length;
  USHORT  MaximumLength;
  PWSTR  Buffer;
} UNICODE_STRING;
typedef UNICODE_STRING *PUNICODE_STRING;
typedef const UNICODE_STRING *PCUNICODE_STRING;

typedef PVOID HANDLE;
typedef HANDLE *PHANDLE, *LPHANDLE;
typedef DWORD LCID;
typedef PDWORD PLCID;
typedef WORD LANGID;
typedef __int64 LONGLONG; //!defined(_M_IX86)
typedef LONGLONG *PLONGLONG;
typedef unsigned __int64 DWORDLONG;
typedef DWORDLONG *PDWORDLONG;
typedef unsigned __int64 ULONGLONG; //!defined(_M_IX86)
typedef ULONGLONG *PULONGLONG;
typedef LONGLONG USN;
typedef BYTE BOOLEAN, *PBOOLEAN;

typedef UINT_PTR WPARAM;
typedef LONG_PTR LPARAM;
typedef LONG_PTR LRESULT;
typedef LONG HRESULT;
typedef WORD ATOM;

typedef HANDLE HHOOK;
typedef HANDLE HGLOBAL;
typedef HANDLE HLOCAL;
typedef HANDLE HGDIOBJ;
typedef HANDLE HACCEL;
typedef HANDLE HBITMAP;
typedef HANDLE HBRUSH;
typedef HANDLE HCOLORSPACE;
typedef HANDLE HDC;
typedef HANDLE HDESK;
typedef HANDLE HENHMETAFILE;
typedef HANDLE HFONT;
typedef HANDLE HICON;
typedef HANDLE HKEY, *PHKEY;
typedef HANDLE HMONITOR;
typedef HANDLE HMENU;
typedef HANDLE HMETAFILE;
typedef HANDLE HINSTANCE;
typedef HINSTANCE HMODULE;
typedef HANDLE HPALETTE;
typedef HANDLE HPEN;
typedef HANDLE HRGN;
typedef HANDLE HRSRC;
typedef HANDLE HWND;
typedef HANDLE HWINSTA;
typedef HANDLE HKL;
typedef HANDLE HCONV;
typedef HANDLE HCONVLIST;
typedef HANDLE HDDEDATA;
typedef HANDLE HDROP;
typedef HANDLE HDWP;
typedef HANDLE HSZ;
typedef HANDLE SC_HANDLE;
typedef LPVOID SC_LOCK;
typedef HANDLE SERVICE_STATUS_HANDLE;
typedef int HFILE;
typedef HICON HCURSOR;
typedef DWORD COLORREF;
typedef DWORD *LPCOLORREF;
typedef DWORD LCTYPE;
typedef DWORD LGRPID;
typedef int WINBOOL;

typedef struct
{
  unsigned long Data1;
  unsigned short Data2;
  unsigned short Data3;
  unsigned char Data4[8];
} GUID;

typedef struct {
  SHORT X;
  SHORT Y;
} COORD;

typedef struct {
  SHORT Left;
  SHORT Top;
  SHORT Right;
  SHORT Bottom;
} SMALL_RECT;

typedef struct {
  WINBOOL bKeyDown;
  WORD wRepeatCount;
  WORD wVirtualKeyCode;
  WORD wVirtualScanCode;
  union {
    WCHAR UnicodeChar;
    CHAR AsciiChar;
  } uChar;
  DWORD dwControlKeyState;
} KEY_EVENT_RECORD;

static const uint32_t
  RIGHT_ALT_PRESSED = 0x1,
  LEFT_ALT_PRESSED = 0x2,
  RIGHT_CTRL_PRESSED = 0x4,
  LEFT_CTRL_PRESSED = 0x8,
  SHIFT_PRESSED = 0x10,
  NUMLOCK_ON = 0x20,
  SCROLLLOCK_ON = 0x40,
  CAPSLOCK_ON = 0x80,
  ENHANCED_KEY = 0x100,
  NLS_DBCSCHAR = 0x10000,
  NLS_ALPHANUMERIC = 0x0,
  NLS_KATAKANA = 0x20000,
  NLS_HIRAGANA = 0x40000,
  NLS_ROMAN = 0x400000,
  NLS_IME_CONVERSION = 0x800000,
  NLS_IME_DISABLE = 0x20000000;

typedef struct {
  COORD dwMousePosition;
  DWORD dwButtonState;
  DWORD dwControlKeyState;
  DWORD dwEventFlags;
} MOUSE_EVENT_RECORD;

static const uint32_t
  FROM_LEFT_1ST_BUTTON_PRESSED = 0x1,
  RIGHTMOST_BUTTON_PRESSED = 0x2,
  FROM_LEFT_2ND_BUTTON_PRESSED = 0x4,
  FROM_LEFT_3RD_BUTTON_PRESSED = 0x8,
  FROM_LEFT_4TH_BUTTON_PRESSED = 0x10;

static const uint32_t
  MOUSE_MOVED = 0x1,
  DOUBLE_CLICK = 0x2,
  MOUSE_WHEELED = 0x4,
  MOUSE_HWHEELED = 0x8;

typedef struct {
  COORD dwSize;
} WINDOW_BUFFER_SIZE_RECORD;

typedef struct {
  UINT dwCommandId;
} MENU_EVENT_RECORD;

typedef struct {
  WINBOOL bSetFocus;
} FOCUS_EVENT_RECORD;

typedef struct {
  WORD EventType;
  union {
    KEY_EVENT_RECORD KeyEvent;
    MOUSE_EVENT_RECORD MouseEvent;
    WINDOW_BUFFER_SIZE_RECORD WindowBufferSizeEvent;
    MENU_EVENT_RECORD MenuEvent;
    FOCUS_EVENT_RECORD FocusEvent;
  } Event;
} INPUT_RECORD;

static const uint32_t
  KEY_EVENT = 0x1,
  MOUSE_EVENT = 0x2,
  WINDOW_BUFFER_SIZE_EVENT = 0x4,
  MENU_EVENT = 0x8,
  FOCUS_EVENT = 0x10;

typedef struct {
  DWORD dwLowDateTime;
  DWORD dwHighDateTime;
} FILETIME;

typedef struct {
  LONG left;
  LONG top;
  LONG right;
  LONG bottom;
} RECT;

typedef struct {
  DWORD nLength;
  VOID *lpSecurityDescriptor;
  WINBOOL bInheritHandle;
} SECURITY_ATTRIBUTES;

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
  FILE_ATTRIBUTE_INTEGRITY_STREAM    = 0x00008000,
  FILE_ATTRIBUTE_VIRTUAL             = 0x00010000,
  FILE_ATTRIBUTE_NO_SCRUB_DATA       = 0x00020000,
  FILE_ATTRIBUTE_VALID_FLAGS         = 0x00017fb7,
  FILE_ATTRIBUTE_VALID_SET_FLAGS     = 0x000031a7,
};

//------------------------------------------------------------------------------
enum {
  LOCALE_USER_DEFAULT   = 0x400,
  LOCALE_SYSTEM_DEFAULT = 0x800,
};

enum {
  NORM_IGNORECASE     = 0x00001,
  NORM_IGNORENONSPACE = 0x00002,
  NORM_IGNORESYMBOLS  = 0x00004,
  SORT_STRINGSORT     = 0x01000,
  NORM_IGNOREKANATYPE = 0x10000,
  NORM_IGNOREWIDTH    = 0x20000,
};

int CompareStringW (/*LCID*/ DWORD Locale, DWORD dwCmpFlags, const wchar_t* lpString1, int cchCount1,
                    const wchar_t* lpString2, int cchCount2);
//------------------------------------------------------------------------------

int      StrCmpLogicalW(const wchar_t*, const wchar_t*);
int      _wcsicmp(const wchar_t*, const wchar_t*);
wchar_t* wcschr(const wchar_t*, wchar_t);
int      wcscmp(const wchar_t*, const wchar_t*);
size_t   wcslen(const wchar_t*);
wchar_t* wcspbrk(const wchar_t *str, const wchar_t *strCharSet);
wchar_t* wcsrchr(const wchar_t*, wchar_t);
size_t   wcsspn(const wchar_t *str, const wchar_t *strCharSet);
wchar_t* wcsstr(const wchar_t *str, const wchar_t *strSearch);
]]
