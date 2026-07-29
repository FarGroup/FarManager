// lf_viewer.c

#include <windows.h>

#include <lua.h>
#include <lauxlib.h>

#include "lf_flags.h"
#include "lf_luafar.h"
#include "lf_service.h"
#include "lf_string.h"
#include "lf_util.h"

static int push_viewer_filename(lua_State *L, intptr_t Id)
{
	PSInfo *Info = GetPluginData(L)->Info;
	size_t size = Info->ViewerControl(Id, VCTL_GETFILENAME, 0, 0);

	if (!size) return 0;

	wchar_t* fname = (wchar_t*)lua_newuserdata(L, size * sizeof(wchar_t));
	size = Info->ViewerControl(Id, VCTL_GETFILENAME, size, fname);

	if (size)
	{
		push_utf8_string(L, fname, -1);
		lua_remove(L, -2);
		return 1;
	}

	lua_pop(L,1);
	return 0;
}

static int viewer_SetKeyBar(lua_State *L)
{
	return SetKeyBar(L, FALSE);
}

static int viewer_Viewer(lua_State *L)
{
	PSInfo *Info = GetPluginData(L)->Info;
	const wchar_t* FileName = check_utf8_string(L, 1, NULL);
	const wchar_t* Title    = opt_utf8_string(L, 2, NULL);
	intptr_t X1 = luaL_optinteger(L, 3, 0);
	intptr_t Y1 = luaL_optinteger(L, 4, 0);
	intptr_t X2 = luaL_optinteger(L, 5, -1);
	intptr_t Y2 = luaL_optinteger(L, 6, -1);
	flags_t  Flags = OptFlags(L, 7, 0);
	intptr_t CodePage = luaL_optinteger(L, 8, CP_DEFAULT);
	intptr_t ret = Info->Viewer(FileName, Title, X1, Y1, X2, Y2, Flags, CodePage);
	lua_pushboolean(L, ret != 0);
	return 1;
}

static int viewer_GetFileName(lua_State *L)
{
	intptr_t ViewerId = luaL_optinteger(L, 1, -1);

	if (!push_viewer_filename(L, ViewerId)) lua_pushnil(L);

	return 1;
}

static int viewer_GetInfo(lua_State *L)
{
	intptr_t ViewerId = luaL_optinteger(L, 1, -1);
	PSInfo *Info = GetPluginData(L)->Info;
	struct ViewerInfo vi = { sizeof(vi) };

	if (Info->ViewerControl(ViewerId, VCTL_GETINFO, 0, &vi))
	{
		lua_createtable(L, 0, 10);
		PutNumToTable(L, "ViewerID", (double) vi.ViewerID);

		if (push_viewer_filename(L, ViewerId))
			lua_setfield(L, -2, "FileName");

		PutNumToTable(L,  "FileSize", (double) vi.FileSize);
		PutNumToTable(L,  "FilePos", (double) vi.FilePos);
		PutNumToTable(L,  "WindowSizeX", vi.WindowSizeX);
		PutNumToTable(L,  "WindowSizeY", vi.WindowSizeY);
		PutNumToTable(L,  "Options", vi.Options);
		PutNumToTable(L,  "TabSize", vi.TabSize);
		PutNumToTable(L,  "LeftPos", vi.LeftPos + 1);
		lua_createtable(L, 0, 3);
		PutNumToTable(L, "CodePage", vi.CurMode.CodePage);
		PutFlagsToTable(L, "Flags",  vi.CurMode.Flags);
		PutNumToTable(L, "ViewMode", vi.CurMode.ViewMode);
		lua_setfield(L, -2, "CurMode");
	}
	else
		lua_pushnil(L);

	return 1;
}

static int viewer_Quit(lua_State *L)
{
	intptr_t ViewerId = luaL_optinteger(L, 1, -1);
	PSInfo *Info = GetPluginData(L)->Info;
	lua_pushboolean(L, Info->ViewerControl(ViewerId, VCTL_QUIT, 0, 0));
	return 1;
}

static int viewer_Redraw(lua_State *L)
{
	intptr_t ViewerId = luaL_optinteger(L, 1, -1);
	PSInfo *Info = GetPluginData(L)->Info;
	Info->ViewerControl(ViewerId, VCTL_REDRAW, 0, 0);
	return 0;
}

static int viewer_Select(lua_State *L)
{
	PSInfo *Info = GetPluginData(L)->Info;
	intptr_t ViewerId = luaL_optinteger(L, 1, -1);
	struct ViewerSelect vs = { sizeof (vs) };
	vs.BlockStartPos = (INT64)luaL_checknumber(L,2);
	vs.BlockLen = (INT64)luaL_checknumber(L,3);
	lua_pushboolean(L, Info->ViewerControl(ViewerId, VCTL_SELECT, 0, &vs) != 0);
	return 1;
}

static int viewer_SetPosition(lua_State *L)
{
	intptr_t ViewerId = luaL_optinteger(L, 1, -1);
	PSInfo *Info = GetPluginData(L)->Info;
	struct ViewerSetPosition vsp = { sizeof(vsp) };

	if (lua_istable(L, 2))
	{
		lua_settop(L, 2);
		vsp.StartPos = (__int64)GetOptNumFromTable(L, "StartPos", 0);
		vsp.LeftPos = (__int64)GetOptNumFromTable(L, "LeftPos", 1) - 1;
		vsp.Flags = CheckFlagsFromTable(L, -1, "Flags");
	}
	else
	{
		vsp.StartPos = (__int64)luaL_optnumber(L,2,0);
		vsp.LeftPos = (__int64)luaL_optnumber(L,3,1) - 1;
		vsp.Flags = OptFlags(L,4,0);
	}

	if (Info->ViewerControl(ViewerId, VCTL_SETPOSITION, 0, &vsp))
		lua_pushnumber(L, (double)vsp.StartPos);
	else
		lua_pushnil(L);

	return 1;
}

static int viewer_SetMode(lua_State *L)
{
	int success;
	struct ViewerSetMode vsm = { sizeof(vsm) };
	intptr_t ViewerId = luaL_optinteger(L, 1, -1);
	luaL_checktype(L, 2, LUA_TTABLE);
	lua_getfield(L, 2, "Type");
	vsm.Type = get_env_flag(L, -1, &success);

	if (!success)
		return lua_pushboolean(L,0), 1;

	lua_getfield(L, 2, "iParam");

	if (lua_isnumber(L, -1))
		vsm.Param.iParam = lua_tointeger(L, -1);
	else
		return lua_pushboolean(L,0), 1;

	lua_getfield(L, 2, "Flags");
	vsm.Flags = get_env_flag(L, -1, &success);

	if (!success)
		return lua_pushboolean(L,0), 1;

	lua_pushboolean(L, GetPluginData(L)->Info->ViewerControl(ViewerId, VCTL_SETMODE, 0, &vsm) != 0);
	return 1;
}

const luaL_Reg viewer_funcs[] =
{
	PAIR( viewer, GetFileName),
	PAIR( viewer, GetInfo),
	PAIR( viewer, Quit),
	PAIR( viewer, Redraw),
	PAIR( viewer, Select),
	PAIR( viewer, SetKeyBar),
	PAIR( viewer, SetMode),
	PAIR( viewer, SetPosition),
	PAIR( viewer, Viewer),

	{NULL, NULL},
};

int luaopen_viewer(lua_State *L)
{
	luaL_register(L, "viewer", viewer_funcs);
	return 0;
}
