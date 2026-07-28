#include <windows.h>
#include <ctype.h>

#include <lua.h>
#include <lauxlib.h>

#include "lf_bit64.h"
#include "lf_flags.h"
#include "lf_util.h"

flags_t get_env_flag(lua_State *L, int pos, int *success)
{
	int dummy;
	const char *str;
	flags_t ret = 0;
	int top = lua_gettop(L);

	if (success)
		*success = TRUE;
	else
		success = &dummy;

	switch(lua_type(L, pos))
	{
		case LUA_TNONE:
		case LUA_TNIL:
			break;

		case LUA_TNUMBER:
			ret = (flags_t)lua_tonumber(L, pos); // IMPORTANT: cast to signed integer.
			break;

		case LUA_TSTRING:
			str = lua_tostring(L, pos);
			lua_getfield(L, LUA_REGISTRYINDEX, FAR_FLAGSTABLE);
			lua_getfield(L, -1, str);

			if (lua_isnil(L, -1)) // not found in far.Flags, look into far.Colors
			{
				if ((lua_getglobal(L, "far"), !lua_istable(L,-1))
						|| (lua_getfield(L,-1,"Colors"), !lua_istable(L,-1)))
				{
					*success = FALSE;
					break;
				}
				lua_getfield(L, -1, str);
			}

			if (lua_type(L, -1) == LUA_TNUMBER)
				ret = (flags_t)lua_tonumber(L, -1); // IMPORTANT: cast to signed integer.
			else if (!bit64_getvalue(L, -1, &ret))
				*success = FALSE;
			break;

		default:
			if (!bit64_getvalue(L, pos, &ret))
				*success = FALSE;
			break;
	}

	lua_settop(L, top);
	return ret;
}

flags_t check_env_flag(lua_State *L, int pos)
{
	int success = FALSE;
	flags_t ret = lua_isnoneornil(L, pos) ? 0 : get_env_flag(L, pos, &success);

	if (!success)
	{
		if (lua_isstring(L, pos))
		{
			lua_pushfstring(L, "invalid flag: \"%s\"", lua_tostring(L, pos));
			luaL_argerror(L, pos, lua_tostring(L, -1));
		}
		else
			luaL_argerror(L, pos, "invalid flag");
	}

	return ret;
}

flags_t GetFlagCombination(lua_State *L, int pos, int *success)
{
	flags_t ret = 0;
	pos = abs_index(L, pos);
	if (success)
		*success = TRUE;

	if (lua_type(L, pos) == LUA_TTABLE)
	{
		lua_pushnil(L);

		while(lua_next(L, pos))
		{
			if (lua_type(L,-2)==LUA_TSTRING && lua_toboolean(L,-1))
			{
				flags_t flag = get_env_flag(L, -2, success);

				if (success == NULL || *success)
					ret |= flag;
				else
					{ lua_pop(L,2); return ret; }
			}

			lua_pop(L, 1);
		}
	}
	else if (lua_type(L, pos) == LUA_TSTRING)
	{
		const char *p = lua_tostring(L, pos), *q;
		for (; *p; p=q)
		{
			int ok;
			while (isspace(*p)) p++;
			if (*p == 0) break;
			for (q=p+1; *q && !isspace(*q); ) q++;
			lua_pushlstring(L, p, q-p);
			flags_t flag = get_env_flag(L, -1, &ok);
			lua_pop(L, 1);
			if (ok)
				ret |= flag;
			else if (success)
				*success = FALSE;
		}
	}
	else
		ret = get_env_flag(L, pos, success);

	return ret;
}

flags_t CheckFlags(lua_State* L, int pos)
{
	int success = FALSE;
	flags_t Flags = lua_isnoneornil(L, pos) ? 0 : GetFlagCombination(L, pos, &success);

	if (!success)
		luaL_error(L, "invalid flag combination");

	return Flags;
}

flags_t OptFlags(lua_State* L, int pos, flags_t dflt)
{
	return lua_isnoneornil(L, pos) ? dflt : CheckFlags(L, pos);
}

flags_t CheckFlagsFromTable(lua_State *L, int pos, const char* key)
{
	flags_t f = 0;
	lua_getfield(L, pos, key);
	if (!lua_isnil(L, -1))
		f = CheckFlags(L, -1);
	lua_pop(L, 1);
	return f;
}

flags_t GetFlagsFromTable(lua_State *L, int pos, const char* key)
{
	lua_getfield(L, pos, key);
	flags_t f = GetFlagCombination(L, -1, NULL);
	lua_pop(L, 1);
	return f;
}

void PutFlagsToTable(lua_State *L, const char* key, flags_t flags)
{
	bit64_push(L, flags);
	lua_setfield(L, -2, key);
}

void PutFlagsToArray(lua_State *L, int index, flags_t flags)
{
	bit64_push(L, flags);
	lua_rawseti(L, -2, index);
}
