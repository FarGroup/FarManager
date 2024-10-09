//---------------------------------------------------------------------------

#include <windows.h>
#include <rpc.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include "version.h"
#include "luafar.h"
#include "util.h"
#include "ustring.h"

#ifndef LUADLL
# if LUA_VERSION_NUM == 501
#  define LUADLL "lua51.dll"
# elif LUA_VERSION_NUM == 502
#  define LUADLL "lua52.dll"
# endif
#endif

typedef struct PluginStartupInfo PSInfo;

extern int bit64_push(lua_State *L, INT64 v);
extern int bit64_pushuserdata(lua_State *L, INT64 v);
extern int bit64_getvalue(lua_State *L, int pos, INT64 *target);

extern int luaopen_bit64(lua_State *L);
extern int luaopen_far_host(lua_State *L);
extern int luaopen_regex(lua_State*);
extern int luaopen_usercontrol(lua_State*);
extern int luaopen_uio(lua_State *L);
extern int luaopen_unicode(lua_State *L);
extern int luaopen_utf8(lua_State *L);
extern int luaopen_upackage(lua_State *L);
extern int luaopen_win(lua_State *L);
extern int luaopen_lpeg(lua_State *L);

extern int  luaB_dofileW(lua_State *L);
extern int  luaB_loadfileW(lua_State *L);
extern int  pcall_msg(lua_State* L, int narg, int nret);
extern void push_flags_table(lua_State *L);
extern void SetFarColors(lua_State *L);
extern void WINAPI FarPanelItemFreeCallback(void* UserData, const struct FarPanelItemFreeInfo* Info);
extern int far_MacroCallFar(lua_State *L);
extern int far_MacroCallToLua(lua_State *L);
extern void PackMacroValues(lua_State* L, size_t Count, const struct FarMacroValue* Values);
extern void PushFarMacroValue(lua_State* L, const struct FarMacroValue* val);
extern int GetExportFunction(lua_State* L, const char* FuncName);
extern BOOL RunDefaultScript(lua_State* L, int ForFirstTime);
extern void PushPluginObject(lua_State* L, HANDLE hPlugin);

const char FarFileFilterType[] = "FarFileFilter";
const char FarTimerType[]      = "FarTimer";
const char FarTimerQueueKey[]  = "FarTimerQueue";
const char FarDialogType[]     = "FarDialog";
const char SettingsType[]      = "FarSettings";
const char SettingsHandles[]   = "FarSettingsHandles";
const char PluginHandleType[]  = "FarPluginHandle";
const char AddMacroDataType[]  = "FarAddMacroData";
const char SavedScreenType[]   = "FarSavedScreen";

const char FAR_VIRTUALKEYS[]   = "far.virtualkeys";
const char FAR_FLAGSTABLE[]    = "far.Flags";
const char FAR_DN_STORAGE[]    = "FAR_DN_STORAGE";

static int InsideFarManager = 1;

const char* VirtualKeyStrings[256] =
{
	// 0x00
	NULL, "LBUTTON", "RBUTTON", "CANCEL",
	"MBUTTON", "XBUTTON1", "XBUTTON2", NULL,
	"BACK", "TAB", NULL, NULL,
	"CLEAR", "RETURN", NULL, NULL,
	// 0x10
	"SHIFT", "CONTROL", "MENU", "PAUSE",
	"CAPITAL", "KANA", NULL, "JUNJA",
	"FINAL", "HANJA", NULL, "ESCAPE",
	NULL, "NONCONVERT", "ACCEPT", "MODECHANGE",
	// 0x20
	"SPACE", "PRIOR", "NEXT", "END",
	"HOME", "LEFT", "UP", "RIGHT",
	"DOWN", "SELECT", "PRINT", "EXECUTE",
	"SNAPSHOT", "INSERT", "DELETE", "HELP",
	// 0x30
	"0", "1", "2", "3",
	"4", "5", "6", "7",
	"8", "9", NULL, NULL,
	NULL, NULL, NULL, NULL,
	// 0x40
	NULL, "A", "B", "C",
	"D", "E", "F", "G",
	"H", "I", "J", "K",
	"L", "M", "N", "O",
	// 0x50
	"P", "Q", "R", "S",
	"T", "U", "V", "W",
	"X", "Y", "Z", "LWIN",
	"RWIN", "APPS", NULL, "SLEEP",
	// 0x60
	"NUMPAD0", "NUMPAD1", "NUMPAD2", "NUMPAD3",
	"NUMPAD4", "NUMPAD5", "NUMPAD6", "NUMPAD7",
	"NUMPAD8", "NUMPAD9", "MULTIPLY", "ADD",
	"SEPARATOR", "SUBTRACT", "DECIMAL", "DIVIDE",
	// 0x70
	"F1", "F2", "F3", "F4",
	"F5", "F6", "F7", "F8",
	"F9", "F10", "F11", "F12",
	"F13", "F14", "F15", "F16",
	// 0x80
	"F17", "F18", "F19", "F20",
	"F21", "F22", "F23", "F24",
	NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL,
	// 0x90
	"NUMLOCK", "SCROLL", "OEM_NEC_EQUAL", "OEM_FJ_MASSHOU",
	"OEM_FJ_TOUROKU", "OEM_FJ_LOYA", "OEM_FJ_ROYA", NULL,
	NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL,
	// 0xA0
	"LSHIFT", "RSHIFT", "LCONTROL", "RCONTROL",
	"LMENU", "RMENU", "BROWSER_BACK", "BROWSER_FORWARD",
	"BROWSER_REFRESH", "BROWSER_STOP", "BROWSER_SEARCH", "BROWSER_FAVORITES",
	"BROWSER_HOME", "VOLUME_MUTE", "VOLUME_DOWN", "VOLUME_UP",
	// 0xB0
	"MEDIA_NEXT_TRACK", "MEDIA_PREV_TRACK", "MEDIA_STOP", "MEDIA_PLAY_PAUSE",
	"LAUNCH_MAIL", "LAUNCH_MEDIA_SELECT", "LAUNCH_APP1", "LAUNCH_APP2",
	NULL, NULL, "OEM_1", "OEM_PLUS",
	"OEM_COMMA", "OEM_MINUS", "OEM_PERIOD", "OEM_2",
	// 0xC0
	"OEM_3", NULL, NULL, NULL,
	NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL,
	// 0xD0
	NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, "OEM_4",
	"OEM_5", "OEM_6", "OEM_7", "OEM_8",
	// 0xE0
	NULL, NULL, "OEM_102", NULL,
	NULL, "PROCESSKEY", NULL, "PACKET",
	NULL, "OEM_RESET", "OEM_JUMP", "OEM_PA1",
	"OEM_PA2", "OEM_PA3", "OEM_WSCTRL", NULL,
	// 0xF0
	NULL, NULL, NULL, NULL,
	NULL, NULL, "ATTN", "CRSEL",
	"EXSEL", "EREOF", "PLAY", "ZOOM",
	"NONAME", "PA1", "OEM_CLEAR", NULL,
};

static lua_CFunction luaopen_bit = NULL;
static lua_CFunction luaopen_ffi = NULL;
static lua_CFunction luaopen_jit = NULL;
int IsLuaJIT(void) { return luaopen_jit != NULL; }

BOOL WINAPI DllMain(HANDLE hDll, DWORD dwReason, LPVOID lpReserved)
{
	(void) lpReserved;
	if (DLL_PROCESS_ATTACH == dwReason && hDll)
	{
		// Try to load LuaJIT 2.0 libraries. This is done dynamically to ensure that
		// LuaFAR works with either Lua 5.1 or LuaJIT 2.0
		HMODULE hLib = GetModuleHandleA(LUADLL);
		if (hLib)
		{
			luaopen_bit = (lua_CFunction)(intptr_t)GetProcAddress(hLib, "luaopen_bit");
			luaopen_ffi = (lua_CFunction)(intptr_t)GetProcAddress(hLib, "luaopen_ffi");
			luaopen_jit = (lua_CFunction)(intptr_t)GetProcAddress(hLib, "luaopen_jit");
		}
	}
	return TRUE;
}

HANDLE GetLuaStateTimerQueue(lua_State *L)
{
	HANDLE hQueue;
	lua_getfield(L, LUA_REGISTRYINDEX, FarTimerQueueKey);
	hQueue = lua_touserdata(L, -1);
	lua_pop(L, 1);
	return hQueue;
}

void DeleteLuaStateTimerQueue(lua_State *L)
{
	lua_pushnil(L);
	lua_setfield(L, LUA_REGISTRYINDEX, FarTimerQueueKey);
}

static TSynchroData* CreateSynchroData(TTimerData *td, int action, int data)
{
	TSynchroData* SD = (TSynchroData*) malloc(sizeof(TSynchroData));
	SD->timerData = td;
	SD->regAction = action;
	SD->data = data;
	return SD;
}

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

static UINT64 get_env_flag(lua_State *L, int pos, int *success)
{
	int dummy;
	const char *str;
	INT64 ret = 0;

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
			ret = (__int64)lua_tonumber(L, pos); // IMPORTANT: cast to signed integer.
			break;

		case LUA_TSTRING:
			str = lua_tostring(L, pos);
			lua_getfield(L, LUA_REGISTRYINDEX, FAR_FLAGSTABLE);
			lua_getfield(L, -1, str);
			if (lua_type(L, -1) == LUA_TNUMBER)
				ret = (__int64)lua_tonumber(L, -1); // IMPORTANT: cast to signed integer.
			else if (!bit64_getvalue(L, -1, &ret))
				*success = FALSE;
			lua_pop(L, 2);
			break;

		default:
			if (!bit64_getvalue(L, pos, &ret))
				*success = FALSE;
			break;
	}

	return ret;
}

