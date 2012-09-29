#include <shlobj.h>
#include "ustring.h"

#define CAST(tp,expr) (tp)(expr)
#define DIM(buff) (sizeof(buff)/sizeof(buff[0]))

// This function was initially taken from Lua 5.0.2 (loadlib.c)
void pusherrorcode(lua_State *L, int error)
{
  wchar_t buffer[256];
  const int BUFSZ = DIM(buffer);
  int num = FormatMessageW(FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
    0, error, 0, buffer, BUFSZ, 0);
  if (num)
    push_utf8_string(L, buffer, num);
  else
    lua_pushfstring(L, "system error %d\n", error);
}

void pusherror(lua_State *L)
{
  pusherrorcode(L, GetLastError());
}

int SysErrorReturn(lua_State *L)
{
  int last_error = GetLastError();
  lua_pushnil(L);
  pusherrorcode(L, last_error);
  return 2;
}

void PutIntToArray(lua_State *L, int key, intptr_t val)
{
  lua_pushinteger(L, key);
  lua_pushnumber(L, (double)val);
  lua_settable(L, -3);
}

void PutIntToTable(lua_State *L, const char *key, intptr_t val)
{
  lua_pushnumber(L, (double)val);
  lua_setfield(L, -2, key);
}

void PutNumToTable(lua_State *L, const char* key, double num)
{
  lua_pushnumber(L, num);
  lua_setfield(L, -2, key);
}

void PutBoolToTable(lua_State *L, const char* key, int num)
{
  lua_pushboolean(L, num);
  lua_setfield(L, -2, key);
}

void PutStrToTable(lua_State *L, const char* key, const char* str)
{
  lua_pushstring(L, str);
  lua_setfield(L, -2, key);
}

void PutStrToArray(lua_State *L, int key, const char* str)
{
  lua_pushinteger(L, key);
  lua_pushstring(L, str);
  lua_settable(L, -3);
}

void PutWStrToTable(lua_State *L, const char* key, const wchar_t* str, intptr_t numchars)
{
  push_utf8_string(L, str, numchars);
  lua_setfield(L, -2, key);
}

void PutWStrToArray(lua_State *L, int key, const wchar_t* str, intptr_t numchars)
{
  lua_pushinteger(L, key);
  push_utf8_string(L, str, numchars);
  lua_settable(L, -3);
}

void PutLStrToTable(lua_State *L, const char* key, const void* str, size_t len)
{
  lua_pushlstring(L, (const char*)str, len);
  lua_setfield(L, -2, key);
}

double GetOptNumFromTable(lua_State *L, const char* key, double dflt)
{
  double ret = dflt;
  lua_getfield(L, -1, key);
  if(lua_isnumber(L,-1))
    ret = lua_tonumber(L, -1);
  lua_pop(L, 1);
  return ret;
}

int GetOptIntFromTable(lua_State *L, const char* key, int dflt)
{
  int ret = dflt;
  lua_getfield(L, -1, key);
  if(lua_isnumber(L,-1))
    ret = (int)lua_tointeger(L, -1);
  lua_pop(L, 1);
  return ret;
}

int GetOptIntFromArray(lua_State *L, int key, int dflt)
{
  int ret = dflt;
  lua_pushinteger(L, key);
  lua_gettable(L, -2);
  if(lua_isnumber(L,-1))
    ret = (int)lua_tointeger(L, -1);
  lua_pop(L, 1);
  return ret;
}

BOOL GetBoolFromTable(lua_State *L, const char* key)
{
  int ret;
  lua_getfield(L, -1, key);
  ret = lua_toboolean(L, -1);
  lua_pop(L, 1);
  return ret;
}

BOOL GetOptBoolFromTable(lua_State *L, const char* key, BOOL dflt)
{
  BOOL ret;
  lua_getfield(L, -1, key);
  ret = lua_isnil(L, -1) ? dflt : lua_toboolean(L, -1);
  lua_pop(L, 1);
  return ret;
}

