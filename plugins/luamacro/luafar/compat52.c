/* Started: 2012-03-27 */

#include "compat52.h"

#if LUA_VERSION_NUM >= 502

void luaL_register(lua_State *L, const char *libname, const luaL_Reg *l)
{
	if(libname)
	{
		lua_createtable(L, 0, 16);
		lua_pushvalue(L, -1);
		lua_setglobal(L, libname);
	}

	luaL_setfuncs(L, l, 0);
}

int luaL_typerror(lua_State *L, int narg, const char *tname)
{
	const char *msg = lua_pushfstring(L, "%s expected, got %s",
	                                  tname, luaL_typename(L, narg));
	return luaL_argerror(L, narg, msg);
}

#endif /* #if LUA_VERSION_NUM >= 502 */
