#include <windows.h>
#include <stdlib.h>

#include "lf_flags.h"
#include "lf_luafar.h"
#include "lf_service.h"
#include "lf_string.h"
#include "lf_util.h"

static int DoAdvControl (lua_State *L, int Command, int Delta)
{
	int pos2 = 2-Delta, pos3 = 3-Delta;
	TPluginData *pd = GetPluginData(L);
	GUID* PluginId = pd->PluginId;
	PSInfo *Info = pd->Info;
	intptr_t Param1 = 0;
	void *Param2 = NULL;

	if (Command != ACTL_SYNCHRO)
		lua_settop(L,pos3);  /* for proper calling GetOptIntFromTable and the like */

	if (Delta == 0)
		Command = (int) check_env_flag(L, 1);

	switch(Command)
	{
		default:
			return luaL_argerror(L, 1, "command not supported");

		case ACTL_COMMIT:
		case ACTL_GETWINDOWCOUNT:
		case ACTL_PROGRESSNOTIFY:
		case ACTL_REDRAWALL:
			break;

		case ACTL_QUIT:
			Param1 = luaL_optinteger(L, pos2, EXIT_SUCCESS);
			break;

		case ACTL_GETFARHWND:
			lua_pushlightuserdata(L, (void*) Info->AdvControl(PluginId, Command, 0, NULL));
			return 1;

		case ACTL_SETCURRENTWINDOW:
			Param1 = luaL_checkinteger(L, pos2) - 1;
			break;

		case ACTL_WAITKEY:
		{
			INPUT_RECORD ir;
			if (!lua_isnoneornil(L, pos3))
			{
				OptInputRecord(L, pd, pos3, &ir);
				Param2 = &ir;
			}
			lua_pushinteger(L, Info->AdvControl(PluginId, Command, Param1, Param2));
			return 1;
		}

		case ACTL_GETCOLOR:
		{
			struct FarColor fc;
			Param1 = check_env_flag(L, pos2);

			if (Info->AdvControl(PluginId, Command, Param1, &fc))
				PushFarColor(L, &fc);
			else
				lua_pushnil(L);

			return 1;
		}

		case ACTL_SYNCHRO:
			if (lua_isfunction(L, pos2)) {
				TSynchroData *sd = CreateSynchroData(SYNCHRO_FUNCTION, 0, NULL);
				int top = lua_gettop(L);
				sd->narg = top - pos2 + 1;
				lua_newtable(L);
				for (int i=pos2,j=1; i <= top; ) {
					lua_pushvalue(L, i++);
					lua_rawseti(L, -2, j++);
				}
				sd->ref = luaL_ref(L, LUA_REGISTRYINDEX);
				lua_pushinteger(L, Info->AdvControl(PluginId, Command, 0, sd));
				return 1;
			}
			else {
				luaL_argcheck(L, lua_isnumber(L,pos2), pos2, "integer or function expected");
				TSynchroData *sd = CreateSynchroData(SYNCHRO_COMMON, lua_tointeger(L,pos2), NULL);
				lua_pushinteger(L, Info->AdvControl(PluginId, Command, 0, sd));
				return 1;
			}

		case ACTL_SETPROGRESSSTATE:
			Param1 = (intptr_t) check_env_flag(L, pos2);
			break;

		case ACTL_SETPROGRESSVALUE:
		{
			struct ProgressValue pv = { sizeof(pv) };
			luaL_checktype(L, pos3, LUA_TTABLE);
			pv.Completed = (UINT64)GetOptNumFromTable(L, "Completed", 0.0);
			pv.Total = (UINT64)GetOptNumFromTable(L, "Total", 100.0);
			lua_pushinteger(L, Info->AdvControl(PluginId, Command, Param1, &pv));
			return 1;
		}

		case ACTL_GETARRAYCOLOR:
		{
			intptr_t len = Info->AdvControl(PluginId, Command, 0, NULL);
			struct FarColor *arr = (struct FarColor*) lua_newuserdata(L, len*sizeof(struct FarColor));
			Info->AdvControl(PluginId, Command, len, arr);
			lua_createtable(L, (int)len, 0);

			for(intptr_t i=0; i < len; i++)
			{
				PushFarColor(L, &arr[i]);
				lua_rawseti(L, -2, (int)i+1);
			}
			return 1;
		}

		case ACTL_GETFARMANAGERVERSION:
		{
			struct VersionInfo vi;
			Info->AdvControl(PluginId, Command, 0, &vi);

			if (lua_toboolean(L, pos2))
			{
				lua_pushinteger(L, vi.Major);
				lua_pushinteger(L, vi.Minor);
				lua_pushinteger(L, vi.Revision);
				lua_pushinteger(L, vi.Build);
				lua_pushinteger(L, vi.Stage);
				return 5;
			}
			lua_pushfstring(L, "%d.%d.%d.%d.%d", vi.Major, vi.Minor, vi.Revision, vi.Build, vi.Stage);
			return 1;
		}

		case ACTL_GETWINDOWINFO:
		{
			struct WindowInfo wi = { sizeof(wi) };
			wi.Pos = luaL_optinteger(L, pos2, 0) - 1;
			intptr_t r = Info->AdvControl(PluginId, Command, 0, &wi);

			if (!r)
				return lua_pushnil(L), 1;

			wi.TypeName = (wchar_t*)
			              lua_newuserdata(L, (wi.TypeNameSize + wi.NameSize) * sizeof(wchar_t));
			wi.Name = wi.TypeName + wi.TypeNameSize;
			r = Info->AdvControl(PluginId, Command, 0, &wi);

			if (!r)
				return lua_pushnil(L), 1;

			lua_createtable(L,0,6);

			switch(wi.Type)
			{
				case WTYPE_DIALOG:
				case WTYPE_VMENU:
				case WTYPE_COMBOBOX:
					NewDialogData(L, Info, (HANDLE)wi.Id, FALSE);
					lua_setfield(L, -2, "Id");
					break;

				default:
					PutIntToTable(L, "Id", (int)wi.Id);
					break;
			}

			PutIntToTable(L, "Pos", wi.Pos + 1);
			PutIntToTable(L, "Type", wi.Type);
			PutFlagsToTable(L, "Flags", wi.Flags);
			PutWStrToTable(L, "TypeName", wi.TypeName, -1);
			PutWStrToTable(L, "Name", wi.Name, -1);
			return 1;
		}

		case ACTL_SETARRAYCOLOR:
		{
			struct FarSetColors fsc = { sizeof(fsc) };
			luaL_checktype(L, pos3, LUA_TTABLE);
			fsc.StartIndex = GetOptIntFromTable(L, "StartIndex", 0);
			lua_getfield(L, pos3, "Flags");
			fsc.Flags = GetFlagCombination(L, -1, NULL);
			fsc.ColorsCount = lua_objlen(L, pos3);
			size_t size = fsc.ColorsCount * sizeof(struct FarColor);
			fsc.Colors = (struct FarColor*) lua_newuserdata(L, size);
			memset(fsc.Colors, 0, size);

			for (int i=0; i < (int)fsc.ColorsCount; i++)
			{
				lua_rawgeti(L, pos3, i+1);
				GetFarColor(L, -1, &fsc.Colors[i]);
				lua_pop(L,1);
			}

			lua_pushinteger(L, Info->AdvControl(PluginId, Command, Param1, &fsc));
			return 1;
		}

		case ACTL_GETFARRECT:
		{
			SMALL_RECT sr;
			if (Info->AdvControl(PluginId, Command, 0, &sr))
			{
				lua_createtable(L, 0, 4);
				PutIntToTable(L, "Left",   sr.Left);
				PutIntToTable(L, "Top",    sr.Top);
				PutIntToTable(L, "Right",  sr.Right);
				PutIntToTable(L, "Bottom", sr.Bottom);
			}
			else
				lua_pushnil(L);

			return 1;
		}

		case ACTL_GETCURSORPOS:
		{
			COORD coord;
			if (Info->AdvControl(PluginId, Command, 0, &coord))
			{
				lua_createtable(L, 0, 2);
				PutIntToTable(L, "X", coord.X);
				PutIntToTable(L, "Y", coord.Y);
			}
			else
				lua_pushnil(L);

			return 1;
		}

		case ACTL_SETCURSORPOS:
		{
			COORD coord;
			luaL_checktype(L, pos3, LUA_TTABLE);
			lua_getfield(L, pos3, "X");
			coord.X = (SHORT) lua_tointeger(L, -1);
			lua_getfield(L, pos3, "Y");
			coord.Y = (SHORT) lua_tointeger(L, -1);
			lua_pushinteger(L, Info->AdvControl(PluginId, Command, Param1, &coord));
			return 1;
		}

		case ACTL_GETWINDOWTYPE:
		{
			struct WindowType wt = { sizeof(wt) };

			if (Info->AdvControl(PluginId, Command, 0, &wt))
			{
				lua_createtable(L, 0, 1);
				lua_pushinteger(L, wt.Type);
				lua_setfield(L, -2, "Type");
			}
			else lua_pushnil(L);

			return 1;
		}
	}

	lua_pushinteger(L, Info->AdvControl(PluginId, Command, Param1, Param2));
	return 1;
}