//---------------------------------------------------------------------------
// Check a multibyte string at 'pos' Lua stack position
// and convert it in place to UTF-16.
// Return a pointer to the converted string.
wchar_t* convert_multibyte_string (lua_State *L, int pos, UINT codepage,
  DWORD dwFlags, size_t* pTrgSize, int can_raise)
{
  size_t sourceLen;
  const char *source;
  wchar_t *target;
  int size;

  if (pos < 0) pos += lua_gettop(L) + 1;

  if (!can_raise && !lua_isstring(L, pos))
    return NULL;

  source = luaL_checklstring(L, pos, &sourceLen);
  if (!pTrgSize)
    ++sourceLen;

  size = MultiByteToWideChar(
    codepage,     // code page
    dwFlags,      // character-type options
    source,       // lpMultiByteStr, pointer to the character string to be converted
    (int)sourceLen, // size, in bytes, of the string pointed to by the lpMultiByteStr
    NULL,         // lpWideCharStr, address of wide-character buffer
    0             // size of buffer (in wide characters)
  );
  if (size == 0 && sourceLen != 0) {
    if (can_raise)
      luaL_argerror(L, pos, "invalid multibyte string");
    return NULL;
  }

  target = (wchar_t*)lua_newuserdata(L, (size+1) * sizeof(wchar_t));
  MultiByteToWideChar(codepage, dwFlags, source, (int)sourceLen, target, size);
  target[size] = L'\0';
  lua_replace(L, pos);
  if (pTrgSize) *pTrgSize = size;
  return target;
}

wchar_t* check_utf8_string (lua_State *L, int pos, size_t* pTrgSize)
{
  return convert_multibyte_string(L, pos, CP_UTF8, 0, pTrgSize, TRUE);
}

wchar_t* utf8_to_utf16 (lua_State *L, int pos, size_t* pTrgSize)
{
  return convert_multibyte_string(L, pos, CP_UTF8, 0, pTrgSize, FALSE);
}

const wchar_t* opt_utf8_string (lua_State *L, int pos, const wchar_t* dflt)
{
  return lua_isnoneornil(L,pos) ? dflt : check_utf8_string(L, pos, NULL);
}

wchar_t* oem_to_utf16 (lua_State *L, int pos, size_t* pTrgSize)
{
  return convert_multibyte_string (L, pos, CP_OEMCP, 0, pTrgSize, FALSE);
}

char* push_multibyte_string (lua_State* L, UINT CodePage, const wchar_t* str,
  intptr_t numchars, DWORD dwFlags)
{
  int targetSize;
  char *target;

  if (str == NULL) { lua_pushnil(L); return NULL; }

  targetSize = WideCharToMultiByte(
    CodePage, // UINT CodePage,
    dwFlags,  // DWORD dwFlags,
    str,      // LPCWSTR lpWideCharStr,
    (int)numchars, // int cchWideChar,
    NULL,     // LPSTR lpMultiByteStr,
    0,        // int cbMultiByte,
    NULL,     // LPCSTR lpDefaultChar,
    NULL      // LPBOOL lpUsedDefaultChar
  );
  if (targetSize == 0 && numchars == -1 && str[0]) {
    luaL_error(L, "invalid UTF-16 string");
  }
  target = (char*)lua_newuserdata(L, targetSize+1);
  WideCharToMultiByte(CodePage, dwFlags, str, (int)numchars, target, targetSize, NULL, NULL);
  if (numchars == -1)
    --targetSize;
  lua_pushlstring(L, target, targetSize);
  lua_remove(L, -2);
  return target;
}

char* push_utf8_string (lua_State* L, const wchar_t* str, intptr_t numchars)
{
  return push_multibyte_string(L, CP_UTF8, str, numchars, 0);
}

char* push_oem_string (lua_State* L, const wchar_t* str, intptr_t numchars)
{
  return push_multibyte_string(L, CP_OEMCP, str, numchars, 0);
}

int ustring_WideCharToMultiByte (lua_State *L)
{
  size_t numchars;
  const wchar_t* src = (const wchar_t*)luaL_checklstring(L, 1, &numchars);
  UINT codepage;
  DWORD dwFlags = 0;
  numchars /= sizeof(wchar_t);
  codepage = (UINT)luaL_checkinteger(L, 2);
  if (lua_isstring(L, 3)) {
    const char *s = lua_tostring(L, 3);
    for (; *s; s++) {
      if      (*s == 'c') dwFlags |= WC_COMPOSITECHECK;
      else if (*s == 'd') dwFlags |= WC_DISCARDNS;
      else if (*s == 's') dwFlags |= WC_SEPCHARS;
      else if (*s == 'f') dwFlags |= WC_DEFAULTCHAR;
    }
  }
  push_multibyte_string(L, codepage, src, numchars, dwFlags);
  return 1;
}

