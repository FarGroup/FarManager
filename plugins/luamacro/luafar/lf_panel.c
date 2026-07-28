#include <windows.h>
#include <stdlib.h>

#include "lf_flags.h"
#include "lf_luafar.h"
#include "lf_service.h"
#include "lf_string.h"
#include "lf_util.h"

HANDLE OptHandle(lua_State *L)
{
	switch(lua_type(L,1))
	{
		case LUA_TNONE:
		case LUA_TNIL:
			break;

		case LUA_TNUMBER:
		{
			lua_Integer whatPanel = lua_tointeger(L,1);
			HANDLE hh = (HANDLE)whatPanel;
			return (hh==PANEL_PASSIVE || hh==PANEL_ACTIVE) ? hh : whatPanel%2 ? PANEL_ACTIVE:PANEL_PASSIVE;
		}

		case LUA_TLIGHTUSERDATA:
			return lua_touserdata(L,1);

		default:
			luaL_typerror(L, 1, "integer or light userdata");
	}
	return NULL;
}

static HANDLE OptHandle2(lua_State *L)
{
	return lua_isnoneornil(L,1) ? (luaL_checkinteger(L,2) % 2 ? PANEL_ACTIVE:PANEL_PASSIVE) : OptHandle(L);
}

static int panel_CheckPanelsExist(lua_State *L)
{
	PSInfo *Info = GetPluginData(L)->Info;
	HANDLE handle = OptHandle(L);
	lua_pushboolean(L, (int)Info->PanelControl(handle, FCTL_CHECKPANELSEXIST, 0, 0));
	return 1;
}

static int panel_ClosePanel(lua_State *L)
{
	PSInfo *Info = GetPluginData(L)->Info;
	HANDLE handle = OptHandle(L);
	const wchar_t *dir = opt_utf8_string(L, 2, NULL);
	lua_pushboolean(L, (int)Info->PanelControl(handle, FCTL_CLOSEPANEL, 0, (void*)dir));
	return 1;
}

static int panel_GetPanelInfo(lua_State *L)
{
	TPluginData *pd = GetPluginData(L);
	HANDLE handle = OptHandle2(L);
	struct PanelInfo pi = { sizeof(pi) };

	if (!pd->Info->PanelControl(handle, FCTL_GETPANELINFO, 0, &pi))
		return lua_pushnil(L), 1;

	lua_createtable(L, 0, 13);
	//-------------------------------------------------------------------------
	PutLStrToTable(L, "OwnerGuid", &pi.OwnerGuid, sizeof(GUID));
	pi.PluginHandle ? lua_pushlightuserdata(L, pi.PluginHandle) : lua_pushnil(L);
	lua_setfield(L, -2, "PluginHandle");
	//-------------------------------------------------------------------------
	if (0 == memcmp(&pi.OwnerGuid, pd->PluginId, sizeof(GUID)))
	{
		PushPluginObject(L, pi.PluginHandle);
		lua_setfield(L, -2, "PluginObject");
	}
	//-------------------------------------------------------------------------
	PutIntToTable(L, "PanelType", pi.PanelType);
	//-------------------------------------------------------------------------
	PutRECTToTable(L, "PanelRect", pi.PanelRect);
	//-------------------------------------------------------------------------
	PutIntToTable(L, "ItemsNumber", pi.ItemsNumber);
	PutIntToTable(L, "SelectedItemsNumber", pi.SelectedItemsNumber);
	//-------------------------------------------------------------------------
	PutIntToTable(L, "CurrentItem", pi.CurrentItem + 1);
	PutIntToTable(L, "TopPanelItem", pi.TopPanelItem + 1);
	PutIntToTable(L, "ViewMode", pi.ViewMode);
	PutIntToTable(L, "SortMode", pi.SortMode);
	PutFlagsToTable(L, "Flags", pi.Flags);
	//-------------------------------------------------------------------------
	return 1;
}