static UINT64 check_env_flag(lua_State *L, int pos)
{
	int success = FALSE;
	UINT64 ret = lua_isnoneornil(L, pos) ? 0 : get_env_flag(L, pos, &success);

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

UINT64 GetFlagCombination(lua_State *L, int pos, int *success)
{
	UINT64 ret = 0;
	UINT64 flag;
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
				flag = get_env_flag(L, -2, success);

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
			flag = get_env_flag(L, -1, &ok);
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

static UINT64 CheckFlags(lua_State* L, int pos)
{
	int success = FALSE;
	UINT64 Flags = lua_isnoneornil(L, pos) ? 0 : GetFlagCombination(L, pos, &success);

	if (!success)
		luaL_error(L, "invalid flag combination");

	return Flags;
}

UINT64 OptFlags(lua_State* L, int pos, UINT64 dflt)
{
	return lua_isnoneornil(L, pos) ? dflt : CheckFlags(L, pos);
}

static UINT64 CheckFlagsFromTable(lua_State *L, int pos, const char* key)
{
	UINT64 f = 0;
	lua_getfield(L, pos, key);
	if (!lua_isnil(L, -1))
		f = CheckFlags(L, -1);
	lua_pop(L, 1);
	return f;
}

UINT64 GetFlagsFromTable(lua_State *L, int pos, const char* key)
{
	UINT64 f;
	lua_getfield(L, pos, key);
	f = GetFlagCombination(L, -1, NULL);
	lua_pop(L, 1);
	return f;
}

void PutFlagsToTable(lua_State *L, const char* key, UINT64 flags)
{
	bit64_push(L, flags);
	lua_setfield(L, -2, key);
}

void PutFlagsToArray(lua_State *L, int index, UINT64 flags)
{
	bit64_push(L, flags);
	lua_rawseti(L, -2, index);
}

TPluginData* GetPluginData(lua_State* L)
{
	static TPluginData FakePluginData;
	TPluginData *pd;

	if (InsideFarManager)
		(void) lua_getallocf(L, (void**)&pd);
	else
	{
		// There is no Far Manager here and no plugin data but some functions
		// need TPluginData::Flags to handle file time resolution.
		pd = &FakePluginData;
	}
	return pd;
}

static void PushPluginHandle(lua_State *L, HANDLE Handle)
{
	if (Handle)
	{
		HANDLE *p = (HANDLE*)lua_newuserdata(L, sizeof(HANDLE));
		*p = Handle;
		luaL_getmetatable(L, PluginHandleType);
		lua_setmetatable(L, -2);
	}
	else
		lua_pushnil(L);
}

static int PluginHandle_rawhandle(lua_State *L)
{
	void* Handle = *(void**)luaL_checkudata(L, 1, PluginHandleType);
	lua_pushlightuserdata(L, Handle);
	return 1;
}

void ConvertLuaValue (lua_State *L, int pos, struct FarMacroValue *target)
{
	INT64 val64;
	int type = lua_type(L, pos);
	pos = abs_index(L, pos);
	target->Type = FMVT_UNKNOWN;

	if (type == LUA_TNUMBER)
	{
		target->Type = FMVT_DOUBLE;
		target->Value.Double = lua_tonumber(L, pos);
	}
	else if (type == LUA_TSTRING)
	{
		target->Type = FMVT_STRING;
		target->Value.String = check_utf8_string(L, pos, NULL);
	}
	else if (type == LUA_TTABLE)
	{
		lua_rawgeti(L,pos,1);
		if (lua_type(L,-1) == LUA_TSTRING)
		{
			target->Type = FMVT_BINARY;
			target->Value.Binary.Data = (void*)lua_tolstring(L, -1, &target->Value.Binary.Size);
		}
		lua_pop(L,1);
	}
	else if (type == LUA_TBOOLEAN)
	{
		target->Type = FMVT_BOOLEAN;
		target->Value.Boolean = lua_toboolean(L, pos);
	}
	else if (type == LUA_TNIL)
	{
		target->Type = FMVT_NIL;
	}
	else if (type == LUA_TLIGHTUSERDATA)
	{
		target->Type = FMVT_POINTER;
		target->Value.Pointer = lua_touserdata(L, pos);
	}
	else if (bit64_getvalue(L, pos, &val64))
	{
		target->Type = FMVT_INTEGER;
		target->Value.Integer = val64;
	}
}

static int far_GetFileOwner(lua_State *L)
{
	wchar_t Owner[512];
	const wchar_t *Computer = opt_utf8_string(L, 1, NULL);
	const wchar_t *Name = check_utf8_string(L, 2, NULL);

	if (GetPluginData(L)->FSF->GetFileOwner(Computer, Name, Owner, ARRSIZE(Owner)))
		push_utf8_string(L, Owner, -1);
	else
		lua_pushnil(L);

	return 1;
}

static int far_GetNumberOfLinks(lua_State *L)
{
	const wchar_t *Name = check_utf8_string(L, 1, NULL);
	int num = (int)GetPluginData(L)->FSF->GetNumberOfLinks(Name);
	return lua_pushinteger(L, num), 1;
}

static int far_GetLuafarVersion(lua_State *L)
{
	if (lua_toboolean(L, 1))
	{
		lua_pushinteger(L, 3);
		lua_pushinteger(L, 0);
		lua_pushinteger(L, 0);
		lua_pushinteger(L, PLUGIN_BUILD);
		return 4;
	}

	lua_pushfstring(L, "3.0.0.%d", (int)PLUGIN_BUILD);
	return 1;
}

static void GetMouseEvent(lua_State *L, MOUSE_EVENT_RECORD* rec)
{
	rec->dwMousePosition.X = GetOptIntFromTable(L, "MousePositionX", 0);
	rec->dwMousePosition.Y = GetOptIntFromTable(L, "MousePositionY", 0);
	rec->dwButtonState = GetOptIntFromTable(L, "ButtonState", 0);
	rec->dwControlKeyState = GetOptIntFromTable(L, "ControlKeyState", 0);
	rec->dwEventFlags = GetOptIntFromTable(L, "EventFlags", 0);
}

void PutMouseEvent(lua_State *L, const MOUSE_EVENT_RECORD* rec, BOOL table_exist)
{
	if (!table_exist)
		lua_createtable(L, 0, 5);

	PutNumToTable(L, "MousePositionX", rec->dwMousePosition.X);
	PutNumToTable(L, "MousePositionY", rec->dwMousePosition.Y);
	PutNumToTable(L, "ButtonState", rec->dwButtonState);
	PutNumToTable(L, "ControlKeyState", rec->dwControlKeyState);
	PutNumToTable(L, "EventFlags", rec->dwEventFlags);
}

// convert a string from utf-8 to wide char and put it into a table,
// to prevent stack overflow and garbage collection
static const wchar_t* StoreTempString(lua_State *L, int store_stack_pos)
{
	const wchar_t *s = check_utf8_string(L,-1,NULL);
	luaL_ref(L, store_stack_pos);
	return s;
}

static void PushEditorSetPosition(lua_State *L, const struct EditorSetPosition *esp)
{
	lua_createtable(L, 0, 6);
	PutIntToTable(L, "CurLine",       esp->CurLine + 1);
	PutIntToTable(L, "CurPos",        esp->CurPos + 1);
	PutIntToTable(L, "CurTabPos",     esp->CurTabPos + 1);
	PutIntToTable(L, "TopScreenLine", esp->TopScreenLine + 1);
	PutIntToTable(L, "LeftPos",       esp->LeftPos + 1);
	PutIntToTable(L, "Overtype",      esp->Overtype);
}

static void FillEditorSetPosition(lua_State *L, struct EditorSetPosition *esp)
{
	esp->CurLine   = GetOptIntFromTable(L, "CurLine", 0) - 1;
	esp->CurPos    = GetOptIntFromTable(L, "CurPos", 0) - 1;
	esp->CurTabPos = GetOptIntFromTable(L, "CurTabPos", 0) - 1;
	esp->TopScreenLine = GetOptIntFromTable(L, "TopScreenLine", 0) - 1;
	esp->LeftPos   = GetOptIntFromTable(L, "LeftPos", 0) - 1;
	esp->Overtype  = GetOptIntFromTable(L, "Overtype", -1);
}

void PushPanelItem(lua_State *L, const struct PluginPanelItem *PanelItem, int NoUserData)
{
	lua_createtable(L, 0, 16); // "PanelItem"
	//-----------------------------------------------------------------------
	PutFileTimeToTable (L, "CreationTime",      PanelItem->CreationTime);
	PutFileTimeToTable (L, "LastAccessTime",    PanelItem->LastAccessTime);
	PutFileTimeToTable (L, "LastWriteTime",     PanelItem->LastWriteTime);
	PutFileTimeToTable (L, "ChangeTime",        PanelItem->ChangeTime);
	PutNumToTable      (L, "FileSize",          (double)PanelItem->FileSize);
	PutNumToTable      (L, "AllocationSize",    (double)PanelItem->AllocationSize);
	PutWStrToTable     (L, "FileName",          PanelItem->FileName, -1);
	PutWStrToTable     (L, "AlternateFileName", PanelItem->AlternateFileName, -1);
	PutFlagsToTable    (L, "Flags",             PanelItem->Flags);
	PutNumToTable      (L, "NumberOfLinks",     (double)PanelItem->NumberOfLinks);
	PutNumToTable      (L, "CRC32",             (double)PanelItem->CRC32);

	PutAttrToTable(L, (int)PanelItem->FileAttributes);

	if (PanelItem->Description)
		PutWStrToTable(L, "Description", PanelItem->Description, -1);

	if (PanelItem->Owner)
		PutWStrToTable(L, "Owner", PanelItem->Owner, -1);

	/* not clear why custom columns are defined on per-file basis */
	if (PanelItem->CustomColumnNumber > 0)
	{
		int j;
		lua_createtable(L, (int)PanelItem->CustomColumnNumber, 0);

		for(j=0; j < (int)PanelItem->CustomColumnNumber; j++)
			PutWStrToArray(L, j+1, PanelItem->CustomColumnData[j], -1);

		lua_setfield(L, -2, "CustomColumnData");
	}

	if (PanelItem->UserData.Data)
	{
		if (!NoUserData)
		{
			if (PanelItem->UserData.FreeData==FarPanelItemFreeCallback)
			{
				// This is a panel of a LuaFAR plugin
				FarPanelItemUserData* ud = (FarPanelItemUserData*)PanelItem->UserData.Data;

				// Compare registries rather than Lua states to allow for different coroutines of the same state
				if (lua_topointer(ud->L, LUA_REGISTRYINDEX) == lua_topointer(L, LUA_REGISTRYINDEX))
				{
					lua_rawgeti(L, LUA_REGISTRYINDEX, ud->ref);
					lua_setfield(L, -2, "UserData");
				}
			}
		}
		else
		{
			lua_pushlightuserdata(L, PanelItem->UserData.Data);
			lua_setfield(L, -2, "ExtUserData"); //use field name different from "UserData" to distinguish later
			lua_pushlightuserdata(L, (void*)(intptr_t)PanelItem->UserData.FreeData);
			lua_setfield(L, -2, "FreeUserData");
		}
	}
}
//---------------------------------------------------------------------------

void PushPanelItems(lua_State *L, const struct PluginPanelItem *PanelItems, size_t ItemsNumber, int NoUserData)
{
	int i;
	lua_createtable(L, (int)ItemsNumber, 0); // "PanelItems"

	for(i=0; i < (int)ItemsNumber; i++)
	{
		PushPanelItem(L, PanelItems + i, NoUserData);
		lua_rawseti(L, -2, i+1);
	}
}
//---------------------------------------------------------------------------

static int far_PluginStartupInfo(lua_State *L)
{
	const wchar_t *p;
	intptr_t len=0;
	TPluginData *pd = GetPluginData(L);
	lua_createtable(L, 0, 4);
	PutWStrToTable(L, "ModuleName", pd->Info->ModuleName, -1);

	for(p=pd->Info->ModuleName; *p; p++)
	{
		if (*p==L'\\') len = p - pd->Info->ModuleName;
	}

	PutWStrToTable(L, "ModuleDir", pd->Info->ModuleName, len+1);
	PutLStrToTable(L, "PluginGuid", pd->PluginId, sizeof(GUID));

	return 1;
}

static int far_GetCurrentDirectory(lua_State *L)
{
	struct FarStandardFunctions *FSF = GetPluginData(L)->FSF;
	size_t size = FSF->GetCurrentDirectory(0, NULL);
	wchar_t* buf = (wchar_t*)lua_newuserdata(L, size * sizeof(wchar_t));
	FSF->GetCurrentDirectory(size, buf);
	push_utf8_string(L, buf, -1);
	return 1;
}

static int push_ev_filename(lua_State *L, int isEditor, intptr_t Id)
{
	wchar_t* fname;
	size_t size;
	PSInfo *Info = GetPluginData(L)->Info;
	size = isEditor ?
	       Info->EditorControl(Id, ECTL_GETFILENAME, 0, 0) :
	       Info->ViewerControl(Id, VCTL_GETFILENAME, 0, 0);

	if (!size) return 0;

	fname = (wchar_t*)lua_newuserdata(L, size * sizeof(wchar_t));
	size = isEditor ?
	       Info->EditorControl(Id, ECTL_GETFILENAME, size, fname) :
	       Info->ViewerControl(Id, VCTL_GETFILENAME, size, fname);

	if (size)
	{
		push_utf8_string(L, fname, -1);
		lua_remove(L, -2);
		return 1;
	}

	lua_pop(L,1);
	return 0;
}

static int editor_GetFileName(lua_State *L)
{
	intptr_t EditorId = luaL_optinteger(L, 1, CURRENT_EDITOR);

	if (!push_ev_filename(L, 1, EditorId)) lua_pushnil(L);

	return 1;
}

static int editor_GetInfo(lua_State *L)
{
	intptr_t EditorId = luaL_optinteger(L, 1, CURRENT_EDITOR);
	PSInfo *Info = GetPluginData(L)->Info;
	struct EditorInfo ei;
	ei.StructSize = sizeof(ei);

	if (!Info->EditorControl(EditorId, ECTL_GETINFO, 0, &ei))
		return lua_pushnil(L), 1;

	lua_createtable(L, 0, 18);
	PutNumToTable(L, "EditorID", (double)ei.EditorID);

	if (push_ev_filename(L, 1, EditorId))
		lua_setfield(L, -2, "FileName");

	PutNumToTable(L, "WindowSizeX", (double) ei.WindowSizeX);
	PutNumToTable(L, "WindowSizeY", (double) ei.WindowSizeY);
	PutNumToTable(L, "TotalLines", (double) ei.TotalLines);
	PutNumToTable(L, "CurLine", (double) ei.CurLine + 1);
	PutNumToTable(L, "CurPos", (double) ei.CurPos + 1);
	PutNumToTable(L, "CurTabPos", (double) ei.CurTabPos + 1);
	PutNumToTable(L, "TopScreenLine", (double) ei.TopScreenLine + 1);
	PutNumToTable(L, "LeftPos", (double) ei.LeftPos + 1);
	PutNumToTable(L, "Overtype", (double) ei.Overtype);
	PutNumToTable(L, "BlockType", (double) ei.BlockType);
	PutNumToTable(L, "BlockStartLine", (double) ei.BlockStartLine + 1);
	PutNumToTable(L, "Options", (double) ei.Options);
	PutNumToTable(L, "TabSize", (double) ei.TabSize);
	PutNumToTable(L, "BookmarkCount", (double) ei.BookmarkCount);
	PutNumToTable(L, "SessionBookmarkCount", (double) ei.SessionBookmarkCount);
	PutNumToTable(L, "CurState", (double) ei.CurState);
	PutNumToTable(L, "CodePage", (double) ei.CodePage);
	return 1;
}

/* t-rex:
 * Для тех кому плохо доходит описываю:
 * Редактор в фаре это двух связный список, указатель на текущюю строку
 * изменяется только при ECTL_SETPOSITION, при использовании любой другой
 * ECTL_* для которой нужно задавать номер строки если этот номер не -1
 * (т.е. текущаая строка) то фар должен найти эту строку в списке (а это
 * занимает дофига времени), поэтому если надо делать несколько ECTL_*
 * (тем более когда они делаются на последовательность строк
 * i,i+1,i+2,...) то перед каждым ECTL_* надо делать ECTL_SETPOSITION а
 * сами ECTL_* вызывать с -1.
 */
static BOOL FastGetString(intptr_t EditorId, intptr_t string_num,
                          struct EditorGetString *egs, PSInfo *Info)
{
	struct EditorSetPosition esp;
	esp.StructSize = sizeof(esp);
	esp.CurLine   = string_num;
	esp.CurPos    = -1;
	esp.CurTabPos = -1;
	esp.TopScreenLine = -1;
	esp.LeftPos   = -1;
	esp.Overtype  = -1;

	if (!Info->EditorControl(EditorId, ECTL_SETPOSITION, 0, &esp))
		return FALSE;

	egs->StringNumber = string_num;
	return Info->EditorControl(EditorId, ECTL_GETSTRING, 0, egs) != 0;
}

// EditorGetString (EditorId, line_num, [mode])
//
//   line_num:  number of line in the Editor, a 1-based integer.
//
//   mode:      0 = returns: table LineInfo;        changes current position: no
//              1 = returns: table LineInfo;        changes current position: yes
//              2 = returns: StringText,StringEOL;  changes current position: yes
//              3 = returns: StringText,StringEOL;  changes current position: no
//
//   return:    either table LineInfo or StringText,StringEOL - depending on `mode` argument.
//
static int _EditorGetString(lua_State *L, int is_wide)
{
	intptr_t EditorId = luaL_optinteger(L, 1, CURRENT_EDITOR);
	PSInfo *Info = GetPluginData(L)->Info;
	intptr_t line_num = luaL_optinteger(L, 2, 0) - 1;
	intptr_t mode = luaL_optinteger(L, 3, 0);
	BOOL res = 0;
	struct EditorGetString egs = {0,0,0,NULL,NULL,0,0};
	egs.StructSize = sizeof(egs);

	if (mode == 0 || mode == 3 || mode == 4)
	{
		egs.StringNumber = line_num;
		res = Info->EditorControl(EditorId, ECTL_GETSTRING, 0, &egs) != 0;
	}
	else if (mode == 1 || mode == 2)
		res = FastGetString(EditorId, line_num, &egs, Info);

	if (res)
	{
		if (mode == 2 || mode == 3)
		{
			if (is_wide)
			{
				push_utf16_string(L, egs.StringText, egs.StringLength);
				push_utf16_string(L, egs.StringEOL, -1);
			}
			else
			{
				push_utf8_string(L, egs.StringText, egs.StringLength);
				push_utf8_string(L, egs.StringEOL, -1);
			}

			return 2;
		}
		else if (mode == 4)
		{
			lua_pushinteger(L, egs.SelStart+1);
			lua_pushinteger(L, egs.SelEnd);
			lua_pushinteger(L, egs.StringLength);
			return 3;
		}
		else
		{
			lua_createtable(L, 0, 6);
			PutNumToTable(L, "StringNumber", (double)egs.StringNumber+1);
			PutNumToTable(L, "StringLength", (double)egs.StringLength);
			PutNumToTable(L, "SelStart", (double)egs.SelStart+1);
			PutNumToTable(L, "SelEnd", (double)egs.SelEnd);

			if (is_wide)
			{
				push_utf16_string(L, egs.StringText, egs.StringLength);
				lua_setfield(L, -2, "StringText");
				push_utf16_string(L, egs.StringEOL, -1);
				lua_setfield(L, -2, "StringEOL");
			}
			else
			{
				PutWStrToTable(L, "StringText",  egs.StringText, egs.StringLength);
				PutWStrToTable(L, "StringEOL",   egs.StringEOL, -1);
			}
		}

		return 1;
	}

	return lua_pushnil(L), 1;
}

static int editor_GetString(lua_State *L) { return _EditorGetString(L, 0); }
static int editor_GetStringW(lua_State *L) { return _EditorGetString(L, 1); }

static int _EditorSetString(lua_State *L, int is_wide)
{
	PSInfo *Info = GetPluginData(L)->Info;
	struct EditorSetString ess;
	intptr_t EditorId = luaL_optinteger(L, 1, CURRENT_EDITOR);
	size_t len;
	ess.StructSize = sizeof(ess);
	ess.StringNumber = luaL_optinteger(L, 2, 0) - 1;

	if (is_wide)
	{
		ess.StringText = check_utf16_string(L, 3, &len);
		ess.StringEOL = opt_utf16_string(L, 4, NULL);

		if (ess.StringEOL)
		{
			lua_pushvalue(L, 4);
			lua_pushliteral(L, "\0\0");
			lua_concat(L, 2);
			ess.StringEOL = (wchar_t*) lua_tostring(L, -1);
		}
	}
	else
	{
		ess.StringText = check_utf8_string(L, 3, &len);
		ess.StringEOL = opt_utf8_string(L, 4, NULL);
	}

	ess.StringLength = len;
	lua_pushboolean(L, Info->EditorControl(EditorId, ECTL_SETSTRING, 0, &ess) != 0);
	return 1;
}

static int editor_SetString(lua_State *L) { return _EditorSetString(L, 0); }
static int editor_SetStringW(lua_State *L) { return _EditorSetString(L, 1); }

static int editor_InsertString(lua_State *L)
{
	PSInfo *Info = GetPluginData(L)->Info;
	intptr_t EditorId = luaL_optinteger(L, 1, CURRENT_EDITOR);
	int indent = lua_toboolean(L, 2);
	lua_pushboolean(L, Info->EditorControl(EditorId, ECTL_INSERTSTRING, 0, &indent) != 0);
	return 1;
}

static int editor_DeleteString(lua_State *L)
{
	PSInfo *Info = GetPluginData(L)->Info;
	intptr_t EditorId = luaL_optinteger(L, 1, CURRENT_EDITOR);
	lua_pushboolean(L, Info->EditorControl(EditorId, ECTL_DELETESTRING, 0, 0) != 0);
	return 1;
}

static int _EditorInsertText(lua_State *L, int is_wide)
{
	intptr_t EditorId = luaL_optinteger(L, 1, CURRENT_EDITOR);
	PSInfo *Info = GetPluginData(L)->Info;
	const wchar_t* text;
	if (is_wide)
	{
		size_t len;
		const char *s = luaL_checklstring(L,2,&len);
		int needZero = 0;
		if (len % sizeof(wchar_t))
		{
			if (s[len-1] && --len)
				needZero = 1;
		}
		else
			needZero = len && (s[len-2] || s[len-1]);

		if (needZero)
		{
			lua_pushlstring(L, s, len);
			lua_pushlstring(L, "\0", 1);
			lua_concat(L, 2);
			text = (const wchar_t*)lua_tostring(L, -1);
		}
		else
			text = len ? (const wchar_t*)s : L"";
	}
	else
	{
		text = check_utf8_string(L,2,NULL);
	}
	lua_pushboolean(L, Info->EditorControl(EditorId, ECTL_INSERTTEXT, 0, (void*)text) != 0);
	return 1;
}

static int editor_InsertText(lua_State *L) { return _EditorInsertText(L, 0); }
static int editor_InsertTextW(lua_State *L) { return _EditorInsertText(L, 1); }

static int editor_DeleteChar(lua_State *L)
{
	intptr_t EditorId = luaL_optinteger(L, 1, CURRENT_EDITOR);
	PSInfo *Info = GetPluginData(L)->Info;
	lua_pushboolean(L, Info->EditorControl(EditorId, ECTL_DELETECHAR, 0, 0) != 0);
	return 1;
}

static int editor_DeleteBlock(lua_State *L)
{
	intptr_t EditorId = luaL_optinteger(L, 1, CURRENT_EDITOR);
	PSInfo *Info = GetPluginData(L)->Info;
	lua_pushboolean(L, Info->EditorControl(EditorId, ECTL_DELETEBLOCK, 0, 0) != 0);
	return 1;
}

static int editor_UndoRedo(lua_State *L)
{
	intptr_t EditorId = luaL_optinteger(L, 1, CURRENT_EDITOR);
	struct EditorUndoRedo eur;
	memset(&eur, 0, sizeof(eur));
	eur.StructSize = sizeof(eur);
	eur.Command = check_env_flag(L, 2);
	lua_pushboolean(L, GetPluginData(L)->Info->EditorControl(EditorId, ECTL_UNDOREDO, 0, &eur) != 0);
	return 1;
}

static void FillKeyBarTitles(lua_State *L, int src_pos, struct KeyBarTitles *kbt)
{
	int store_pos, i;
	size_t size;
	lua_newtable(L);
	store_pos = lua_gettop(L);
	//-------------------------------------------------------------------------
	memset(kbt, 0, sizeof(*kbt));
	kbt->CountLabels = lua_objlen(L, src_pos);
	size = kbt->CountLabels * sizeof(struct KeyBarLabel);
	kbt->Labels = (struct KeyBarLabel*)lua_newuserdata(L, size);
	memset(kbt->Labels, 0, size);

	for(i=0; i < (int)kbt->CountLabels; i++)
	{
		lua_rawgeti(L, src_pos, i+1);

		if (!lua_istable(L, -1))
		{
			kbt->CountLabels = i;
			lua_pop(L, 1);
			break;
		}

		kbt->Labels[i].Key.VirtualKeyCode = GetOptIntFromTable(L, "VirtualKeyCode", 0);
		kbt->Labels[i].Key.ControlKeyState = CAST(DWORD,CheckFlagsFromTable(L, -1, "ControlKeyState"));
		//-----------------------------------------------------------------------
		lua_getfield(L, -1, "Text");
		kbt->Labels[i].Text = StoreTempString(L, store_pos);
		//-----------------------------------------------------------------------
		lua_getfield(L, -1, "LongText");
		kbt->Labels[i].LongText = StoreTempString(L, store_pos);
		//-----------------------------------------------------------------------
		lua_pop(L, 1);
	}
}

static int SetKeyBar(lua_State *L, BOOL editor)
{
	intptr_t result;
	void* param = NULL;
	struct KeyBarTitles kbt;
	struct FarSetKeyBarTitles skbt;
	intptr_t Id = luaL_optinteger(L, 1, -1);
	PSInfo *Info = GetPluginData(L)->Info;
	enum { REDRAW=-1, RESTORE=0 }; // corresponds to FAR API
	BOOL argfail = FALSE;

	if (lua_isstring(L,2))
	{
		const char* p = lua_tostring(L,2);

		if (0 == strcmp("redraw", p)) param = (void*)REDRAW;
		else if (0 == strcmp("restore", p)) param = (void*)RESTORE;
		else argfail = TRUE;
	}
	else if (lua_istable(L,2))
	{
		param = &skbt;
		FillKeyBarTitles(L, 2, &kbt);
		skbt.StructSize = sizeof(skbt);
		skbt.Titles = &kbt;
	}
	else
		argfail = TRUE;

	if (argfail)
		return luaL_argerror(L, 2, "must be 'redraw', 'restore', or table");

	result = editor ? Info->EditorControl(Id, ECTL_SETKEYBAR, 0, param) :
	         Info->ViewerControl(Id, VCTL_SETKEYBAR, 0, param);
	lua_pushboolean(L, result != 0);
	return 1;
}

static int editor_SetKeyBar(lua_State *L)
{
	return SetKeyBar(L, TRUE);
}

static int viewer_SetKeyBar(lua_State *L)
{
	return SetKeyBar(L, FALSE);
}

static int editor_SetParam(lua_State *L)
{
	wchar_t buf[256];
	int tp;
	intptr_t result;
	intptr_t EditorId = luaL_optinteger(L, 1, CURRENT_EDITOR);
	PSInfo *Info = GetPluginData(L)->Info;
	struct EditorSetParameter esp;
	memset(&esp, 0, sizeof(esp));
	esp.StructSize = sizeof(esp);
	esp.Type = check_env_flag(L,2);
	//-----------------------------------------------------
	tp = lua_type(L,3);

	if (tp == LUA_TNUMBER)
		esp.Param.iParam = lua_tointeger(L,3);
	else if (tp == LUA_TBOOLEAN)
		esp.Param.iParam = lua_toboolean(L,3);
	else if (tp == LUA_TSTRING)
		esp.Param.wszParam = (wchar_t*)check_utf8_string(L,3,NULL);

	//-----------------------------------------------------
	if (esp.Type == ESPT_GETWORDDIV)
	{
		esp.Param.wszParam = buf;
		esp.Size = ARRSIZE(buf);
	}

	//-----------------------------------------------------
	esp.Flags = GetFlagCombination(L, 4, NULL);
	//-----------------------------------------------------
	result = Info->EditorControl(EditorId, ECTL_SETPARAM, 0, &esp);
	lua_pushboolean(L, result != 0);

	if (result && esp.Type == ESPT_GETWORDDIV)
	{
		push_utf8_string(L,buf,-1); return 2;
	}

	return 1;
}

static int editor_SetPosition(lua_State *L)
{
	intptr_t EditorId = luaL_optinteger(L, 1, CURRENT_EDITOR);
	PSInfo *Info = GetPluginData(L)->Info;
	struct EditorSetPosition esp;
	esp.StructSize = sizeof(esp);

	if (lua_istable(L, 2))
	{
		lua_settop(L, 2);
		FillEditorSetPosition(L, &esp);
	}
	else
	{
		esp.CurLine   = luaL_optinteger(L,2,0) - 1;
		esp.CurPos    = luaL_optinteger(L,3,0) - 1;
		esp.CurTabPos = luaL_optinteger(L,4,0) - 1;
		esp.TopScreenLine = luaL_optinteger(L,5,0) - 1;
		esp.LeftPos   = luaL_optinteger(L,6,0) - 1;
		esp.Overtype  = luaL_optinteger(L,7,-1);
	}

	lua_pushboolean(L, (int)Info->EditorControl(EditorId, ECTL_SETPOSITION, 0, &esp));
	return 1;
}

static int editor_Redraw(lua_State *L)
{
	intptr_t EditorId = luaL_optinteger(L, 1, CURRENT_EDITOR);
	PSInfo *Info = GetPluginData(L)->Info;
	lua_pushboolean(L, (int)Info->EditorControl(EditorId, ECTL_REDRAW, 0, 0));
	return 1;
}

static int editor_ExpandTabs(lua_State *L)
{
	intptr_t EditorId = luaL_optinteger(L, 1, CURRENT_EDITOR);
	PSInfo *Info = GetPluginData(L)->Info;
	intptr_t line_num = luaL_optinteger(L, 2, 0) - 1;
	lua_pushboolean(L, (int)Info->EditorControl(EditorId, ECTL_EXPANDTABS, 0, &line_num));
	return 1;
}

static int PushBookmarks(lua_State *L, int command)
{
	intptr_t EditorId = luaL_optinteger(L, 1, CURRENT_EDITOR);
	size_t size = GetPluginData(L)->Info->EditorControl(EditorId, command, 0, NULL);
	if (size)
	{
		struct EditorBookmarks *ebm = (struct EditorBookmarks*)lua_newuserdata(L, size);
		ebm->StructSize = sizeof(*ebm);
		ebm->Size = size;
		if (GetPluginData(L)->Info->EditorControl(EditorId, command, 0, ebm))
		{
			int i;
			lua_createtable(L, (int)ebm->Count, 0);
			for(i=0; i < (int)ebm->Count; i++)
			{
				lua_pushinteger(L, i+1);
				lua_createtable(L, 0, 4);
				PutNumToTable(L, "Line", (double) ebm->Line[i] + 1);
				PutNumToTable(L, "Cursor", (double) ebm->Cursor[i] + 1);
				PutNumToTable(L, "ScreenLine", (double) ebm->ScreenLine[i] + 1);
				PutNumToTable(L, "LeftPos", (double) ebm->LeftPos[i] + 1);
				lua_rawset(L, -3);
			}
			return 1;
		}
	}
	return lua_pushnil(L), 1;
}

static int editor_GetBookmarks(lua_State *L)
{
	return PushBookmarks(L, ECTL_GETBOOKMARKS);
}

static int editor_GetSessionBookmarks(lua_State *L)
{
	return PushBookmarks(L, ECTL_GETSESSIONBOOKMARKS);
}

static int editor_AddSessionBookmark(lua_State *L)
{
	intptr_t EditorId = luaL_optinteger(L, 1, CURRENT_EDITOR);
	PSInfo *Info = GetPluginData(L)->Info;
	lua_pushboolean(L, Info->EditorControl(EditorId, ECTL_ADDSESSIONBOOKMARK, 0, 0) != 0);
	return 1;
}

static int editor_ClearSessionBookmarks(lua_State *L)
{
	intptr_t EditorId = luaL_optinteger(L, 1, CURRENT_EDITOR);
	PSInfo *Info = GetPluginData(L)->Info;
	lua_pushboolean(L, Info->EditorControl(EditorId, ECTL_CLEARSESSIONBOOKMARKS, 0, 0) != 0);
	return 1;
}

static int editor_DeleteSessionBookmark(lua_State *L)
{
	intptr_t EditorId = luaL_optinteger(L, 1, CURRENT_EDITOR);
	intptr_t num = luaL_optinteger(L, 2, 0) - 1;
	PSInfo *Info = GetPluginData(L)->Info;
	lua_pushboolean(L, Info->EditorControl(EditorId, ECTL_DELETESESSIONBOOKMARK, 0, (void*)num) != 0);
	return 1;
}

static int editor_NextSessionBookmark(lua_State *L)
{
	intptr_t EditorId = luaL_optinteger(L, 1, CURRENT_EDITOR);
	PSInfo *Info = GetPluginData(L)->Info;
	lua_pushboolean(L, Info->EditorControl(EditorId, ECTL_NEXTSESSIONBOOKMARK, 0, 0) != 0);
	return 1;
}

static int editor_PrevSessionBookmark(lua_State *L)
{
	intptr_t EditorId = luaL_optinteger(L, 1, CURRENT_EDITOR);
	PSInfo *Info = GetPluginData(L)->Info;
	lua_pushboolean(L, Info->EditorControl(EditorId, ECTL_PREVSESSIONBOOKMARK, 0, 0) != 0);
	return 1;
}

static int editor_SetTitle(lua_State *L)
{
	intptr_t EditorId = luaL_optinteger(L, 1, CURRENT_EDITOR);
	PSInfo *Info = GetPluginData(L)->Info;
	const wchar_t* text = opt_utf8_string(L, 2, NULL);
	lua_pushboolean(L, (int)Info->EditorControl(EditorId, ECTL_SETTITLE, 0, (void*)text));
	return 1;
}

static int editor_GetTitle(lua_State *L)
{
	intptr_t EditorId = luaL_optinteger(L, 1, CURRENT_EDITOR);
	PSInfo *Info = GetPluginData(L)->Info;
	intptr_t size = Info->EditorControl(EditorId, ECTL_GETTITLE, 0, NULL);
	if (size)
	{
		wchar_t* buf = (wchar_t*)lua_newuserdata(L, size*sizeof(wchar_t));
		if (size == Info->EditorControl(EditorId, ECTL_GETTITLE, size, buf))
		{
			push_utf8_string(L, buf, -1);
			return 1;
		}
	}
	lua_pushnil(L);
	return 1;
}

static int editor_Quit(lua_State *L)
{
	intptr_t EditorId = luaL_optinteger(L, 1, CURRENT_EDITOR);
	PSInfo *Info = GetPluginData(L)->Info;
	lua_pushboolean(L, (int)Info->EditorControl(EditorId, ECTL_QUIT, 0, 0));
	return 1;
}

static int FillEditorSelect(lua_State *L, int pos_table, struct EditorSelect *es)
{
	int success;
	lua_getfield(L, pos_table, "BlockType");
	es->BlockType = CAST(int, get_env_flag(L, -1, &success));

	if (!success)
	{
		lua_pop(L,1);
		return 0;
	}

	lua_pushvalue(L, pos_table);
	es->BlockStartLine = GetOptIntFromTable(L, "BlockStartLine", 0) - 1;
	es->BlockStartPos  = GetOptIntFromTable(L, "BlockStartPos", 0) - 1;
	es->BlockWidth     = GetOptIntFromTable(L, "BlockWidth", -1);
	es->BlockHeight    = GetOptIntFromTable(L, "BlockHeight", -1);
	lua_pop(L,2);
	return 1;
}

static int editor_Select(lua_State *L)
{
	int success = TRUE;
	intptr_t EditorId = luaL_optinteger(L, 1, CURRENT_EDITOR);
	PSInfo *Info = GetPluginData(L)->Info;
	struct EditorSelect es;
	es.StructSize = sizeof(es);

	if (lua_istable(L, 2))
		success = FillEditorSelect(L, 2, &es);
	else
	{
		es.BlockType = CAST(int, check_env_flag(L, 2));
		es.BlockStartLine = luaL_optinteger(L, 3, 0) - 1;
		es.BlockStartPos  = luaL_optinteger(L, 4, 0) - 1;
		es.BlockWidth     = luaL_optinteger(L, 5, -1);
		es.BlockHeight    = luaL_optinteger(L, 6, -1);
	}

	lua_pushboolean(L, success && Info->EditorControl(EditorId, ECTL_SELECT, 0, &es));
	return 1;
}

// This function is that long because FAR API does not supply needed
// information directly.
static int editor_GetSelection(lua_State *L)
{
	intptr_t BlockStartPos, h, from, to;
	intptr_t EditorId = luaL_optinteger(L, 1, CURRENT_EDITOR);
	PSInfo *Info = GetPluginData(L)->Info;
	struct EditorInfo EI;
	struct EditorGetString egs;
	struct EditorSetPosition esp;
	EI.StructSize = sizeof(EI);
	egs.StructSize = sizeof(egs);
	esp.StructSize = sizeof(esp);
	Info->EditorControl(EditorId, ECTL_GETINFO, 0, &EI);

	if (EI.BlockType == BTYPE_NONE || !FastGetString(EditorId, EI.BlockStartLine, &egs, Info))
		return lua_pushnil(L), 1;

	lua_createtable(L, 0, 5);
	PutIntToTable(L, "BlockType", EI.BlockType);
	PutIntToTable(L, "StartLine", EI.BlockStartLine+1);
	BlockStartPos = egs.SelStart;
	PutIntToTable(L, "StartPos", BlockStartPos+1);
	// binary search for a non-block line
	h = 100; // arbitrary small number
	from = EI.BlockStartLine;

	for(to = from+h; to < EI.TotalLines; to = from + (h*=2))
	{
		if (!FastGetString(EditorId, to, &egs, Info))
			return lua_pushnil(L), 1;

		if (egs.SelStart < 0)
			break;
	}

	if (to >= EI.TotalLines)
		to = EI.TotalLines - 1;

	// binary search for the last block line
	while(from != to)
	{
		intptr_t curr = (from + to + 1) / 2;

		if (!FastGetString(EditorId, curr, &egs, Info))
			return lua_pushnil(L), 1;

		if (egs.SelStart < 0)
		{
			if (curr == to)
				break;

			to = curr;      // curr was not selected
		}
		else
		{
			from = curr;    // curr was selected
		}
	}

	if (!FastGetString(EditorId, from, &egs, Info))
		return lua_pushnil(L), 1;

	PutIntToTable(L, "EndLine", from+1);
	PutIntToTable(L, "EndPos", egs.SelEnd);
	// restore current position, since FastGetString() changed it
	esp.CurLine       = EI.CurLine;
	esp.CurPos        = EI.CurPos;
	esp.CurTabPos     = EI.CurTabPos;
	esp.TopScreenLine = EI.TopScreenLine;
	esp.LeftPos       = EI.LeftPos;
	esp.Overtype      = EI.Overtype;
	Info->EditorControl(EditorId, ECTL_SETPOSITION, 0, &esp);
	return 1;
}

static int _EditorTabConvert(lua_State *L, int Operation)
{
	intptr_t EditorId = luaL_optinteger(L,1,CURRENT_EDITOR);
	PSInfo *Info = GetPluginData(L)->Info;
	struct EditorConvertPos ecp;
	ecp.StructSize = sizeof(ecp);
	ecp.StringNumber = luaL_optinteger(L,2,0) - 1;
	ecp.SrcPos = luaL_checkinteger(L,3) - 1;

	if (Info->EditorControl(EditorId, Operation, 0, &ecp))
		lua_pushinteger(L, ecp.DestPos+1);
	else
		lua_pushnil(L);

	return 1;
}

static int editor_TabToReal(lua_State *L)
{
	return _EditorTabConvert(L, ECTL_TABTOREAL);
}

static int editor_RealToTab(lua_State *L)
{
	return _EditorTabConvert(L, ECTL_REALTOTAB);
}

int GetFarColor(lua_State *L, int pos, struct FarColor* Color)
{
	if (lua_istable(L, pos))
	{
		lua_pushvalue(L, pos);
		Color->Flags = CheckFlagsFromTable(L, -1, "Flags");
		Color->Foreground.ForegroundColor = CAST(COLORREF, GetOptNumFromTable(L, "ForegroundColor", 0));
		Color->Background.BackgroundColor = CAST(COLORREF, GetOptNumFromTable(L, "BackgroundColor", 0));
		Color->Underline.UnderlineColor = 0;
		Color->Reserved = 0;
		lua_pop(L, 1);
		return 1;
	}
	else if (lua_isnumber(L, pos))
	{
		DWORD num = (DWORD)lua_tonumber(L, pos);
		Color->Flags = FCF_4BITMASK;
		Color->Foreground.ForegroundColor = (num & 0x0F) | ALPHAMASK;
		Color->Background.BackgroundColor = ((num>>4) & 0x0F) | ALPHAMASK;
		Color->Underline.UnderlineColor = 0;
		Color->Reserved = 0;
		return 1;
	}
	return 0;
}

void PushFarColor(lua_State *L, const struct FarColor* Color)
{
	lua_createtable(L, 0, 3);
	PutFlagsToTable(L, "Flags", Color->Flags);
	PutNumToTable(L, "ForegroundColor", Color->Foreground.ForegroundColor);
	PutNumToTable(L, "BackgroundColor", Color->Background.BackgroundColor);
}

static void GetOptGuid(lua_State *L, int pos, GUID* target, const GUID* source)
{
	if (lua_type(L, pos) == LUA_TSTRING && lua_objlen(L, pos) >= sizeof(GUID))
		*target = *CAST(const GUID*, lua_tostring(L, pos));
	else if (lua_isnoneornil(L, pos))
		*target = *source;
	else
		luaL_argerror(L, pos, "GUID required");
}

static int editor_AddColor(lua_State *L)
{
	TPluginData *pd = GetPluginData(L);
	intptr_t EditorId;
	struct EditorColor ec;
	memset(&ec, 0, sizeof(ec));
	ec.StructSize = sizeof(ec);
	ec.ColorItem = 0;
	EditorId        = luaL_optinteger(L, 1, CURRENT_EDITOR);
	ec.StringNumber = luaL_optinteger(L, 2, 0) - 1;
	ec.StartPos     = luaL_checkinteger(L, 3) - 1;
	ec.EndPos       = luaL_checkinteger(L, 4) - 1;
	ec.Flags        = OptFlags(L, 5, 0);
	luaL_argcheck(L, GetFarColor(L, 6, &ec.Color), 6, "table or number expected");
	ec.Priority     = CAST(unsigned, luaL_optnumber(L, 7, EDITOR_COLOR_NORMAL_PRIORITY));
	GetOptGuid(L, 8, &ec.Owner, pd->PluginId);
	lua_pushboolean(L, pd->Info->EditorControl(EditorId, ECTL_ADDCOLOR, 0, &ec) != 0);
	return 1;
}

static int editor_DelColor(lua_State *L)
{
	TPluginData *pd = GetPluginData(L);
	intptr_t EditorId;
	struct EditorDeleteColor edc;
	edc.StructSize = sizeof(edc);
	EditorId         = luaL_optinteger(L, 1, CURRENT_EDITOR);
	edc.StringNumber = luaL_optinteger(L, 2, 0) - 1;
	edc.StartPos     = luaL_optinteger(L, 3, 0) - 1;
	GetOptGuid(L, 4, &edc.Owner, pd->PluginId);
	lua_pushboolean(L, pd->Info->EditorControl(EditorId, ECTL_DELCOLOR, 0, &edc) != 0);
	return 1;
}

static int editor_GetColor(lua_State *L)
{
	PSInfo *Info = GetPluginData(L)->Info;
	intptr_t EditorId;
	struct EditorColor ec;
	memset(&ec, 0, sizeof(ec));
	ec.StructSize = sizeof(ec);
	EditorId        = luaL_optinteger(L, 1, CURRENT_EDITOR);
	ec.StringNumber = luaL_optinteger(L, 2, 0) - 1;
	ec.ColorItem    = luaL_checkinteger(L, 3);

	if (Info->EditorControl(EditorId, ECTL_GETCOLOR, 0, &ec))
	{
		lua_createtable(L, 0, 6);
		PutNumToTable(L, "StartPos", (double)ec.StartPos+1);
		PutNumToTable(L, "EndPos", (double)ec.EndPos+1);
		PutNumToTable(L, "Priority", (double)ec.Priority);
		PutFlagsToTable(L, "Flags",  ec.Flags);
		PushFarColor(L, &ec.Color);
		lua_setfield(L, -2, "Color");
		PutLStrToTable(L, "Owner", (const void*)&ec.Owner, sizeof(ec.Owner));
	}
	else
		lua_pushnil(L);

	return 1;
}

static int editor_SaveFile(lua_State *L)
{
	PSInfo *Info = GetPluginData(L)->Info;
	struct EditorSaveFile esf;
	intptr_t EditorId = luaL_optinteger(L, 1, CURRENT_EDITOR);
	esf.StructSize = sizeof(esf);
	esf.FileName = opt_utf8_string(L, 2, NULL);
	esf.FileEOL = opt_utf8_string(L, 3, NULL);
	esf.CodePage = luaL_optinteger(L, 4, CP_DEFAULT);
	lua_pushboolean(L, (int)Info->EditorControl(EditorId, ECTL_SAVEFILE, 0, &esf));
	return 1;
}

void PushInputRecord(lua_State *L, const INPUT_RECORD* ir)
{
	lua_newtable(L);
	PutIntToTable(L, "EventType", ir->EventType);

	switch(ir->EventType)
	{
		case KEY_EVENT:
			PutBoolToTable(L,"KeyDown", ir->Event.KeyEvent.bKeyDown);
			PutNumToTable(L, "RepeatCount", ir->Event.KeyEvent.wRepeatCount);
			PutNumToTable(L, "VirtualKeyCode", ir->Event.KeyEvent.wVirtualKeyCode);
			PutNumToTable(L, "VirtualScanCode", ir->Event.KeyEvent.wVirtualScanCode);
			PutWStrToTable(L, "UnicodeChar", &ir->Event.KeyEvent.uChar.UnicodeChar, 1);
			PutNumToTable(L, "ControlKeyState", ir->Event.KeyEvent.dwControlKeyState);
			break;

		case MOUSE_EVENT:
			PutMouseEvent(L, &ir->Event.MouseEvent, TRUE);
			break;

		case WINDOW_BUFFER_SIZE_EVENT:
			PutNumToTable(L, "SizeX", ir->Event.WindowBufferSizeEvent.dwSize.X);
			PutNumToTable(L, "SizeY", ir->Event.WindowBufferSizeEvent.dwSize.Y);
			break;

		case MENU_EVENT:
			PutNumToTable(L, "CommandId", ir->Event.MenuEvent.dwCommandId);
			break;

		case FOCUS_EVENT:
			PutBoolToTable(L,"SetFocus", ir->Event.FocusEvent.bSetFocus);
			break;

		default:
			break;
	}
}

static int editor_ReadInput(lua_State *L)
{
	intptr_t EditorId = luaL_optinteger(L, 1, CURRENT_EDITOR);
	PSInfo *Info = GetPluginData(L)->Info;
	INPUT_RECORD ir;

	if (Info->EditorControl(EditorId, ECTL_READINPUT, 0, &ir))
		PushInputRecord(L, &ir);
	else
		lua_pushnil(L);

	return 1;
}

void FillInputRecord(lua_State *L, int pos, INPUT_RECORD *ir)
{
	int success = 0;
	pos = abs_index(L, pos);
	luaL_checktype(L, pos, LUA_TTABLE);
	memset(ir, 0, sizeof(INPUT_RECORD));
	// determine event type
	lua_getfield(L, pos, "EventType");
	ir->EventType = CAST(WORD, get_env_flag(L, -1, &success));
	if (success)
	{
		if (ir->EventType == 0)
		{
			ir->EventType = KEY_EVENT;
		}
		success = ir->EventType == KEY_EVENT
			|| ir->EventType == MOUSE_EVENT
			|| ir->EventType == WINDOW_BUFFER_SIZE_EVENT
			|| ir->EventType == MENU_EVENT
			|| ir->EventType == FOCUS_EVENT;
	}
	if (!success)
		luaL_error(L, "invalid 'EventType' specified");

	lua_pop(L, 1);
	lua_pushvalue(L, pos);

	switch(ir->EventType)
	{
		case KEY_EVENT:
			ir->Event.KeyEvent.bKeyDown = GetOptBoolFromTable(L, "KeyDown", TRUE);
			ir->Event.KeyEvent.wRepeatCount = GetOptIntFromTable(L, "RepeatCount", 1);
			ir->Event.KeyEvent.wVirtualKeyCode = GetOptIntFromTable(L, "VirtualKeyCode", 0);
			ir->Event.KeyEvent.wVirtualScanCode = GetOptIntFromTable(L, "VirtualScanCode", 0);
			lua_getfield(L, -1, "UnicodeChar");
			ir->Event.KeyEvent.uChar.UnicodeChar = *opt_utf8_string(L, -1, L"");
			lua_pop(L, 1);
			ir->Event.KeyEvent.dwControlKeyState = GetOptIntFromTable(L, "ControlKeyState", 0);
			break;

		case MOUSE_EVENT:
			GetMouseEvent(L, &ir->Event.MouseEvent);
			break;

		case WINDOW_BUFFER_SIZE_EVENT:
			ir->Event.WindowBufferSizeEvent.dwSize.X = GetOptIntFromTable(L, "SizeX", 0);
			ir->Event.WindowBufferSizeEvent.dwSize.Y = GetOptIntFromTable(L, "SizeY", 0);
			break;

		case MENU_EVENT:
			ir->Event.MenuEvent.dwCommandId = GetOptIntFromTable(L, "CommandId", 0);
			break;

		case FOCUS_EVENT:
			ir->Event.FocusEvent.bSetFocus = GetOptBoolFromTable(L, "SetFocus", FALSE);
			break;
	}

	lua_pop(L, 1);
}

static void OptInputRecord(lua_State* L, TPluginData *pd, int pos, INPUT_RECORD* ir)
{
	if (lua_istable(L, pos))
		FillInputRecord(L, pos, ir);
	else if (lua_type(L, pos) == LUA_TSTRING)
	{
		wchar_t* name = check_utf8_string(L, pos, NULL);

		if (!pd->FSF->FarNameToInputRecord(name, ir))
			luaL_argerror(L, pos, "invalid key");
	}
	else
	{
		memset(ir, 0, sizeof(INPUT_RECORD));
		ir->EventType = KEY_EVENT;
	}
}

static int editor_ProcessInput(lua_State *L)
{
	INPUT_RECORD ir;
	intptr_t EditorId = luaL_optinteger(L, 1, CURRENT_EDITOR);
	luaL_checktype(L, 2, LUA_TTABLE);
	FillInputRecord(L, 2, &ir);
	lua_pushboolean(L, GetPluginData(L)->Info->EditorControl(EditorId, ECTL_PROCESSINPUT, 0, &ir) != 0);
	return 1;
}

static int editor_SubscribeChangeEvent(lua_State *L)
{
	TPluginData *pd = GetPluginData(L);
	struct EditorSubscribeChangeEvent data;
	intptr_t EditorId = luaL_optinteger(L, 1, CURRENT_EDITOR);
	int command = lua_toboolean(L, 2) ? ECTL_SUBSCRIBECHANGEEVENT : ECTL_UNSUBSCRIBECHANGEEVENT;

	data.StructSize = sizeof(data);
	data.PluginId = *pd->PluginId;
	lua_pushboolean(L, pd->Info->EditorControl(EditorId, command, 0, &data) != 0);

	return 1;
}

// Item, Position = Menu (Properties, Items [, Breakkeys])
// Parameters:
//   Properties -- a table
//   Items      -- an array of tables
//   BreakKeys  -- an array of strings with special syntax
// Return value:
//   Item:
//     a table  -- the table of selected item (or of breakkey) is returned
//     a nil    -- menu canceled by the user
//   Position:
//     a number -- position of selected menu item
//     a nil    -- menu canceled by the user
static int far_Menu(lua_State *L)
{
	enum {
		POS_PROPS = 1, // properties
		POS_ITEMS = 2, // items
		POS_BKEYS = 3, // break keys
		POS_STORE = 4, // temporary storage
	};

	TPluginData *pd = GetPluginData(L);
	int X = -1, Y = -1, MaxHeight = 0;
	UINT64 Flags = FMENU_WRAPMODE;
	const wchar_t *Title = L"Menu", *Bottom = NULL, *HelpTopic = NULL;
	intptr_t SelectIndex = 0, ItemsNumber, ret;
	int i;
	intptr_t BreakCode = 0, *pBreakCode;
	int NumBreakCodes = 0;
	const GUID* MenuGuid = NULL;
	struct FarMenuItem *Items, *pItem;
	struct FarKey *pBreakKeys;

	luaL_checktype(L, POS_PROPS, LUA_TTABLE);
	luaL_checktype(L, POS_ITEMS, LUA_TTABLE);
	ItemsNumber = lua_objlen(L, POS_ITEMS);

	lua_settop(L, POS_BKEYS);     // cut unneeded parameters; make stack predictable
	lua_newtable(L); // temporary store; at stack position 4

	if (!lua_isnil(L,POS_BKEYS) && !lua_istable(L,POS_BKEYS) && lua_type(L,POS_BKEYS)!=LUA_TSTRING)
		return luaL_argerror(L, POS_BKEYS, "must be table, string or nil");

	// Properties
	lua_pushvalue(L, POS_PROPS);
	X = GetOptIntFromTable(L, "X", -1);
	Y = GetOptIntFromTable(L, "Y", -1);
	MaxHeight = GetOptIntFromTable(L, "MaxHeight", 0);

	lua_getfield(L, POS_PROPS, "Flags");
	if (!lua_isnil(L, -1)) Flags = CheckFlags(L, -1);

	lua_getfield(L, POS_PROPS, "Title");
	if (lua_isstring(L,-1))    Title = StoreTempString(L, POS_STORE);

	lua_getfield(L, POS_PROPS, "Bottom");
	if (lua_isstring(L,-1))    Bottom = StoreTempString(L, POS_STORE);

	lua_getfield(L, POS_PROPS, "HelpTopic");
	if (lua_isstring(L,-1))    HelpTopic = StoreTempString(L, POS_STORE);

	lua_getfield(L, POS_PROPS, "SelectIndex");
	if ((SelectIndex = lua_tointeger(L,-1)) > ItemsNumber)
		SelectIndex = 0;

	lua_getfield(L, POS_PROPS, "Id");
	if (lua_type(L,-1)==LUA_TSTRING && lua_objlen(L,-1)==sizeof(GUID))
		MenuGuid = (const GUID*)lua_tostring(L, -1);

	lua_settop (L, POS_STORE);

	// Items
	Items = (struct FarMenuItem*)lua_newuserdata(L, ItemsNumber*sizeof(struct FarMenuItem));
	memset(Items, 0, ItemsNumber*sizeof(struct FarMenuItem));
	pItem = Items;

	for(i=0; i < ItemsNumber; i++,pItem++,lua_pop(L,1))
	{
		static const char key[] = "text";
		lua_pushinteger(L, i+1);
		lua_gettable(L, POS_ITEMS);

		if (!lua_istable(L, -1))
			return luaLF_SlotError(L, i+1, "table");

		//-------------------------------------------------------------------------
		lua_getfield(L, -1, key);

		if (lua_isstring(L,-1))  pItem->Text = StoreTempString(L, POS_STORE);
		else if (!lua_isnil(L,-1)) return luaLF_FieldError(L, key, "string");

		if (!pItem->Text)
			lua_pop(L, 1);

		//-------------------------------------------------------------------------
		lua_getfield(L,-1,"checked");

		if (lua_type(L,-1) == LUA_TSTRING)
		{
			const wchar_t* s = utf8_to_utf16(L,-1,NULL);

			if (s) pItem->Flags |= s[0];
		}
		else if (lua_toboolean(L,-1)) pItem->Flags |= MIF_CHECKED;

		lua_pop(L,1);

		//-------------------------------------------------------------------------
		if (GetBoolFromTable(L, "separator")) pItem->Flags |= MIF_SEPARATOR;

		if (GetBoolFromTable(L, "disable"))   pItem->Flags |= MIF_DISABLE;

		if (GetBoolFromTable(L, "grayed"))    pItem->Flags |= MIF_GRAYED;

		if (GetBoolFromTable(L, "hidden"))    pItem->Flags |= MIF_HIDDEN;

		if (SelectIndex==0 && GetBoolFromTable(L, "selected")) pItem->Flags |= MIF_SELECTED;

		//-------------------------------------------------------------------------
		lua_getfield(L, -1, "AccelKey");

		if (lua_istable(L, -1))
		{
			pItem->AccelKey.VirtualKeyCode = GetOptIntFromTable(L, "VirtualKeyCode", 0);
			pItem->AccelKey.ControlKeyState = GetOptIntFromTable(L, "ControlKeyState", 0);
		}
		else if (lua_tostring(L, -1) && utf8_to_utf16(L, -1, NULL)) // lua_tostring is used on purpose
		{
			INPUT_RECORD Rec;
			if (pd->FSF->FarNameToInputRecord((const wchar_t*)lua_touserdata(L,-1), &Rec)
				&& Rec.EventType == KEY_EVENT)
			{
				pItem->AccelKey.VirtualKeyCode = Rec.Event.KeyEvent.wVirtualKeyCode;
				pItem->AccelKey.ControlKeyState = Rec.Event.KeyEvent.dwControlKeyState;
			}
		}

		lua_pop(L, 1);
	}

	if (SelectIndex > 0)
		Items[SelectIndex-1].Flags |= MIF_SELECTED;

	// Break Keys
	pBreakKeys = NULL;
	pBreakCode = NULL;
	if (lua_type(L, POS_BKEYS) == LUA_TSTRING)
	{
		const char *q, *ptr = lua_tostring(L, POS_BKEYS);
		lua_newtable(L);
		while (*ptr)
		{
			while (isspace(*ptr)) ptr++;
			if (*ptr == 0) break;
			q = ptr++;
			while(*ptr && !isspace(*ptr)) ptr++;
			lua_createtable(L,0,1);
			lua_pushlstring(L,q,ptr-q);
			lua_setfield(L,-2,"BreakKey");
			lua_rawseti(L,-2,++NumBreakCodes);
		}
		lua_replace(L, POS_BKEYS);
	}
	else
		NumBreakCodes = lua_istable(L,POS_BKEYS) ? (int)lua_objlen(L,POS_BKEYS) : 0;

	if (NumBreakCodes)
	{
		char buf[32];
		int ind;
		struct FarKey* BreakKeys = (struct FarKey*)lua_newuserdata(L, (1+NumBreakCodes)*sizeof(struct FarKey));
		// get virtualkeys table from the registry; push it on top
		lua_pushstring(L, FAR_VIRTUALKEYS);
		lua_rawget(L, LUA_REGISTRYINDEX);
		// push breakkeys table on top
		lua_pushvalue(L, POS_BKEYS);        // vk=-2; bk=-1;

		for(ind=0; ind < NumBreakCodes; ind++)
		{
			DWORD mod = 0;
			const char* s;
			char* vk;  // virtual key
			WORD VirtualKeyCode;

			BreakKeys[ind].VirtualKeyCode = 0xFF; // preset to invalid value !=0, since 0 marks end-of-array for Far.
			BreakKeys[ind].ControlKeyState = LEFT_CTRL_PRESSED|LEFT_ALT_PRESSED|SHIFT_PRESSED;

			// get next break key (optional modifier plus virtual key)
			lua_pushinteger(L,ind+1);       // vk=-3; bk=-2;
			lua_gettable(L,-2);             // vk=-3; bk=-2; bki=-1;

			if (!lua_istable(L,-1)) { lua_pop(L,1); continue; }

			lua_getfield(L, -1, "BreakKey");// vk=-4; bk=-3;bki=-2;bknm=-1;

			if (!lua_isstring(L,-1)) { lua_pop(L,2); continue; }

			// first try to use "Far key names" instead of "virtual key names"
			if (utf8_to_utf16(L, -1, NULL))
			{
				INPUT_RECORD Rec;
				if (pd->FSF->FarNameToInputRecord((const wchar_t*)lua_touserdata(L,-1), &Rec)
					&& Rec.EventType == KEY_EVENT)
				{
					BreakKeys[ind].VirtualKeyCode = Rec.Event.KeyEvent.wVirtualKeyCode;
					BreakKeys[ind].ControlKeyState = Rec.Event.KeyEvent.dwControlKeyState;
					lua_pop(L, 2);
					continue; // success
				}
				// restore the original string
				lua_pop(L, 1);
				lua_getfield(L, -1, "BreakKey");// vk=-4; bk=-3;bki=-2;bknm=-1;
			}

			// separate modifier and virtual key strings
			s = lua_tostring(L,-1);

			if (strlen(s) >= sizeof(buf)) { lua_pop(L,2); continue; }

			strcpy(buf, s);
			_strupr(buf);
			vk = strchr(buf, '+');  // virtual key

			if (vk)
			{
				*vk++ = '\0';

				if (strchr(buf,'C')) mod |= LEFT_CTRL_PRESSED;

				if (strchr(buf,'A')) mod |= LEFT_ALT_PRESSED;

				if (strchr(buf,'S')) mod |= SHIFT_PRESSED;
			}
			else
				vk = buf;

			// replace on stack: break key name with virtual key name
			lua_pop(L, 1);
			lua_pushstring(L, vk);        // vk=-4; bk=-3;bki=-2;vknm=-1;

			// get virtual key and break key values
			lua_rawget(L,-4);               // vk=-4; bk=-3;
			VirtualKeyCode = (WORD)lua_tointeger(L,-1);
			if (VirtualKeyCode)
			{
				BreakKeys[ind].VirtualKeyCode = VirtualKeyCode;
				BreakKeys[ind].ControlKeyState = mod;
			}
			lua_pop(L,2);                   // vk=-2; bk=-1;
		}

		BreakKeys[ind].VirtualKeyCode = 0; // required by FAR API
		pBreakKeys = BreakKeys;
		pBreakCode = &BreakCode;
	}

	ret = pd->Info->Menu(pd->PluginId, MenuGuid, X, Y, MaxHeight, Flags, Title,
	                     Bottom, HelpTopic, pBreakKeys, pBreakCode, Items, ItemsNumber);

	if (NumBreakCodes && (BreakCode != -1))
	{
		lua_pushinteger(L, BreakCode+1);
		lua_gettable(L, POS_BKEYS);
	}
	else if (ret == -1)
		return lua_pushnil(L), 1;
	else
	{
		lua_pushinteger(L, ret+1);
		lua_gettable(L, POS_ITEMS);
	}

	lua_pushinteger(L, ret+1);
	return 2;
}

// Return:   -1 if escape pressed, else - button number chosen (0 based).
int LF_Message(lua_State *L,
               const wchar_t* aMsg,      // if multiline, then lines must be separated by '\n'
               const wchar_t* aTitle,
               const wchar_t* aButtons,  // if multiple, then captions must be separated by ';'
               const char*    aFlags,
               const wchar_t* aHelpTopic,
               const GUID*    aMessageGuid)
{
	const wchar_t **items, **pItems;
	wchar_t** allocLines;
	int nAlloc;
	wchar_t *lastDelim, *MsgCopy, *start, *pos;
	TPluginData *pd = GetPluginData(L);
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	HANDLE hnd = GetStdHandle(STD_OUTPUT_HANDLE);
	int ret = GetConsoleScreenBufferInfo(hnd, &csbi);
	const int max_len   = ret ? csbi.srWindow.Right - csbi.srWindow.Left+1-14 : 66;
	const int max_lines = ret ? csbi.srWindow.Bottom - csbi.srWindow.Top+1-5 : 20;
	int num_lines = 0, num_buttons = 0;
	UINT64 Flags = 0;
	// Buttons
	wchar_t *BtnCopy = NULL, *ptr = NULL;
	int wrap = !(aFlags && strchr(aFlags, 'n'));

	if (*aButtons == L';')
	{
		const wchar_t* p = aButtons + 1;

		if (!_wcsicmp(p, L"Ok"))               Flags = FMSG_MB_OK;
		else if (!_wcsicmp(p, L"OkCancel"))         Flags = FMSG_MB_OKCANCEL;
		else if (!_wcsicmp(p, L"AbortRetryIgnore")) Flags = FMSG_MB_ABORTRETRYIGNORE;
		else if (!_wcsicmp(p, L"YesNo"))            Flags = FMSG_MB_YESNO;
		else if (!_wcsicmp(p, L"YesNoCancel"))      Flags = FMSG_MB_YESNOCANCEL;
		else if (!_wcsicmp(p, L"RetryCancel"))      Flags = FMSG_MB_RETRYCANCEL;
		else
			while(*aButtons == L';') aButtons++;
	}
	if (Flags == 0)
	{
		// Buttons: 1-st pass, determining number of buttons
		BtnCopy = _wcsdup(aButtons);
		ptr = BtnCopy;

		while(*ptr && (num_buttons < 64))
		{
			while(*ptr == L';')
				ptr++; // skip semicolons

			if (*ptr)
			{
				++num_buttons;
				ptr = wcschr(ptr, L';');

				if (!ptr) break;
			}
		}
	}

	items = (const wchar_t**) malloc((1+max_lines+num_buttons) * sizeof(wchar_t*));
	allocLines = (wchar_t**) malloc(max_lines * sizeof(wchar_t*)); // array of pointers to allocated lines
	nAlloc = 0;                                                    // number of allocated lines
	pItems = items;
	// Title
	*pItems++ = aTitle;
	// Message lines
	lastDelim = NULL;
	MsgCopy = _wcsdup(aMsg);
	start = pos = MsgCopy;

	while(num_lines < max_lines)
	{
		if (*pos == 0)                          // end of the entire message
		{
			*pItems++ = start;
			++num_lines;
			break;
		}
		else if (*pos == L'\n')                 // end of a message line
		{
			*pItems++ = start;
			*pos = L'\0';
			++num_lines;
			start = ++pos;
			lastDelim = NULL;
		}
		else if (pos-start < max_len)            // characters inside the line
		{
			if (wrap && !iswalnum(*pos) && *pos != L'_' && *pos != L'\'' && *pos != L'\"')
				lastDelim = pos;

			pos++;
		}
		else if (wrap)                          // the 1-st character beyond the line
		{
			size_t len;
			wchar_t **q;
			pos = lastDelim ? lastDelim+1 : pos;
			len = pos - start;
			q = &allocLines[nAlloc++]; // line allocation is needed
			*pItems++ = *q = (wchar_t*) malloc((len+1)*sizeof(wchar_t));
			wcsncpy(*q, start, len);
			(*q)[len] = L'\0';
			++num_lines;
			start = pos;
			lastDelim = NULL;
		}
		else
			pos++;
	}

	if (*aButtons != L';')
	{
		// Buttons: 2-nd pass.
		int i;
		ptr = BtnCopy;

		for(i=0; i < num_buttons; i++)
		{
			while(*ptr == L';')
				++ptr;

			if (*ptr)
			{
				*pItems++ = ptr;
				ptr = wcschr(ptr, L';');

				if (ptr)
					*ptr++ = 0;
				else
					break;
			}
			else break;
		}
	}

	// Flags
	if (aFlags)
	{
		if (strchr(aFlags, 'w')) Flags |= FMSG_WARNING;

		if (strchr(aFlags, 'e')) Flags |= FMSG_ERRORTYPE;

		if (strchr(aFlags, 'k')) Flags |= FMSG_KEEPBACKGROUND;

		if (strchr(aFlags, 'l')) Flags |= FMSG_LEFTALIGN;
	}

	// Id
	if (aMessageGuid == NULL) aMessageGuid = pd->PluginId;

	ret = (int)pd->Info->Message(pd->PluginId, aMessageGuid, Flags, aHelpTopic,
	                             items, 1+num_lines+num_buttons, num_buttons);
	free(BtnCopy);

	while(nAlloc) free(allocLines[--nAlloc]);

	free(allocLines);
	free(MsgCopy);
	free(CAST(void*, items));
	return ret;
}

void LF_Error(lua_State *L, const wchar_t* aMsg)
{
	PSInfo *Info = GetPluginData(L)->Info;

	if (Info == NULL)
		return;

	if (!aMsg) aMsg = L"<non-string error message>";

	lua_pushlstring(L, (const char*)Info->ModuleName, wcslen(Info->ModuleName)*sizeof(wchar_t));
	lua_pushlstring(L, (const char*)L":\n", 4);
	LF_Gsub(L, aMsg, L"\n\t", L"\n   ");
	lua_concat(L, 3);
	LF_Message(L, (const wchar_t*)lua_tostring(L,-1), L"Error", L"OK", "wl", NULL, NULL);
	lua_pop(L, 1);
}

// 1-st param: message text (if multiline, then lines must be separated by '\n')
// 2-nd param: message title (if absent or nil, then "Message" is used)
// 3-rd param: buttons (if multiple, then captions must be separated by ';';
//             if absent or nil, then one button "OK" is used).
// 4-th param: flags
// 5-th param: help topic
// 6-th param: Id
// Return: -1 if escape pressed, else - button number chosen (1 based).
static int far_Message(lua_State *L)
{
	int ret;
	const wchar_t *Msg, *Title, *Buttons, *HelpTopic;
	const char *Flags;
	const GUID *Id;
	luaL_checkany(L,1);
	lua_settop(L,6);
	Msg = NULL;

	if (lua_isstring(L, 1))
		Msg = check_utf8_string(L, 1, NULL);
	else
	{
		lua_getglobal(L, "tostring");

		if (lua_isfunction(L,-1))
		{
			lua_pushvalue(L,1);
			lua_call(L,1,1);
			Msg = check_utf8_string(L,-1,NULL);
		}

		if (Msg == NULL) luaL_argerror(L, 1, "cannot convert to string");

		lua_replace(L,1);
	}

	Title   = opt_utf8_string(L, 2, L"Message");
	Buttons = opt_utf8_string(L, 3, L";OK");
	Flags   = luaL_optstring(L, 4, "");
	HelpTopic = opt_utf8_string(L, 5, NULL);
	Id = (lua_type(L,6)==LUA_TSTRING && lua_objlen(L,6)==sizeof(GUID)) ?
	     (const GUID*)lua_tostring(L,6) : NULL;
	ret = LF_Message(L, Msg, Title, Buttons, Flags, HelpTopic, Id);
	lua_pushinteger(L, ret<0 ? ret : ret+1);
	return 1;
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
	struct PanelInfo pi;
	pi.StructSize = sizeof(pi);

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
	lua_createtable(L, 0, 4); // "PanelRect"
	PutIntToTable(L, "left", pi.PanelRect.left);
	PutIntToTable(L, "top", pi.PanelRect.top);
	PutIntToTable(L, "right", pi.PanelRect.right);
	PutIntToTable(L, "bottom", pi.PanelRect.bottom);
	lua_setfield(L, -2, "PanelRect");
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
	struct PanelRedrawInfo pri;
	pri.StructSize = sizeof(pri);

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
	int param1 = CAST(int, check_env_flag(L,3));
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
	struct FarPanelDirectory fpd;
	HANDLE handle = OptHandle2(L);
	memset(&fpd, 0, sizeof(fpd)); // also sets fpd.PluginId = FarId
	fpd.StructSize = sizeof(fpd);

	if (lua_istable(L, 3))
	{
		size_t len;
		const GUID* id;
		lua_getfield(L, 3, "PluginId");
		id = (const GUID*)lua_tolstring(L, -1, &len);

		if (id && len == sizeof(GUID)) fpd.PluginId = *id;

		lua_getfield(L, 3, "Name");  if (lua_isstring(L, -1)) fpd.Name = check_utf8_string(L, -1, NULL);

		lua_getfield(L, 3, "Param"); if (lua_isstring(L, -1)) fpd.Param = check_utf8_string(L, -1, NULL);

		lua_getfield(L, 3, "File");  if (lua_isstring(L, -1)) fpd.File = check_utf8_string(L, -1, NULL);
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
	intptr_t ret = Info->PanelControl(handle, FCTL_SETCMDLINEPOS, pos, 0);
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
	struct CmdLineSelect cms;
	cms.StructSize = sizeof(cms);

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
	struct CmdLineSelect cms;
	cms.StructSize = sizeof(cms);
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
static int ChangePanelSelection(lua_State *L, BOOL op_set)
{
	PSInfo *Info = GetPluginData(L)->Info;
	intptr_t itemindex = -1, numItems, command;
	intptr_t state;
	struct PanelInfo pi;
	HANDLE handle = OptHandle2(L);

	if (lua_isnumber(L,3))
	{
		itemindex = lua_tointeger(L,3) - 1;

		if (itemindex < 0) return luaL_argerror(L, 3, "non-positive index");
	}
	else if (!lua_istable(L,3))
		return luaL_typerror(L, 3, "number or table");

	state = op_set ? lua_toboolean(L,4) : 0;
	// get panel info
	pi.StructSize = sizeof(pi);

	if (!Info->PanelControl(handle, FCTL_GETPANELINFO, 0, &pi) ||
	        (pi.PanelType != PTYPE_FILEPANEL))
		return lua_pushboolean(L,0), 1;

	numItems = op_set ? pi.ItemsNumber : pi.SelectedItemsNumber;
	command  = op_set ? FCTL_SETSELECTION : FCTL_CLEARSELECTION;

	if (itemindex >= 0 && itemindex < numItems)
		Info->PanelControl(handle, command, itemindex, (void*)state);
	else
	{
		intptr_t i, len = lua_objlen(L,3);

		for(i=1; i<=len; i++)
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
	return ChangePanelSelection(L, TRUE);
}

static int panel_ClearSelection(lua_State *L)
{
	return ChangePanelSelection(L, FALSE);
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

// GetDirList (Dir)
//   Dir:     Name of the directory to scan (full pathname).
static int far_GetDirList(lua_State *L)
{
	PSInfo *Info = GetPluginData(L)->Info;
	const wchar_t *Dir = check_utf8_string(L, 1, NULL);
	struct PluginPanelItem *PanelItems;
	size_t ItemsNumber;

	if (Info->GetDirList(Dir, &PanelItems, &ItemsNumber))
	{
		int i;
		lua_createtable(L, (int)ItemsNumber, 0); // "PanelItems"

		for(i=0; i < (int)ItemsNumber; i++)
		{
			PushPanelItem(L, PanelItems + i, 0);
			lua_rawseti(L, -2, i+1);
		}

		Info->FreeDirList(PanelItems, ItemsNumber);
	}
	else
		lua_pushnil(L);

	return 1;
}

// GetPluginDirList (hPanel, Dir)
//   hPanel:          Panel handle.
//   Dir:             Name of the directory to scan (full pathname).
static int far_GetPluginDirList(lua_State *L)
{
	TPluginData *pd = GetPluginData(L);
	HANDLE handle = OptHandle(L);
	const wchar_t *Dir = check_utf8_string(L, 2, NULL);
	struct PanelInfo pi;
	pi.StructSize = sizeof(pi);

	if (handle && pd->Info->PanelControl(handle, FCTL_GETPANELINFO, 0, &pi) && (pi.Flags & PFLAGS_PLUGIN))
	{
		struct PluginPanelItem *PanelItems;
		size_t ItemsNumber;
		if (pd->Info->GetPluginDirList(&pi.OwnerGuid, handle, Dir, &PanelItems, &ItemsNumber))
		{
			PushPanelItems(L, PanelItems, ItemsNumber, 0);
			pd->Info->FreePluginDirList(handle, PanelItems, ItemsNumber);
			return 1;
		}
	}
	lua_pushnil(L);
	return 1;
}

static int SavedScreen_tostring (lua_State *L)
{
	void **pp = (void**)luaL_checkudata(L, 1, SavedScreenType);
	if (*pp)
		lua_pushfstring(L, "%s (%p)", SavedScreenType, *pp);
	else
		lua_pushfstring(L, "%s (freed)", SavedScreenType);
	return 1;
}

// RestoreScreen (handle)
//   handle:    handle of saved screen.
static int far_RestoreScreen(lua_State *L)
{
	if (lua_isnoneornil(L, 1))
		GetPluginData(L)->Info->RestoreScreen(NULL);
	else
	{
		void **pp = (void**)luaL_checkudata(L, 1, SavedScreenType);
		if (*pp)
		{
			GetPluginData(L)->Info->RestoreScreen(*pp);
			*pp = NULL;
		}
	}
	return 0;
}

// FreeScreen (handle)
//   handle:    handle of saved screen.
static int far_FreeScreen(lua_State *L)
{
	void **pp = (void**)luaL_checkudata(L, 1, SavedScreenType);
	if (*pp)
	{
		GetPluginData(L)->Info->FreeScreen(*pp);
		*pp = NULL;
	}
	return 0;
}

// handle = SaveScreen (X1,Y1,X2,Y2)
//   handle:    handle of saved screen, [lightuserdata]
static int far_SaveScreen(lua_State *L)
{
	intptr_t X1 = luaL_optinteger(L,1,0);
	intptr_t Y1 = luaL_optinteger(L,2,0);
	intptr_t X2 = luaL_optinteger(L,3,-1);
	intptr_t Y2 = luaL_optinteger(L,4,-1);

	*(void**)lua_newuserdata(L, sizeof(void*)) = GetPluginData(L)->Info->SaveScreen(X1,Y1,X2,Y2);
	luaL_getmetatable(L, SavedScreenType);
	lua_setmetatable(L, -2);
	return 1;
}

static UINT64 GetDialogItemType(lua_State* L, int key, int item)
{
	int success;
	UINT64 iType;
	lua_pushinteger(L, key);
	lua_gettable(L, -2);
	iType = get_env_flag(L, -1, &success);

	if (!success)
	{
		const char* sType = lua_tostring(L, -1);
		return luaL_error(L, "%s - unsupported type in dialog item %d", sType, item);
	}

	lua_pop(L, 1);
	return iType;
}

// the table is on lua stack top
static UINT64 GetItemFlags(lua_State* L, int flag_index, int item_index)
{
	UINT64 flags;
	int success;
	lua_pushinteger(L, flag_index);
	lua_gettable(L, -2);
	flags = GetFlagCombination(L, -1, &success);

	if (!success)
		return luaL_error(L, "unsupported flag in dialog item %d", item_index);

	lua_pop(L, 1);
	return flags;
}

// list table is on Lua stack top
struct FarList* CreateList(lua_State *L, int historyindex)
{
	int i, n = (int)lua_objlen(L,-1);
	struct FarList* list = (struct FarList*)lua_newuserdata(L,
	                       sizeof(struct FarList) + n*sizeof(struct FarListItem)); // +2
	int len = (int)lua_objlen(L, historyindex);
	lua_rawseti(L, historyindex, ++len);  // +1; put into "histories" table to avoid being gc'ed
	list->StructSize = sizeof(struct FarList);
	list->ItemsNumber = n;
	list->Items = (struct FarListItem*)(list+1);

	for(i=0; i<n; i++)
	{
		struct FarListItem *p = list->Items + i;
		lua_pushinteger(L, i+1); // +2
		lua_gettable(L,-2);      // +2

		if (lua_type(L,-1) != LUA_TTABLE)
			luaL_error(L, "value at index %d is not a table", i+1);

		p->Text = NULL;
		lua_getfield(L, -1, "Text"); // +3

		if (lua_isstring(L,-1))
		{
			lua_pushvalue(L,-1);       // +4
			p->Text = check_utf8_string(L,-1,NULL); // +4
			lua_rawseti(L, historyindex, ++len);  // +3
		}

		lua_pop(L, 1);                 // +2
		p->Flags = CheckFlagsFromTable(L, -1, "Flags");
		lua_pop(L, 1);                 // +1
	}

	return list;
}

static void PushList (lua_State *L, const struct FarList *list)
{
	int i;
	lua_createtable(L, (int)list->ItemsNumber, 0);
	for (i=0; i<(int)list->ItemsNumber; i++)
	{
		lua_createtable(L,0,2);
		PutFlagsToTable(L, "Flags", list->Items[i].Flags);
		PutWStrToTable(L, "Text", list->Items[i].Text, -1);
		lua_rawseti(L,-2,i+1);
		if (list->Items[i].Flags & LIF_SELECTED)
			PutIntToTable(L, "SelectIndex", i+1);
	}
}

//	enum FARDIALOGITEMTYPES Type;            1
//	intptr_t X1,Y1,X2,Y2;                    2,3,4,5
//	union
//	{
//		intptr_t Selected;                     6
//		struct FarList *ListItems;             6
//		struct FAR_CHAR_INFO *VBuf;            6
//		intptr_t Reserved0;                    6
//	}
//#ifndef __cplusplus
//	Param
//#endif
//	;
//	const wchar_t *History;                  7
//	const wchar_t *Mask;                     8
//	FARDIALOGITEMFLAGS Flags;                9
//	const wchar_t *Data;                     10
//	size_t MaxLength;                        11  // terminate 0 not included (if == 0 string size is unlimited)
//	intptr_t UserData;                       12
//	intptr_t Reserved[2];


// item table is on Lua stack top
static void SetFarDialogItem(lua_State *L, struct FarDialogItem* Item, int itemindex,
                             int historyindex)
{
	int len;
	memset(Item, 0, sizeof(struct FarDialogItem));
	Item->Type  = GetDialogItemType(L, 1, itemindex+1);
	Item->X1    = GetIntFromArray(L, 2);
	Item->Y1    = GetIntFromArray(L, 3);
	Item->X2    = GetIntFromArray(L, 4);
	Item->Y2    = GetIntFromArray(L, 5);
	Item->Flags = GetItemFlags(L, 9, itemindex+1);

	if (Item->Type==DI_LISTBOX || Item->Type==DI_COMBOBOX)
	{
		int SelectIndex;
		lua_rawgeti(L, -1, 6);             // +1

		if (lua_type(L,-1) != LUA_TTABLE)
			luaLF_SlotError(L, 6, "table");

		Item->Param.ListItems = CreateList(L, historyindex);
		SelectIndex = GetOptIntFromTable(L, "SelectIndex", -1);

		if (SelectIndex > 0 && SelectIndex <= (int)lua_objlen(L,-1))
			Item->Param.ListItems->Items[SelectIndex-1].Flags |= LIF_SELECTED;

		lua_pop(L,1);                      // 0
	}
	else if (Item->Type == DI_USERCONTROL)
	{
		lua_rawgeti(L, -1, 6);
		if (lua_type(L,-1) == LUA_TUSERDATA)
		{
			TFarUserControl* fuc = CheckFarUserControl(L, -1);
			Item->Param.VBuf = fuc->VBuf;
		}
		lua_pop(L,1);
	}
	else
		Item->Param.Selected = GetIntFromArray(L, 6);

	//---------------------------------------------------------------------------
	if (Item->Flags & DIF_HISTORY)
	{
		lua_rawgeti(L, -1, 7);                          // +1
		Item->History = opt_utf8_string(L, -1, NULL);   // +1
		len = (int)lua_objlen(L, historyindex);
		lua_rawseti(L, historyindex, len+1);  // +0; put into "histories" table to avoid being gc'ed
	}

	//---------------------------------------------------------------------------
	lua_rawgeti(L, -1, 8);                       // +1
	Item->Mask = opt_utf8_string(L, -1, NULL);   // +1
	len = (int)lua_objlen(L, historyindex);
	lua_rawseti(L, historyindex, len+1);  // +0; put into "histories" table to avoid being gc'ed
	//---------------------------------------------------------------------------
	Item->MaxLength = GetOptIntFromArray(L, 11, 0);
	lua_pushinteger(L, 10); // +1
	lua_gettable(L, -2);    // +1

	if (lua_isstring(L, -1))
	{
		Item->Data = check_utf8_string(L, -1, NULL);  // +1
		len = (int)lua_objlen(L, historyindex);
		lua_rawseti(L, historyindex, len+1);  // +0; put into "histories" table to avoid being gc'ed
	}
	else
		lua_pop(L, 1);

	//---------------------------------------------------------------------------
	lua_rawgeti(L, -1, 12);
	Item->UserData = lua_tointeger(L, -1);
	lua_pop(L, 1);
}

static void PushDlgItem(lua_State *L, const struct FarDialogItem* pItem, BOOL table_exist)
{
	if (! table_exist)
		lua_createtable(L, 12, 0);

	PutIntToArray(L, 1, pItem->Type);
	PutIntToArray(L, 2, pItem->X1);
	PutIntToArray(L, 3, pItem->Y1);
	PutIntToArray(L, 4, pItem->X2);
	PutIntToArray(L, 5, pItem->Y2);

	if ((pItem->Type == DI_LISTBOX || pItem->Type == DI_COMBOBOX) && pItem->Param.ListItems)
	{
		PushList(L, pItem->Param.ListItems);
		lua_rawseti(L, -2, 6);
	}
	else if (pItem->Type == DI_USERCONTROL)
	{
		lua_pushinteger(L, 6);
		lua_pushlightuserdata(L, pItem->Param.VBuf);
		lua_settable(L, -3);
	}
	else
		PutIntToArray(L, 6, pItem->Param.Selected);

	PutWStrToArray(L, 7, pItem->History, -1);
	PutWStrToArray(L, 8, pItem->Mask, -1);
	PutFlagsToArray(L, 9, pItem->Flags);
	lua_pushinteger(L, 10);
	push_utf8_string(L, pItem->Data, -1);
	lua_settable(L, -3);
	PutIntToArray(L, 11, pItem->MaxLength);
	lua_pushinteger(L, 12);
	lua_pushinteger(L, pItem->UserData);
	lua_rawset(L, -3);
}

static void PushDlgItemNum(lua_State *L, HANDLE hDlg, int numitem, int pos_table,
                           PSInfo *Info)
{
	struct FarGetDialogItem fgdi = { sizeof(struct FarGetDialogItem), 0, 0 };
	fgdi.Size = Info->SendDlgMessage(hDlg, DM_GETDLGITEM, numitem, &fgdi);

	if (fgdi.Size > 0)
	{
		BOOL table_exist;
		fgdi.Item = (struct FarDialogItem*) lua_newuserdata(L, fgdi.Size);
		Info->SendDlgMessage(hDlg, DM_GETDLGITEM, numitem, &fgdi);
		table_exist = lua_istable(L, pos_table);

		if (table_exist)
			lua_pushvalue(L, pos_table);

		PushDlgItem(L, fgdi.Item, table_exist);
		lua_remove(L, -2);
	}
	else
		lua_pushnil(L);
}

static int SetDlgItem(lua_State *L, HANDLE hDlg, int numitem, int pos_table,
                      PSInfo *Info)
{
	struct FarDialogItem DialogItem;
	lua_newtable(L);
	lua_replace(L,1);
	luaL_checktype(L, pos_table, LUA_TTABLE);
	lua_pushvalue(L, pos_table);
	SetFarDialogItem(L, &DialogItem, numitem, 1);
	lua_pushboolean(L, (int)Info->SendDlgMessage(hDlg, DM_SETDLGITEM, numitem, &DialogItem));
	return 1;
}

TDialogData* NewDialogData(lua_State* L, PSInfo *Info, HANDLE hDlg, BOOL isOwned)
{
	TDialogData *dd = (TDialogData*) lua_newuserdata(L, sizeof(TDialogData));
	dd->L        = GetPluginData(L)->MainLuaState;
	dd->Info     = Info;
	dd->hDlg     = hDlg;
	dd->isOwned  = isOwned;
	dd->wasError = FALSE;
	dd->isModal  = TRUE;
	dd->dataRef  = LUA_REFNIL;
	luaL_getmetatable(L, FarDialogType);
	lua_setmetatable(L, -2);

	if (isOwned)
	{
		lua_newtable(L);
		lua_setfenv(L, -2);
	}

	return dd;
}

TDialogData* CheckDialog(lua_State* L, int pos)
{
	return (TDialogData*)luaL_checkudata(L, pos, FarDialogType);
}

TDialogData* CheckValidDialog(lua_State* L, int pos)
{
	TDialogData* dd = CheckDialog(L, pos);
	luaL_argcheck(L, dd->hDlg != INVALID_HANDLE_VALUE, pos, "closed dialog");
	return dd;
}

HANDLE CheckDialogHandle(lua_State* L, int pos)
{
	return CheckValidDialog(L, pos)->hDlg;
}

int DialogHandleEqual(lua_State* L)
{
	TDialogData* dd1 = CheckDialog(L, 1);
	TDialogData* dd2 = CheckDialog(L, 2);
	lua_pushboolean(L, dd1->hDlg == dd2->hDlg);
	return 1;
}

int PushDMParams (lua_State *L, intptr_t Msg, intptr_t Param1)
{
	if (! ((Msg>DM_FIRST && Msg<=DM_GETDIALOGTITLE) || Msg==DM_USER))
		return 0;

	lua_pushinteger(L, Msg);             //+1

	// Param1
	switch(Msg)
	{
		case DM_CLOSE:
			lua_pushinteger(L, Param1<=0 ? Param1 : Param1+1);
			break;

		case DM_ENABLEREDRAW:
		case DM_GETDIALOGINFO:
		case DM_GETDIALOGTITLE:
		case DM_GETDLGDATA:
		case DM_GETDLGRECT:
		case DM_GETDROPDOWNOPENED:
		case DM_GETFOCUS:
		case DM_KEY:
		case DM_MOVEDIALOG:
		case DM_REDRAW:
		case DM_RESIZEDIALOG:
		case DM_SETDLGDATA:
		case DM_SETINPUTNOTIFY:
		case DM_SHOWDIALOG:
		case DM_USER:
			lua_pushinteger(L, Param1);
			break;

		default: // dialog element position
			lua_pushinteger(L, Param1+1);
			break;
	}

	return 1;
}

static int DoSendDlgMessage (lua_State *L, intptr_t Msg, int delta)
{
	typedef struct { void *Id; int Ref; } listdata_t;
	TPluginData *pluginData = GetPluginData(L);
	PSInfo *Info = pluginData->Info;
	intptr_t Param1=0, res=0, res_incr=0;
	void* Param2 = NULL;
	wchar_t buf[512];
	int pos2 = 2-delta, pos3 = 3-delta, pos4 = 4-delta;
	//---------------------------------------------------------------------------
	COORD coord;
	SMALL_RECT small_rect;
	//---------------------------------------------------------------------------
	lua_settop(L, pos4); //many cases below rely on top==pos4
	HANDLE hDlg = CheckDialogHandle(L, 1);
	if (delta == 0)
		Msg = CAST(int, check_env_flag(L, 2));

	// Param1
	switch(Msg)
	{
		case DM_CLOSE:
			Param1 = luaL_optinteger(L,pos3,-1);
			if (Param1>0) --Param1;
			break;

		case DM_GETDLGDATA:
		case DM_SETDLGDATA:
			break;

		case DM_ENABLEREDRAW:
		case DM_GETDIALOGINFO:
		case DM_GETDIALOGTITLE:
		case DM_GETDLGRECT:
		case DM_GETDROPDOWNOPENED:
		case DM_GETFOCUS:
		case DM_KEY:
		case DM_MOVEDIALOG:
		case DM_REDRAW:
		case DM_RESIZEDIALOG:
		case DM_SETINPUTNOTIFY:
		case DM_SHOWDIALOG:
		case DM_USER:
		// DN_*
		case DN_DRAGGED:
		case DN_DRAWDIALOG:
		case DN_DRAWDIALOGDONE:
			Param1 = luaL_optinteger(L,pos3,0);
			break;

		default: // dialog element position
			Param1 = luaL_optinteger(L,pos3,1) - 1;
			break;
	}

	// res_incr
	switch(Msg)
	{
		case DM_GETFOCUS:
		case DM_LISTADDSTR:
			res_incr=1;
			break;

		default:
			res_incr=0;
			break;
	}

	switch(Msg)
	{
		default:
			luaL_argerror(L, pos2, "operation not implemented");
			break;

		case DM_CLOSE:
		case DM_EDITUNCHANGEDFLAG:
		case DM_ENABLE:
		case DM_ENABLEREDRAW:
		case DM_GETCHECK:
		case DM_GETCOMBOBOXEVENT:
		case DM_GETCURSORSIZE:
		case DM_GETDROPDOWNOPENED:
		case DM_GETFOCUS:
		case DM_GETITEMDATA:
		case DM_LISTSORT:
		case DM_REDRAW:               // alias: DM_SETREDRAW
		case DM_SET3STATE:
		case DM_SETCURSORSIZE:
		case DM_SETDROPDOWNOPENED:
		case DM_SETFOCUS:
		case DM_SETITEMDATA:
		case DM_SETMAXTEXTLENGTH:     // alias: DM_SETTEXTLENGTH
		case DM_SETINPUTNOTIFY:
		case DM_SHOWDIALOG:
		case DM_SHOWITEM:
		case DM_USER:
		// DN_*
		case DN_BTNCLICK:
		case DN_DRAGGED:
		case DN_DRAWDIALOG:
		case DN_DRAWDIALOGDONE:
		case DN_DROPDOWNOPENED:
			Param2 = (void*)(intptr_t)luaL_optint(L,pos4,0);
			break;

		case DM_LISTGETDATASIZE:
			Param2 = (void*)(intptr_t)(luaL_optint(L,pos4,1) - 1);
			break;

		case DM_LISTADDSTR:
		case DM_ADDHISTORY:
		case DM_SETHISTORY:
		case DM_SETTEXTPTR:
			Param2 = (void*)opt_utf8_string(L, pos4, NULL);
			break;

		case DM_SETCHECK:
			res = lua_isboolean(L,pos4) ? (lua_toboolean(L,pos4) ? BSTATE_CHECKED : BSTATE_UNCHECKED)
				: check_env_flag(L, pos4);
			Param2 = (void*) res;
			break;

		case DM_GETCURSORPOS:
			if (Info->SendDlgMessage(hDlg, Msg, Param1, &coord))
			{
				lua_createtable(L,0,2);
				PutNumToTable(L, "X", coord.X);
				PutNumToTable(L, "Y", coord.Y);
				return 1;
			}
			return lua_pushnil(L), 1;

		case DM_GETDIALOGINFO:
		{
			struct DialogInfo dlg_info;
			dlg_info.StructSize = sizeof(dlg_info);
			if (Info->SendDlgMessage(hDlg, Msg, Param1, &dlg_info))
			{
				lua_createtable(L,0,2);
				PutLStrToTable(L, "Id", (const char*)&dlg_info.Id, sizeof(dlg_info.Id));
				PutLStrToTable(L, "Owner", (const char*)&dlg_info.Owner, sizeof(dlg_info.Owner));
				return 1;
			}
			return lua_pushnil(L), 1;
		}

		case DM_GETDLGDATA: {
			TDialogData *dd = (TDialogData*) Info->SendDlgMessage(hDlg,Msg,0,0);
			lua_rawgeti(L, LUA_REGISTRYINDEX, dd->dataRef);
			return 1;
		}

		case DM_SETDLGDATA: {
			TDialogData *dd = (TDialogData*) Info->SendDlgMessage(hDlg,DM_GETDLGDATA,0,0);
			lua_rawgeti(L, LUA_REGISTRYINDEX, dd->dataRef);
			luaL_unref(L, LUA_REGISTRYINDEX, dd->dataRef);
			lua_pushvalue(L, pos3);
			dd->dataRef = luaL_ref(L, LUA_REGISTRYINDEX);
			return 1;
		}

		case DM_GETDLGRECT:
		case DM_GETITEMPOSITION:
			if (Info->SendDlgMessage(hDlg, Msg, Param1, &small_rect))
			{
				lua_createtable(L,0,4);
				PutNumToTable(L, "Left", small_rect.Left);
				PutNumToTable(L, "Top", small_rect.Top);
				PutNumToTable(L, "Right", small_rect.Right);
				PutNumToTable(L, "Bottom", small_rect.Bottom);
				return 1;
			}
			return lua_pushnil(L), 1;

		case DM_GETEDITPOSITION:
		{
			struct EditorSetPosition esp;
			esp.StructSize = sizeof(esp);

			if (Info->SendDlgMessage(hDlg, Msg, Param1, &esp))
				return PushEditorSetPosition(L, &esp), 1;

			return lua_pushnil(L), 1;
		}

		case DM_GETSELECTION:
		{
			struct EditorSelect es;
			es.StructSize = sizeof(es);

			if (Info->SendDlgMessage(hDlg, Msg, Param1, &es))
			{
				lua_createtable(L,0,5);
				PutNumToTable(L, "BlockType", (double) es.BlockType);
				PutNumToTable(L, "BlockStartLine", (double) es.BlockStartLine+1);
				PutNumToTable(L, "BlockStartPos", (double) es.BlockStartPos+1);
				PutNumToTable(L, "BlockWidth", (double) es.BlockWidth);
				PutNumToTable(L, "BlockHeight", (double) es.BlockHeight);
				return 1;
			}
			return lua_pushnil(L), 1;
		}

		case DM_SETSELECTION:
		{
			struct EditorSelect es;
			es.StructSize = sizeof(es);
			luaL_checktype(L, pos4, LUA_TTABLE);

			if (FillEditorSelect(L, pos4, &es))
				lua_pushinteger(L, Info->SendDlgMessage(hDlg, Msg, Param1, &es));
			else
				lua_pushinteger(L,0);

			return 1;
		}

		case DM_GETTEXT:
		case DM_GETDIALOGTITLE:
		{
			struct FarDialogItemData fdid;
			size_t size;
			fdid.StructSize = sizeof(fdid);
			fdid.PtrLength = (size_t) Info->SendDlgMessage(hDlg, Msg, Param1, NULL);
			fdid.PtrData = (wchar_t*) malloc((fdid.PtrLength+1) * sizeof(wchar_t));
			size = Info->SendDlgMessage(hDlg, Msg, Param1, &fdid);
			push_utf8_string(L, size?fdid.PtrData:L"", size);
			free(fdid.PtrData);
			return 1;
		}

		case DM_GETCONSTTEXTPTR:
		{
			wchar_t *ptr = (wchar_t*)Info->SendDlgMessage(hDlg, Msg, Param1, 0);
			push_utf8_string(L, ptr ? ptr:L"", -1);
			return 1;
		}

		case DM_SETTEXT:
		{
			struct FarDialogItemData fdid;
			fdid.StructSize = sizeof(fdid);
			fdid.PtrLength = 0;
			fdid.PtrData = (wchar_t*)check_utf8_string(L, pos4, &fdid.PtrLength);
			lua_pushinteger(L, Info->SendDlgMessage(hDlg, Msg, Param1, &fdid));
			return 1;
		}

		case DM_KEY:
		{
			size_t i, count;
			INPUT_RECORD *arr;
			if (lua_istable(L,pos4))
			{
				count = lua_objlen(L, pos4);
				arr = (INPUT_RECORD*)lua_newuserdata(L, count * sizeof(INPUT_RECORD));
				for(i=0; i<count; i++)
				{
					lua_pushinteger(L,i+1);
					lua_gettable(L,pos4);
					if (!lua_istable(L,-1))
					{
						luaL_error(L, "element #%d in argument #%d is not a table", i+1, pos4);
					}
					FillInputRecord(L, -1, arr+i);
					lua_pop(L,1);
				}
				lua_pushinteger(L, Info->SendDlgMessage(hDlg, Msg, count, arr));
			}
			else if (lua_isstring(L,pos4))
			{
				wchar_t *str = check_utf8_string(L,pos4,NULL);
				wchar_t *p, *q;
				for (p=str,count=0; *p; count++)
				{
					while(iswspace(*p)) p++;
					if (*p == 0) break;
					while(*p && !iswspace(*p)) p++;
				}
				arr = (INPUT_RECORD*)lua_newuserdata(L, count * sizeof(INPUT_RECORD));
				for(i=0,p=str; i<count; i++)
				{
					while(iswspace(*p)) p++;
					q = p;
					while(*p && !iswspace(*p)) p++;
					*p++ = 0;
					if (!pluginData->FSF->FarNameToInputRecord(q, arr+i))
						luaL_argerror(L, pos4, "invalid key");
				}
				lua_pushinteger(L, Info->SendDlgMessage(hDlg, Msg, count, arr));
			}
			else
				luaL_typerror(L, pos4, "table or string");

			return 1;
		}

		case DM_LISTADD:
		case DM_LISTSET:
		{
			luaL_checktype(L, pos4, LUA_TTABLE);
			lua_createtable(L,1,0); // "history table"
			lua_replace(L,1);
			lua_settop(L,pos4);
			Param2 = CreateList(L, 1);
			break;
		}

		case DM_LISTDELETE:
		{
			struct FarListDelete fld;
			fld.StructSize = sizeof(fld);
			if (lua_isnoneornil(L, pos4))
				lua_pushinteger(L, Info->SendDlgMessage(hDlg, Msg, Param1, NULL));
			else
			{
				luaL_checktype(L, pos4, LUA_TTABLE);
				fld.StartIndex = GetOptIntFromTable(L, "StartIndex", 1) - 1;
				fld.Count = GetOptIntFromTable(L, "Count", 1);
				lua_pushinteger(L, Info->SendDlgMessage(hDlg, Msg, Param1, &fld));
			}
			return 1;
		}

		case DM_LISTFINDSTRING:
		{
			struct FarListFind flf;
			flf.StructSize = sizeof(flf);
			luaL_checktype(L, pos4, LUA_TTABLE);
			flf.StartIndex = GetOptIntFromTable(L, "StartIndex", 1) - 1;
			lua_getfield(L, pos4, "Pattern");
			flf.Pattern = check_utf8_string(L, -1, NULL);
			lua_getfield(L, pos4, "Flags");
			flf.Flags = get_env_flag(L, -1, NULL);
			res = Info->SendDlgMessage(hDlg, Msg, Param1, &flf);
			res < 0 ? lua_pushnil(L) : lua_pushinteger(L, res+1);
			return 1;
		}

		case DM_LISTGETCURPOS:
		{
			struct FarListPos flp;
			flp.StructSize = sizeof(flp);
			Info->SendDlgMessage(hDlg, Msg, Param1, &flp);
			lua_createtable(L,0,2);
			PutIntToTable(L, "SelectPos", flp.SelectPos+1);
			PutIntToTable(L, "TopPos", flp.TopPos+1);
			return 1;
		}

		case DM_LISTGETITEM:
		{
			struct FarListGetItem flgi;
			flgi.StructSize = sizeof(flgi);
			flgi.ItemIndex = luaL_checkinteger(L, pos4) - 1;
			if (Info->SendDlgMessage(hDlg, Msg, Param1, &flgi))
			{
				lua_createtable(L,0,2);
				PutFlagsToTable(L, "Flags", flgi.Item.Flags);
				PutWStrToTable(L, "Text", flgi.Item.Text, -1);
				return 1;
			}

			return lua_pushnil(L), 1;
		}

		case DM_LISTGETTITLES:
		{
			struct FarListTitles flt;
			flt.StructSize = sizeof(flt);
			flt.Title = buf;
			flt.Bottom = buf + ARRSIZE(buf)/2;
			flt.TitleSize = ARRSIZE(buf)/2;
			flt.BottomSize = ARRSIZE(buf)/2;
			if (Info->SendDlgMessage(hDlg, Msg, Param1, &flt))
			{
				lua_createtable(L,0,2);
				PutWStrToTable(L, "Title", flt.Title, -1);
				PutWStrToTable(L, "Bottom", flt.Bottom, -1);
				return 1;
			}

			return lua_pushnil(L), 1;
		}

		case DM_LISTSETTITLES:
		{
			struct FarListTitles flt;
			flt.StructSize = sizeof(flt);
			luaL_checktype(L, pos4, LUA_TTABLE);
			lua_getfield(L, pos4, "Title");
			flt.Title = lua_isstring(L,-1) ? check_utf8_string(L,-1,NULL) : NULL;
			lua_getfield(L, pos4, "Bottom");
			flt.Bottom = lua_isstring(L,-1) ? check_utf8_string(L,-1,NULL) : NULL;
			lua_pushinteger(L, Info->SendDlgMessage(hDlg, Msg, Param1, &flt));
			return 1;
		}

		case DM_LISTINFO:
		{
			struct FarListInfo fli;
			fli.StructSize = sizeof(fli);
			if (Info->SendDlgMessage(hDlg, Msg, Param1, &fli))
			{
				lua_createtable(L,0,6);
				PutFlagsToTable(L, "Flags", fli.Flags);
				PutIntToTable(L, "ItemsNumber", fli.ItemsNumber);
				PutIntToTable(L, "SelectPos", fli.SelectPos+1);
				PutIntToTable(L, "TopPos", fli.TopPos+1);
				PutIntToTable(L, "MaxHeight", fli.MaxHeight);
				PutIntToTable(L, "MaxLength", fli.MaxLength);
				return 1;
			}
			return lua_pushnil(L), 1;
		}

		case DM_LISTINSERT:
		{
			struct FarListInsert flins;
			flins.StructSize = sizeof(flins);
			luaL_checktype(L, pos4, LUA_TTABLE);
			flins.Index = GetOptIntFromTable(L, "Index", 1) - 1;
			lua_getfield(L, pos4, "Text");
			flins.Item.Text = lua_isstring(L,-1) ? check_utf8_string(L,-1,NULL) : NULL;
			flins.Item.Flags = CheckFlagsFromTable(L, pos4, "Flags");
			res = Info->SendDlgMessage(hDlg, Msg, Param1, &flins);
			res < 0 ? lua_pushnil(L) : lua_pushinteger(L, res);
			return 1;
		}

		case DM_LISTUPDATE:
		{
			struct FarListUpdate flu;
			flu.StructSize = sizeof(flu);
			luaL_checktype(L, pos4, LUA_TTABLE);
			flu.Index = GetOptIntFromTable(L, "Index", 1) - 1;
			lua_getfield(L, pos4, "Text");
			flu.Item.Text = lua_isstring(L,-1) ? check_utf8_string(L,-1,NULL) : NULL;
			flu.Item.Flags = CheckFlagsFromTable(L, pos4, "Flags");
			lua_pushboolean(L, Info->SendDlgMessage(hDlg, Msg, Param1, &flu) != 0);
			return 1;
		}

		case DM_LISTSETCURPOS:
		{
			struct FarListPos flp;
			flp.StructSize = sizeof(flp);
			luaL_checktype(L, pos4, LUA_TTABLE);
			flp.SelectPos = GetOptIntFromTable(L, "SelectPos", 1) - 1;
			flp.TopPos = GetOptIntFromTable(L, "TopPos", 1) - 1;
			lua_pushinteger(L, 1 + Info->SendDlgMessage(hDlg, Msg, Param1, &flp));
			return 1;
		}

		case DM_LISTSETDATA:
		{
			listdata_t Data, *oldData;
			intptr_t Index;
			struct FarListItemData flid;

			luaL_checktype(L, pos4, LUA_TTABLE);
			Index = GetOptIntFromTable(L, "Index", 1) - 1;
			lua_getfenv(L, 1);
			lua_getfield(L, pos4, "Data");
			if (lua_isnil(L,-1)) // nil is not allowed
			{
				lua_pushinteger(L,0);
				return 1;
			}

			oldData = (listdata_t*)Info->SendDlgMessage(hDlg, DM_LISTGETDATA, Param1, (void*)Index);
			if (oldData &&
				sizeof(listdata_t) == Info->SendDlgMessage(hDlg, DM_LISTGETDATASIZE, Param1, (void*)Index) &&
				oldData->Id == pluginData)
			{
				luaL_unref(L, -2, oldData->Ref);
			}
			Data.Id = pluginData;
			Data.Ref = luaL_ref(L, -2);
			flid.StructSize = sizeof(flid);
			flid.Index = Index;
			flid.Data = &Data;
			flid.DataSize = sizeof(Data);
			lua_pushinteger(L, Info->SendDlgMessage(hDlg, Msg, Param1, &flid));
			return 1;
		}

		case DM_LISTGETDATA:
		{
			intptr_t Index = luaL_checkinteger(L, pos4) - 1;
			listdata_t *Data = (listdata_t*)Info->SendDlgMessage(hDlg, Msg, Param1, (void*)Index);
			if (Data)
			{
				if (sizeof(listdata_t) == Info->SendDlgMessage(hDlg, DM_LISTGETDATASIZE, Param1, (void*)Index) &&
					Data->Id == pluginData)
				{
					lua_getfenv(L, 1);
					lua_rawgeti(L, -1, Data->Ref);
				}
				else
					lua_pushlightuserdata(L, Data);
			}
			else
				lua_pushnil(L);

			return 1;
		}

		case DM_GETDLGITEM:
			return PushDlgItemNum(L, hDlg, (int)Param1, pos4, Info), 1;

		case DM_SETDLGITEM:
			return SetDlgItem(L, hDlg, (int)Param1, pos4, Info);

		case DM_MOVEDIALOG:
		case DM_RESIZEDIALOG:
		case DM_SETCURSORPOS:
		{
			COORD *c;
			luaL_checktype(L, pos4, LUA_TTABLE);
			coord.X = GetOptIntFromTable(L, "X", 0);
			coord.Y = GetOptIntFromTable(L, "Y", 0);

			if (Msg == DM_SETCURSORPOS)
			{
				lua_pushinteger(L, Info->SendDlgMessage(hDlg, Msg, Param1, &coord));
				return 1;
			}
			c = (COORD*) Info->SendDlgMessage(hDlg, Msg, Param1, &coord);
			lua_createtable(L, 0, 2);
			PutIntToTable(L, "X", c->X);
			PutIntToTable(L, "Y", c->Y);
			return 1;
		}

		case DM_SETITEMPOSITION:
			luaL_checktype(L, pos4, LUA_TTABLE);
			small_rect.Left = GetOptIntFromTable(L, "Left", 0);
			small_rect.Top = GetOptIntFromTable(L, "Top", 0);
			small_rect.Right = GetOptIntFromTable(L, "Right", 0);
			small_rect.Bottom = GetOptIntFromTable(L, "Bottom", 0);
			Param2 = &small_rect;
			break;

		case DM_SETCOMBOBOXEVENT:
			Param2 = (void*)(intptr_t)OptFlags(L, pos4, 0);
			break;

		case DM_SETEDITPOSITION:
		{
			struct EditorSetPosition esp;
			esp.StructSize = sizeof(esp);
			luaL_checktype(L, pos4, LUA_TTABLE);
			lua_settop(L, pos4);
			FillEditorSetPosition(L, &esp);
			lua_pushinteger(L, Info->SendDlgMessage(hDlg, Msg, Param1, &esp));
			return 1;
		}

		case DN_CONTROLINPUT:
		{
			INPUT_RECORD rec;
			OptInputRecord(L, pluginData, pos4, &rec);
			lua_pushinteger(L, Info->SendDlgMessage(hDlg, Msg, Param1, &rec));
			return 1;
		}
	}

	res = Info->SendDlgMessage(hDlg, Msg, Param1, Param2);
	lua_pushinteger(L, res + res_incr);
	return 1;
}

#define DlgMethod(name,msg) \
static int dlg_##name(lua_State *L) { return DoSendDlgMessage(L,msg,1); }

static int far_SendDlgMessage(lua_State *L) { return DoSendDlgMessage(L,0,0); }

DlgMethod( AddHistory,             DM_ADDHISTORY)
DlgMethod( Close,                  DM_CLOSE)
DlgMethod( EditUnchangedFlag,      DM_EDITUNCHANGEDFLAG)
DlgMethod( Enable,                 DM_ENABLE)
DlgMethod( EnableRedraw,           DM_ENABLEREDRAW)
DlgMethod( GetCheck,               DM_GETCHECK)
DlgMethod( GetComboboxEvent,       DM_GETCOMBOBOXEVENT)
DlgMethod( GetConstTextPtr,        DM_GETCONSTTEXTPTR)
DlgMethod( GetCursorPos,           DM_GETCURSORPOS)
DlgMethod( GetCursorSize,          DM_GETCURSORSIZE)
DlgMethod( GetDialogInfo,          DM_GETDIALOGINFO)
DlgMethod( GetDialogTitle,         DM_GETDIALOGTITLE)
DlgMethod( GetDlgData,             DM_GETDLGDATA)
DlgMethod( GetDlgItem,             DM_GETDLGITEM)
DlgMethod( GetDlgRect,             DM_GETDLGRECT)
DlgMethod( GetDropdownOpened,      DM_GETDROPDOWNOPENED)
DlgMethod( GetEditPosition,        DM_GETEDITPOSITION)
DlgMethod( GetFocus,               DM_GETFOCUS)
DlgMethod( GetItemData,            DM_GETITEMDATA)
DlgMethod( GetItemPosition,        DM_GETITEMPOSITION)
DlgMethod( GetSelection,           DM_GETSELECTION)
DlgMethod( GetText,                DM_GETTEXT)
DlgMethod( Key,                    DM_KEY)
DlgMethod( ListAdd,                DM_LISTADD)
DlgMethod( ListAddStr,             DM_LISTADDSTR)
DlgMethod( ListDelete,             DM_LISTDELETE)
DlgMethod( ListFindString,         DM_LISTFINDSTRING)
DlgMethod( ListGetCurPos,          DM_LISTGETCURPOS)
DlgMethod( ListGetData,            DM_LISTGETDATA)
DlgMethod( ListGetDataSize,        DM_LISTGETDATASIZE)
DlgMethod( ListGetItem,            DM_LISTGETITEM)
DlgMethod( ListGetTitles,          DM_LISTGETTITLES)
DlgMethod( ListInfo,               DM_LISTINFO)
DlgMethod( ListInsert,             DM_LISTINSERT)
DlgMethod( ListSet,                DM_LISTSET)
DlgMethod( ListSetCurPos,          DM_LISTSETCURPOS)
DlgMethod( ListSetData,            DM_LISTSETDATA)
DlgMethod( ListSetTitles,          DM_LISTSETTITLES)
DlgMethod( ListSort,               DM_LISTSORT)
DlgMethod( ListUpdate,             DM_LISTUPDATE)
DlgMethod( MoveDialog,             DM_MOVEDIALOG)
DlgMethod( Redraw,                 DM_REDRAW)
DlgMethod( ResizeDialog,           DM_RESIZEDIALOG)
DlgMethod( Set3State,              DM_SET3STATE)
DlgMethod( SetCheck,               DM_SETCHECK)
DlgMethod( SetComboboxEvent,       DM_SETCOMBOBOXEVENT)
DlgMethod( SetCursorPos,           DM_SETCURSORPOS)
DlgMethod( SetCursorSize,          DM_SETCURSORSIZE)
DlgMethod( SetDlgData,             DM_SETDLGDATA)
DlgMethod( SetDlgItem,             DM_SETDLGITEM)
DlgMethod( SetDropdownOpened,      DM_SETDROPDOWNOPENED)
DlgMethod( SetEditPosition,        DM_SETEDITPOSITION)
DlgMethod( SetFocus,               DM_SETFOCUS)
DlgMethod( SetHistory,             DM_SETHISTORY)
DlgMethod( SetInputNotify,         DM_SETINPUTNOTIFY)
DlgMethod( SetItemData,            DM_SETITEMDATA)
DlgMethod( SetItemPosition,        DM_SETITEMPOSITION)
DlgMethod( SetMaxTextLength,       DM_SETMAXTEXTLENGTH)
DlgMethod( SetSelection,           DM_SETSELECTION)
DlgMethod( SetText,                DM_SETTEXT)
DlgMethod( SetTextPtr,             DM_SETTEXTPTR)
DlgMethod( ShowDialog,             DM_SHOWDIALOG)
DlgMethod( ShowItem,               DM_SHOWITEM)
DlgMethod( User,                   DM_USER)

int PushDNParams (lua_State *L, intptr_t Msg, intptr_t Param1, void *Param2)
{
	// Param1
	switch(Msg)
	{
		case DN_CTLCOLORDIALOG:
		case DN_DRAGGED:
		case DN_DRAWDIALOG:
		case DN_DRAWDIALOGDONE:
		case DN_INPUT:
		case DN_RESIZECONSOLE:
			break;

		case DN_CLOSE:
		case DN_CONTROLINPUT:
		case DN_GOTFOCUS:
		case DN_KILLFOCUS:
			if (Param1 >= 0)
				++Param1;
			break;

		case DN_BTNCLICK:
		case DN_CTLCOLORDLGITEM:
		case DN_CTLCOLORDLGLIST:
		case DN_DRAWDLGITEM:
		case DN_DRAWDLGITEMDONE:
		case DN_DROPDOWNOPENED:
		case DN_EDITCHANGE:
		case DN_GETVALUE:
		case DN_HELP:
		case DN_HOTKEY:
		case DN_INITDIALOG:
		case DN_LISTCHANGE:
		case DN_LISTHOTKEY:
			++Param1; // dialog element position
			break;

		default:
			return FALSE;
	}

	lua_pushinteger(L, Msg);       //+1
	lua_pushinteger(L, Param1);    //+2

	// Param2
	switch(Msg)
	{
		case DN_CONTROLINPUT:   // TODO
		case DN_INPUT:          // TODO was: (Msg == DN_MOUSEEVENT)
		case DN_HOTKEY:
			PushInputRecord(L, (const INPUT_RECORD*)Param2);
			break;

		case DN_CTLCOLORDIALOG:
			PushFarColor(L, (struct FarColor*) Param2);
			break;

		case DN_CTLCOLORDLGITEM:
		case DN_CTLCOLORDLGLIST:
		{
			int i;
			struct FarDialogItemColors* fdic = (struct FarDialogItemColors*) Param2;
			lua_createtable(L, (int)fdic->ColorsCount, 1);
			PutFlagsToTable(L, "Flags", fdic->Flags);
			for(i=0; i < (int)fdic->ColorsCount; i++)
			{
				PushFarColor(L, &fdic->Colors[i]);
				lua_rawseti(L, -2, i+1);
			}
			break;
		}

		case DN_DRAWDLGITEM:
		case DN_EDITCHANGE:
			PushDlgItem(L, (struct FarDialogItem*)Param2, FALSE);
			break;

		case DN_GETVALUE:
		{
			struct FarGetValue *fgv = (struct FarGetValue*) Param2;
			lua_newtable(L);
			PutIntToTable(L, "GetType", fgv->Type);
			PutIntToTable(L, "ValType", fgv->Value.Type);
			PushFarMacroValue(L, &fgv->Value);
			lua_setfield(L, -2, "Value");
			break;
		}

		case DN_HELP:
			push_utf8_string(L, Param2 ? (wchar_t*)Param2 : L"", -1);
			break;

		case DN_LISTCHANGE:
		case DN_LISTHOTKEY:
			lua_pushinteger(L, (intptr_t)Param2+1);  // make list positions 1-based
			break;

		case DN_RESIZECONSOLE:
		{
			COORD* coord = (COORD*)Param2;
			lua_createtable(L, 0, 2);
			PutIntToTable(L, "X", coord->X);
			PutIntToTable(L, "Y", coord->Y);
			break;
		}

		default:
			lua_pushinteger(L, (intptr_t)Param2);  //+3
			break;
	}

	return TRUE;
}

intptr_t ProcessDNResult(lua_State *L, intptr_t Msg, void *Param2)
{
	intptr_t ret = 0;
	switch(Msg)
	{
		case DN_CTLCOLORDLGLIST:
		case DN_CTLCOLORDLGITEM:
			if ((ret = lua_istable(L,-1)) != 0)
			{
				struct FarDialogItemColors* fdic = (struct FarDialogItemColors*) Param2;
				int i;
				size_t len = lua_objlen(L, -1);

				if (len > fdic->ColorsCount) len = fdic->ColorsCount;

				for(i = 0; i < (int)len; i++)
				{
					lua_rawgeti(L, -1, i+1);
					GetFarColor(L, -1, &fdic->Colors[i]);
					lua_pop(L, 1);
				}
			}
			break;

		case DN_CTLCOLORDIALOG:
			ret = GetFarColor(L, -1, (struct FarColor*)Param2);
			break;

		case DN_HELP:
			if ((ret = (intptr_t)utf8_to_utf16(L, -1, NULL)) != 0)
			{
				lua_getfield(L, LUA_REGISTRYINDEX, FAR_DN_STORAGE);
				lua_pushvalue(L, -2);                // keep stack balanced
				lua_setfield(L, -2, "helpstring");   // protect from garbage collector
				lua_pop(L, 1);
			}
			break;

		case DN_GETVALUE:
			if ((ret = lua_istable(L,-1)) != 0)
			{
				struct FarMacroValue tempValue;
				struct FarGetValue *fgv = (struct FarGetValue*) Param2;
				lua_getfield(L, -1, "Value");
				ConvertLuaValue(L, -1, &tempValue);
				if (tempValue.Type == fgv->Value.Type)
				{
					fgv->Value = tempValue;
					if (fgv->Value.Type == FMVT_STRING)
					{
						lua_getfield(L, LUA_REGISTRYINDEX, FAR_DN_STORAGE);
						lua_pushvalue(L, -2);                   // keep stack balanced
						lua_setfield(L, -2, "getvaluestring");  // protect from garbage collector
						lua_pop(L, 1);
					}
				}
				else if (tempValue.Type==FMVT_DOUBLE && fgv->Value.Type==FMVT_INTEGER)
					fgv->Value.Value.Integer = (__int64)tempValue.Value.Double;
				else
					ret = 0;

				lua_pop(L, 1);
			}
			break;

		case DN_KILLFOCUS:
			ret = lua_tointeger(L, -1) - 1;
			break;

		default:
			ret = lua_isnumber(L, -1) ? lua_tointeger(L, -1) : (lua_Integer)lua_toboolean(L, -1);
			break;
	}
	return ret;
}

static intptr_t DoDlgProc(lua_State *L, PSInfo *Info, TDialogData *dd, HANDLE hDlg, intptr_t Msg, intptr_t Param1, void *Param2)
{
	intptr_t ret;

	if (!dd || dd->wasError)
		return Info->DefDlgProc(hDlg, Msg, Param1, Param2);

	lua_pushlightuserdata(L, dd);        //+1   retrieve the table
	lua_rawget(L, LUA_REGISTRYINDEX);    //+1
	lua_rawgeti(L, -1, 2);               //+2   retrieve the procedure
	lua_rawgeti(L, -2, 3);               //+3   retrieve the handle
	lua_remove(L, -3);                   //+2

	if (Msg == DN_INITDIALOG) {
		lua_pushinteger(L, Msg);                         //+3
		lua_pushinteger(L, Param1 + 1);                  //+4
		lua_rawgeti(L, LUA_REGISTRYINDEX, dd->dataRef);  //+5
	}
	else {
		if (!PushDNParams(L, Msg, Param1, Param2)) {     //+5
			lua_pop(L, 2);
			return Info->DefDlgProc(hDlg, Msg, Param1, Param2);
		}
	}

	if (pcall_msg(L, 4, 1))  //+2
	{
		dd->wasError = TRUE;
		Info->SendDlgMessage(hDlg, DM_CLOSE, -1, 0);
		return Info->DefDlgProc(hDlg, Msg, Param1, Param2);
	}

	ret = lua_isnil(L, -1) ?
		Info->DefDlgProc(hDlg, Msg, Param1, Param2) :
		ProcessDNResult(L, Msg, Param2);

	lua_pop(L, 1);
	return ret;
}

static void RemoveDialogFromRegistry(lua_State *L, TDialogData *dd)
{
	luaL_unref(dd->L, LUA_REGISTRYINDEX, dd->dataRef);
	dd->hDlg = INVALID_HANDLE_VALUE;
	lua_pushlightuserdata(L, dd);
	lua_pushnil(L);
	lua_rawset(L, LUA_REGISTRYINDEX);
}

static __inline BOOL NonModal(TDialogData *dd)
{
	return dd && !dd->isModal;
}

intptr_t LF_DlgProc(lua_State *L, HANDLE hDlg, intptr_t Msg, intptr_t Param1, void *Param2)
{
	intptr_t ret;
	PSInfo *Info = GetPluginData(L)->Info;
	TDialogData *dd = (TDialogData*) Info->SendDlgMessage(hDlg,DM_GETDLGDATA,0,0);

	if (Msg == DN_INITDIALOG && dd->hDlg == INVALID_HANDLE_VALUE)
	{
		dd->hDlg = hDlg;
	}

	if (dd)
	{
		L = dd->L; // the dialog may be called from a lua_State other than the main one
	}
	ret = DoDlgProc(L, Info, dd, hDlg, Msg, Param1, Param2);
	if (Msg == DN_CLOSE && ret && NonModal(dd))
	{
		Info->SendDlgMessage(hDlg, DM_SETDLGDATA, 0, 0);
		RemoveDialogFromRegistry(L, dd);
	}
	return ret;
}

static int far_DialogInit(lua_State *L)
{
	enum { POS_HISTORIES=1, POS_ITEMS=2 };
	intptr_t ItemsNumber, i;
	struct FarDialogItem *Items;
	UINT64 Flags;
	TDialogData *dd;
	FARAPIDEFDLGPROC Proc;
	void *Param;
	TPluginData *pd = GetPluginData(L);
	GUID Id;
	intptr_t X1, Y1, X2, Y2;
	const wchar_t *HelpTopic;

	memset(&Id, 0, sizeof(Id));
	if (lua_type(L,1) == LUA_TSTRING) {
		if (lua_objlen(L,1) >= sizeof(GUID))
			Id = *(const GUID*)lua_tostring(L, 1);
	}
	else if (!lua_isnoneornil(L,1))
		return luaL_typerror(L, 1, "optional string");

	X1 = luaL_checkinteger(L, 2);
	Y1 = luaL_checkinteger(L, 3);
	X2 = luaL_checkinteger(L, 4);
	Y2 = luaL_checkinteger(L, 5);
	HelpTopic = opt_utf8_string(L, 6, NULL);

	luaL_checktype(L, 7, LUA_TTABLE);
	lua_newtable(L);  // create a "histories" table, to prevent history strings
	// from being garbage collected too early
	lua_replace(L, POS_HISTORIES);
	ItemsNumber = lua_objlen(L, 7);
	Items = (struct FarDialogItem*)lua_newuserdata(L, ItemsNumber * sizeof(struct FarDialogItem));
	lua_replace(L, POS_ITEMS);

	for(i=0; i < ItemsNumber; i++) {
		lua_pushinteger(L, i+1);
		lua_gettable(L, 7);
		if (lua_type(L, -1) == LUA_TTABLE) {
			SetFarDialogItem(L, Items+i, (int)i, POS_HISTORIES);
			lua_pop(L, 1);
		}
		else
			return luaL_error(L, "Items[%d] is not a table", (int)i+1);
	}

	// 8-th parameter (flags)
	Flags = OptFlags(L, 8, 0);
	dd = NewDialogData(L, pd->Info, INVALID_HANDLE_VALUE, TRUE);
	// 9-th parameter (DlgProc function)
	Proc = NULL;
	Param = NULL;

	if (lua_isfunction(L, 9))
	{
		Proc = pd->DlgProc;
		Param = dd;
		if (lua_gettop(L) >= 10) {
			lua_pushvalue(L,10);
			dd->dataRef = luaL_ref(L, LUA_REGISTRYINDEX);
		}
	}

	// Put some values into the registry
	lua_pushlightuserdata(L, dd); // important: index it with dd
	lua_createtable(L, 3, 0);
	lua_pushvalue(L, POS_HISTORIES);  // store the "histories" table
	lua_rawseti(L, -2, 1);

	if (lua_isfunction(L, 9))
	{
		lua_pushvalue(L, 9);    // store the procedure
		lua_rawseti(L, -2, 2);
		lua_pushvalue(L, -3);   // store the handle
		lua_rawseti(L, -2, 3);
	}

	lua_rawset(L, LUA_REGISTRYINDEX);

	dd->hDlg = pd->Info->DialogInit(pd->PluginId, &Id, X1, Y1, X2, Y2, HelpTopic,
	                                Items, ItemsNumber, 0, Flags, Proc, Param);

	if (dd->hDlg == INVALID_HANDLE_VALUE)
	{
		RemoveDialogFromRegistry(L, dd);
		lua_pushnil(L);
	}
	else
	{
		dd->isModal = (Flags&FDLG_NONMODAL) == 0;
	}

	return 1;
}

static void free_dialog(TDialogData* dd)
{
	lua_State* L = dd->L;

	if (dd->isOwned && dd->isModal && dd->hDlg != INVALID_HANDLE_VALUE)
	{
		dd->Info->DialogFree(dd->hDlg);
		RemoveDialogFromRegistry(L, dd);
	}
}

static int far_DialogRun(lua_State *L)
{
	TDialogData* dd = CheckValidDialog(L, 1);
	intptr_t result = dd->Info->DialogRun(dd->hDlg);
	if (result >= 0) ++result;

	if (dd->wasError)
	{
		free_dialog(dd);
		luaL_error(L, "error occured in dialog procedure");
	}

	lua_pushinteger(L, (int)result);
	return 1;
}

static int far_DialogFree(lua_State *L)
{
	free_dialog(CheckDialog(L, 1));
	return 0;
}

static int dialog_tostring(lua_State *L)
{
	TDialogData* dd = CheckDialog(L, 1);

	if (dd->hDlg != INVALID_HANDLE_VALUE)
		lua_pushfstring(L, "%s (%p)", FarDialogType, dd->hDlg);
	else
		lua_pushfstring(L, "%s (closed)", FarDialogType);

	return 1;
}

static int dialog_rawhandle(lua_State *L)
{
	TDialogData* dd = CheckDialog(L, 1);

	if (dd->hDlg != INVALID_HANDLE_VALUE)
		lua_pushlightuserdata(L, dd->hDlg);
	else
		lua_pushnil(L);

	return 1;
}

static int far_GetDlgItem(lua_State *L)
{
	PSInfo *Info = GetPluginData(L)->Info;
	HANDLE hDlg = CheckDialogHandle(L,1);
	int numitem = (int)luaL_checkinteger(L,2) - 1;
	PushDlgItemNum(L, hDlg, numitem, 3, Info);
	return 1;
}

static int far_SetDlgItem(lua_State *L)
{
	PSInfo *Info = GetPluginData(L)->Info;
	HANDLE hDlg = CheckDialogHandle(L,1);
	int numitem = (int)luaL_checkinteger(L,2) - 1;
	return SetDlgItem(L, hDlg, numitem, 3, Info);
}

static int far_SubscribeDialogDrawEvents(lua_State *L)
{
	GetPluginData(L)->Flags |= PDF_DIALOGEVENTDRAWENABLE;
	return 0;
}

static int editor_Editor(lua_State *L)
{
	PSInfo *Info = GetPluginData(L)->Info;
	const wchar_t* FileName = check_utf8_string(L, 1, NULL);
	const wchar_t* Title    = opt_utf8_string(L, 2, NULL);
	intptr_t X1 = luaL_optinteger(L, 3, 0);
	intptr_t Y1 = luaL_optinteger(L, 4, 0);
	intptr_t X2 = luaL_optinteger(L, 5, -1);
	intptr_t Y2 = luaL_optinteger(L, 6, -1);
	UINT64 Flags = OptFlags(L,7,0);
	intptr_t StartLine = luaL_optinteger(L, 8, -1);
	intptr_t StartChar = luaL_optinteger(L, 9, -1);
	intptr_t CodePage  = luaL_optinteger(L, 10, CP_DEFAULT);
	intptr_t ret = Info->Editor(FileName, Title, X1, Y1, X2, Y2, Flags,
	                            StartLine, StartChar, CodePage);
	lua_pushinteger(L, (int)ret);
	return 1;
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
	UINT64 Flags = OptFlags(L, 7, 0);
	intptr_t CodePage = luaL_optinteger(L, 8, CP_DEFAULT);
	intptr_t ret = Info->Viewer(FileName, Title, X1, Y1, X2, Y2, Flags, CodePage);
	lua_pushboolean(L, ret != 0);
	return 1;
}

static int viewer_GetFileName(lua_State *L)
{
	intptr_t ViewerId = luaL_optinteger(L, 1, -1);

	if (!push_ev_filename(L, 0, ViewerId)) lua_pushnil(L);

	return 1;
}

static int viewer_GetInfo(lua_State *L)
{
	intptr_t ViewerId = luaL_optinteger(L, 1, -1);
	PSInfo *Info = GetPluginData(L)->Info;
	struct ViewerInfo vi;
	vi.StructSize = sizeof(vi);

	if (Info->ViewerControl(ViewerId, VCTL_GETINFO, 0, &vi))
	{
		lua_createtable(L, 0, 10);
		PutNumToTable(L, "ViewerID", (double) vi.ViewerID);

		if (push_ev_filename(L, 0, ViewerId))
			lua_setfield(L, -2, "FileName");

		PutNumToTable(L,  "FileSize", (double) vi.FileSize);
		PutNumToTable(L,  "FilePos", (double) vi.FilePos);
		PutNumToTable(L,  "WindowSizeX", (double) vi.WindowSizeX);
		PutNumToTable(L,  "WindowSizeY", (double) vi.WindowSizeY);
		PutNumToTable(L,  "Options", (double) vi.Options);
		PutNumToTable(L,  "TabSize", (double) vi.TabSize);
		PutNumToTable(L,  "LeftPos", (double) vi.LeftPos + 1);
		lua_createtable(L, 0, 3);
		PutNumToTable(L, "CodePage", (double) vi.CurMode.CodePage);
		PutFlagsToTable(L, "Flags",    vi.CurMode.Flags);
		PutNumToTable(L, "ViewMode", (double) vi.CurMode.ViewMode);
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
	Info->ViewerControl(ViewerId, VCTL_QUIT, 0, 0);
	return 0;
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
	struct ViewerSelect vs;
	vs.StructSize = sizeof(vs);
	vs.BlockStartPos = (INT64)luaL_checknumber(L,2);
	vs.BlockLen = (INT64)luaL_checknumber(L,3);
	lua_pushboolean(L, Info->ViewerControl(ViewerId, VCTL_SELECT, 0, &vs) != 0);
	return 1;
}

static int viewer_SetPosition(lua_State *L)
{
	intptr_t ViewerId = luaL_optinteger(L, 1, -1);
	PSInfo *Info = GetPluginData(L)->Info;
	struct ViewerSetPosition vsp;
	vsp.StructSize = sizeof(vsp);

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
	intptr_t ViewerId;
	struct ViewerSetMode vsm;
	memset(&vsm, 0, sizeof(vsm));
	vsm.StructSize = sizeof(vsm);
	ViewerId = luaL_optinteger(L, 1, -1);
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

static int far_ShowHelp(lua_State *L)
{
	const wchar_t *ModuleName = (const wchar_t*)luaL_checkstring(L, 1);
	const wchar_t *HelpTopic = opt_utf8_string(L,2,NULL);
	UINT64 Flags = OptFlags(L,3,0);
	PSInfo *Info = GetPluginData(L)->Info;
	if ((Flags & FHELP_GUID) == 0)
		ModuleName = check_utf8_string(L,1,NULL);
	lua_pushboolean(L, Info->ShowHelp(ModuleName, HelpTopic, Flags));
	return 1;
}

// DestText = far.InputBox(Title,Prompt,HistoryName,SrcText,DestLength,HelpTopic,Flags)
// all arguments are optional
static int far_InputBox(lua_State *L)
{
	TPluginData *pd = GetPluginData(L);
	const GUID *Id = (lua_type(L,1)==LUA_TSTRING && lua_objlen(L,1)==sizeof(GUID)) ?
	                 (const GUID*)lua_tostring(L, 1) : pd->PluginId;
	const wchar_t *Title       = opt_utf8_string(L, 2, L"Input Box");
	const wchar_t *Prompt      = opt_utf8_string(L, 3, L"Enter the text:");
	const wchar_t *HistoryName = opt_utf8_string(L, 4, NULL);
	const wchar_t *SrcText     = opt_utf8_string(L, 5, L"");
	intptr_t DestLength        = luaL_optinteger(L, 6, 1024);
	const wchar_t *HelpTopic   = opt_utf8_string(L, 7, NULL);
	UINT64 Flags = OptFlags(L, 8, FIB_ENABLEEMPTY|FIB_BUTTONS|FIB_NOAMPERSAND);
	wchar_t *DestText;
	intptr_t res;

	if (DestLength < 0) DestLength = 0;

	DestText = (wchar_t*) malloc(sizeof(wchar_t)*(DestLength+1));
	res = pd->Info->InputBox(pd->PluginId, Id, Title, Prompt, HistoryName, SrcText,
	                         DestText, DestLength+1, HelpTopic, Flags);

	if (res) push_utf8_string(L, DestText, -1);
	else lua_pushnil(L);

	free(DestText);
	return 1;
}

static int far_GetMsg(lua_State *L)
{
	intptr_t MsgId = luaL_checkinteger(L, 1);
	if (MsgId >= 0)
	{
		GUID guid;
		TPluginData *pd = GetPluginData(L);
		const wchar_t* str;

		GetOptGuid(L, 2, &guid, pd->PluginId);
		str = pd->Info->GetMsg(&guid, MsgId);
		if (str)
			push_utf8_string(L, str, -1);
		else
			lua_pushnil(L);
	}
	else
		lua_pushnil(L); // (MsgId < 0) crashes FAR

	return 1;
}

static int far_Text(lua_State *L)
{
	PSInfo *Info = GetPluginData(L)->Info;
	const wchar_t *Str;
	struct FarColor fc = { FCF_4BITMASK, {0x0F}, {0x00} };
	intptr_t X = luaL_optinteger(L, 1, 0);
	intptr_t Y = luaL_optinteger(L, 2, 0);
	GetFarColor(L, 3, &fc);
	Str = opt_utf8_string(L, 4, NULL);
	Info->Text(X, Y, &fc, Str);
	return 0;
}

static int far_CopyToClipboard(lua_State *L)
{
	int ret;
	if (lua_isnoneornil(L,1))
		ret = GetPluginData(L)->FSF->CopyToClipboard(FCT_STREAM,NULL);
	else
	{
		const wchar_t *str = check_utf8_string(L,1,NULL);
		enum FARCLIPBOARD_TYPE type = (enum FARCLIPBOARD_TYPE) OptFlags(L,2,FCT_STREAM);
		ret = GetPluginData(L)->FSF->CopyToClipboard(type,str);
	}
	return lua_pushboolean(L, ret), 1;
}

static int far_PasteFromClipboard(lua_State *L)
{
	struct FarStandardFunctions *FSF = GetPluginData(L)->FSF;
	enum FARCLIPBOARD_TYPE type = (enum FARCLIPBOARD_TYPE) OptFlags(L,1,FCT_ANY);
	size_t len = FSF->PasteFromClipboard(type,NULL,0);

	if (len)
	{
		wchar_t *buf = (wchar_t*) malloc(len * sizeof(wchar_t));

		if (buf)
		{
			FSF->PasteFromClipboard(type,buf,len);
			push_utf8_string(L,buf,len-1);
			free(buf);
			return 1;
		}
	}

	lua_pushnil(L);
	return 1;
}

static int far_InputRecordToName(lua_State *L)
{
	wchar_t buf[256];
	INPUT_RECORD ir;
	size_t result;
	FillInputRecord(L, 1, &ir);
	result = GetPluginData(L)->FSF->FarInputRecordToName(&ir, buf, ARRSIZE(buf)-1);

	if (result > 0)
	{
		if (lua_toboolean(L, 2))
		{
			static const char C[]="RCtrl", A[]="RAlt", S[]="Shift";
			const char *p;
			push_utf8_string(L, buf, -1);
			p = lua_tostring(L, -1);

			if (!strncmp(p, C+1, 4))       { lua_pushstring(L, C+1);  p += 4; }
			else if (!strncmp(p, C, 5))    { lua_pushstring(L, C); p += 5; }
			else lua_pushboolean(L, 0);

			if (!strncmp(p, A+1, 3))       { lua_pushstring(L, A+1);  p += 3; }
			else if (!strncmp(p, A, 4))    { lua_pushstring(L, A); p += 4; }
			else lua_pushboolean(L, 0);

			if (!strncmp(p, S, 5))         { lua_pushstring(L, S); p += 5; }
			else lua_pushboolean(L, 0);

			*p ? lua_pushstring(L, p) : lua_pushboolean(L, 0);
			return 4;
		}
		else
			push_utf8_string(L, buf, -1);
	}
	else
		lua_pushnil(L);

	return 1;
}

static int far_NameToInputRecord(lua_State *L)
{
	INPUT_RECORD ir;
	const wchar_t* str = check_utf8_string(L, 1, NULL);

	if (GetPluginData(L)->FSF->FarNameToInputRecord(str, &ir))
		PushInputRecord(L, &ir);
	else
		lua_pushnil(L);

	return 1;
}

static int far_LStricmp(lua_State *L)
{
	const wchar_t* s1 = check_utf8_string(L, 1, NULL);
	const wchar_t* s2 = check_utf8_string(L, 2, NULL);
	lua_pushinteger(L, GetPluginData(L)->FSF->LStricmp(s1, s2));
	return 1;
}

static int far_LStrnicmp(lua_State *L)
{
	const wchar_t* s1 = check_utf8_string(L, 1, NULL);
	const wchar_t* s2 = check_utf8_string(L, 2, NULL);
	intptr_t num = luaL_checkinteger(L, 3);

	if (num < 0) num = 0;

	lua_pushinteger(L, GetPluginData(L)->FSF->LStrnicmp(s1, s2, num));
	return 1;
}

// Result = far.ProcessName (Op, Mask, Name, Flags, Size)
//   @Op: PN_CMPNAME, PN_CMPNAMELIST, PN_GENERATENAME, PN_CHECKMASK
//   @Mask: string
//   @Name: string
//   @Flags: PN_SKIPPATH, PN_SHOWERRORMESSAGE
//   @Size: integer 0...0xFFFF
//   @Result: boolean
static int _ProcessName (lua_State *L, UINT64 Op)
{
  struct FarStandardFunctions *FSF = GetPluginData(L)->FSF;

	int pos2=2, pos3=3, pos4=4;
	if (Op == 0xFFFFFFFF)
		Op = CheckFlags(L, 1);
	else {
		--pos2, --pos3, --pos4;
		if (Op == PN_CHECKMASK)
			--pos4;
	}
	const wchar_t* Mask = check_utf8_string(L, pos2, NULL);
	const wchar_t* Name = (Op == PN_CHECKMASK) ? L"" : check_utf8_string(L, pos3, NULL);
	int Flags = Op | OptFlags(L, pos4, 0);

	if (Op == PN_CMPNAME || Op == PN_CMPNAMELIST || Op == PN_CHECKMASK) {
		size_t result = FSF->ProcessName(Mask, (wchar_t*)Name, 0, Flags);
		lua_pushboolean(L, (int)result);
	}
	else if (Op == PN_GENERATENAME) {
		UINT64 Size = luaL_optinteger(L, pos4+1, 0) & 0xFFFF;
		const int BUFSIZE = 1024;
		wchar_t* buf = (wchar_t*)lua_newuserdata(L, BUFSIZE * sizeof(wchar_t));
		wcsncpy(buf, Mask, BUFSIZE-1);
		buf[BUFSIZE-1] = 0;

		size_t result = FSF->ProcessName(Name, buf, BUFSIZE, Flags|Size);
		if (result)
			push_utf8_string(L, buf, -1);
		else
			lua_pushboolean(L, (int)result);
	}
	else
		luaL_argerror(L, 1, "command not supported");

	return 1;
}

static int far_ProcessName  (lua_State *L) { return _ProcessName(L, 0xFFFFFFFF);      }
static int far_CmpName      (lua_State *L) { return _ProcessName(L, PN_CMPNAME);      }
static int far_CmpNameList  (lua_State *L) { return _ProcessName(L, PN_CMPNAMELIST);  }
static int far_CheckMask    (lua_State *L) { return _ProcessName(L, PN_CHECKMASK);    }
static int far_GenerateName (lua_State *L) { return _ProcessName(L, PN_GENERATENAME); }

static int far_GetReparsePointInfo(lua_State *L)
{
	wchar_t* Dest;
	struct FarStandardFunctions *FSF = GetPluginData(L)->FSF;
	const wchar_t* Src = check_utf8_string(L, 1, NULL);
	size_t size = FSF->GetReparsePointInfo(Src, NULL, 0);

	if (size == 0)
		return lua_pushnil(L), 1;

	Dest = (wchar_t*)lua_newuserdata(L, size * sizeof(wchar_t));
	FSF->GetReparsePointInfo(Src, Dest, size);
	return push_utf8_string(L, Dest, -1), 1;
}

static int far_LIsAlpha(lua_State *L)
{
	const wchar_t* str = check_utf8_string(L, 1, NULL);
	lua_pushboolean(L, GetPluginData(L)->FSF->LIsAlpha(*str) != 0);
	return 1;
}

static int far_LIsAlphanum(lua_State *L)
{
	const wchar_t* str = check_utf8_string(L, 1, NULL);
	lua_pushboolean(L, GetPluginData(L)->FSF->LIsAlphanum(*str) != 0);
	return 1;
}

static int far_LIsLower(lua_State *L)
{
	const wchar_t* str = check_utf8_string(L, 1, NULL);
	lua_pushboolean(L, GetPluginData(L)->FSF->LIsLower(*str) != 0);
	return 1;
}

static int far_LIsUpper(lua_State *L)
{
	const wchar_t* str = check_utf8_string(L, 1, NULL);
	lua_pushboolean(L, GetPluginData(L)->FSF->LIsUpper(*str) != 0);
	return 1;
}

static int convert_buf(lua_State *L, int command)
{
	struct FarStandardFunctions *FSF = GetPluginData(L)->FSF;
	size_t len;
	wchar_t* dest = check_utf8_string(L, 1, &len);

	if (command=='l')
		FSF->LLowerBuf(dest,len);
	else
		FSF->LUpperBuf(dest,len);

	push_utf8_string(L, dest, len);
	return 1;
}

static int far_LLowerBuf(lua_State *L)
{
	return convert_buf(L, 'l');
}

static int far_LUpperBuf(lua_State *L)
{
	return convert_buf(L, 'u');
}

static int far_MkTemp(lua_State *L)
{
	const wchar_t* prefix = opt_utf8_string(L, 1, NULL);
	const int dim = 4096;
	wchar_t* dest = (wchar_t*)lua_newuserdata(L, dim * sizeof(wchar_t));

	if (GetPluginData(L)->FSF->MkTemp(dest, dim, prefix))
		push_utf8_string(L, dest, -1);
	else
		lua_pushnil(L);

	return 1;
}

static int far_MkLink(lua_State *L)
{
	const wchar_t* src = check_utf8_string(L, 1, NULL);
	const wchar_t* dst = check_utf8_string(L, 2, NULL);
	UINT64 type = CheckFlags(L, 3);
	UINT64 flags = OptFlags(L, 4, 0);
	lua_pushboolean(L, GetPluginData(L)->FSF->MkLink(src, dst, type, flags));
	return 1;
}

static int far_GetPathRoot(lua_State *L)
{
	const wchar_t* Path = check_utf8_string(L, 1, NULL);
	struct FarStandardFunctions *FSF = GetPluginData(L)->FSF;
	size_t size = FSF->GetPathRoot(Path, NULL, 0);
	wchar_t* Root = (wchar_t*)lua_newuserdata(L, (size+1) * sizeof(wchar_t));
	*Root = L'\0';
	FSF->GetPathRoot(Path, Root, size);
	push_utf8_string(L, Root, -1);
	return 1;
}

static int truncstring(lua_State *L, int op)
{
	struct FarStandardFunctions *FSF = GetPluginData(L)->FSF;
	const wchar_t *Src = check_utf8_string(L, 1, NULL), *ptr;
	wchar_t *Trg;
	intptr_t MaxLen = luaL_checkinteger(L, 2);
	intptr_t SrcLen = wcslen(Src);

	if (MaxLen < 0) MaxLen = 0;
	else if (MaxLen > SrcLen) MaxLen = SrcLen;

	Trg = (wchar_t*)lua_newuserdata(L, (1 + SrcLen) * sizeof(wchar_t));
	wcscpy(Trg, Src);
	ptr = (op == 'p') ? FSF->TruncPathStr(Trg, MaxLen) : FSF->TruncStr(Trg, MaxLen);
	return push_utf8_string(L, ptr, -1), 1;
}

static int far_TruncPathStr(lua_State *L)
{
	return truncstring(L, 'p');
}

static int far_TruncStr(lua_State *L)
{
	return truncstring(L, 's');
}

typedef struct
{
	lua_State *L;
	int nparams;
	int err;
	DWORD attr_incl;
	DWORD attr_excl;
} FrsData;

static int WINAPI FrsUserFunc(const struct PluginPanelItem *FData, const wchar_t *FullName,
                              void *Param)
{
	FrsData *Data = (FrsData*)Param;
	lua_State *L = Data->L;
	int i, nret = lua_gettop(L);

	if ((FData->FileAttributes & Data->attr_excl) != 0 || (FData->FileAttributes & Data->attr_incl) != Data->attr_incl)
		return TRUE; // attributes mismatch

	lua_pushvalue(L, 3); // push the Lua function
	PushPanelItem(L, FData, 0);
	push_utf8_string(L, FullName, -1);
	for (i=1; i<=Data->nparams; i++)
		lua_pushvalue(L, 4+i);

	Data->err = lua_pcall(L, 2+Data->nparams, LUA_MULTRET, 0);

	nret = lua_gettop(L) - nret;
	if (!Data->err && (nret==0 || lua_toboolean(L,-nret)==0))
	{
		lua_pop(L, nret);
		return TRUE;
	}
	return FALSE;
}

static int far_RecursiveSearch(lua_State *L)
{
	UINT64 Flags;
	FrsData Data = { L,0,0,0,0 };
	const wchar_t *InitDir = check_utf8_string(L, 1, NULL);
	wchar_t *Mask = check_utf8_string(L, 2, NULL);
	wchar_t *MaskEnd;

	luaL_checktype(L, 3, LUA_TFUNCTION);
	if ((MaskEnd=wcsstr(Mask, L">>")) != NULL)
	{
		*MaskEnd = 0;
		SetAttrWords(MaskEnd+2, &Data.attr_incl, &Data.attr_excl);
	}
	Flags = OptFlags(L, 4, 0);
	if (lua_gettop(L) == 3)
		lua_pushnil(L);

	Data.nparams = lua_gettop(L) - 4;
	lua_checkstack(L, 256);

	GetPluginData(L)->FSF->FarRecursiveSearch(InitDir, Mask, FrsUserFunc, Flags, &Data);

	if (Data.err)
		LF_Error(L, check_utf8_string(L, -1, NULL));
	return Data.err ? 0 : lua_gettop(L) - Data.nparams - 4;
}

static int far_ConvertPath(lua_State *L)
{
	struct FarStandardFunctions *FSF = GetPluginData(L)->FSF;
	const wchar_t *Src = check_utf8_string(L, 1, NULL);
	enum CONVERTPATHMODES Mode = lua_isnoneornil(L,2) ?
	                             CPM_FULL : (enum CONVERTPATHMODES)check_env_flag(L,2);
	size_t Size = FSF->ConvertPath(Mode, Src, NULL, 0);
	wchar_t* Target = (wchar_t*)lua_newuserdata(L, Size*sizeof(wchar_t));
	FSF->ConvertPath(Mode, Src, Target, Size);
	push_utf8_string(L, Target, -1);
	return 1;
}

static int DoAdvControl (lua_State *L, int Command, int Delta)
{
	int pos2 = 2-Delta, pos3 = 3-Delta;
	TPluginData *pd = GetPluginData(L);
	GUID* PluginId = pd->PluginId;
	PSInfo *Info = pd->Info;
	intptr_t Param1 = 0;
	void *Param2 = NULL;
	lua_settop(L,pos3);  /* for proper calling GetOptIntFromTable and the like */

	if (Delta == 0)
		Command = CAST(int, check_env_flag(L, 1));

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
			lua_pushlightuserdata(L, CAST(void*, Info->AdvControl(PluginId, Command, 0, NULL)));
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
			Param1 = luaL_checkinteger(L, pos2);

			if (Info->AdvControl(PluginId, Command, Param1, &fc))
				PushFarColor(L, &fc);
			else
				lua_pushnil(L);

			return 1;
		}

		case ACTL_SYNCHRO:
		{
			intptr_t p = luaL_checkinteger(L, pos2);
			Param2 = CreateSynchroData(NULL, 0, (int)p);
			break;
		}

		case ACTL_SETPROGRESSSTATE:
			Param1 = (intptr_t) check_env_flag(L, pos2);
			break;

		case ACTL_SETPROGRESSVALUE:
		{
			struct ProgressValue pv;
			luaL_checktype(L, pos3, LUA_TTABLE);
			pv.StructSize = sizeof(pv);
			pv.Completed = (UINT64)GetOptNumFromTable(L, "Completed", 0.0);
			pv.Total = (UINT64)GetOptNumFromTable(L, "Total", 100.0);
			lua_pushinteger(L, Info->AdvControl(PluginId, Command, Param1, &pv));
			return 1;
		}

		case ACTL_GETARRAYCOLOR:
		{
			intptr_t len = Info->AdvControl(PluginId, Command, 0, NULL), i;
			struct FarColor *arr = (struct FarColor*) lua_newuserdata(L, len*sizeof(struct FarColor));
			Info->AdvControl(PluginId, Command, len, arr);
			lua_createtable(L, (int)len, 0);

			for(i=0; i < len; i++)
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

			if (lua_toboolean(L, 2))
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
			intptr_t r;
			struct WindowInfo wi;
			memset(&wi, 0, sizeof(wi));
			wi.StructSize = sizeof(wi);
			wi.Pos = luaL_optinteger(L, pos2, 0) - 1;
			r = Info->AdvControl(PluginId, Command, 0, &wi);

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
					NewDialogData(L, Info, CAST(HANDLE, wi.Id), FALSE);
					lua_setfield(L, -2, "Id");
					break;

				default:
					PutIntToTable(L, "Id", CAST(int, wi.Id));
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
			struct FarSetColors fsc;
			size_t size;
			int i;
			luaL_checktype(L, pos3, LUA_TTABLE);
			fsc.StructSize = sizeof(fsc);
			fsc.StartIndex = GetOptIntFromTable(L, "StartIndex", 0);
			lua_getfield(L, pos3, "Flags");
			fsc.Flags = GetFlagCombination(L, -1, NULL);
			fsc.ColorsCount = lua_objlen(L, pos3);
			size = fsc.ColorsCount * sizeof(struct FarColor);
			fsc.Colors = (struct FarColor*) lua_newuserdata(L, size);
			memset(fsc.Colors, 0, size);

			for(i=0; i < (int)fsc.ColorsCount; i++)
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
			struct WindowType wt;
			wt.StructSize = sizeof(wt);

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
AdvCommand( GetFarmanagerVersion,   ACTL_GETFARMANAGERVERSION)
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
AdvCommand( Waitkey,                ACTL_WAITKEY)

static int far_MacroLoadAll(lua_State* L)
{
	TPluginData *pd = GetPluginData(L);
	struct FarMacroLoad Data;
	Data.StructSize = sizeof(Data);
	Data.Path = opt_utf8_string(L, 1, NULL);
	Data.Flags = OptFlags(L, 2, 0);
	lua_pushboolean(L, pd->Info->MacroControl(pd->PluginId, MCTL_LOADALL, 0, (void*)&Data) != 0);
	return 1;
}

static int far_MacroSaveAll(lua_State* L)
{
	TPluginData *pd = GetPluginData(L);
	lua_pushboolean(L, pd->Info->MacroControl(pd->PluginId, MCTL_SAVEALL, 0, 0) != 0);
	return 1;
}

static int far_MacroGetState(lua_State* L)
{
	TPluginData *pd = GetPluginData(L);
	lua_pushinteger(L, pd->Info->MacroControl(pd->PluginId, MCTL_GETSTATE, 0, 0));
	return 1;
}

static int far_MacroGetArea(lua_State* L)
{
	TPluginData *pd = GetPluginData(L);
	lua_pushinteger(L, pd->Info->MacroControl(pd->PluginId, MCTL_GETAREA, 0, 0));
	return 1;
}

static int MacroSendString(lua_State* L, int Param1)
{
	TPluginData *pd = GetPluginData(L);
	struct MacroSendMacroText smt;
	memset(&smt, 0, sizeof(smt));
	smt.StructSize = sizeof(smt);
	smt.SequenceText = check_utf8_string(L, 1, NULL);
	smt.Flags = OptFlags(L, 2, 0);
	if (Param1 == MSSC_POST)
		OptInputRecord(L, pd, 3, &smt.AKey);

	lua_pushboolean(L, pd->Info->MacroControl(pd->PluginId, MCTL_SENDSTRING, Param1, &smt) != 0);
	return 1;
}

static int far_MacroPost(lua_State* L)
{
	return MacroSendString(L, MSSC_POST);
}

static int far_MacroCheck(lua_State* L)
{
	return MacroSendString(L, MSSC_CHECK);
}

static int far_MacroGetLastError(lua_State* L)
{
	TPluginData *pd = GetPluginData(L);
	intptr_t size = pd->Info->MacroControl(pd->PluginId, MCTL_GETLASTERROR, 0, NULL);

	if (size)
	{
		struct MacroParseResult *mpr = (struct MacroParseResult*)lua_newuserdata(L, size);
		mpr->StructSize = sizeof(*mpr);
		pd->Info->MacroControl(pd->PluginId, MCTL_GETLASTERROR, size, mpr);
		lua_createtable(L, 0, 4);
		PutIntToTable(L, "ErrCode", mpr->ErrCode);
		PutIntToTable(L, "ErrPosX", mpr->ErrPos.X);
		PutIntToTable(L, "ErrPosY", mpr->ErrPos.Y);
		PutWStrToTable(L, "ErrSrc", mpr->ErrSrc, -1);
	}
	else
		lua_pushboolean(L, 0);

	return 1;
}

typedef struct
{
	lua_State *L;
	int funcref;
} MacroAddData;

intptr_t WINAPI MacroAddCallback (void* Id, FARADDKEYMACROFLAGS Flags)
{
	lua_State *L;
	int result = TRUE;
	MacroAddData *data = (MacroAddData*)Id;
	if ((L = data->L) == NULL)
		return FALSE;

	lua_rawgeti(L, LUA_REGISTRYINDEX, data->funcref);

	if (lua_type(L,-1) == LUA_TFUNCTION)
	{
		lua_pushlightuserdata(L, Id);
		lua_rawget(L, LUA_REGISTRYINDEX);
		bit64_push(L, Flags);
		result = !lua_pcall(L, 2, 1, 0) && lua_toboolean(L, -1);
	}

	lua_pop(L, 1);
	return result;
}

static int far_MacroAdd(lua_State* L)
{
	TPluginData *pd = GetPluginData(L);
	struct MacroAddMacro data;
	memset(&data, 0, sizeof(data));
	data.StructSize = sizeof(data);
	data.Area = OptFlags(L, 1, MACROAREA_COMMON);
	data.Flags = OptFlags(L, 2, 0);
	OptInputRecord(L, pd, 3, &data.AKey);
	data.SequenceText = check_utf8_string(L, 4, NULL);
	data.Description = opt_utf8_string(L, 5, L"");
	lua_settop(L, 7);
	if (lua_toboolean(L, 6))
	{
		luaL_checktype(L, 6, LUA_TFUNCTION);
		data.Callback = MacroAddCallback;
	}
	data.Id = lua_newuserdata(L, sizeof(MacroAddData));
	data.Priority = luaL_optinteger(L, 7, 50);

	if (pd->Info->MacroControl(pd->PluginId, MCTL_ADDMACRO, 0, &data))
	{
		MacroAddData* Id = (MacroAddData*)data.Id;
		lua_isfunction(L, 6) ? lua_pushvalue(L, 6) : lua_pushboolean(L, 1);
		Id->funcref = luaL_ref(L, LUA_REGISTRYINDEX);
		Id->L = pd->MainLuaState;
		luaL_getmetatable(L, AddMacroDataType);
		lua_setmetatable(L, -2);
		lua_pushlightuserdata(L, Id); // Place it in the registry to protect from gc. It should be collected only at lua_close().
		lua_pushvalue(L, -2);
		lua_rawset(L, LUA_REGISTRYINDEX);
	}
	else
		lua_pushnil(L);

	return 1;
}

static int far_MacroDelete(lua_State* L)
{
	TPluginData *pd = GetPluginData(L);
	MacroAddData *Id;
	int result = FALSE;

	Id = (MacroAddData*)luaL_checkudata(L, 1, AddMacroDataType);
	if (Id->L)
	{
		result = (int)pd->Info->MacroControl(pd->PluginId, MCTL_DELMACRO, 0, Id);
		if (result)
		{
			luaL_unref(L, LUA_REGISTRYINDEX, Id->funcref);
			Id->L = NULL;
			lua_pushlightuserdata(L, Id);
			lua_pushnil(L);
			lua_rawset(L, LUA_REGISTRYINDEX);
		}
	}

	lua_pushboolean(L, result);
	return 1;
}

static int AddMacroData_gc(lua_State* L)
{
	far_MacroDelete(L);
	return 0;
}

static int far_MacroExecute(lua_State* L)
{
	TPluginData *pd = GetPluginData(L);
	int top = lua_gettop(L);

	struct MacroExecuteString Data;
	Data.StructSize = sizeof(Data);
	Data.SequenceText = check_utf8_string(L, 1, NULL);
	Data.Flags = OptFlags(L,2,0);
	Data.InCount = 0;

	if (top > 2)
	{
		size_t i;
		Data.InCount = top-2;
		Data.InValues = (struct FarMacroValue*)lua_newuserdata(L, Data.InCount*sizeof(struct FarMacroValue));
		memset(Data.InValues, 0, Data.InCount*sizeof(struct FarMacroValue));
		for (i=0; i<Data.InCount; i++)
			ConvertLuaValue(L, (int)i+3, Data.InValues+i);
	}

	if (pd->Info->MacroControl(pd->PluginId, MCTL_EXECSTRING, 0, &Data))
		PackMacroValues(L, Data.OutCount, Data.OutValues);
	else
		lua_pushnil(L);

	return 1;
}

static int far_CPluginStartupInfo(lua_State *L)
{
	return lua_pushlightuserdata(L, (void*)GetPluginData(L)->Info), 1;
}

void pushFileTime(lua_State *L, const FILETIME *ft)
{
	long long llFileTime = ft->dwLowDateTime + 0x100000000LL * ft->dwHighDateTime;
	if (! (GetPluginData(L)->Flags & PDF_FULL_TIME_RESOLUTION))
		lua_pushnumber(L, (double)(llFileTime / 10000));
	else
		bit64_pushuserdata(L, llFileTime);
}

static int far_MakeMenuItems(lua_State *L)
{
	int argn = lua_gettop(L);
	lua_createtable(L, argn, 0);               //+1 (items)

	if (argn > 0)
	{
		int item = 1, i;
		char delim[] = { 226,148,130,0 };        // Unicode char 9474 in UTF-8
		char buf_prefix[64], buf_space[64];
		int maxno = 0;
		size_t len_prefix;

		for (i=argn; i; maxno++,i/=10) {}
		len_prefix = sprintf(buf_space, "%*s%s ", maxno, "", delim);

		for(i=1; i<=argn; i++)
		{
			size_t j, len_arg;
			const char *start;
			char* str;

			lua_getglobal(L, "tostring");          //+2

			if (i == 1 && lua_type(L,-1) != LUA_TFUNCTION)
				luaL_error(L, "global `tostring' is not function");

			lua_pushvalue(L, i);                   //+3

			if (0 != lua_pcall(L, 1, 1, 0))         //+2 (items,str)
				luaL_error(L, lua_tostring(L, -1));

			if (lua_type(L, -1) != LUA_TSTRING)
				luaL_error(L, "tostring() returned a non-string value");

			sprintf(buf_prefix, "%*d%s ", maxno, i, delim);
			start = lua_tolstring(L, -1, &len_arg);
			str = (char*) malloc(len_arg + 1);
			memcpy(str, start, len_arg + 1);

			for (j=0; j<len_arg; j++)
				if (str[j] == '\0') str[j] = ' ';

			for (start=str; start; )
			{
				size_t len_text;
				char *line;
				const char* nl = strchr(start, '\n');

				lua_newtable(L);                     //+3 (items,str,curr_item)
				len_text = nl ? (nl++) - start : (str+len_arg) - start;
				line = (char*) malloc(len_prefix + len_text);
				memcpy(line, buf_prefix, len_prefix);
				memcpy(line + len_prefix, start, len_text);

				lua_pushlstring(L, line, len_prefix + len_text);
				free(line);
				lua_setfield(L, -2, "text");         //+3
				lua_pushvalue(L, i);
				lua_setfield(L, -2, "arg");          //+3
				lua_rawseti(L, -3, item++);          //+2 (items,str)
				strcpy(buf_prefix, buf_space);
				start = nl;
			}

			free(str);
			lua_pop(L, 1);                         //+1 (items)
		}
	}

	return 1;
}

static int far_Show(lua_State *L)
{
	const char* f =
	    "local items,n=...\n"
	    "local bottom=n==0 and 'No arguments' or n==1 and '1 argument' or n..' arguments'\n"
	    "return far.Menu({Title='',Bottom=bottom,Flags='FMENU_SHOWAMPERSAND'},items,"
	    "{{BreakKey='SPACE'}})";
	int argn = lua_gettop(L);
	far_MakeMenuItems(L);

	if (luaL_loadstring(L, f) != 0)
		luaL_error(L, lua_tostring(L, -1));

	lua_pushvalue(L, -2);
	lua_pushinteger(L, argn);

	if (lua_pcall(L, 2, LUA_MULTRET, 0) != 0)
		luaL_error(L, lua_tostring(L, -1));

	return lua_gettop(L) - argn - 1;
}

void NewVirtualKeyTable(lua_State* L, BOOL twoways)
{
	int i;
	lua_createtable(L, twoways ? 256:0, 200);

	for(i=0; i<256; i++)
	{
		const char* str = VirtualKeyStrings[i];

		if (str)
		{
			lua_pushinteger(L, i);
			lua_setfield(L, -2, str);
		}

		if (twoways)
		{
			lua_pushstring(L, str ? str : "");
			lua_rawseti(L, -2, i);
		}
	}
}

HANDLE* CheckFileFilter(lua_State* L, int pos)
{
	return (HANDLE*)luaL_checkudata(L, pos, FarFileFilterType);
}

HANDLE CheckValidFileFilter(lua_State* L, int pos)
{
	HANDLE h = *CheckFileFilter(L, pos);
	luaL_argcheck(L,h != INVALID_HANDLE_VALUE,pos,"attempt to access invalid file filter");
	return h;
}

static int far_CreateFileFilter(lua_State *L)
{
	PSInfo *Info = GetPluginData(L)->Info;
	HANDLE hHandle = (luaL_checkinteger(L,1) % 2) ? PANEL_ACTIVE:PANEL_PASSIVE;
	int filterType = CAST(int, check_env_flag(L,2));
	HANDLE* pOutHandle = (HANDLE*)lua_newuserdata(L, sizeof(HANDLE));

	if (Info->FileFilterControl(hHandle, FFCTL_CREATEFILEFILTER, filterType, pOutHandle))
	{
		luaL_getmetatable(L, FarFileFilterType);
		lua_setmetatable(L, -2);
	}
	else
		lua_pushnil(L);

	return 1;
}

static int filefilter_Free(lua_State *L)
{
	HANDLE *h = CheckFileFilter(L, 1);

	if (*h != INVALID_HANDLE_VALUE)
	{
		PSInfo *Info = GetPluginData(L)->Info;
		lua_pushboolean(L, Info->FileFilterControl(*h, FFCTL_FREEFILEFILTER, 0, 0) != 0);
		*h = INVALID_HANDLE_VALUE;
	}
	else
		lua_pushboolean(L,0);

	return 1;
}

static int filefilter_gc(lua_State *L)
{
	filefilter_Free(L);
	return 0;
}

static int filefilter_tostring(lua_State *L)
{
	HANDLE *h = CheckFileFilter(L, 1);

	if (*h != INVALID_HANDLE_VALUE)
		lua_pushfstring(L, "%s (%p)", FarFileFilterType, h);
	else
		lua_pushfstring(L, "%s (closed)", FarFileFilterType);

	return 1;
}

static int filefilter_OpenMenu(lua_State *L)
{
	HANDLE h = CheckValidFileFilter(L, 1);
	PSInfo *Info = GetPluginData(L)->Info;
	lua_pushboolean(L, Info->FileFilterControl(h, FFCTL_OPENFILTERSMENU, 0, 0) != 0);
	return 1;
}

static int filefilter_Starting(lua_State *L)
{
	HANDLE h = CheckValidFileFilter(L, 1);
	PSInfo *Info = GetPluginData(L)->Info;
	lua_pushboolean(L, Info->FileFilterControl(h, FFCTL_STARTINGTOFILTER, 0, 0) != 0);
	return 1;
}

static int filefilter_IsFileInFilter(lua_State *L)
{
	struct PluginPanelItem ppi;
	PSInfo *Info = GetPluginData(L)->Info;
	HANDLE h = CheckValidFileFilter(L, 1);
	luaL_checktype(L, 2, LUA_TTABLE);
	lua_settop(L, 2);                // +2
	FillPluginPanelItem(L, &ppi, 0); // +6
	lua_pushboolean(L, Info->FileFilterControl(h, FFCTL_ISFILEINFILTER, 0, &ppi) != 0);
	return 1;
}

static int plugin_load(lua_State *L, enum FAR_PLUGINS_CONTROL_COMMANDS command)
{
	PSInfo *Info = GetPluginData(L)->Info;
	int param1 = CAST(int, check_env_flag(L, 1));
	void *param2 = check_utf8_string(L, 2, NULL);
	intptr_t result = Info->PluginsControl(INVALID_HANDLE_VALUE, command, param1, param2);

	if (result) PushPluginHandle(L, CAST(HANDLE, result));
	else lua_pushnil(L);

	return 1;
}

static int far_LoadPlugin(lua_State *L) { return plugin_load(L, PCTL_LOADPLUGIN); }
static int far_ForcedLoadPlugin(lua_State *L) { return plugin_load(L, PCTL_FORCEDLOADPLUGIN); }

static int far_UnloadPlugin(lua_State *L)
{
	PSInfo *Info = GetPluginData(L)->Info;
	void* Handle = *(void**)luaL_checkudata(L, 1, PluginHandleType);
	lua_pushboolean(L, Info->PluginsControl(Handle, PCTL_UNLOADPLUGIN, 0, 0) != 0);
	return 1;
}

static int far_FindPlugin(lua_State *L)
{
	PSInfo *Info = GetPluginData(L)->Info;
	int param1 = CAST(int, check_env_flag(L, 1));
	void *param2 = NULL;

	if (param1 == PFM_MODULENAME)
		param2 = check_utf8_string(L, 2, NULL);
	else if (param1 == PFM_GUID)
	{
		size_t len;
		param2 = CAST(void*, luaL_checklstring(L, 2, &len));

		if (len < sizeof(GUID)) param2 = NULL;
	}

	if (param2)
	{
		intptr_t handle = Info->PluginsControl(NULL, PCTL_FINDPLUGIN, param1, param2);

		if (handle)
		{
			PushPluginHandle(L, CAST(HANDLE, handle));
			return 1;
		}
	}

	lua_pushnil(L);
	return 1;
}

static void PutPluginMenuItemToTable(lua_State *L, const char* field, const struct PluginMenuItem *mi)
{
	lua_createtable(L, 0, 3);
	{
		int i;
		PutIntToTable(L, "Count", mi->Count);
		lua_createtable(L, (int) mi->Count, 0); // Guids
		lua_createtable(L, (int) mi->Count, 0); // Strings

		for(i=0; i < (int) mi->Count; i++)
		{
			lua_pushlstring(L, CAST(const char*, mi->Guids + i), sizeof(GUID));
			lua_rawseti(L, -3, i+1);
			push_utf8_string(L, mi->Strings[i], -1);
			lua_rawseti(L, -2, i+1);
		}

		lua_setfield(L, -3, "Strings");
		lua_setfield(L, -2, "Guids");
	}
	lua_setfield(L, -2, field);
}

static void PutVersionInfoToTable(lua_State *L, const char* field, const struct VersionInfo *vi)
{
	lua_createtable(L, 5, 0);
	PutIntToArray(L, 1, vi->Major);
	PutIntToArray(L, 2, vi->Minor);
	PutIntToArray(L, 3, vi->Revision);
	PutIntToArray(L, 4, vi->Build);
	PutIntToArray(L, 5, vi->Stage);
	lua_setfield(L, -2, field);
}

static int far_GetPluginInformation(lua_State *L)
{
	struct FarGetPluginInformation *pi;
	PSInfo *Info = GetPluginData(L)->Info;
	HANDLE Handle = *(HANDLE*)luaL_checkudata(L, 1, PluginHandleType);
	size_t size = Info->PluginsControl(Handle, PCTL_GETPLUGININFORMATION, 0, 0);

	if (size == 0) return lua_pushnil(L), 1;

	pi = (struct FarGetPluginInformation *)lua_newuserdata(L, size);
	pi->StructSize = sizeof(*pi);

	if (!Info->PluginsControl(Handle, PCTL_GETPLUGININFORMATION, size, pi))
		return lua_pushnil(L), 1;

	lua_createtable(L, 0, 4);
	{
		PutWStrToTable(L, "ModuleName", pi->ModuleName, -1);
		PutFlagsToTable(L, "Flags", pi->Flags);
		lua_createtable(L, 0, 6); // PInfo
		{
			PutIntToTable(L, "StructSize", pi->PInfo->StructSize);
			PutFlagsToTable(L, "Flags", pi->PInfo->Flags);
			PutPluginMenuItemToTable(L, "DiskMenu", &pi->PInfo->DiskMenu);
			PutPluginMenuItemToTable(L, "PluginMenu", &pi->PInfo->PluginMenu);
			PutPluginMenuItemToTable(L, "PluginConfig", &pi->PInfo->PluginConfig);

			if (pi->PInfo->CommandPrefix)
				PutWStrToTable(L, "CommandPrefix", pi->PInfo->CommandPrefix, -1);

			lua_setfield(L, -2, "PInfo");
		}
		lua_createtable(L, 0, 7); // GInfo
		{
			PutIntToTable(L, "StructSize", pi->GInfo->StructSize);
			PutVersionInfoToTable(L, "MinFarVersion", &pi->GInfo->MinFarVersion);
			PutVersionInfoToTable(L, "Version", &pi->GInfo->Version);
			PutLStrToTable(L, "Guid", (const char*)&pi->GInfo->Guid, sizeof(GUID));
			PutWStrToTable(L, "Title", pi->GInfo->Title, -1);
			PutWStrToTable(L, "Description", pi->GInfo->Description, -1);
			PutWStrToTable(L, "Author", pi->GInfo->Author, -1);
			lua_setfield(L, -2, "GInfo");
		}
	}
	return 1;
}

static int far_GetPlugins(lua_State *L)
{
	PSInfo *Info = GetPluginData(L)->Info;
	int count = (int)Info->PluginsControl(INVALID_HANDLE_VALUE, PCTL_GETPLUGINS, 0, 0);
	lua_createtable(L, count, 0);

	if (count > 0)
	{
		int i;
		HANDLE *handles = lua_newuserdata(L, count*sizeof(HANDLE));
		count = (int)Info->PluginsControl(INVALID_HANDLE_VALUE, PCTL_GETPLUGINS, count, handles);

		for(i=0; i<count; i++)
		{
			PushPluginHandle(L, handles[i]);
			lua_rawseti(L, -3, i+1);
		}

		lua_pop(L, 1);
	}

	return 1;
}

static int far_IsPluginLoaded(lua_State *L)
{
	UUID uuid;
	size_t len;
	intptr_t handle;
	int result = 0;
	const char *guid = luaL_checklstring(L, 1, &len);
	PSInfo *Info = GetPluginData(L)->Info;

	if (len == 16)
		uuid = *(UUID*)guid;
	else
		luaL_argcheck(L, UuidFromStringA((unsigned char*)guid, &uuid) == RPC_S_OK, 1, "invalid GUID");

	handle = Info->PluginsControl(NULL, PCTL_FINDPLUGIN, PFM_GUID, &uuid);
	if (handle)
	{
		size_t size = Info->PluginsControl((HANDLE)handle, PCTL_GETPLUGININFORMATION, 0, 0);
		if (size)
		{
			struct FarGetPluginInformation *pi = (struct FarGetPluginInformation *)malloc(size);
			pi->StructSize = sizeof(*pi);
			if (Info->PluginsControl((HANDLE)handle, PCTL_GETPLUGININFORMATION, size, pi))
				result = (pi->Flags & FPF_LOADED) ? 1:0;

			free(pi);
		}
	}
	lua_pushboolean(L, result);
	return 1;
}

static int far_XLat(lua_State *L)
{
	size_t size;
	wchar_t *Line = check_utf8_string(L, 1, &size), *str;
	intptr_t StartPos = luaL_optinteger(L, 2, 1) - 1;
	intptr_t EndPos = luaL_optinteger(L, 3, size);
	UINT64 Flags = OptFlags(L, 4, 0);
	StartPos < 0 ? StartPos = 0 : StartPos > (intptr_t)size ? StartPos = size : 0;
	EndPos < StartPos ? EndPos = StartPos : EndPos > (intptr_t)size ? EndPos = size : 0;
	str = GetPluginData(L)->FSF->XLat(Line, StartPos, EndPos, Flags);
	str ? (void)push_utf8_string(L, str, -1) : lua_pushnil(L);
	return 1;
}

static int far_FormatFileSize(lua_State *L)
{
	uint64_t Size = (uint64_t) luaL_checknumber(L, 1);
	int Width = (int)luaL_checkinteger(L, 2);
	if (abs(Width) > 10000)
		return luaL_error(L, "the 'Width' argument exceeds 10000");

	UINT64 Flags = OptFlags(L, 3, 0) & ~FFFS_MINSIZEINDEX_MASK;
	Flags |= luaL_optinteger(L, 4, 0) & FFFS_MINSIZEINDEX_MASK;

	TPluginData *pd = GetPluginData(L);
	size_t bufsize = pd->FSF->FormatFileSize(Size, Width, Flags, NULL, 0);
	wchar_t *buf = (wchar_t*) lua_newuserdata(L, bufsize*sizeof(wchar_t));

	pd->FSF->FormatFileSize(Size, Width, Flags, buf, bufsize);
	push_utf8_string(L, buf, -1);
	return 1;
}

static int far_FarClock(lua_State *L)
{
	UINT64 c = GetPluginData(L)->FSF->FarClock();
	lua_pushnumber(L, (double)c);
	return 1;
}

void CALLBACK TimerCallback(void *lpParameter, BOOLEAN TimerOrWaitFired)
{
	TTimerData *td = (TTimerData*)lpParameter;
	TSynchroData *sd;
	(void)TimerOrWaitFired;
	if (!td->needClose && td->enabled)
	{
		sd = CreateSynchroData(td, LUAFAR_TIMER_CALL, 0);
		td->Info->AdvControl(td->PluginGuid, ACTL_SYNCHRO, 0, sd);
	}
}

static int far_Timer(lua_State *L)
{
	TPluginData *pd;
	TTimerData *td;
	HANDLE hQueue;
	int interval, index, tabSize;

	interval = (int)luaL_checkinteger(L, 1);
	luaL_checktype(L, 2, LUA_TFUNCTION);

	tabSize = lua_gettop(L);

	lua_createtable(L, tabSize, 1);         // place the function at [1]
	lua_pushinteger(L, tabSize);
	lua_setfield(L, -2, "n");
	lua_pushvalue(L, 2);
	lua_rawseti(L, -2, 1);

	td = (TTimerData*)lua_newuserdata(L, sizeof(TTimerData));
	luaL_getmetatable(L, FarTimerType);
	lua_setmetatable(L, -2);
	lua_pushvalue(L, -1);
	lua_rawseti(L, -3, 2);                  // place the userdata at [2]

	for (index=3; index<=tabSize; index++)  // place the arguments, if any
	{
		lua_pushvalue(L, index);
		lua_rawseti(L, -3, index);
	}

	pd = GetPluginData(L);
	td->Info = pd->Info;
	td->PluginGuid = pd->PluginId;
	td->interval = interval < 1 ? 1 : interval;

	lua_pushvalue(L, -2);
	td->tabRef = luaL_ref(L, LUA_REGISTRYINDEX);
	td->needClose = FALSE;
	td->enabled = 1;
	hQueue = GetLuaStateTimerQueue(L);

	if (hQueue && CreateTimerQueueTimer(&td->hTimer,hQueue,TimerCallback,td,td->interval,td->interval,WT_EXECUTEDEFAULT))
		return 1;

	luaL_unref(L, LUA_REGISTRYINDEX, td->tabRef);
	return lua_pushnil(L), 1;
}

TTimerData* CheckTimer(lua_State* L, int pos)
{
	return (TTimerData*)luaL_checkudata(L, pos, FarTimerType);
}

TTimerData* CheckValidTimer(lua_State* L, int pos)
{
	TTimerData* td = CheckTimer(L, pos);
	luaL_argcheck(L, !td->needClose, pos, "attempt to access closed timer");
	return td;
}

static int timer_Close(lua_State *L)
{
	HANDLE hQueue;
	TSynchroData* sd;
	TTimerData* td = CheckTimer(L, 1);
	if (!td->needClose)
	{
		td->needClose = TRUE;
		hQueue = GetLuaStateTimerQueue(L);
		if (hQueue)
			DeleteTimerQueueTimer(hQueue, td->hTimer, NULL);
		sd = CreateSynchroData(td, LUAFAR_TIMER_UNREF, 0);
		td->Info->AdvControl(td->PluginGuid, ACTL_SYNCHRO, 0, sd);
	}
	return 0;
}

static int timer_gc(lua_State *L)
{
	HANDLE hQueue;
	TTimerData* td = CheckTimer(L, 1);
	if (!td->needClose)
	{
		td->needClose = TRUE;
		hQueue = GetLuaStateTimerQueue(L);
		if (hQueue)
			DeleteTimerQueueTimer(hQueue, td->hTimer, NULL);
	}
	return 0;
}

static int timer_tostring(lua_State *L)
{
	TTimerData* td = CheckTimer(L, 1);

	if (!td->needClose)
		lua_pushfstring(L, "%s (%p)", FarTimerType, td);
	else
		lua_pushfstring(L, "%s (closed)", FarTimerType);

	return 1;
}

static int timer_index(lua_State *L)
{
	TTimerData* td = CheckTimer(L, 1);
	const char* method = luaL_checkstring(L, 2);

	if (!strcmp(method, "Close"))
		lua_pushcfunction(L, timer_Close);
	else if (!strcmp(method, "Enabled"))
		lua_pushboolean(L, td->enabled);
	else if (!strcmp(method, "Interval"))
		lua_pushinteger(L, td->interval);
	else if (!strcmp(method, "OnTimer"))
	{
		lua_rawgeti(L, LUA_REGISTRYINDEX, td->tabRef);
		lua_rawgeti(L, -1, 1);
	}
	else if (!strcmp(method, "Closed"))
		lua_pushboolean(L, td->needClose);
	else
		luaL_error(L, "attempt to call non-existent method");

	return 1;
}

static int timer_newindex(lua_State *L)
{
	TTimerData* td = CheckValidTimer(L, 1);
	const char* method = luaL_checkstring(L, 2);

	if (!strcmp(method, "Enabled"))
	{
		luaL_checkany(L, 3);
		td->enabled = lua_toboolean(L, 3);
	}
	else if (!strcmp(method, "Interval"))
	{
		int interval = (int)luaL_checkinteger(L, 3);
		HANDLE hQueue = GetLuaStateTimerQueue(L);
		if (hQueue)
		{
			td->interval = interval < 1 ? 1 : interval;
			ChangeTimerQueueTimer(hQueue, td->hTimer, td->interval, td->interval);
		}
	}
	else if (!strcmp(method, "OnTimer"))
	{
		luaL_checktype(L, 3, LUA_TFUNCTION);
		lua_rawgeti(L, LUA_REGISTRYINDEX, td->tabRef);
		lua_pushvalue(L, 3);
		lua_rawseti(L, -2, 1);
	}
	else luaL_error(L, "attempt to call non-existent method");

	return 0;
}

BOOL dir_exist(const wchar_t* path)
{
	DWORD attr = GetFileAttributesW(path);
	return (attr != 0xFFFFFFFF) && (attr & FILE_ATTRIBUTE_DIRECTORY);
}

typedef struct
{
	HANDLE Handle;
	BOOL IsFarSettings;
} FarSettingsUdata;


static int far_CreateSettings(lua_State *L)
{
	size_t len = 0;
	const char* strId;
	const GUID* ParamId;
	FarSettingsUdata *udata;
	struct FarSettingsCreate fsc;
	BOOL IsFarSettings = 0;
	TPluginData *pd = GetPluginData(L);
	int location;
	strId = luaL_optlstring(L, 1, NULL, &len);

	if (strId == NULL)
		ParamId = pd->PluginId;
	else
	{
		if (len == 3 && strcmp(strId, "far") == 0)
			IsFarSettings = 1;
		else if (len == sizeof(GUID))
			IsFarSettings = !memcmp(strId, &FarGuid, len);
		else
		{
			lua_pushnil(L);
			return 1;
		}

		ParamId = IsFarSettings? &FarGuid : CAST(const GUID*, strId);
	}

	location = CAST(int, OptFlags(L, 2, PSL_ROAMING));
	fsc.StructSize = sizeof(fsc);
	fsc.Guid = *ParamId;

	if (!pd->Info->SettingsControl(INVALID_HANDLE_VALUE, SCTL_CREATE, location, &fsc))
	{
		lua_pushnil(L);
		return 1;
	}

	lua_getfield(L, LUA_REGISTRYINDEX, SettingsHandles);
	udata = (FarSettingsUdata*)lua_newuserdata(L, sizeof(FarSettingsUdata));
	udata->Handle = fsc.Handle;
	udata->IsFarSettings = IsFarSettings;
	luaL_getmetatable(L, SettingsType);
	lua_setmetatable(L, -2);
	lua_pushvalue(L, -1);
	lua_pushinteger(L, 1);
	lua_rawset(L, -4);
	return 1;
}

static FarSettingsUdata* GetSettingsUdata(lua_State *L, int pos)
{
	return luaL_checkudata(L, pos, SettingsType);
}

static FarSettingsUdata* CheckSettings(lua_State *L, int pos)
{
	FarSettingsUdata* udata = GetSettingsUdata(L, pos);

	if (udata->Handle == INVALID_HANDLE_VALUE)
	{
		const char* s = lua_pushfstring(L, "attempt to access a closed %s", SettingsType);
		luaL_argerror(L, pos, s);
	}

	return udata;
}

static int Settings_set(lua_State *L)
{
	struct FarSettingsItem fsi;
	FarSettingsUdata* udata = CheckSettings(L, 1);
	fsi.StructSize = sizeof(fsi);
	fsi.Root = (size_t)check_env_flag(L, 2);
	fsi.Name = opt_utf8_string(L, 3, NULL);
	fsi.Type = (enum FARSETTINGSTYPES) check_env_flag(L, 4);

	if (fsi.Type == FST_QWORD)
		fsi.Value.Number = GetFlagCombination(L, 5, NULL);
	else if (fsi.Type == FST_STRING)
		fsi.Value.String = check_utf8_string(L, 5, NULL);
	else if (fsi.Type == FST_DATA)
		fsi.Value.Data.Data = luaL_checklstring(L, 5, &fsi.Value.Data.Size);
	else
		return lua_pushboolean(L,0), 1;

	lua_pushboolean(L, GetPluginData(L)->Info->SettingsControl(udata->Handle, SCTL_SET, 0, &fsi) != 0);
	return 1;
}

static int Settings_get(lua_State *L)
{
	struct FarSettingsItem fsi;
	FarSettingsUdata* udata = CheckSettings(L, 1);
	fsi.StructSize = sizeof(fsi);
	fsi.Root = (size_t)check_env_flag(L, 2);
	fsi.Name = check_utf8_string(L, 3, NULL);
	fsi.Type = (enum FARSETTINGSTYPES) check_env_flag(L, 4);

	if (GetPluginData(L)->Info->SettingsControl(udata->Handle, SCTL_GET, 0, &fsi))
	{
		if (fsi.Type == FST_QWORD)
			bit64_push(L, fsi.Value.Number);
		else if (fsi.Type == FST_STRING)
			push_utf8_string(L, fsi.Value.String, -1);
		else if (fsi.Type == FST_DATA)
			lua_pushlstring(L, fsi.Value.Data.Data, fsi.Value.Data.Size);
		else
			lua_pushnil(L);
	}
	else
		lua_pushnil(L);

	return 1;
}

static int Settings_delete(lua_State *L)
{
	struct FarSettingsValue fsv;
	FarSettingsUdata* udata = CheckSettings(L, 1);
	fsv.StructSize = sizeof(fsv);
	fsv.Root = (size_t)check_env_flag(L, 2);
	fsv.Value = opt_utf8_string(L, 3, NULL);
	lua_pushboolean(L, GetPluginData(L)->Info->SettingsControl(udata->Handle, SCTL_DELETE, 0, &fsv) != 0);
	return 1;
}

static int Settings_createsubkey(lua_State *L)
{
	PSInfo *Info = GetPluginData(L)->Info;
	const wchar_t *description;
	struct FarSettingsValue fsv;
	intptr_t subkey;
	FarSettingsUdata* udata = CheckSettings(L, 1);
	fsv.StructSize = sizeof(fsv);
	fsv.Root = (size_t)check_env_flag(L, 2);
	fsv.Value = check_utf8_string(L, 3, NULL);
	description = opt_utf8_string(L, 4, NULL);
	subkey = Info->SettingsControl(udata->Handle, SCTL_CREATESUBKEY, 0, &fsv);

	if (subkey != 0)
	{
		if (description != NULL)
		{
			struct FarSettingsItem fsi;
			fsi.StructSize = sizeof(fsi);
			fsi.Root = subkey;
			fsi.Name = NULL;
			fsi.Type = FST_STRING;
			fsi.Value.String = description;
			Info->SettingsControl(udata->Handle, SCTL_SET, 0, &fsi);
		}

		lua_pushinteger(L, subkey);
	}
	else
		lua_pushnil(L);

	return 1;
}

static int Settings_opensubkey(lua_State *L)
{
	intptr_t subkey;
	struct FarSettingsValue fsv;
	FarSettingsUdata* udata = CheckSettings(L, 1);
	fsv.StructSize = sizeof(fsv);
	fsv.Root = (size_t)check_env_flag(L, 2);
	fsv.Value = check_utf8_string(L, 3, NULL);
	subkey = GetPluginData(L)->Info->SettingsControl(udata->Handle, SCTL_OPENSUBKEY, 0, &fsv);

	if (subkey != 0)
		lua_pushinteger(L, subkey);
	else
		lua_pushnil(L);

	return 1;
}

static int Settings_enum(lua_State *L)
{
	struct FarSettingsEnum fse;
	intptr_t i, from = 1, to = -1;
	FarSettingsUdata* udata = CheckSettings(L, 1);
	fse.StructSize = sizeof(fse);
	fse.Root = (size_t)check_env_flag(L, 2);

	if (!lua_isnoneornil(L, 3))  from = luaL_checkinteger(L, 3);

	if (!lua_isnoneornil(L, 4))  to = luaL_checkinteger(L, 4);

	if (GetPluginData(L)->Info->SettingsControl(udata->Handle, SCTL_ENUM, 0, &fse))
	{
		if (from < 1 && (from += fse.Count + 1) < 1) from = 1;

		--from;

		if (to < 0 && (to += fse.Count + 1) < 0) to = 0;

		if (to > (int)fse.Count) to = fse.Count;

		lua_createtable(L, (int)fse.Count, 1);
		PutIntToTable(L, "Count", (int)fse.Count);

		for(i = from; i < to; i++)
		{
			if (udata->IsFarSettings)
			{
				const struct FarSettingsHistory *fsh = fse.Value.Histories + i;
				lua_createtable(L, 0, 6);

				if (fsh->Name) PutWStrToTable(L, "Name", fsh->Name, -1);

				if (fsh->Param) PutWStrToTable(L, "Param", fsh->Param, -1);

				PutLStrToTable(L, "PluginId", &fsh->PluginId, sizeof(GUID));

				if (fsh->File) PutWStrToTable(L, "File", fsh->File, -1);

				pushFileTime(L, &fsh->Time);
				lua_setfield(L, -2, "Time");
				PutBoolToTable(L, "Lock", fsh->Lock);
			}
			else
			{
				lua_createtable(L, 0, 2);
				PutWStrToTable(L, "Name", fse.Value.Items[i].Name, -1);
				PutIntToTable(L, "Type", fse.Value.Items[i].Type);
			}

			lua_rawseti(L, -2, (int)(i-from+1));
		}
	}
	else
		lua_pushnil(L);

	return 1;
}

static int Settings_free(lua_State *L)
{
	FarSettingsUdata* udata = GetSettingsUdata(L, 1);

	if (udata->Handle != INVALID_HANDLE_VALUE)
	{
		PSInfo *Info = GetPluginData(L)->Info;
		Info->SettingsControl(udata->Handle, SCTL_FREE, 0, 0);
		udata->Handle = INVALID_HANDLE_VALUE;
		lua_getfield(L, LUA_REGISTRYINDEX, SettingsHandles);
		lua_pushvalue(L, 1);
		lua_pushnil(L);
		lua_rawset(L, -3);
	}

	return 0;
}

static int far_FreeSettings(lua_State *L)
{
	lua_getfield(L, LUA_REGISTRYINDEX, SettingsHandles);
	lua_pushnil(L);

	while(lua_next(L, -2))
	{
		lua_pushcfunction(L, Settings_free);
		lua_pushvalue(L, -3);
		lua_call(L, 1, 0);
		lua_pop(L, 1);
	}

	lua_pop(L, 1); // mandatory, since this function is called directly from pcall_msg
	return 0;
}

static int Settings_tostring(lua_State *L)
{
	FarSettingsUdata* udata = GetSettingsUdata(L, 1);

	if (udata->Handle != INVALID_HANDLE_VALUE)
		lua_pushfstring(L, "%s (%p)", SettingsType, udata->Handle);
	else
		lua_pushfstring(L, "%s (closed)", SettingsType);

	return 1;
}

static int far_ColorDialog(lua_State *L)
{
	UINT64 Flags;
	struct FarColor Color;
	TPluginData *pd = GetPluginData(L);

	if (!GetFarColor(L, 1, &Color))
	{
		Color.Foreground.ForegroundColor = 0x0F | ALPHAMASK;
		Color.Background.BackgroundColor = 0x00 | ALPHAMASK;
		Color.Flags = FCF_4BITMASK;
	}

	Flags = OptFlags(L, 2, 0);

	if (pd->Info->ColorDialog(pd->PluginId, Flags, &Color))
		PushFarColor(L, &Color);
	else
		lua_pushnil(L);

	return 1;
}

static int far_RunDefaultScript(lua_State *L)
{
	lua_pushboolean(L, RunDefaultScript(L, 0));
	return 1;
}

static int far_FileTimeResolution(lua_State *L)
{
	lua_Integer op = luaL_optinteger(L, 1, 0);
	TPluginData *pd = GetPluginData(L);
	int ret = (pd->Flags & PDF_FULL_TIME_RESOLUTION) ? 2:1;
	if (op == 1)
		pd->Flags &= ~PDF_FULL_TIME_RESOLUTION;
	else if (op == 2)
		pd->Flags |= PDF_FULL_TIME_RESOLUTION;
	lua_pushinteger(L, ret);
	return 1;
}

static int far_DetectCodePage(lua_State *L)
{
	int codepage;
	struct DetectCodePageInfo Info;
	Info.StructSize = sizeof(Info);
	Info.FileName = check_utf8_string(L, 1, NULL);
	codepage = GetPluginData(L)->FSF->DetectCodePage(&Info);
	if (codepage)
		lua_pushinteger(L, codepage);
	else
		lua_pushnil(L);
	return 1;
}

#define PAIR(prefix,txt) {#txt, prefix ## _ ## txt}

const luaL_Reg timer_methods[] =
{
	{"__gc",                timer_gc},
	{"__tostring",          timer_tostring},
	{"__index",             timer_index},
	{"__newindex",          timer_newindex},
	{NULL, NULL},
};

const luaL_Reg filefilter_methods[] =
{
	{"__gc",                filefilter_gc},
	{"__tostring",          filefilter_tostring},
	{"FreeFileFilter",      filefilter_Free},
	{"OpenFiltersMenu",     filefilter_OpenMenu},
	{"StartingToFilter",    filefilter_Starting},
	{"IsFileInFilter",      filefilter_IsFileInFilter},
	{NULL, NULL},
};

const luaL_Reg dialog_methods[] =
{
	{"__gc",                far_DialogFree},
	{"__tostring",          dialog_tostring},
	{"rawhandle",           dialog_rawhandle},
	{"send",                far_SendDlgMessage},

	PAIR( dlg, AddHistory),
	PAIR( dlg, Close),
	PAIR( dlg, EditUnchangedFlag),
	PAIR( dlg, Enable),
	PAIR( dlg, EnableRedraw),
	PAIR( dlg, GetCheck),
	PAIR( dlg, GetComboboxEvent),
	PAIR( dlg, GetConstTextPtr),
	PAIR( dlg, GetCursorPos),
	PAIR( dlg, GetCursorSize),
	PAIR( dlg, GetDialogInfo),
	PAIR( dlg, GetDialogTitle),
	PAIR( dlg, GetDlgData),
	PAIR( dlg, GetDlgItem),
	PAIR( dlg, GetDlgRect),
	PAIR( dlg, GetDropdownOpened),
	PAIR( dlg, GetEditPosition),
	PAIR( dlg, GetFocus),
	PAIR( dlg, GetItemData),
	PAIR( dlg, GetItemPosition),
	PAIR( dlg, GetSelection),
	PAIR( dlg, GetText),
	PAIR( dlg, Key),
	PAIR( dlg, ListAdd),
	PAIR( dlg, ListAddStr),
	PAIR( dlg, ListDelete),
	PAIR( dlg, ListFindString),
	PAIR( dlg, ListGetCurPos),
	PAIR( dlg, ListGetData),
	PAIR( dlg, ListGetDataSize),
	PAIR( dlg, ListGetItem),
	PAIR( dlg, ListGetTitles),
	PAIR( dlg, ListInfo),
	PAIR( dlg, ListInsert),
	PAIR( dlg, ListSet),
	PAIR( dlg, ListSetCurPos),
	PAIR( dlg, ListSetData),
	PAIR( dlg, ListSetTitles),
	PAIR( dlg, ListSort),
	PAIR( dlg, ListUpdate),
	PAIR( dlg, MoveDialog),
	PAIR( dlg, Redraw),
	PAIR( dlg, ResizeDialog),
	PAIR( dlg, Set3State),
	PAIR( dlg, SetCheck),
	PAIR( dlg, SetComboboxEvent),
	PAIR( dlg, SetCursorPos),
	PAIR( dlg, SetCursorSize),
	PAIR( dlg, SetDlgData),
	PAIR( dlg, SetDlgItem),
	PAIR( dlg, SetDropdownOpened),
	PAIR( dlg, SetEditPosition),
	PAIR( dlg, SetFocus),
	PAIR( dlg, SetHistory),
	PAIR( dlg, SetInputNotify),
	PAIR( dlg, SetItemData),
	PAIR( dlg, SetItemPosition),
	PAIR( dlg, SetMaxTextLength),
	PAIR( dlg, SetSelection),
	PAIR( dlg, SetText),
	PAIR( dlg, SetTextPtr),
	PAIR( dlg, ShowDialog),
	PAIR( dlg, ShowItem),
	PAIR( dlg, User),

	{NULL, NULL},
};

static const luaL_Reg actl_funcs[] =
{
	PAIR( adv, Commit),
	PAIR( adv, GetArrayColor),
	PAIR( adv, GetColor),
	PAIR( adv, GetCursorPos),
	PAIR( adv, GetFarHwnd),
	PAIR( adv, GetFarmanagerVersion),
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
	PAIR( adv, Waitkey),

	{NULL, NULL},
};

const luaL_Reg Settings_methods[] =
{
	{"__gc",                Settings_free},
	{"__tostring",          Settings_tostring},
	{"Delete",              Settings_delete},
	{"Enum",                Settings_enum},
	{"Free",                Settings_free},
	{"Get",                 Settings_get},
	{"Set",                 Settings_set},
	{"CreateSubkey",        Settings_createsubkey},
	{"OpenSubkey",          Settings_opensubkey},
	{NULL, NULL},
};

const luaL_Reg editor_funcs[] =
{
	PAIR( editor, AddColor),
	PAIR( editor, AddSessionBookmark),
	PAIR( editor, ClearSessionBookmarks),
	PAIR( editor, DelColor),
	PAIR( editor, DeleteBlock),
	PAIR( editor, DeleteChar),
	PAIR( editor, DeleteSessionBookmark),
	PAIR( editor, DeleteString),
	PAIR( editor, Editor),
	PAIR( editor, ExpandTabs),
	PAIR( editor, GetBookmarks),
	PAIR( editor, GetColor),
	PAIR( editor, GetFileName),
	PAIR( editor, GetInfo),
	PAIR( editor, GetSelection),
	PAIR( editor, GetSessionBookmarks),
	PAIR( editor, GetString),
	PAIR( editor, GetStringW),
	PAIR( editor, GetTitle),
	PAIR( editor, InsertString),
	PAIR( editor, InsertText),
	PAIR( editor, InsertTextW),
	PAIR( editor, NextSessionBookmark),
	PAIR( editor, PrevSessionBookmark),
	PAIR( editor, ProcessInput),
	PAIR( editor, Quit),
	PAIR( editor, ReadInput),
	PAIR( editor, RealToTab),
	PAIR( editor, Redraw),
	PAIR( editor, SaveFile),
	PAIR( editor, Select),
	PAIR( editor, SetKeyBar),
	PAIR( editor, SetParam),
	PAIR( editor, SetPosition),
	PAIR( editor, SetString),
	PAIR( editor, SetStringW),
	PAIR( editor, SetTitle),
	PAIR( editor, SubscribeChangeEvent),
	PAIR( editor, TabToReal),
	PAIR( editor, UndoRedo),

	{NULL, NULL},
};

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

const luaL_Reg far_funcs[] =
{
	PAIR( far, AdvControl),
	PAIR( far, CPluginStartupInfo),
	PAIR( far, CheckMask),
	PAIR( far, CmpName),
	PAIR( far, CmpNameList),
	PAIR( far, ColorDialog),
	PAIR( far, ConvertPath),
	PAIR( far, CopyToClipboard),
	PAIR( far, CreateFileFilter),
	PAIR( far, CreateSettings),
	PAIR( far, DetectCodePage),
	PAIR( far, DialogFree),
	PAIR( far, DialogInit),
	PAIR( far, DialogRun),
	PAIR( far, FarClock),
	PAIR( far, FileTimeResolution),
	PAIR( far, FindPlugin),
	PAIR( far, ForcedLoadPlugin),
	PAIR( far, FormatFileSize),
	PAIR( far, FreeScreen),
	PAIR( far, FreeSettings),
	PAIR( far, GenerateName),
	PAIR( far, GetCurrentDirectory),
	PAIR( far, GetDirList),
	PAIR( far, GetDlgItem),
	PAIR( far, GetFileOwner),
	PAIR( far, GetLuafarVersion),
	PAIR( far, GetMsg),
	PAIR( far, GetNumberOfLinks),
	PAIR( far, GetPathRoot),
	PAIR( far, GetPluginDirList),
	PAIR( far, GetPluginInformation),
	PAIR( far, GetPlugins),
	PAIR( far, GetReparsePointInfo),
	PAIR( far, InputBox),
	PAIR( far, InputRecordToName),
	PAIR( far, IsPluginLoaded),
	PAIR( far, LIsAlpha),
	PAIR( far, LIsAlphanum),
	PAIR( far, LIsLower),
	PAIR( far, LIsUpper),
	PAIR( far, LLowerBuf),
	PAIR( far, LStricmp),
	PAIR( far, LStrnicmp),
	PAIR( far, LUpperBuf),
	PAIR( far, LoadPlugin),
	PAIR( far, MacroAdd),
	PAIR( far, MacroCheck),
	PAIR( far, MacroDelete),
	PAIR( far, MacroExecute),
	PAIR( far, MacroGetArea),
	PAIR( far, MacroGetLastError),
	PAIR( far, MacroGetState),
	PAIR( far, MacroLoadAll),
	PAIR( far, MacroPost),
	PAIR( far, MacroSaveAll),
	PAIR( far, MakeMenuItems),
	PAIR( far, Menu),
	PAIR( far, Message),
	PAIR( far, MkLink),
	PAIR( far, MkTemp),
	PAIR( far, NameToInputRecord),
	PAIR( far, PasteFromClipboard),
	PAIR( far, PluginStartupInfo),
	PAIR( far, ProcessName),
	PAIR( far, RecursiveSearch),
	PAIR( far, RestoreScreen),
	PAIR( far, RunDefaultScript),
	PAIR( far, SaveScreen),
	PAIR( far, SendDlgMessage),
	PAIR( far, SetDlgItem),
	PAIR( far, Show),
	PAIR( far, ShowHelp),
	PAIR( far, SubscribeDialogDrawEvents),
	PAIR( far, Text),
	PAIR( far, Timer),
	PAIR( far, TruncPathStr),
	PAIR( far, TruncStr),
	PAIR( far, UnloadPlugin),
	PAIR( far, XLat),

	{NULL, NULL}
};

static const char far_Dialog[] =
"function far.Dialog (Id,X1,Y1,X2,Y2,HelpTopic,Items,Flags,DlgProc,Param)\n"
  "local hDlg = far.DialogInit(Id,X1,Y1,X2,Y2,HelpTopic,Items,Flags,DlgProc,Param)\n"
  "if hDlg == nil then return nil end\n"

  "local ret = far.DialogRun(hDlg)\n"
  "for i, item in ipairs(Items) do\n"
    "local newitem = far.GetDlgItem(hDlg, i)\n"
    "if type(item[6]) == 'table' then\n"
      "local pos = far.SendDlgMessage(hDlg, 'DM_LISTGETCURPOS', i, 0)\n"
      "item[6].SelectIndex = pos.SelectPos\n"
    "else\n"
      "item[6] = newitem[6]\n"
    "end\n"
    "item[10] = newitem[10]\n"
  "end\n"

  "far.DialogFree(hDlg)\n"
  "return ret\n"
"end";

static const char utf8_reformat[] =
"function utf8.reformat (patt, ...)\n"
  "local args = { ... }\n"
  "local function Subst (i, m, f)\n"
    "i = tonumber(i)\n"
    "f = f:match('[^s]')\n"
    "return args[i] and ('%' .. m .. (f or 's')):format(f and args[i] or tostring(args[i])) or ''\n"
  "end\n"

  "patt = patt:gsub('%f[%%{]{(%d+):?(%-?%d*%.?%d*)([A-Za-z]?)}', Subst):gsub('%%{', '{')\n"
  "return patt:format(...)\n"
"end";

static int luaopen_far(lua_State *L)
{
	HANDLE TimerQueue = CreateTimerQueue();
	if (TimerQueue)
	{
		lua_pushlightuserdata(L, TimerQueue);
		lua_setfield(L, LUA_REGISTRYINDEX, FarTimerQueueKey);
	}

	lua_newtable(L);
	lua_setfield(L, LUA_REGISTRYINDEX, FAR_DN_STORAGE);

	NewVirtualKeyTable(L, FALSE);
	lua_setfield(L, LUA_REGISTRYINDEX, FAR_VIRTUALKEYS);
	luaL_register(L, "far", far_funcs);

	luaopen_far_host(L);
	lua_setfield(L, -2, "Host");

	if (GetPluginData(L)->Info->Private)
	{
		lua_pushcfunction(L, far_MacroCallFar);
		lua_setfield(L, -2, "MacroCallFar");
		lua_pushcfunction(L, far_MacroCallToLua);
		lua_setfield(L, -2, "MacroCallToLua");
	}

	push_flags_table(L);
	lua_pushvalue(L, -1);
	lua_setfield(L, -3, "Flags");
	lua_setfield(L, LUA_REGISTRYINDEX, FAR_FLAGSTABLE);

	SetFarColors(L);

	luaL_register(L, "editor", editor_funcs);
	luaL_register(L, "viewer", viewer_funcs);
	luaL_register(L, "panel",  panel_funcs);
	luaL_register(L, "actl",   actl_funcs);

	luaL_newmetatable(L, FarFileFilterType);
	lua_pushvalue(L,-1);
	lua_setfield(L, -2, "__index");
	luaL_register(L, NULL, filefilter_methods);

	luaL_newmetatable(L, FarTimerType);
	luaL_register(L, NULL, timer_methods);

	luaL_newmetatable(L, FarDialogType);
	lua_pushvalue(L,-1);
	lua_setfield(L, -2, "__index");
	lua_pushcfunction(L, DialogHandleEqual);
	lua_setfield(L, -2, "__eq");
	luaL_register(L, NULL, dialog_methods);

	luaL_newmetatable(L, SettingsType);
	lua_pushvalue(L,-1);
	lua_setfield(L, -2, "__index");
	luaL_register(L, NULL, Settings_methods);
	lua_newtable(L);
	lua_newtable(L);
	lua_pushliteral(L, "k");
	lua_setfield(L, -2, "__mode");
	lua_setmetatable(L, -2);
	lua_setfield(L, LUA_REGISTRYINDEX, SettingsHandles);

	(void) luaL_dostring(L, far_Dialog);

	luaL_newmetatable(L, PluginHandleType);
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	lua_pushcfunction(L, PluginHandle_rawhandle);
	lua_setfield(L, -2, "rawhandle");

	luaL_newmetatable(L, AddMacroDataType);
	lua_pushcfunction(L, AddMacroData_gc);
	lua_setfield(L, -2, "__gc");

	luaL_newmetatable(L, SavedScreenType);
	lua_pushcfunction(L, far_FreeScreen);
	lua_setfield(L, -2, "__gc");
	lua_pushcfunction(L, SavedScreen_tostring);
	lua_setfield(L, -2, "__tostring");

	return 0;
}

void LF_RunLuafarInit(lua_State* L)
{
	const wchar_t *filename = L"\\luafar_init.lua";
	wchar_t buf[2048];
	int size;

	size = GetEnvironmentVariableW(L"FARPROFILE", buf, ARRSIZE(buf));
	if (size && (size + wcslen(filename) < ARRSIZE(buf)))
	{
		DWORD attr = GetFileAttributesW(wcscat(buf, filename));
		if (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY) == 0)
		{
			int status = LF_LoadFile(L, buf) || lua_pcall(L,0,0,0);
			if (status)
			{
				LF_Error(L, check_utf8_string(L, -1, NULL));
				lua_pop(L, 1);
			}
		}
	}
}

static const luaL_Reg lualibs[] =
{
#if LUA_VERSION_NUM == 501
	{"",              luaopen_base},
	{LUA_LOADLIBNAME, luaopen_upackage},  // changed
#else
	{"_G",            luaopen_base},
	{LUA_LOADLIBNAME, luaopen_upackage},  // changed
	{LUA_BITLIBNAME,  luaopen_bit32},
	{LUA_COLIBNAME,   luaopen_coroutine},
#endif
	{LUA_TABLIBNAME,  luaopen_table},
	{LUA_IOLIBNAME,   luaopen_uio},       // changed
	{LUA_OSLIBNAME,   luaopen_os},
	{LUA_STRLIBNAME,  luaopen_string},
	{LUA_MATHLIBNAME, luaopen_math},
	{LUA_DBLIBNAME,   luaopen_debug},
	//-------------------------------------------------
	{"bit64",         luaopen_bit64},
	{"unicode",       luaopen_unicode},
	{"utf8",          luaopen_utf8},
	{"win",           luaopen_win},
	{"lpeg",          luaopen_lpeg},
	{NULL, NULL}
};

void LF_InitLuaState1(lua_State *L, lua_CFunction aOpenLibs)
{
	const luaL_Reg *lib;

	FP_PROTECT();

	// open Lua libraries
	for(lib=lualibs; lib->func; lib++)
	{
#if LUA_VERSION_NUM == 501
		lua_pushcfunction(L, lib->func);
		lua_pushstring(L, lib->name);
		lua_call(L, 1, 0);
#elif LUA_VERSION_NUM == 502
		luaL_requiref(L, lib->name, lib->func, 1);
		lua_pop(L, 1);  /* remove lib */
#endif
	}

	lua_getglobal(L, "utf8");                   //+1
	lua_getglobal(L, "string");                 //+2
	// utf8.dump = string.dump
	lua_getfield(L, -1, "dump");                //+3
	lua_setfield(L, -3, "dump");                //+2
	// utf8.rep = string.rep
	lua_getfield(L, -1, "rep");                 //+3
	lua_setfield(L, -3, "rep");                 //+2
	// getmetatable("").__index = utf8
	lua_pushliteral(L, "");                     //+3
	lua_getmetatable(L, -1);                    //+4
	lua_pushvalue(L, -4);                       //+5
	lua_setfield(L, -2, "__index");	            //+4
	lua_pop(L, 4);                              //+0
	// add utf8.reformat
	(void) luaL_dostring(L, utf8_reformat);

	// unicode.utf8 = utf8 (for backward compatibility;)
	lua_newtable(L);
	lua_getglobal(L, "utf8");
	lua_setfield(L, -2, "utf8");
	lua_setglobal(L, "unicode");

	// utf8.cfind = utf8.find (for backward compatibility;)
	lua_getglobal(L, "utf8");
	lua_getfield(L, -1, "find");
	lua_setfield(L, -2, "cfind");
	lua_pop(L, 1);

#if LUA_VERSION_NUM == 501
	if (IsLuaJIT())
	{
		if (luaopen_bit)
		{
			lua_pushcfunction(L, luaopen_bit);
			lua_pushstring(L, "bit");
			lua_call(L, 1, 0);
		}
		if (luaopen_jit)
		{
			lua_pushcfunction(L, luaopen_jit);
			lua_pushstring(L, "jit");
			lua_call(L, 1, 0);
		}
		if (luaopen_ffi)
		{
			luaL_findtable(L, LUA_REGISTRYINDEX, "_PRELOAD", 1);
			lua_pushcfunction(L, luaopen_ffi);
			lua_setfield(L, -2, "ffi");
			lua_pop(L, 1);
		}
	}
#endif

	if (aOpenLibs) aOpenLibs(L);

	lua_pushcfunction(L, luaB_dofileW);
	lua_setglobal(L, "dofile");
	lua_pushcfunction(L, luaB_loadfileW);
	lua_setglobal(L, "loadfile");
}

static const luaL_Reg lualibs_extra[] =
{
	{"bit64",         luaopen_bit64},
	{"unicode",       luaopen_unicode},
	{"utf8",          luaopen_utf8},
	{"win",           luaopen_win},
	{"lpeg",          luaopen_lpeg},
	{NULL, NULL}
};

static void LoadExtraLibraries(lua_State *L)
{
	const luaL_Reg *lib;

	FP_PROTECT();

	// open Lua libraries
	for(lib=lualibs_extra; lib->func; lib++)
	{
		lua_pushcfunction(L, lib->func);
		lua_pushstring(L, lib->name);
		lua_call(L, 1, 0);
	}

	// add "luafar" namespace with a few functions
	lua_newtable(L);
	lua_pushcfunction(L, far_GetLuafarVersion);
	lua_setfield(L, -2, "GetLuafarVersion");
	lua_pushcfunction(L, far_FileTimeResolution);
	lua_setfield(L, -2, "FileTimeResolution");
	lua_setglobal(L, "luafar");

	// add utf8.reformat
	(void) luaL_dostring(L, utf8_reformat);

	// getmetatable("").__index = utf8
	lua_pushliteral(L, "");
	lua_getmetatable(L, -1);
	lua_getglobal(L, "utf8");
	lua_setfield(L, -2, "__index");
	lua_pop(L, 2);
}

static void* CustomAllocator(void *ud, void *ptr, size_t osize, size_t nsize)
{
	return ((TPluginData*)ud)->origAlloc(((TPluginData*)ud)->origUserdata, ptr, osize, nsize);
}

void LF_InitLuaState2(lua_State *L, TPluginData *aInfo)
{
	FP_PROTECT();
	aInfo->MainLuaState = L;
	aInfo->Flags = 0;
	aInfo->origAlloc = lua_getallocf(L, &aInfo->origUserdata);
	lua_setallocf(L, CustomAllocator, aInfo);
	// open "far" library
	lua_pushcfunction(L, luaopen_far);
	lua_call(L, 0, 0);
	// open "regex" library
	lua_pushcfunction(L, luaopen_regex);
	lua_pushliteral(L, "regex");
	lua_call(L, 1, 0);
	// open "usercontrol" library
	lua_pushcfunction(L, luaopen_usercontrol);
	lua_call(L, 0, 0);
}

// This exported function is needed for old builds of the plugins.
intptr_t LF_MacroCallback(lua_State* L, void* Id, FARADDKEYMACROFLAGS Flags) { return 0; }

int LF_DoFile(lua_State *L, const wchar_t *fname, int argc, wchar_t* argv[])
{
	int status;

	if ((status = LF_LoadFile(L, fname)) == 0)
	{
		int i;

		for(i=0; i < argc; i++)
			push_utf8_string(L, argv[i], -1);

		status = lua_pcall(L, argc, 0, 0);
	}

	if (status)
	{
		fprintf(stderr, "%s\n", lua_tostring(L, -1));
		lua_pop(L, 1);
	}

	return status;
}

const LuafarAPI api_functions = {
	0,

	check_utf8_string,
	opt_utf8_string,
	push_utf8_string,
	check_utf16_string,
	opt_utf16_string,
	push_utf16_string,
	utf8_to_utf16,

	GetBoolFromTable,
	GetOptBoolFromTable,
	GetOptIntFromArray,
	GetOptIntFromTable,
	GetOptNumFromTable,
	PutBoolToTable,
	PutIntToArray,
	PutIntToTable,
	PutLStrToTable,
	PutNumToTable,
	PutStrToArray,
	PutStrToTable,
	PutWStrToArray,
	PutWStrToTable,

	GetExportFunction,
	pcall_msg,

	bit64_pushuserdata,
	bit64_push,
	bit64_getvalue,
};

void LF_GetLuafarAPI (LuafarAPI* target)
{
	size_t size = target->StructSize;
	memset(target, 0, size); // fill target with nulls (it helps to detect missing functions)
	if (size > sizeof(LuafarAPI))
		size = sizeof(LuafarAPI);
	memcpy(target, &api_functions, size);
	target->StructSize = size;
}

// This function makes possible use of luafar3.dll as a library without Far Manager.
// It is called by means of: require("luafar3")
__declspec(dllexport) int luaopen_luafar3 (lua_State *L)
{
	InsideFarManager = 0;

	lua_getglobal(L, "far");
	if (lua_istable(L, -1))
	{
		lua_getfield(L, -1, "ConvertPath");
		InsideFarManager = lua_isfunction(L, -1);
		lua_pop(L, 1);
	}
	lua_pop(L, 1);

	if (! InsideFarManager)
		LoadExtraLibraries(L);

	return 0;
}
