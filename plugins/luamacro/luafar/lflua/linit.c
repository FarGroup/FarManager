#include "lua.h"

extern void LF_InitLuaState1(lua_State *L, lua_CFunction aOpenLibs);

void luaL_openlibs (lua_State *L) {
  LF_InitLuaState1(L, NULL);  /* open libraries */
}