static int get_panel_item(lua_State *L, int command)
{
	struct FarGetPluginPanelItem fgppi = { sizeof(struct FarGetPluginPanelItem),0,0 };
	PSInfo *Info = GetPluginData(L)->Info;
	HANDLE handle = OptHandle2(L);
	intptr_t index = luaL_optinteger(L,3,1) - 1;
	if (index >= 0 || command == FCTL_GETCURRENTPANELITEM)
	{
		fgppi.Size = Info->PanelControl(handle, command, index, &fgppi);
		if (fgppi.Size)
		{
			fgppi.Item = (struct PluginPanelItem*)lua_newuserdata(L, fgppi.Size);
			if (Info->PanelControl(handle, command, index, &fgppi))
			{
				PushPanelItem(L, fgppi.Item, 0);
				return 1;
			}
		}
	}
	return lua_pushnil(L), 1;
}

static int panel_GetPanelItem(lua_State *L)
{
	return get_panel_item(L, FCTL_GETPANELITEM);
}

static int panel_GetSelectedPanelItem(lua_State *L)
{
	return get_panel_item(L, FCTL_GETSELECTEDPANELITEM);
}

static int panel_GetCurrentPanelItem(lua_State *L)
{
	return get_panel_item(L, FCTL_GETCURRENTPANELITEM);
}

static int get_string_info(lua_State *L, int command)
{
	PSInfo *Info = GetPluginData(L)->Info;
	HANDLE handle = OptHandle2(L);
	intptr_t size = Info->PanelControl(handle, command, 0, 0);

	if (size)
	{
		wchar_t *buf = (wchar_t*)lua_newuserdata(L, size * sizeof(wchar_t));

		if (Info->PanelControl(handle, command, size, buf))
		{
			push_utf8_string(L, buf, -1);
			return 1;
		}
	}

	return lua_pushnil(L), 1;
}

static int panel_GetPanelFormat(lua_State *L)
{
	return get_string_info(L, FCTL_GETPANELFORMAT);
}

static int panel_GetPanelHostFile(lua_State *L)
{
	return get_string_info(L, FCTL_GETPANELHOSTFILE);
}

static int panel_GetColumnTypes(lua_State *L)
{
	return get_string_info(L, FCTL_GETCOLUMNTYPES);
}

static int panel_GetColumnWidths(lua_State *L)
{
	return get_string_info(L, FCTL_GETCOLUMNWIDTHS);
}

static int panel_GetPanelPrefix(lua_State *L)
{
	return get_string_info(L, FCTL_GETPANELPREFIX);
}

static int panel_RedrawPanel(lua_State *L)
{
	PSInfo *Info = GetPluginData(L)->Info;
	void *param2 = NULL;
	HANDLE handle = OptHandle2(L);
	struct PanelRedrawInfo pri = { sizeof(pri) };

	if (lua_istable(L, 3))
	{
		param2 = &pri;
		lua_getfield(L, 3, "CurrentItem");
		pri.CurrentItem = lua_tointeger(L, -1) - 1;
		lua_getfield(L, 3, "TopPanelItem");
		pri.TopPanelItem = lua_tointeger(L, -1) - 1;
	}

	lua_pushboolean(L, Info->PanelControl(handle, FCTL_REDRAWPANEL, 0, param2) != 0);
	return 1;
}

static int SetPanelBooleanProperty(lua_State *L, int command)
{
	PSInfo *Info = GetPluginData(L)->Info;
	HANDLE handle = OptHandle2(L);
	int param1 = lua_toboolean(L,3);
	lua_pushboolean(L, Info->PanelControl(handle, command, param1, 0) != 0);
	return 1;
}

static int SetPanelIntegerProperty(lua_State *L, int command)
{
	PSInfo *Info = GetPluginData(L)->Info;
	HANDLE handle = OptHandle2(L);
	int param1 = (int) check_env_flag(L,3);
	lua_pushboolean(L, Info->PanelControl(handle, command, param1, 0) != 0);
	return 1;
}

static int panel_SetSortOrder(lua_State *L)
{
	return SetPanelBooleanProperty(L, FCTL_SETSORTORDER);
}

static int panel_SetDirectoriesFirst(lua_State *L)
{
	return SetPanelBooleanProperty(L, FCTL_SETDIRECTORIESFIRST);
}

static int panel_UpdatePanel(lua_State *L)
{
	return SetPanelBooleanProperty(L, FCTL_UPDATEPANEL);
}