int ustring_MultiByteToWideChar (lua_State *L)
{
  wchar_t* Trg;
  size_t TrgSize;
  UINT codepage;
  DWORD dwFlags = 0;
  (void) luaL_checkstring(L, 1);
  codepage = (UINT)luaL_checkinteger(L, 2);
  if (lua_isstring(L, 3)) {
    const char *s = lua_tostring(L, 3);
    for (; *s; s++) {
      if      (*s == 'p') dwFlags |= MB_PRECOMPOSED;
      else if (*s == 'c') dwFlags |= MB_COMPOSITE;
      else if (*s == 'e') dwFlags |= MB_ERR_INVALID_CHARS;
      else if (*s == 'u') dwFlags |= MB_USEGLYPHCHARS;
    }
  }
  Trg = convert_multibyte_string(L, 1, codepage, dwFlags, &TrgSize, FALSE);
  if (Trg) {
    lua_pushlstring(L, (const char*)Trg, TrgSize * sizeof(wchar_t));
    return 1;
  }
  return SysErrorReturn(L);
}

int ustring_OemToUtf8 (lua_State *L)
{
  size_t len;
  wchar_t* buf;
  (void) luaL_checklstring(L, 1, &len);
  buf = oem_to_utf16(L, 1, &len);
  push_utf8_string(L, buf, len);
  return 1;
}

int ustring_Utf8ToOem (lua_State *L)
{
  size_t len;
  const wchar_t* buf = check_utf8_string(L, 1, &len);
  push_oem_string(L, buf, len);
  return 1;
}

int ustring_Utf16ToUtf8 (lua_State *L)
{
  size_t len;
  const wchar_t *ws = (const wchar_t*) luaL_checklstring(L, 1, &len);
  push_utf8_string(L, ws, len/sizeof(wchar_t));
  return 1;
}

int ustring_Utf8ToUtf16 (lua_State *L)
{
  size_t len;
  const wchar_t *ws = check_utf8_string(L, 1, &len);
  lua_pushlstring(L, (const char*) ws, len*sizeof(wchar_t));
  return 1;
}

int ustring_GetACP (lua_State* L) {
  return lua_pushinteger (L, GetACP()), 1;
}

int ustring_GetOEMCP (lua_State* L) {
  return lua_pushinteger (L, GetOEMCP()), 1;
}

struct EnumCP_struct {
  lua_State* L;
  int N;
} EnumCP;

BOOL CALLBACK EnumCodePagesProc(wchar_t* CodePageString)
{
  PutWStrToArray(EnumCP.L, ++EnumCP.N, CodePageString, -1);
  return TRUE;
}

int ustring_EnumSystemCodePages(lua_State *L)
{
  DWORD flags = lua_toboolean(L,1) ? CP_SUPPORTED : CP_INSTALLED;
  lua_newtable(L);
  EnumCP.L = L;
  EnumCP.N = 0;
  if (EnumSystemCodePagesW(EnumCodePagesProc, flags))
    return 1;
  return SysErrorReturn(L);
}

int ustring_GetCPInfo(lua_State *L)
{
  UINT codepage;
  CPINFOEXW info;
  memset(&info, 0, sizeof(info));
  codepage = (UINT)luaL_checkinteger(L, 1);
  if (!GetCPInfoExW(codepage, 0, &info))
    return SysErrorReturn(L);
  lua_createtable(L, 0, 6);
  PutNumToTable  (L, "MaxCharSize",  info.MaxCharSize);
  PutLStrToTable (L, "DefaultChar",  (const char*)info.DefaultChar, MAX_DEFAULTCHAR);
  PutLStrToTable (L, "LeadByte",     (const char*)info.LeadByte, MAX_LEADBYTES);
  PutWStrToTable (L, "UnicodeDefaultChar", &info.UnicodeDefaultChar, 1);
  PutNumToTable  (L, "CodePage",     info.CodePage);
  PutWStrToTable (L, "CodePageName", info.CodePageName, -1);
  return 1;
}

int ustring_GetLogicalDriveStrings (lua_State *L)
{
  int i;
  wchar_t* buf;
  DWORD len = GetLogicalDriveStringsW(0, NULL);
  if (len) {
    buf = (wchar_t*)lua_newuserdata(L, (len+1)*sizeof(wchar_t));
    if (GetLogicalDriveStringsW(len, buf)) {
      lua_newtable(L);
      for(i=1; TRUE; i++) {
        if (*buf == 0) break;
        PutWStrToArray(L, i, buf, -1);
        buf += wcslen(buf) + 1;
      }
      return 1;
    }
  }
  return SysErrorReturn(L);
}

