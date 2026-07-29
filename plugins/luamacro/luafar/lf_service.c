//---------------------------------------------------------------------------

#include <windows.h>
#include <rpc.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

#include "lf_bit64.h"
#include "lf_common.h"
#include "lf_flags.h"
#include "lf_luafar.h"
#include "lf_service.h"
#include "lf_string.h"
#include "lf_util.h"
#include "lf_version.h"

#ifndef LUADLL
# if LUA_VERSION_NUM == 501
#  define LUADLL "lua51.dll"
# elif LUA_VERSION_NUM == 502
#  define LUADLL "lua52.dll"
# endif
#endif

extern int luaopen_far_host(lua_State *L);
extern int luaopen_regex(lua_State*);
extern int luaopen_usercontrol(lua_State*);
extern int luaopen_uio(lua_State *L);
extern int luaopen_unicode(lua_State *L);
extern int luaopen_utf8(lua_State *L);
extern int luaopen_upackage(lua_State *L);
extern int luaopen_win(lua_State *L);
extern int luaopen_lpeg(lua_State *L);
extern int luaopen_editor(lua_State *L);
extern int luaopen_dialog(lua_State *L);
extern int luaopen_panel(lua_State *L);
extern int luaopen_actl(lua_State *L);
extern int luaopen_macro(lua_State *L);
extern int luaopen_viewer(lua_State *L);

extern int  luaB_dofileW(lua_State *L);
extern int  luaB_loadfileW(lua_State *L);
extern void push_flags_table(lua_State *L);
extern void SetFarColors(lua_State *L);
extern void WINAPI FarPanelItemFreeCallback(void* UserData, const struct FarPanelItemFreeInfo* Info);
extern int far_MacroCallFar(lua_State *L);
extern int far_MacroCallToLua(lua_State *L);
extern int GetExportFunction(lua_State* L, const char* FuncName);
extern BOOL RunDefaultScript(lua_State* L, int ForFirstTime);

const char FarTimerQueueKey[]  = "FarTimerQueue";
const char SettingsHandles[]   = "FarSettingsHandles";

const char FAR_VIRTUALKEYS[]   = "far.virtualkeys";
const char FAR_FLAGSTABLE[]    = "far.Flags";

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

TSynchroData* CreateSynchroData(int type, int data, TTimerData *td)
{
	TSynchroData* SD = (TSynchroData*) malloc(sizeof(TSynchroData));
	SD->type = type;
	SD->data = data;
	SD->ref = LUA_REFNIL;
	SD->timerData = td;
	return SD;
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
		luaL_getmetatable(L, TYPE_PLUGINHANDLE);
		lua_setmetatable(L, -2);
	}
	else
		lua_pushnil(L);
}