static int panel_SetSortMode(lua_State *L)
{
	return SetPanelIntegerProperty(L, FCTL_SETSORTMODE);
}

static int panel_SetViewMode(lua_State *L)
{
	return SetPanelIntegerProperty(L, FCTL_SETVIEWMODE);
}

static int panel_GetPanelDirectory(lua_State *L)
{
	TPluginData *pd = GetPluginData(L);
	HANDLE handle = OptHandle2(L);
	intptr_t size = pd->Info->PanelControl(handle, FCTL_GETPANELDIRECTORY, 0, NULL);

	if (size)
	{
		struct FarPanelDirectory *fpd = (struct FarPanelDirectory*)lua_newuserdata(L, size);
		memset(fpd, 0, size);
		fpd->StructSize = sizeof(*fpd);

		if (pd->Info->PanelControl(handle, FCTL_GETPANELDIRECTORY, size, fpd))
		{
			lua_createtable(L, 0, 4);
			PutWStrToTable(L, "Name",  fpd->Name, -1);
			PutWStrToTable(L, "Param", fpd->Param, -1);
			PutWStrToTable(L, "File",  fpd->File, -1);
			PutLStrToTable(L, "PluginId", &fpd->PluginId, sizeof(fpd->PluginId));
			return 1;
		}
	}

	return lua_pushnil(L), 1;
}

static int panel_SetPanelDirectory(lua_State *L)
{
	TPluginData *pd = GetPluginData(L);
	struct FarPanelDirectory fpd = { sizeof(fpd) }; // also sets fpd.PluginId = FarId
	HANDLE handle = OptHandle2(L);

	if (lua_istable(L, 3))
	{
		size_t len;
		lua_getfield(L, 3, "PluginId");
		const GUID* id = (const GUID*)lua_tolstring(L, -1, &len);

		if (id && len == sizeof(GUID)) fpd.PluginId = *id;

		lua_getfield(L, 3, "Name");
		if (lua_isstring(L, -1)) fpd.Name = check_utf8_string(L, -1, NULL);

		lua_getfield(L, 3, "Param");
		if (lua_isstring(L, -1)) fpd.Param = check_utf8_string(L, -1, NULL);

		lua_getfield(L, 3, "File");
		if (lua_isstring(L, -1)) fpd.File = check_utf8_string(L, -1, NULL);
	}
	else if (lua_isstring(L, 3))
		fpd.Name = check_utf8_string(L, 3, NULL);
	else
		luaL_argerror(L, 3, "table or string");

	lua_pushboolean(L, pd->Info->PanelControl(handle, FCTL_SETPANELDIRECTORY, 0, &fpd) != 0);
	return 1;
}

static int panel_GetCmdLine(lua_State *L)
{
	PSInfo *Info = GetPluginData(L)->Info;
	HANDLE handle = OptHandle(L);
	intptr_t size = Info->PanelControl(handle, FCTL_GETCMDLINE, 0, 0);
	wchar_t *buf = (wchar_t*) malloc(size*sizeof(wchar_t));
	Info->PanelControl(handle, FCTL_GETCMDLINE, size, buf);
	push_utf8_string(L, buf, -1);
	free(buf);
	return 1;
}

static int panel_SetCmdLine(lua_State *L)
{
	PSInfo *Info = GetPluginData(L)->Info;
	HANDLE handle = OptHandle(L);
	wchar_t* str = check_utf8_string(L, 2, NULL);
	lua_pushboolean(L, Info->PanelControl(handle, FCTL_SETCMDLINE, 0, str) != 0);
	return 1;
}

static int panel_GetCmdLinePos(lua_State *L)
{
	PSInfo *Info = GetPluginData(L)->Info;
	HANDLE handle = OptHandle(L);
	int pos;
	Info->PanelControl(handle, FCTL_GETCMDLINEPOS, 0, &pos) ?
	lua_pushinteger(L, pos+1) : lua_pushnil(L);
	return 1;
}

