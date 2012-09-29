#include "ustring.h"
#include "util.h"

#define CAST(tp,expr) (tp)(expr)

// stack[-2] - table
// stack[-1] - value
int luaLF_SlotError (lua_State *L, int key, const char* expected_typename)
{
  return luaL_error (L,
    "bad field [%d] in table stackpos=%d (%s expected got %s)",
    key, abs_index(L,-2), expected_typename, luaL_typename(L,-1));
}

// stack[-2] - table
// stack[-1] - value
int luaLF_FieldError (lua_State *L, const char* key, const char* expected_typename)
{
  return luaL_error (L,
    "bad field '%s' in table stackpos=%d (%s expected got %s)",
    key, abs_index(L,-2), expected_typename, luaL_typename(L,-1));
}

int GetIntFromArray(lua_State *L, int index)
{
  int ret;
  lua_pushinteger(L, index);
  lua_gettable(L, -2);
  if (!lua_isnumber (L,-1))
    return luaLF_SlotError (L, index, "number");
  ret = (int)lua_tointeger(L, -1);
  lua_pop(L, 1);
  return ret;
}

unsigned __int64 GetFileSizeFromTable(lua_State *L, const char *key)
{
  unsigned __int64 size;
  lua_getfield(L, -1, key);
  if (lua_isnumber(L, -1))
    size = (unsigned __int64) lua_tonumber(L, -1);
  else
    size = 0;
  lua_pop(L, 1);
  return size;
}

FILETIME GetFileTimeFromTable(lua_State *L, const char *key)
{
  FILETIME ft;
  lua_getfield(L, -1, key);
  if (lua_isnumber(L, -1)) {
    long long tm = (long long) lua_tonumber(L, -1);
    tm *= 10000; // convert ms units to 100ns ones
    ft.dwHighDateTime = CAST(DWORD, tm / 0x100000000ll);
    ft.dwLowDateTime  = tm % 0x100000000ll;
  }
  else
    ft.dwLowDateTime = ft.dwHighDateTime = 0;
  lua_pop(L, 1);
  return ft;
}

void PutFileTimeToTable(lua_State *L, const char* key, FILETIME ft)
{
  LARGE_INTEGER li;
  li.LowPart = ft.dwLowDateTime;
  li.HighPart = ft.dwHighDateTime;
  PutNumToTable(L, key, CAST(double, li.QuadPart/10000)); // convert 100ns units to 1ms ones
}

int GetAttrFromTable(lua_State *L)
{
  int attr = 0;
  lua_getfield(L, -1, "FileAttributes");
  if (lua_isstring(L, -1))
    attr = DecodeAttributes(lua_tostring(L, -1));
  lua_pop(L, 1);
  return attr;
}

void InitSynchroData (TSynchroData* SD, int a, int b, int c, int d)
{
  SD->regAction = a;
  SD->objRef = b;
  SD->funcRef = c;
  SD->data = d;
}
