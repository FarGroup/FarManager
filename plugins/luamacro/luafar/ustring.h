#ifndef _USTRING_H
#define _USTRING_H

#include <windows.h>
#include <stdint.h>

#define ARRSIZE(buff) (sizeof(buff)/sizeof(buff[0]))
#define CAST(tp,expr) ((tp)(expr))

#ifdef __cplusplus
extern "C" {
#endif

#include <lua.h>
#include <lauxlib.h>

LUALIB_API int luaopen_ustring(lua_State *L);

void pusherrorcode(lua_State *L, int error);
void pusherror(lua_State *L);
int  SysErrorReturn(lua_State *L);

BOOL   GetBoolFromTable(lua_State *L, const char* key);
BOOL   GetOptBoolFromTable(lua_State *L, const char* key, BOOL dflt);
int    GetOptIntFromArray(lua_State *L, int key, int dflt);
int    GetOptIntFromTable(lua_State *L, const char* key, int dflt);
double GetOptNumFromTable(lua_State *L, const char* key, double dflt);
void   PutBoolToTable(lua_State *L, const char* key, int num);
void   PutIntToArray(lua_State *L, int key, intptr_t val);
void   PutIntToTable(lua_State *L, const char *key, intptr_t val);
void   PutLStrToTable(lua_State *L, const char* key, const void* str, size_t len);
void   PutNumToTable(lua_State *L, const char* key, double num);
void   PutStrToArray(lua_State *L, int key, const char* str);
void   PutStrToTable(lua_State *L, const char* key, const char* str);
void   PutWStrToArray(lua_State *L, int key, const wchar_t* str, intptr_t numchars);
void   PutWStrToTable(lua_State *L, const char* key, const wchar_t* str, intptr_t numchars);

wchar_t* check_utf8_string(lua_State *L, int pos, size_t* pTrgSize);
wchar_t* utf8_to_utf16(lua_State *L, int pos, size_t* pTrgSize);
const wchar_t* opt_utf8_string(lua_State *L, int pos, const wchar_t* dflt);
wchar_t* oem_to_utf16(lua_State *L, int pos, size_t* pTrgSize);
char* push_utf8_string(lua_State* L, const wchar_t* str, intptr_t numchars);
char* push_oem_string(lua_State* L, const wchar_t* str, intptr_t numchars);
void  push_utf16_string(lua_State* L, const wchar_t* str, intptr_t numchars);
const wchar_t* check_utf16_string(lua_State *L, int pos, size_t *len);
const wchar_t* opt_utf16_string(lua_State *L, int pos, const wchar_t *dflt);

int  DecodeAttributes(const char* str);
void PushAttrString(lua_State *L, int attr);
void PutAttrToTable(lua_State *L, int attr);
int  SetAttr(lua_State *L, const wchar_t* fname, unsigned attr);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
inline wchar_t* check_utf8_string(lua_State *L, int pos)
{
	return check_utf8_string(L, pos, NULL);
}

inline wchar_t* utf8_to_utf16(lua_State *L, int pos)
{
	return utf8_to_utf16(L, pos, NULL);
}

inline char* push_utf8_string(lua_State* L, const wchar_t* str)
{
	return push_utf8_string(L, str, -1);
}

inline void PutWStrToArray(lua_State *L, int key, const wchar_t* str)
{
	PutWStrToArray(L, key, str, -1);
}

inline void PutWStrToTable(lua_State *L, const char* key, const wchar_t* str)
{
	PutWStrToTable(L, key, str, -1);
}
#endif

#endif // #ifndef _USTRING_H
