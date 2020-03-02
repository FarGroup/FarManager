#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include "compat52.h"

#define MAX53 0x1FFFFFFFFFFFFFLL
typedef __int64 INT64;
typedef unsigned __int64 UINT64;
static int f_new(lua_State *L); /* forward declaration */

const char metatable_name[] = "64 bit integer";

int bit64_pushuserdata(lua_State *L, INT64 v)
{
	*(INT64*)lua_newuserdata(L, sizeof(INT64)) = v;
	luaL_getmetatable(L, metatable_name);
	lua_setmetatable(L, -2);
	return 1;
}

int bit64_push(lua_State *L, INT64 v)
{
	if((v >= 0 && v <= MAX53) || (v < 0 && v >= -MAX53))
		lua_pushnumber(L, (double)v);
	else
		bit64_pushuserdata(L, v);

	return 1;
}

int bit64_getvalue(lua_State *L, int pos, INT64 *target)
{
	if(lua_type(L,pos)==LUA_TUSERDATA)
	{
		int equal;
		lua_getmetatable(L, pos);
		luaL_getmetatable(L, metatable_name);
		equal = lua_rawequal(L,-1,-2);
		lua_pop(L,2);

		if(equal && target)
		{
			*target = *(INT64*)lua_touserdata(L,pos);
		}

		return equal;
	}

	return 0;
}

INT64 check64(lua_State *L, int pos, int* success)
{
	INT64 ret;
	int tp = lua_type(L, pos);

	if(success) *success = 1;
	if(pos < 0) pos += lua_gettop(L) + 1; /* mandatory in this function */

	if(tp == LUA_TNUMBER)
	{
		double dd = lua_tonumber(L, pos);
		if ((dd>=0 && dd<=0x7fffffffffffffffULL) || (dd<0 && -dd<=0x8000000000000000ULL))
			return (INT64)dd;
	}
	else
	{
		if(tp == LUA_TSTRING)
		{
			lua_pushcfunction(L, f_new);
			lua_pushvalue(L, pos);
			lua_call(L, 1, 1);
			lua_replace(L, pos);
		}
		if(bit64_getvalue(L, pos, &ret))
			return ret;
	}

	if (success) *success=0;
	else luaL_argerror(L, pos, "bad int64");

	return 0;
}

static int band(lua_State *L)
{
	int i;
	UINT64 v = check64(L,1,NULL);
	int n = lua_gettop(L);

	for(i=2; i<=n; i++)
		v &= check64(L,i,NULL);

	return bit64_push(L,v);
}

static int bor(lua_State *L)
{
	int i;
	UINT64 v = check64(L,1,NULL);
	int n = lua_gettop(L);

	for(i=2; i<=n; i++)
		v |= check64(L,i,NULL);

	return bit64_push(L,v);
}

static int bxor(lua_State *L)
{
	int i;
	UINT64 v = check64(L,1,NULL);
	int n = lua_gettop(L);

	for(i=2; i<=n; i++)
		v ^= check64(L,i,NULL);

	return bit64_push(L,v);
}

static int bnot(lua_State *L)
{
	return bit64_push(L, ~check64(L,1,NULL));
}

static int lshift(lua_State *L)
{
	UINT64 v = check64(L, 1, NULL);
	unsigned int n = luaL_checkint(L, 2);
	return bit64_push(L, n < 64 ? (v << n) : 0);
}

static int rshift(lua_State *L)
{
	UINT64 v = check64(L, 1, NULL);
	unsigned int n = luaL_checkint(L, 2);
	return bit64_push(L, n < 64 ? (v >> n) : 0);
}

static int arshift(lua_State *L)
{
	INT64 v = check64(L, 1, NULL);
	unsigned int n = luaL_checkint(L, 2);
	return bit64_push(L, n < 64 ? (v >> n) : ((v >> 63) >> 1));
}

static int f_new(lua_State *L)
{
	int type = lua_type(L, 1);

	if(type == LUA_TSTRING)
	{
		INT64 v = 0;
		size_t i = 0;
		size_t len;
		const char* s = lua_tolstring(L, 1, &len);

		if(s[0]=='-')
			i++;

		if(len-i > 2 && len-i <= 18 && s[i]=='0' && (s[i+1]=='x' || s[i+1]=='X'))
		{
			i += 2;

			for(; i<len; i++)
			{
				int a;

				if(s[i] >= '0' && s[i] <= '9')      a = s[i] - '0';
				else if(s[i] >= 'A' && s[i] <= 'F') a = s[i] + 10 - 'A';
				else if(s[i] >= 'a' && s[i] <= 'f') a = s[i] + 10 - 'a';
				else break;

				v = (v << 4) | a;
			}

			if(i == len)
				return bit64_pushuserdata(L, s[0] == '-' ? -v : v);
		}
		else if(len-i > 0)
		{
			for(; i<len; i++)
			{
				if(s[i] >= '0' && s[i] <= '9')
					v = (v * 10) + s[i] - '0';
				else break;
			}

			if(i == len)
				return bit64_pushuserdata(L, s[0] == '-' ? -v : v);
		}
	}
	else if(type == LUA_TNUMBER)
	{
		double d = lua_tonumber(L, 1);
		INT64 v = (INT64)lua_tonumber(L, 1);
		if (d == v)
			return bit64_pushuserdata(L, v);
	}
	else
	{
		INT64 v;

		if(bit64_getvalue(L, 1, &v))
			return bit64_pushuserdata(L, v);
	}

	lua_pushnil(L);
	return 1;
}