#define AdvCommand(name,command) \
static int adv_##name(lua_State *L) { return DoAdvControl(L,command,1); }

static int far_AdvControl(lua_State *L) { return DoAdvControl(L,0,0); }

AdvCommand( Commit,                 ACTL_COMMIT)
AdvCommand( GetArrayColor,          ACTL_GETARRAYCOLOR)
AdvCommand( GetColor,               ACTL_GETCOLOR)
AdvCommand( GetCursorPos,           ACTL_GETCURSORPOS)
AdvCommand( GetFarHwnd,             ACTL_GETFARHWND)
AdvCommand( GetFarManagerVersion,   ACTL_GETFARMANAGERVERSION)
AdvCommand( GetFarRect,             ACTL_GETFARRECT)
AdvCommand( GetWindowCount,         ACTL_GETWINDOWCOUNT)
AdvCommand( GetWindowInfo,          ACTL_GETWINDOWINFO)
AdvCommand( GetWindowType,          ACTL_GETWINDOWTYPE)
AdvCommand( ProgressNotify,         ACTL_PROGRESSNOTIFY)
AdvCommand( Quit,                   ACTL_QUIT)
AdvCommand( RedrawAll,              ACTL_REDRAWALL)
AdvCommand( SetArrayColor,          ACTL_SETARRAYCOLOR)
AdvCommand( SetCurrentWindow,       ACTL_SETCURRENTWINDOW)
AdvCommand( SetCursorPos,           ACTL_SETCURSORPOS)
AdvCommand( SetProgressState,       ACTL_SETPROGRESSSTATE)
AdvCommand( SetProgressValue,       ACTL_SETPROGRESSVALUE)
AdvCommand( Synchro,                ACTL_SYNCHRO)
AdvCommand( WaitKey,                ACTL_WAITKEY)

static const luaL_Reg far_funcs[] =
{
	PAIR( far, AdvControl),

	{NULL, NULL},
};

static const luaL_Reg actl_funcs[] =
{
	PAIR( adv, Commit),
	PAIR( adv, GetArrayColor),
	PAIR( adv, GetColor),
	PAIR( adv, GetCursorPos),
	PAIR( adv, GetFarHwnd),
	PAIR( adv, GetFarManagerVersion),
	PAIR( adv, GetFarRect),
	PAIR( adv, GetWindowCount),
	PAIR( adv, GetWindowInfo),
	PAIR( adv, GetWindowType),
	PAIR( adv, ProgressNotify),
	PAIR( adv, Quit),
	PAIR( adv, RedrawAll),
	PAIR( adv, SetArrayColor),
	PAIR( adv, SetCurrentWindow),
	PAIR( adv, SetCursorPos),
	PAIR( adv, SetProgressState),
	PAIR( adv, SetProgressValue),
	PAIR( adv, Synchro),
	PAIR( adv, WaitKey),

	{NULL, NULL},
};

int luaopen_actl(lua_State *L)
{
	luaL_register(L, "far", far_funcs);
	luaL_register(L, "actl", actl_funcs);
	return 0;
}