static int panel_SetCmdLinePos(lua_State *L)
{
	PSInfo *Info = GetPluginData(L)->Info;
	HANDLE handle = OptHandle(L);
	intptr_t pos = luaL_checkinteger(L, 2) - 1;
	intptr_t ret = Info->PanelControl(handle, FCTL_SETCMDLINEPOS, pos, NULL);
	return lua_pushboolean(L, ret != 0), 1;
}

static int panel_InsertCmdLine(lua_State *L)
{
	PSInfo *Info = GetPluginData(L)->Info;
	HANDLE handle = OptHandle(L);
	wchar_t* str = check_utf8_string(L, 2, NULL);
	lua_pushboolean(L, Info->PanelControl(handle, FCTL_INSERTCMDLINE, 0, str) != 0);
	return 1;
}

static int panel_GetCmdLineSelection(lua_State *L)
{
	PSInfo *Info = GetPluginData(L)->Info;
	HANDLE handle = OptHandle(L);
	struct CmdLineSelect cms = { sizeof(cms) };

	if (Info->PanelControl(handle, FCTL_GETCMDLINESELECTION, 0, &cms))
	{
		if (cms.SelStart < 0) cms.SelStart = 0;

		if (cms.SelEnd < 0) cms.SelEnd = 0;

		lua_pushinteger(L, cms.SelStart + 1);
		lua_pushinteger(L, cms.SelEnd);
		return 2;
	}

	return lua_pushnil(L), 1;
}

static int panel_SetCmdLineSelection(lua_State *L)
{
	PSInfo *Info = GetPluginData(L)->Info;
	HANDLE handle = OptHandle(L);
	struct CmdLineSelect cms = { sizeof(cms) };
	cms.SelStart = luaL_checkinteger(L, 2) - 1;
	cms.SelEnd = luaL_checkinteger(L, 3);

	if (cms.SelStart < -1) cms.SelStart = -1;

	if (cms.SelEnd < -1) cms.SelEnd = -1;

	lua_pushboolean(L, Info->PanelControl(handle, FCTL_SETCMDLINESELECTION, 0, &cms) != 0);
	return 1;
}

// CtrlSetSelection   (handle, whatpanel, items, selection)
// CtrlClearSelection (handle, whatpanel, items)
//   handle:       handle
//   whatpanel:    1=active_panel, 0=inactive_panel
//   items:        either number of an item, or a list of item numbers
//   selection:    boolean
static int ChangePanelSelection(lua_State *L, intptr_t command)
{
	PSInfo *Info = GetPluginData(L)->Info;
	intptr_t itemindex = -1;
	HANDLE handle = OptHandle2(L);

	if (lua_isnumber(L,3))
	{
		itemindex = lua_tointeger(L,3) - 1;

		if (itemindex < 0) return luaL_argerror(L, 3, "non-positive index");
	}
	else if (!lua_istable(L,3))
		return luaL_typerror(L, 3, "number or table");

	intptr_t state = (command == FCTL_SETSELECTION) ? lua_toboolean(L,4) : 0;

	// get panel info
	struct PanelInfo pi = { sizeof(pi) };
	if (!Info->PanelControl(handle, FCTL_GETPANELINFO, 0, &pi) || (pi.PanelType != PTYPE_FILEPANEL))
		return lua_pushboolean(L,0), 1;

	intptr_t numItems = (command == FCTL_SETSELECTION) ? pi.ItemsNumber : pi.SelectedItemsNumber;

	if (itemindex >= 0 && itemindex < numItems)
		Info->PanelControl(handle, command, itemindex, (void*)state);
	else
	{
		intptr_t len = lua_objlen(L,3);

		for(intptr_t i=1; i<=len; i++)
		{
			lua_pushinteger(L, i);
			lua_gettable(L,3);

			if (lua_isnumber(L,-1))
			{
				itemindex = lua_tointeger(L,-1) - 1;

				if (itemindex >= 0 && itemindex < numItems)
					Info->PanelControl(handle, command, itemindex, (void*)state);
			}

			lua_pop(L,1);
		}
	}

	return lua_pushboolean(L,1), 1;
}

static int panel_SetSelection(lua_State *L)
{
	return ChangePanelSelection(L, FCTL_SETSELECTION);
}

static int panel_ClearSelection(lua_State *L)
{
	return ChangePanelSelection(L, FCTL_CLEARSELECTION);
}

