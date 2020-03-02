/* Started: 2012-03-27 */

#ifndef COMPAT52_H
#define COMPAT52_H

#include <lua.h>
#include <lauxlib.h>

#if LUA_VERSION_NUM >= 502

#define lua_objlen lua_rawlen
#define lua_setfenv lua_setuservalue
#define lua_getfenv lua_getuservalue

void luaL_register(lua_State *L, const char *libname, const luaL_Reg *l);
int luaL_typerror(lua_State *L, int narg, const char *tname);

#endif /* #if LUA_VERSION_NUM >= 502 */

#endif /* #ifndef COMPAT52_H */