static int f_tostring(lua_State *L)
{
	char buf[32];
	sprintf(buf, "%I64d", check64(L,1,NULL));
	lua_pushstring(L, buf);
	return 1;
}

static int f_type(lua_State *L)
{
	bit64_getvalue(L,1,NULL) ? lua_pushstring(L,metatable_name):lua_pushnil(L);
	return 1;
}

static int f_add(lua_State *L)
{
	int i;
	INT64 v = check64(L,1,NULL);
	int n = lua_gettop(L);

	for(i=2; i<=n; i++)
		v += check64(L,i,NULL);

	return bit64_push(L,v);
}

static int f_sub(lua_State *L)
{
	INT64 a1 = check64(L,1,NULL);
	INT64 a2 = check64(L,2,NULL);
	return bit64_push(L, a1-a2);
}

static int f_mul(lua_State *L)
{
	int i;
	INT64 v = check64(L,1,NULL);
	int n = lua_gettop(L);

	for(i=2; i<=n; i++)
		v *= check64(L,i,NULL);

	return bit64_push(L,v);
}

static int f_div(lua_State *L)
{
	INT64 a1 = check64(L,1,NULL);
	INT64 a2 = check64(L,2,NULL);

	if(a2 != 0)
		bit64_push(L, a1/a2);
	else
	{
		if(a1 > 0)      bit64_push(L, 0x7FFFFFFFFFFFFFFFll);
		else if(a1 < 0) bit64_push(L, 0x8000000000000000ll);
		else             bit64_push(L, 1);
	}

	return 1;
}

static int f_mod(lua_State *L)
{
	INT64 a1 = check64(L,1,NULL);
	INT64 a2 = check64(L,2,NULL);

	if(a2==0) a2=1;

	return bit64_push(L, a1%a2);
}

static int f_unm(lua_State *L)
{
	INT64 a1 = check64(L,1,NULL);
	return bit64_push(L, -a1);
}

static int f_eq(lua_State *L)
{
	INT64 a1 = check64(L,1,NULL);
	INT64 a2 = check64(L,2,NULL);
	lua_pushboolean(L, a1==a2);
	return 1;
}

static int f_lt(lua_State *L)
{
	INT64 a1 = check64(L,1,NULL);
	INT64 a2 = check64(L,2,NULL);
	lua_pushboolean(L, a1<a2);
	return 1;
}

static int f_le(lua_State *L)
{
	INT64 a1 = check64(L,1,NULL);
	INT64 a2 = check64(L,2,NULL);
	lua_pushboolean(L, a1<=a2);
	return 1;
}

static int f_tonumber(lua_State *L)
{
	lua_pushnumber(L, (double)check64(L,1,NULL));
	return 1;
}

static const luaL_Reg funcs[] =
{
	{ "bnot",       bnot    },
	{ "band",       band    },
	{ "bor",        bor     },
	{ "bxor",       bxor    },
	{ "lshift",     lshift  },
	{ "rshift",     rshift  },
	{ "arshift",    arshift },

	{ "new",        f_new   },
	{ "add",        f_add   },
	{ "sub",        f_sub   },
	{ "mul",        f_mul   },
	{ "div",        f_div   },
	{ "mod",        f_mod   },
	{ "eq",         f_eq    },
	{ "lt",         f_lt    },
	{ "le",         f_le    },
	{ "type",       f_type  },

	{ NULL,         NULL    },
};

static const luaL_Reg metamethods[] =
{
	{ "__add",      f_add   },
	{ "__sub",      f_sub   },
	{ "__mul",      f_mul   },
	{ "__div",      f_div   },
	{ "__mod",      f_mod   },
	{ "__unm",      f_unm   },
	{ "__eq",       f_eq    },
	{ "__lt",       f_lt    },
	{ "__le",       f_le    },
	{ "__tostring", f_tostring },
	{ "tonumber",   f_tonumber },
	{ NULL,         NULL    },
};

LUALIB_API int luaopen_bit64(lua_State *L)
{
	luaL_newmetatable(L, metatable_name);
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	luaL_register(L, NULL, metamethods);
	luaL_register(L, "bit64", funcs);
	return 1;
}
