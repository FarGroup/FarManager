/* lusercontrol.c */

#include "ustring.h"
#include "util.h"
#include "compat52.h"

#define TYPE_USERCONTROL "far_usercontrol"

TFarUserControl* CheckFarUserControl(lua_State* L, int pos)
{
	TFarUserControl* fuc = (TFarUserControl*)luaL_checkudata(L, pos, TYPE_USERCONTROL);
	return fuc;
}

static intptr_t CheckFarUserControlIndex(lua_State* L, TFarUserControl* fuc, int pos)
{
	intptr_t index = luaL_checkinteger(L, pos);
	luaL_argcheck(L, index > 0 && index <= fuc->Size, pos, "bounds checking failed");
	return index-1;
}

static int uc_tostring(lua_State* L)
{
	TFarUserControl* fuc = CheckFarUserControl(L, 1);
	lua_pushfstring(L, "%s (%p)", TYPE_USERCONTROL, fuc);
	return 1;
}

int uc_len(lua_State* L)
{
	TFarUserControl* fuc = CheckFarUserControl(L, 1);
	lua_pushinteger(L, fuc->Size);
	return 1;
}

static int uc_index(lua_State* L)
{
	TFarUserControl* fuc = CheckFarUserControl(L, 1);
	const char* method = luaL_checkstring(L, 2);
	if(!strcmp(method, "rawhandle"))
	{
		lua_pushlightuserdata(L, fuc->VBuf);
	}
	else
	{
		intptr_t index = CheckFarUserControlIndex(L, fuc, 2);
		lua_createtable(L, 0, 2);
		PutWStrToTable(L, "Char", &fuc->VBuf[index].Char, 1);
		PushFarColor(L, &fuc->VBuf[index].Attributes);
		lua_setfield(L, -2, "Attributes");
	}
	return 1;
}

static int uc_newindex(lua_State* L)
{
	TFarUserControl* fuc = CheckFarUserControl(L, 1);
	intptr_t index = CheckFarUserControlIndex(L, fuc, 2);
	luaL_checktype(L, 3, LUA_TTABLE);
	lua_getfield(L, 3, "Char");
	if(lua_type(L, -1) == LUA_TNUMBER)
	{
		fuc->VBuf[index].Char = (WCHAR)luaL_checkinteger(L, -1);
	}
	else
	{
		const wchar_t* Char = check_utf8_string(L, -1, NULL);
		fuc->VBuf[index].Char = Char[0];
	}
	lua_pop(L, 1);
	lua_getfield(L, 3, "Attributes");
	GetFarColor(L, -1, &fuc->VBuf[index].Attributes);
	lua_pop(L, 1);
	return 0;
}

static TFarUserControl* push_far_usercontrol(lua_State* L, intptr_t X, intptr_t Y)
{
	TFarUserControl* fuc = (TFarUserControl*)lua_newuserdata(L, sizeof(TFarUserControl) + sizeof(struct FAR_CHAR_INFO)*(X*Y-1));
	fuc->X = X;
	fuc->Y = Y;
	fuc->Size = X*Y;
	luaL_getmetatable(L, TYPE_USERCONTROL);
	lua_setmetatable(L, -2);
	return fuc;
}

static int uc_New(lua_State* L)
{
	intptr_t X = luaL_checkinteger(L, 1), Y = luaL_checkinteger(L, 2);
	luaL_argcheck(L, X > 0, 1, "X must be positive");
	luaL_argcheck(L, Y > 0, 2, "Y must be positive");
	push_far_usercontrol(L, X, Y);
	return 1;
}

const luaL_Reg usercontrol_methods[] =
{
	{"__tostring", uc_tostring},
	{"__len", uc_len},
	{"__index", uc_index},
	{"__newindex",uc_newindex},
	{NULL, NULL}
};

const luaL_Reg usercontrol_functions[] =
{
	{"CreateUserControl", uc_New},
	{NULL, NULL}
};

int luaopen_usercontrol(lua_State* L)
{
	luaL_newmetatable(L, TYPE_USERCONTROL);
	luaL_register(L, NULL, usercontrol_methods);
	luaL_register(L, "far", usercontrol_functions);
	return 1;
}