static int panel_BeginSelection(lua_State *L)
{
	intptr_t res = GetPluginData(L)->Info->PanelControl(OptHandle2(L), FCTL_BEGINSELECTION, 0, 0);
	return lua_pushboolean(L, (int)res), 1;
}

static int panel_EndSelection(lua_State *L)
{
	intptr_t res = GetPluginData(L)->Info->PanelControl(OptHandle2(L), FCTL_ENDSELECTION, 0, 0);
	return lua_pushboolean(L, (int)res), 1;
}

// CtrlSetUserScreen (handle, scrolltype)
//   handle:       FALSE=INVALID_HANDLE_VALUE, TRUE=lua_State*
static int panel_SetUserScreen(lua_State *L)
{
	PSInfo *Info = GetPluginData(L)->Info;
	HANDLE handle = OptHandle(L);
	intptr_t scrolltype = luaL_optinteger(L,2,0);
	int ret = Info->PanelControl(handle, FCTL_SETUSERSCREEN, scrolltype, 0) != 0;
	lua_pushboolean(L, ret);
	return 1;
}

// CtrlGetUserScreen (handle, scrolltype)
//   handle:       FALSE=INVALID_HANDLE_VALUE, TRUE=lua_State*
static int panel_GetUserScreen(lua_State *L)
{
	PSInfo *Info = GetPluginData(L)->Info;
	HANDLE handle = OptHandle(L);
	intptr_t scrolltype = luaL_optinteger(L,2,0);
	int ret = Info->PanelControl(handle, FCTL_GETUSERSCREEN, scrolltype, 0) != 0;
	lua_pushboolean(L, ret);
	return 1;
}

static int panel_IsActivePanel(lua_State *L)
{
	PSInfo *Info = GetPluginData(L)->Info;
	HANDLE handle = OptHandle(L);
	lua_pushboolean(L, Info->PanelControl(handle, FCTL_ISACTIVEPANEL, 0, 0) != 0);
	return 1;
}

static int panel_SetActivePanel(lua_State *L)
{
	PSInfo *Info = GetPluginData(L)->Info;
	HANDLE handle = OptHandle2(L);
	lua_pushboolean(L, Info->PanelControl(handle, FCTL_SETACTIVEPANEL, 0, 0) != 0);
	return 1;
}

const luaL_Reg panel_funcs[] =
{
	PAIR( panel, BeginSelection),
	PAIR( panel, CheckPanelsExist),
	PAIR( panel, ClearSelection),
	PAIR( panel, ClosePanel),
	PAIR( panel, EndSelection),
	PAIR( panel, GetCmdLine),
	PAIR( panel, GetCmdLinePos),
	PAIR( panel, GetCmdLineSelection),
	PAIR( panel, GetColumnTypes),
	PAIR( panel, GetColumnWidths),
	PAIR( panel, GetCurrentPanelItem),
	PAIR( panel, GetPanelDirectory),
	PAIR( panel, GetPanelFormat),
	PAIR( panel, GetPanelHostFile),
	PAIR( panel, GetPanelInfo),
	PAIR( panel, GetPanelItem),
	PAIR( panel, GetPanelPrefix),
	PAIR( panel, GetSelectedPanelItem),
	PAIR( panel, GetUserScreen),
	PAIR( panel, InsertCmdLine),
	PAIR( panel, IsActivePanel),
	PAIR( panel, RedrawPanel),
	PAIR( panel, SetActivePanel),
	PAIR( panel, SetCmdLine),
	PAIR( panel, SetCmdLinePos),
	PAIR( panel, SetCmdLineSelection),
	PAIR( panel, SetDirectoriesFirst),
	PAIR( panel, SetPanelDirectory),
	PAIR( panel, SetSelection),
	PAIR( panel, SetSortMode),
	PAIR( panel, SetSortOrder),
	PAIR( panel, SetUserScreen),
	PAIR( panel, SetViewMode),
	PAIR( panel, UpdatePanel),

	{NULL, NULL},
};

int luaopen_panel(lua_State *L)
{
	luaL_register(L, "panel", panel_funcs);
	lua_pop(L, 1);
	return 0;
}