int ustring_GetDriveType (lua_State *L)
{
  const wchar_t *root = opt_utf8_string(L, 1, NULL);
  const char* out;
  UINT tp = GetDriveTypeW(root);
  switch(tp) {
    default:
    case 0:               out = "unknown type";      break;
    case 1:               out = "no root directory"; break;
    case DRIVE_REMOVABLE: out = "removable";         break;
    case DRIVE_FIXED:     out = "fixed";             break;
    case DRIVE_REMOTE:    out = "remote";            break;
    case DRIVE_CDROM:     out = "cdrom";             break;
    case DRIVE_RAMDISK:   out = "ramdisk";           break;
  }
  lua_pushstring(L, out);
  return 1;
}

int ustring_Uuid (lua_State* L)
{
  UUID uuid;
  if (lua_gettop(L) == 0 || !lua_toboolean(L, 1)) {
    // generate new UUID
    if (UuidCreate(&uuid) == RPC_S_OK) {
      lua_pushlstring(L, (const char*)&uuid, sizeof(UUID));
      return 1;
    }
  }
  else {
    size_t len;
    const char* arg1 = luaL_checklstring(L, 1, &len);
    if (len == sizeof(UUID)) {
      // convert given UUID to string
      unsigned char* p;
      if (UuidToStringA((UUID*)arg1, &p) == RPC_S_OK) {
        lua_pushstring(L, (char*)p);
        RpcStringFreeA(&p);
        return 1;
      }
    }
    else {
      // convert string UUID representation to UUID
      if (UuidFromStringA((unsigned char*)arg1, &uuid) == RPC_S_OK) {
        lua_pushlstring(L, (const char*)&uuid, sizeof(UUID));
        return 1;
      }
    }
  }
  lua_pushnil(L);
  return 1;
}

int ustring_SearchPath (lua_State *L)
{
  const wchar_t* lpPath = opt_utf8_string (L, 1, NULL);
  const wchar_t* lpFileName = check_utf8_string (L, 2, NULL);
  const wchar_t* lpExtension = opt_utf8_string (L, 3, NULL);

  wchar_t buf[2048];
  wchar_t* lpFilePart;
  DWORD result = SearchPathW(
    lpPath,         // address of search path
    lpFileName,	    // address of filename
    lpExtension,	  // address of extension
    sizeof(buf)/sizeof(wchar_t),	  // size, in characters, of buffer
    buf,	          // address of buffer for found filename
    &lpFilePart 	  // address of pointer to file component
   );
  if (result > 0) {
    push_utf8_string (L, buf, -1);
    push_utf8_string (L, lpFilePart, -1);
    return 2;
  }
  return 0;
}

int ustring_GlobalMemoryStatus (lua_State *L)
{
  MEMORYSTATUSEX ms;
  ms.dwLength = sizeof(ms);
  if (0 == GlobalMemoryStatusEx(&ms))
    return SysErrorReturn(L);
  lua_createtable(L, 0, 8);
  PutNumToTable(L, "MemoryLoad",           ms.dwMemoryLoad);
  PutNumToTable(L, "TotalPhys",            CAST(double, ms.ullTotalPhys));
  PutNumToTable(L, "AvailPhys",            CAST(double, ms.ullAvailPhys));
  PutNumToTable(L, "TotalPageFile",        CAST(double, ms.ullTotalPageFile));
  PutNumToTable(L, "AvailPageFile",        CAST(double, ms.ullAvailPageFile));
  PutNumToTable(L, "TotalVirtual",         CAST(double, ms.ullTotalVirtual));
  PutNumToTable(L, "AvailVirtual",         CAST(double, ms.ullAvailVirtual));
  PutNumToTable(L, "AvailExtendedVirtual", CAST(double, ms.ullAvailExtendedVirtual));
  return 1;
}

int ustring_Sleep (lua_State *L)
{
  Sleep( (DWORD)luaL_checknumber(L, 1) );
  return 0;
}

void PushAttrString(lua_State *L, int attr)
{
  char buf[16], *p = buf;
  if(attr & FILE_ATTRIBUTE_ARCHIVE)    *p++ = 'a';
  if(attr & FILE_ATTRIBUTE_READONLY)   *p++ = 'r';
  if(attr & FILE_ATTRIBUTE_HIDDEN)     *p++ = 'h';
  if(attr & FILE_ATTRIBUTE_SYSTEM)     *p++ = 's';
  if(attr & FILE_ATTRIBUTE_DIRECTORY)  *p++ = 'd';
  if(attr & FILE_ATTRIBUTE_COMPRESSED) *p++ = 'c';
  if(attr & FILE_ATTRIBUTE_OFFLINE)    *p++ = 'o';
  if(attr & FILE_ATTRIBUTE_TEMPORARY)  *p++ = 't';
  if(attr & FILE_ATTRIBUTE_SPARSE_FILE)   *p++ = 'p';
  if(attr & FILE_ATTRIBUTE_REPARSE_POINT) *p++ = 'e';
  *p = '\0';
  lua_pushstring(L, buf);
}