static int PluginHandle_rawhandle(lua_State *L)
{
	void* Handle = *(void**)luaL_checkudata(L, 1, TYPE_PLUGINHANDLE);
	lua_pushlightuserdata(L, Handle);
	return 1;
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
		lua_createtable(L, (int)PanelItem->CustomColumnNumber, 0);

		for(int j=0; j < (int)PanelItem->CustomColumnNumber; j++)
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

void PutRECTToTable(lua_State *L, const char* key, RECT rect)
{
	lua_createtable(L, 0, 4);
	PutIntToTable(L, "left", rect.left);
	PutIntToTable(L, "top", rect.top);
	PutIntToTable(L, "right", rect.right);
	PutIntToTable(L, "bottom", rect.bottom);
	lua_setfield(L, -2, key);
}
//---------------------------------------------------------------------------

void PushPanelItems(lua_State *L, const struct PluginPanelItem *PanelItems, size_t ItemsNumber, int NoUserData)
{
	lua_createtable(L, (int)ItemsNumber, 0); // "PanelItems"

	for(int i=0; i < (int)ItemsNumber; i++)
	{
		PushPanelItem(L, PanelItems + i, NoUserData);
		lua_rawseti(L, -2, i+1);
	}
}
//---------------------------------------------------------------------------

static int far_PluginStartupInfo(lua_State *L)
{
	intptr_t len=0;
	TPluginData *pd = GetPluginData(L);
	lua_createtable(L, 0, 4);
	PutWStrToTable(L, "ModuleName", pd->Info->ModuleName, -1);

	for(const wchar_t *p = pd->Info->ModuleName; *p; p++)
	{
		if (*p == L'\\')
			len = p - pd->Info->ModuleName;
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

static void FillKeyBarTitles(lua_State *L, int src_pos, struct KeyBarTitles *kbt)
{
	lua_newtable(L);
	int store_pos = lua_gettop(L);
	//-------------------------------------------------------------------------
	memset(kbt, 0, sizeof(*kbt));
	kbt->CountLabels = lua_objlen(L, src_pos);
	size_t size = kbt->CountLabels * sizeof(struct KeyBarLabel);
	kbt->Labels = (struct KeyBarLabel*)lua_newuserdata(L, size);
	memset(kbt->Labels, 0, size);

	for(int i=0; i < (int)kbt->CountLabels; i++)
	{
		lua_rawgeti(L, src_pos, i+1);

		if (!lua_istable(L, -1))
		{
			kbt->CountLabels = i;
			lua_pop(L, 1);
			break;
		}

		kbt->Labels[i].Key.VirtualKeyCode = GetOptIntFromTable(L, "VirtualKeyCode", 0);
		kbt->Labels[i].Key.ControlKeyState = (DWORD) CheckFlagsFromTable(L, -1, "ControlKeyState");
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

int SetKeyBar(lua_State *L, BOOL editor)
{
	void* param = NULL;
	struct KeyBarTitles kbt;
	struct FarSetKeyBarTitles skbt = { sizeof(skbt) };
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
		skbt.Titles = &kbt;
	}
	else
		argfail = TRUE;

	if (argfail)
		return luaL_argerror(L, 2, "must be 'redraw', 'restore', or table");

	intptr_t result = editor ? Info->EditorControl(Id, ECTL_SETKEYBAR, 0, param) :
	         Info->ViewerControl(Id, VCTL_SETKEYBAR, 0, param);
	lua_pushboolean(L, result != 0);
	return 1;
}

int GetFarColor(lua_State *L, int pos, struct FarColor* Color)
{
	if (lua_istable(L, pos))
	{
		lua_pushvalue(L, pos);
		Color->Flags = CheckFlagsFromTable(L, -1, "Flags");
		Color->Foreground.ForegroundColor = (COLORREF) GetOptNumFromTable(L, "ForegroundColor", 0);
		Color->Background.BackgroundColor = (COLORREF) GetOptNumFromTable(L, "BackgroundColor", 0);
		Color->Underline.UnderlineColor = (COLORREF) GetOptNumFromTable(L, "UnderlineColor", 0);
		Color->Reserved = 0;
		lua_pop(L, 1);
		return 1;
	}
	else if (lua_isnumber(L, pos))
	{
		DWORD num = (DWORD)lua_tonumber(L, pos);
		Color->Flags = FCF_INDEXMASK;
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
	PutNumToTable(L, "UnderlineColor", Color->Underline.UnderlineColor);
}

void GetOptGuid(lua_State *L, int pos, GUID* target, const GUID* source)
{
	if (lua_type(L, pos) == LUA_TSTRING && lua_objlen(L, pos) >= sizeof(GUID))
		*target = *(const GUID*) lua_tostring(L, pos);
	else if (lua_isnoneornil(L, pos))
		*target = *source;
	else
		luaL_argerror(L, pos, "GUID required");
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

void FillInputRecord(lua_State *L, int pos, INPUT_RECORD *ir)
{
	int success = 0;
	pos = abs_index(L, pos);
	luaL_checktype(L, pos, LUA_TTABLE);
	memset(ir, 0, sizeof(INPUT_RECORD));
	// determine event type
	lua_getfield(L, pos, "EventType");
	ir->EventType = (WORD) get_env_flag(L, -1, &success);
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

void OptInputRecord(lua_State* L, TPluginData *pd, int pos, INPUT_RECORD* ir)
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

	luaL_checktype(L, POS_PROPS, LUA_TTABLE);
	luaL_checktype(L, POS_ITEMS, LUA_TTABLE);
	intptr_t ItemsNumber = lua_objlen(L, POS_ITEMS);

	lua_settop(L, POS_BKEYS);     // cut unneeded parameters; make stack predictable
	lua_newtable(L); // temporary store; at stack position 4

	if (!lua_isnil(L,POS_BKEYS) && !lua_istable(L,POS_BKEYS) && lua_type(L,POS_BKEYS)!=LUA_TSTRING)
		return luaL_argerror(L, POS_BKEYS, "must be table, string or nil");

	// Properties
	lua_pushvalue(L, POS_PROPS);
	int X = GetOptIntFromTable(L, "X", -1);
	int Y = GetOptIntFromTable(L, "Y", -1);
	int MaxHeight = GetOptIntFromTable(L, "MaxHeight", 0);

	UINT64 Flags = FMENU_WRAPMODE;
	lua_getfield(L, POS_PROPS, "Flags");
	if (!lua_isnil(L, -1)) Flags = CheckFlags(L, -1);

	const wchar_t *Title = L"Menu";
	lua_getfield(L, POS_PROPS, "Title");
	if (lua_isstring(L,-1))    Title = StoreTempString(L, POS_STORE);

	const wchar_t *Bottom = NULL;
	lua_getfield(L, POS_PROPS, "Bottom");
	if (lua_isstring(L,-1))    Bottom = StoreTempString(L, POS_STORE);

	const wchar_t *HelpTopic = NULL;
	lua_getfield(L, POS_PROPS, "HelpTopic");
	if (lua_isstring(L,-1))    HelpTopic = StoreTempString(L, POS_STORE);

	intptr_t SelectIndex = 0;
	lua_getfield(L, POS_PROPS, "SelectIndex");
	if ((SelectIndex = lua_tointeger(L,-1)) > ItemsNumber)
		SelectIndex = 0;

	const GUID* MenuGuid = NULL;
	lua_getfield(L, POS_PROPS, "Id");
	if (lua_type(L,-1)==LUA_TSTRING && lua_objlen(L,-1)==sizeof(GUID))
		MenuGuid = (const GUID*)lua_tostring(L, -1);

	lua_settop (L, POS_STORE);

	// Items
	struct FarMenuItem *Items =
		(struct FarMenuItem*)lua_newuserdata(L, ItemsNumber*sizeof(struct FarMenuItem));
	memset(Items, 0, ItemsNumber*sizeof(struct FarMenuItem));
	struct FarMenuItem *pItem = Items;

	for(int i=0; i < ItemsNumber; i++,pItem++,lua_pop(L,1))
	{
		static const char key[] = "text";
		lua_pushinteger(L, i+1);
		lua_gettable(L, POS_ITEMS);

		if (lua_isstring(L, -1)) { // convert a string to a table element
			lua_createtable(L, 0, 1);
			lua_insert(L, -2);
			lua_setfield(L, -2, key);
		}
		else if (!lua_istable(L, -1))
			return luaLF_SlotError(L, i+1, "string or table");

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
	intptr_t BreakCode = 0;
	int NumBreakCodes = 0;
	struct FarKey *pBreakKeys = NULL;
	intptr_t *pBreakCode = NULL;
	if (lua_type(L, POS_BKEYS) == LUA_TSTRING)
	{
		const char *ptr = lua_tostring(L, POS_BKEYS);
		lua_newtable(L);
		while (*ptr)
		{
			while (isspace(*ptr)) ptr++;
			if (*ptr == 0) break;
			const char *q = ptr++;
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
		struct FarKey* BreakKeys = (struct FarKey*)lua_newuserdata(L, (1+NumBreakCodes)*sizeof(struct FarKey));
		// get virtualkeys table from the registry; push it on top
		lua_pushstring(L, FAR_VIRTUALKEYS);
		lua_rawget(L, LUA_REGISTRYINDEX);
		// push breakkeys table on top
		lua_pushvalue(L, POS_BKEYS);        // vk=-2; bk=-1;

		int ind_target = 0;
		for (int ind=0; ind < NumBreakCodes; ind++)
		{
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
					BreakKeys[ind_target].VirtualKeyCode = Rec.Event.KeyEvent.wVirtualKeyCode;
					BreakKeys[ind_target].ControlKeyState = Rec.Event.KeyEvent.dwControlKeyState;
					ind_target++;
					lua_pop(L, 2);
					continue; // success
				}
				// restore the original string
				lua_pop(L, 1);
				lua_getfield(L, -1, "BreakKey");// vk=-4; bk=-3;bki=-2;bknm=-1;
			}

			// separate modifier and virtual key strings
			const char* s = lua_tostring(L,-1);

			char buf[32];
			DWORD mod = 0;
			if (strlen(s) >= sizeof(buf)) { lua_pop(L,2); continue; }

			strcpy(buf, s);
			_strupr(buf);
			char* vk = strchr(buf, '+');  // virtual key

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
			WORD VirtualKeyCode = (WORD)lua_tointeger(L,-1);
			if (VirtualKeyCode)
			{
				BreakKeys[ind_target].VirtualKeyCode = VirtualKeyCode;
				BreakKeys[ind_target].ControlKeyState = mod;
				ind_target++;
			}
			lua_pop(L,2);                   // vk=-2; bk=-1;
		}

		BreakKeys[ind_target].VirtualKeyCode = 0; // required by FAR API
		pBreakKeys = BreakKeys;
		pBreakCode = &BreakCode;
	}

	intptr_t ret = pd->Info->Menu(pd->PluginId, MenuGuid, X, Y, MaxHeight, Flags, Title,
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
int LF_Message(lua_State* L,
	const wchar_t* aMsg,      // if multiline, then lines must be separated by '\n'
	const wchar_t* aTitle,
	const wchar_t* aButtons,  // if multiple, then captions must be separated by ';'
	const char*    aFlags,
	const wchar_t* aHelpTopic,
	const GUID*    aMessageGuid)
{
	TPluginData *pd = GetPluginData(L);
	SMALL_RECT sr;
	int ret = pd->Info->AdvControl(pd->PluginId, ACTL_GETFARRECT, 0, &sr);
	const int max_len = ret ? sr.Right - sr.Left + 1 - 14 : 66;
	const int max_lines = ret ? sr.Bottom - sr.Top + 1 - 5 : 20;

	int num_lines = 0, num_buttons = 0;

	// Buttons
	wchar_t *BtnCopy = NULL;
	int wrap = !(aFlags && strchr(aFlags, 'n'));
	uint64_t Flags = 0;

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
		wchar_t *ptr = BtnCopy;

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

	const wchar_t **items = (const wchar_t**) malloc((1+max_lines+num_buttons) * sizeof(wchar_t*));
	wchar_t **allocLines = (wchar_t**) malloc(max_lines * sizeof(wchar_t*)); // array of pointers to allocated lines
	int nAlloc = 0; // number of allocated lines

	// Title
	const wchar_t **pItems = items;
	*pItems++ = aTitle;

	// Message lines
	wchar_t *lastDelim = NULL;
	wchar_t *MsgCopy = _wcsdup(aMsg);
	wchar_t *start = MsgCopy, *pos = MsgCopy;

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
			pos = lastDelim ? lastDelim+1 : pos;
			size_t len = pos - start;
			wchar_t **q = &allocLines[nAlloc++]; // line allocation is needed
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
		wchar_t *ptr = BtnCopy;

		for(int i=0; i < num_buttons; i++)
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
	while (nAlloc) {
		free(allocLines[--nAlloc]);
	}
	free(allocLines);
	free(MsgCopy);
	free(items);

	return ret;
}

void LF_Error(lua_State *L, const wchar_t* aMsg)
{
	TPluginData *pd = GetPluginData(L);
	if (pd->Flags & PDF_MUTE_ERRORS_1)
		return;

	PSInfo *Info = pd->Info;
	if (Info == NULL)
		return;

	if (!aMsg) aMsg = L"<non-string error message>";

	lua_pushlstring(L, (const char*)Info->ModuleName, wcslen(Info->ModuleName)*sizeof(wchar_t));
	lua_pushlstring(L, (const char*)L":\n", 4);
	LF_Gsub(L, aMsg, L"\n\t", L"\n   ");
	lua_concat(L, 3);
	if (1 == LF_Message(L, (const wchar_t*)lua_tostring(L,-1), L"Error", L"OK;Mute", "wl", NULL, NULL))
		pd->Flags |= PDF_MUTE_ERRORS_1;

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
	luaL_checkany(L,1);
	lua_settop(L,6);

	size_t MsgLen;
	const char *str = global_tolstring(L, 1, &MsgLen);
	char *copy = malloc(MsgLen);
	for (size_t i=0; i < MsgLen; i++) {
		copy[i] = str[i] ? str[i] : ' ';  // replace '\0' with a space
	}
	lua_pop(L, 1);
	lua_pushlstring(L, copy, MsgLen);
	free(copy);

	const wchar_t *Msg = check_utf8_string(L, -1, NULL);
	lua_replace(L,1);

	const wchar_t *Title     = opt_utf8_string(L, 2, L"Message");
	const wchar_t *Buttons   = opt_utf8_string(L, 3, L";OK");
	const char *Flags        = luaL_optstring(L, 4, "");
	const wchar_t *HelpTopic = opt_utf8_string(L, 5, NULL);
	const GUID *Id = (lua_type(L,6)==LUA_TSTRING && lua_objlen(L,6)==sizeof(GUID)) ?
	     (const GUID*)lua_tostring(L,6) : NULL;
	int ret = LF_Message(L, Msg, Title, Buttons, Flags, HelpTopic, Id);
	lua_pushinteger(L, ret<0 ? ret : ret+1);
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
		lua_createtable(L, (int)ItemsNumber, 0); // "PanelItems"

		for(int i=0; i < (int)ItemsNumber; i++)
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
	struct PanelInfo pi = { sizeof(pi) };

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
	void **pp = (void**)luaL_checkudata(L, 1, TYPE_SAVEDSCREEN);
	if (*pp)
		lua_pushfstring(L, "%s (%p)", TYPE_SAVEDSCREEN, *pp);
	else
		lua_pushfstring(L, "%s (freed)", TYPE_SAVEDSCREEN);
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
		void **pp = (void**)luaL_checkudata(L, 1, TYPE_SAVEDSCREEN);
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
	void **pp = (void**)luaL_checkudata(L, 1, TYPE_SAVEDSCREEN);
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
	luaL_getmetatable(L, TYPE_SAVEDSCREEN);
	lua_setmetatable(L, -2);
	return 1;
}

static int far_ShowHelp(lua_State *L)
{
	const wchar_t *ModuleName = (const wchar_t*)luaL_checkstring(L, 1);
	const wchar_t *HelpTopic = opt_utf8_string(L,2,NULL);
	flags_t Flags = OptFlags(L,3,0);
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
	flags_t Flags = OptFlags(L, 8, FIB_ENABLEEMPTY|FIB_BUTTONS|FIB_NOAMPERSAND);

	if (DestLength < 0) DestLength = 0;

	wchar_t *DestText = (wchar_t*) malloc(sizeof(wchar_t)*(DestLength+1));
	intptr_t res = pd->Info->InputBox(pd->PluginId, Id, Title, Prompt, HistoryName, SrcText,
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
	struct FarColor fc = { FCF_4BITMASK, {0x0F}, {0x00} };
	intptr_t X = luaL_optinteger(L, 1, 0);
	intptr_t Y = luaL_optinteger(L, 2, 0);
	GetFarColor(L, 3, &fc);
	const wchar_t *Str = opt_utf8_string(L, 4, NULL);
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
	FillInputRecord(L, 1, &ir);
	size_t result = GetPluginData(L)->FSF->FarInputRecordToName(&ir, buf, ARRSIZE(buf)-1);

	if (result > 0)
	{
		if (lua_toboolean(L, 2))
		{
			static const char C[]="RCtrl", A[]="RAlt", S[]="Shift";
			push_utf8_string(L, buf, -1);
			const char *p = lua_tostring(L, -1);

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
	struct FarStandardFunctions *FSF = GetPluginData(L)->FSF;
	const wchar_t* Src = check_utf8_string(L, 1, NULL);
	size_t size = FSF->GetReparsePointInfo(Src, NULL, 0);

	if (size == 0)
		return lua_pushnil(L), 1;

	wchar_t* Dest = (wchar_t*)lua_newuserdata(L, size * sizeof(wchar_t));
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
	const wchar_t* target = check_utf8_string(L, 1, NULL);
	const wchar_t* linkname = check_utf8_string(L, 2, NULL);
	flags_t linktype = OptFlags(L, 3, LINK_SYMLINK);
	flags_t flags = OptFlags(L, 4, 0);
	lua_pushboolean(L, GetPluginData(L)->FSF->MkLink(target, linkname, linktype, flags));
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
	const wchar_t *Src = check_utf8_string(L, 1, NULL);
	intptr_t MaxLen = luaL_checkinteger(L, 2);
	intptr_t SrcLen = wcslen(Src);

	if (MaxLen < 0) MaxLen = 0;
	else if (MaxLen > SrcLen) MaxLen = SrcLen;

	wchar_t *Trg = (wchar_t*)lua_newuserdata(L, (1 + SrcLen) * sizeof(wchar_t));
	wcscpy(Trg, Src);
	const wchar_t *ptr = (op == 'p') ? FSF->TruncPathStr(Trg, MaxLen) : FSF->TruncStr(Trg, MaxLen);
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
	int nret = lua_gettop(L);

	if ((FData->FileAttributes & Data->attr_excl) != 0 || (FData->FileAttributes & Data->attr_incl) != Data->attr_incl)
		return TRUE; // attributes mismatch

	lua_pushvalue(L, 3); // push the Lua function
	PushPanelItem(L, FData, 0);
	push_utf8_string(L, FullName, -1);
	for (int i=1; i<=Data->nparams; i++)
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

	flags_t Flags = OptFlags(L, 4, 0);
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
		int item = 1;
		char delim[] = { 226,148,130,0 };        // Unicode char 9474 in UTF-8
		char buf_prefix[64], buf_space[64];
		int maxno = 0;
		size_t len_prefix;

		for (int i=argn; i; i/=10) maxno++;
		len_prefix = sprintf(buf_space, "%*s%s ", maxno, "", delim);

		for(int i=1; i<=argn; i++)
		{
			size_t len_arg;

			const char *start = global_tolstring(L, i, &len_arg); //+2
			sprintf(buf_prefix, "%*d%s ", maxno, i, delim);
			char *str = (char*) malloc(len_arg + 1);
			memcpy(str, start, len_arg + 1);
			lua_pop(L, 1);                         //+1 (items)

			for (size_t j=0; j<len_arg; j++)
				if (str[j] == '\0') str[j] = ' ';

			for (start=str; start; )
			{
				const char* nl = strchr(start, '\n');

				lua_newtable(L);                     //+2 (items,curr_item)
				size_t len_text = nl ? (nl++) - start : (str+len_arg) - start;
				char *line = (char*) malloc(len_prefix + len_text);
				memcpy(line, buf_prefix, len_prefix);
				memcpy(line + len_prefix, start, len_text);

				lua_pushlstring(L, line, len_prefix + len_text);
				free(line);
				lua_setfield(L, -2, "text");         //+2
				lua_pushvalue(L, i);
				lua_setfield(L, -2, "arg");          //+2
				lua_rawseti(L, -2, item++);          //+1 (items)
				strcpy(buf_prefix, buf_space);
				start = nl;
			}

			free(str);
		}
	}

	return 1;
}

static int far_Show(lua_State *L)
{
	const char* f =
		"local items, n = ...\n"
		"local bot = n==0 and 'No arguments' or n==1 and '1 argument' or n..' arguments'\n"
		"local it, pos = far.Menu({Title=''; Bottom=bot; Flags='FMENU_SHOWAMPERSAND FMENU_WRAPMODE'},\n"
		"  items, 'Space CtrlC CtrlIns')\n"
		"if items[pos] and (it.BreakKey=='CtrlC' or it.BreakKey=='CtrlIns') then\n"
		"  far.CopyToClipboard(tostring(items[pos].arg)) end\n"
		"return it, pos";
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
	lua_createtable(L, twoways ? 256:0, 200);

	for (int i=0; i<256; i++)
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
	return (HANDLE*)luaL_checkudata(L, pos, TYPE_FILEFILTER);
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
	int filterType = (int) check_env_flag(L,2);
	HANDLE* pOutHandle = (HANDLE*)lua_newuserdata(L, sizeof(HANDLE));

	if (Info->FileFilterControl(hHandle, FFCTL_CREATEFILEFILTER, filterType, pOutHandle))
	{
		luaL_getmetatable(L, TYPE_FILEFILTER);
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
		lua_pushfstring(L, "%s (%p)", TYPE_FILEFILTER, h);
	else
		lua_pushfstring(L, "%s (closed)", TYPE_FILEFILTER);

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
	int param1 = (int) check_env_flag(L, 1);
	void *param2 = check_utf8_string(L, 2, NULL);
	intptr_t result = Info->PluginsControl(INVALID_HANDLE_VALUE, command, param1, param2);

	if (result) PushPluginHandle(L, (HANDLE)result);
	else lua_pushnil(L);

	return 1;
}

static int far_LoadPlugin(lua_State *L) { return plugin_load(L, PCTL_LOADPLUGIN); }
static int far_ForcedLoadPlugin(lua_State *L) { return plugin_load(L, PCTL_FORCEDLOADPLUGIN); }

static int far_UnloadPlugin(lua_State *L)
{
	PSInfo *Info = GetPluginData(L)->Info;
	void* Handle = *(void**)luaL_checkudata(L, 1, TYPE_PLUGINHANDLE);
	lua_pushboolean(L, Info->PluginsControl(Handle, PCTL_UNLOADPLUGIN, 0, 0) != 0);
	return 1;
}

static int far_FindPlugin(lua_State *L)
{
	PSInfo *Info = GetPluginData(L)->Info;
	int param1 = (int) check_env_flag(L, 1);
	void *param2 = NULL;

	if (param1 == PFM_MODULENAME)
		param2 = check_utf8_string(L, 2, NULL);
	else if (param1 == PFM_GUID)
	{
		size_t len;
		param2 = (void*) luaL_checklstring(L, 2, &len);

		if (len < sizeof(GUID)) param2 = NULL;
	}

	if (param2)
	{
		intptr_t handle = Info->PluginsControl(NULL, PCTL_FINDPLUGIN, param1, param2);

		if (handle)
		{
			PushPluginHandle(L, (HANDLE)handle);
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
		PutIntToTable(L, "Count", mi->Count);
		lua_createtable(L, (int) mi->Count, 0); // Guids
		lua_createtable(L, (int) mi->Count, 0); // Strings

		for (int i=0; i < (int) mi->Count; i++)
		{
			lua_pushlstring(L, (const char*)(mi->Guids + i), sizeof(GUID));
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
	HANDLE Handle = *(HANDLE*)luaL_checkudata(L, 1, TYPE_PLUGINHANDLE);
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
		HANDLE *handles = lua_newuserdata(L, count*sizeof(HANDLE));
		count = (int)Info->PluginsControl(INVALID_HANDLE_VALUE, PCTL_GETPLUGINS, count, handles);

		for(int i=0; i<count; i++)
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
	int result = 0;
	const char *guid = luaL_checklstring(L, 1, &len);
	PSInfo *Info = GetPluginData(L)->Info;

	if (len == 16)
		uuid = *(UUID*)guid;
	else
		luaL_argcheck(L, UuidFromStringA((unsigned char*)guid, &uuid) == RPC_S_OK, 1, "invalid GUID");

	intptr_t handle = Info->PluginsControl(NULL, PCTL_FINDPLUGIN, PFM_GUID, &uuid);
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
	wchar_t *Line = check_utf8_string(L, 1, &size);
	intptr_t StartPos = luaL_optinteger(L, 2, 1) - 1;
	intptr_t EndPos = luaL_optinteger(L, 3, size);
	flags_t Flags = OptFlags(L, 4, 0);
	StartPos < 0 ? StartPos = 0 : StartPos > (intptr_t)size ? StartPos = size : 0;
	EndPos < StartPos ? EndPos = StartPos : EndPos > (intptr_t)size ? EndPos = size : 0;
	wchar_t *str = GetPluginData(L)->FSF->XLat(Line, StartPos, EndPos, Flags);
	if (str) push_utf8_string(L, str, -1);
	else lua_pushnil(L);
	return 1;
}

static int far_FormatFileSize(lua_State *L)
{
	uint64_t Size = (uint64_t) luaL_checknumber(L, 1);
	int Width = (int)luaL_checkinteger(L, 2);
	if (abs(Width) > 10000)
		return luaL_error(L, "the 'Width' argument exceeds 10000");

	flags_t Flags = OptFlags(L, 3, 0) & ~FFFS_MINSIZEINDEX_MASK;
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
	(void)TimerOrWaitFired;
	if (!td->needClose && td->enabled)
	{
		TSynchroData *sd = CreateSynchroData(SYNCHRO_TIMER_CALL, 0, td);
		td->Info->AdvControl(td->PluginGuid, ACTL_SYNCHRO, 0, sd);
	}
}

static int far_Timer(lua_State *L)
{
	int interval = (int)luaL_checkinteger(L, 1);
	luaL_checktype(L, 2, LUA_TFUNCTION);

	int tabSize = lua_gettop(L);

	lua_createtable(L, tabSize, 1);         // place the function at [1]
	lua_pushinteger(L, tabSize);
	lua_setfield(L, -2, "n");
	lua_pushvalue(L, 2);
	lua_rawseti(L, -2, 1);

	TTimerData *td = (TTimerData*)lua_newuserdata(L, sizeof(TTimerData));
	luaL_getmetatable(L, TYPE_TIMER);
	lua_setmetatable(L, -2);
	lua_pushvalue(L, -1);
	lua_rawseti(L, -3, 2);                  // place the userdata at [2]

	for (int index=3; index<=tabSize; index++)  // place the arguments, if any
	{
		lua_pushvalue(L, index);
		lua_rawseti(L, -3, index);
	}

	TPluginData *pd = GetPluginData(L);
	td->Info = pd->Info;
	td->PluginGuid = pd->PluginId;
	td->interval = interval < 1 ? 1 : interval;

	lua_pushvalue(L, -2);
	td->tabRef = luaL_ref(L, LUA_REGISTRYINDEX);
	td->needClose = FALSE;
	td->enabled = 1;
	HANDLE hQueue = GetLuaStateTimerQueue(L);

	if (hQueue && CreateTimerQueueTimer(&td->hTimer,hQueue,TimerCallback,td,td->interval,td->interval,WT_EXECUTEDEFAULT))
		return 1;

	luaL_unref(L, LUA_REGISTRYINDEX, td->tabRef);
	return lua_pushnil(L), 1;
}

TTimerData* CheckTimer(lua_State* L, int pos)
{
	return (TTimerData*)luaL_checkudata(L, pos, TYPE_TIMER);
}

TTimerData* CheckValidTimer(lua_State* L, int pos)
{
	TTimerData* td = CheckTimer(L, pos);
	luaL_argcheck(L, !td->needClose, pos, "attempt to access closed timer");
	return td;
}

static int timer_Close(lua_State *L)
{
	TTimerData* td = CheckTimer(L, 1);
	if (!td->needClose)
	{
		td->needClose = TRUE;
		HANDLE hQueue = GetLuaStateTimerQueue(L);
		if (hQueue)
			DeleteTimerQueueTimer(hQueue, td->hTimer, NULL);
		TSynchroData* sd = CreateSynchroData(SYNCHRO_TIMER_UNREF, 0, td);
		td->Info->AdvControl(td->PluginGuid, ACTL_SYNCHRO, 0, sd);
	}
	return 0;
}

static int timer_gc(lua_State *L)
{
	TTimerData* td = CheckTimer(L, 1);
	if (!td->needClose)
	{
		td->needClose = TRUE;
		HANDLE hQueue = GetLuaStateTimerQueue(L);
		if (hQueue)
			DeleteTimerQueueTimer(hQueue, td->hTimer, NULL);
	}
	return 0;
}

static int timer_tostring(lua_State *L)
{
	TTimerData* td = CheckTimer(L, 1);

	if (!td->needClose)
		lua_pushfstring(L, "%s (%p)", TYPE_TIMER, td);
	else
		lua_pushfstring(L, "%s (closed)", TYPE_TIMER);

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

typedef struct
{
	HANDLE Handle;
	BOOL IsFarSettings;
} FarSettingsUdata;


static int far_CreateSettings(lua_State *L)
{
	size_t len = 0;
	const GUID* ParamId;
	BOOL IsFarSettings = 0;
	TPluginData *pd = GetPluginData(L);
	const char* strId = luaL_optlstring(L, 1, NULL, &len);

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

		ParamId = IsFarSettings? &FarGuid : (const GUID*)strId;
	}

	int location = (int) OptFlags(L, 2, PSL_ROAMING);
	struct FarSettingsCreate fsc = { sizeof(fsc) };
	fsc.Guid = *ParamId;

	if (!pd->Info->SettingsControl(INVALID_HANDLE_VALUE, SCTL_CREATE, location, &fsc))
	{
		lua_pushnil(L);
		return 1;
	}

	lua_getfield(L, LUA_REGISTRYINDEX, SettingsHandles);
	FarSettingsUdata *udata = (FarSettingsUdata*)lua_newuserdata(L, sizeof(FarSettingsUdata));
	udata->Handle = fsc.Handle;
	udata->IsFarSettings = IsFarSettings;
	luaL_getmetatable(L, TYPE_SETTINGS);
	lua_setmetatable(L, -2);
	lua_pushvalue(L, -1);
	lua_pushinteger(L, 1);
	lua_rawset(L, -4);
	return 1;
}

static FarSettingsUdata* GetSettingsUdata(lua_State *L, int pos)
{
	return luaL_checkudata(L, pos, TYPE_SETTINGS);
}

static FarSettingsUdata* CheckSettings(lua_State *L, int pos)
{
	FarSettingsUdata* udata = GetSettingsUdata(L, pos);

	if (udata->Handle == INVALID_HANDLE_VALUE)
	{
		const char* s = lua_pushfstring(L, "attempt to access a closed %s", TYPE_SETTINGS);
		luaL_argerror(L, pos, s);
	}

	return udata;
}

static int Settings_set(lua_State *L)
{
	struct FarSettingsItem fsi = { sizeof(fsi) };
	FarSettingsUdata* udata = CheckSettings(L, 1);
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
	struct FarSettingsItem fsi = { sizeof(fsi) };
	FarSettingsUdata* udata = CheckSettings(L, 1);
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
	struct FarSettingsValue fsv = { sizeof(fsv) };
	FarSettingsUdata* udata = CheckSettings(L, 1);
	fsv.Root = (size_t)check_env_flag(L, 2);
	fsv.Value = opt_utf8_string(L, 3, NULL);
	lua_pushboolean(L, GetPluginData(L)->Info->SettingsControl(udata->Handle, SCTL_DELETE, 0, &fsv) != 0);
	return 1;
}

static int Settings_createsubkey(lua_State *L)
{
	PSInfo *Info = GetPluginData(L)->Info;
	struct FarSettingsValue fsv = { sizeof(fsv) };

	FarSettingsUdata* udata = CheckSettings(L, 1);
	fsv.Root = (size_t)check_env_flag(L, 2);
	fsv.Value = check_utf8_string(L, 3, NULL);
	const wchar_t *description = opt_utf8_string(L, 4, NULL);

	intptr_t subkey = Info->SettingsControl(udata->Handle, SCTL_CREATESUBKEY, 0, &fsv);
	if (subkey != 0)
	{
		if (description != NULL)
		{
			struct FarSettingsItem fsi = { sizeof(fsi) };
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
	struct FarSettingsValue fsv = { sizeof(fsv) };
	FarSettingsUdata* udata = CheckSettings(L, 1);
	fsv.Root = (size_t)check_env_flag(L, 2);
	fsv.Value = check_utf8_string(L, 3, NULL);
	intptr_t subkey = GetPluginData(L)->Info->SettingsControl(udata->Handle, SCTL_OPENSUBKEY, 0, &fsv);

	if (subkey != 0)
		lua_pushinteger(L, subkey);
	else
		lua_pushnil(L);

	return 1;
}

static int Settings_enum(lua_State *L)
{
	struct FarSettingsEnum fse = { sizeof(fse) };
	intptr_t from = 1, to = -1;
	FarSettingsUdata* udata = CheckSettings(L, 1);
	fse.Root = (size_t)check_env_flag(L, 2);

	if (!lua_isnoneornil(L, 3))
		from = luaL_checkinteger(L, 3);

	if (!lua_isnoneornil(L, 4))
		to = luaL_checkinteger(L, 4);

	if (GetPluginData(L)->Info->SettingsControl(udata->Handle, SCTL_ENUM, 0, &fse))
	{
		if (from < 1 && (from += fse.Count + 1) < 1)
			from = 1;

		--from;

		if (to < 0 && (to += fse.Count + 1) < 0)
			to = 0;

		if (to > (int)fse.Count)
			to = fse.Count;

		lua_createtable(L, (int)fse.Count, 1);
		PutIntToTable(L, "Count", (int)fse.Count);

		for(intptr_t i = from; i < to; i++)
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
		lua_pushfstring(L, "%s (%p)", TYPE_SETTINGS, udata->Handle);
	else
		lua_pushfstring(L, "%s (closed)", TYPE_SETTINGS);

	return 1;
}

static int far_ColorDialog(lua_State *L)
{
	struct FarColor Color;
	TPluginData *pd = GetPluginData(L);

	if (!GetFarColor(L, 1, &Color))
	{
		Color.Foreground.ForegroundColor = 0x0F | ALPHAMASK;
		Color.Background.BackgroundColor = 0x00 | ALPHAMASK;
		Color.Flags = FCF_4BITMASK;
	}

	flags_t Flags = OptFlags(L, 2, 0);

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
	struct DetectCodePageInfo Info = { sizeof(Info) };
	Info.FileName = check_utf8_string(L, 1, NULL);
	int codepage = GetPluginData(L)->FSF->DetectCodePage(&Info);
	if (codepage)
		lua_pushinteger(L, codepage);
	else
		lua_pushnil(L);
	return 1;
}

static int far_GetPluginId(lua_State *L)
{
	lua_pushlstring(L, (char*)GetPluginData(L)->PluginId, sizeof(UUID));
	return 1;
}

static int far_GetErrorMode(lua_State *L)
{
	TPluginData *pd = GetPluginData(L);
	lua_Integer Mode = (pd->Flags & PDF_MUTE_ERRORS_1) ? 0x01 : 0x00;
	lua_pushinteger(L, Mode);
	return 1;
}

static int far_SetErrorMode(lua_State *L)
{
	TPluginData *pd = GetPluginData(L);
	lua_Integer PrevMode = (pd->Flags & PDF_MUTE_ERRORS_1) ? 0x01 : 0x00;
	lua_Integer NewMode = luaL_checkinteger(L, 1);
	if (NewMode & 0x01)
		pd->Flags |= PDF_MUTE_ERRORS_1;
	else
		pd->Flags &= ~PDF_MUTE_ERRORS_1;

	lua_pushinteger(L, PrevMode);
	return 1;
}

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

const luaL_Reg far_funcs[] =
{
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
	PAIR( far, GetErrorMode),
	PAIR( far, GetFileOwner),
	PAIR( far, GetLuafarVersion),
	PAIR( far, GetMsg),
	PAIR( far, GetNumberOfLinks),
	PAIR( far, GetPathRoot),
	PAIR( far, GetPluginDirList),
	PAIR( far, GetPluginId),
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
	PAIR( far, SetErrorMode),
	PAIR( far, Show),
	PAIR( far, ShowHelp),
	PAIR( far, Text),
	PAIR( far, Timer),
	PAIR( far, TruncPathStr),
	PAIR( far, TruncStr),
	PAIR( far, UnloadPlugin),
	PAIR( far, XLat),

	{NULL, NULL}
};

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

	NewVirtualKeyTable(L, FALSE);
	lua_setfield(L, LUA_REGISTRYINDEX, FAR_VIRTUALKEYS);
	luaL_register(L, "far", far_funcs);
	PutStrToTable (L, "Flavor", "far3");

	lua_pushcfunction(L, luaopen_dialog);
	lua_pushvalue(L, -2);
	lua_call(L, 1, 0);

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

	luaL_newmetatable(L, TYPE_FILEFILTER);
	lua_pushvalue(L,-1);
	lua_setfield(L, -2, "__index");
	luaL_register(L, NULL, filefilter_methods);

	luaL_newmetatable(L, TYPE_TIMER);
	luaL_register(L, NULL, timer_methods);

	luaL_newmetatable(L, TYPE_SETTINGS);
	lua_pushvalue(L,-1);
	lua_setfield(L, -2, "__index");
	luaL_register(L, NULL, Settings_methods);
	lua_newtable(L);
	lua_newtable(L);
	lua_pushliteral(L, "k");
	lua_setfield(L, -2, "__mode");
	lua_setmetatable(L, -2);
	lua_setfield(L, LUA_REGISTRYINDEX, SettingsHandles);

	luaL_newmetatable(L, TYPE_PLUGINHANDLE);
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	lua_pushcfunction(L, PluginHandle_rawhandle);
	lua_setfield(L, -2, "rawhandle");

	luaL_newmetatable(L, TYPE_SAVEDSCREEN);
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
	{"editor",        luaopen_editor},
	{"panel",         luaopen_panel},
	{"",              luaopen_actl},
	{"",              luaopen_macro},
	{"",              luaopen_viewer},
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
	lua_setfield(L, -2, "__index");             //+4
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

	if (aOpenLibs) {
		lua_pushcfunction(L, aOpenLibs);
		lua_call(L, 0, 0);
	}

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
		for(int i=0; i < argc; i++)
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