void PutAttrToTable(lua_State *L, int attr)
{
  PushAttrString(L, attr);
  lua_setfield(L, -2, "FileAttributes");
}

int DecodeAttributes(const char* str)
{
  int attr = 0;
  for (; *str; str++) {
    char c = *str;
    if      (c == 'a' || c == 'A') attr |= FILE_ATTRIBUTE_ARCHIVE;
    else if (c == 'r' || c == 'R') attr |= FILE_ATTRIBUTE_READONLY;
    else if (c == 'h' || c == 'H') attr |= FILE_ATTRIBUTE_HIDDEN;
    else if (c == 's' || c == 'S') attr |= FILE_ATTRIBUTE_SYSTEM;
    else if (c == 'd' || c == 'D') attr |= FILE_ATTRIBUTE_DIRECTORY;
    else if (c == 'c' || c == 'C') attr |= FILE_ATTRIBUTE_COMPRESSED;
    else if (c == 'o' || c == 'O') attr |= FILE_ATTRIBUTE_OFFLINE;
    else if (c == 't' || c == 'T') attr |= FILE_ATTRIBUTE_TEMPORARY;
    else if (c == 'p' || c == 'P') attr |= FILE_ATTRIBUTE_SPARSE_FILE;
    else if (c == 'e' || c == 'E') attr |= FILE_ATTRIBUTE_REPARSE_POINT;
  }
  return attr;
}

// for reusing code
int SetAttr(lua_State *L, const wchar_t* fname, unsigned attr)
{
  if (SetFileAttributesW(fname, attr))
    return lua_pushboolean(L, 1), 1;
  return SysErrorReturn(L);
}

int ustring_SetFileAttr(lua_State *L)
{
  return SetAttr(L, check_utf8_string(L,1,NULL), DecodeAttributes(luaL_checkstring(L,2)));
}

int ustring_GetFileAttr(lua_State *L)
{
  DWORD attr = GetFileAttributesW(check_utf8_string(L,1,NULL));
  if(attr == 0xFFFFFFFF) lua_pushnil(L);
  else PushAttrString(L, attr);
  return 1;
}

int ustring_SHGetFolderPath(lua_State *L)
{
  wchar_t pszPath[MAX_PATH];
  int nFolder = (int)luaL_checkinteger(L, 1);
  DWORD dwFlags = (DWORD)luaL_optnumber(L, 2, 0);
  HRESULT result = SHGetFolderPathW(
    NULL,         // __in   HWND hwndOwner,
    nFolder,      // __in   int nFolder,
    NULL,         // __in   HANDLE hToken,
    dwFlags,      // __in   DWORD dwFlags,
    pszPath);     // __out  LPTSTR pszPath);
  if (result == S_OK)
    push_utf8_string(L, pszPath, -1);
  else
    lua_pushnil(L);
  return 1;
}

void push_utf16_string (lua_State* L, const wchar_t* str, intptr_t numchars)
{
  if (numchars < 0)
    numchars = wcslen(str);
  lua_pushlstring(L, (const char*)str, numchars*sizeof(wchar_t));
}

int ustring_sub(lua_State *L)
{
  size_t len;
  intptr_t from, to;
  const char* s = luaL_checklstring(L, 1, &len);
  len /= sizeof(wchar_t);

  from = luaL_optinteger(L, 2, 1);
  if (from < 0) from += len+1;
  if (--from < 0) from = 0;
  else if ((size_t)from > len) from = len;

  to = luaL_optinteger(L, 3, -1);
  if (to < 0) to += len+1;
  if (to < from) to = from;
  else if ((size_t)to > len) to = len;

  lua_pushlstring(L, s + from*sizeof(wchar_t), (to-from)*sizeof(wchar_t));
  return 1;
}

int ustring_len(lua_State *L)
{
  size_t len;
  (void) luaL_checklstring(L, 1, &len);
  lua_pushinteger(L, len / sizeof(wchar_t));
  return 1;
}

const wchar_t* check_utf16_string(lua_State *L, int pos, size_t *len)
{
  size_t ln;
  const wchar_t* s = (const wchar_t*)luaL_checklstring(L, pos, &ln);
  if (len) *len = ln / sizeof(wchar_t);
  return s;
}

const wchar_t* opt_utf16_string(lua_State *L, int pos, const wchar_t *dflt)
{
  const wchar_t* s = (const wchar_t*)luaL_optstring(L, pos, (const char*)dflt);
  return s;
}
