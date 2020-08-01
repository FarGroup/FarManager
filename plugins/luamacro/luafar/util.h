#ifndef LUAFAR_UTIL_H
#define LUAFAR_UTIL_H

#include <plugin.hpp>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#ifdef __GNUC__ //FIXME: #include <float.h> works with MinGW64 but does not with MinGW.
/* Control word masks for unMask */
#define	_MCW_EM		0x0008001F	/* Error masks */
_CRTIMP unsigned int __cdecl __MINGW_NOTHROW _control87 (unsigned int unNew, unsigned int unMask);
#else
#include <float.h>
#endif //__GNUC__

// Prevent crashes on divide by 0, etc., due to plugins activating FPU exceptions.
// (it takes approximately 20 nanosec.)
#define FP_PROTECT() _control87(_MCW_EM,_MCW_EM)

/* convert a stack index to positive */
#define abs_index(L,i) ((i)>0 || (i)<=LUA_REGISTRYINDEX ? (i):lua_gettop(L)+(i)+1)

#define LUAFAR_TIMER_CALL  0x1
#define LUAFAR_TIMER_UNREF 0x2

typedef struct
{
	lua_State *L;
	int ref;
} FarPanelItemUserData;

typedef struct
{
	GUID* PluginGuid;
	struct PluginStartupInfo *Info;
	int interval;   // timer period, in milliseconds
	int tabRef;     // reference of a Lua table in the registry
	int needClose;  // timer needs to be closed; boolean value
	int enabled;    // timer is enabled; the callback function is called only when (enabled != 0)
	HANDLE hTimer;  // timer handle
} TTimerData;

typedef struct
{
	TTimerData *timerData;
	int regAction;
	int data;
} TSynchroData;

typedef struct
{
	lua_State         *L;
	struct PluginStartupInfo *Info;
	HANDLE            hDlg;
	BOOL              isOwned;
	BOOL              wasError;
	BOOL              isModal;
} TDialogData;

int   GetAttrFromTable(lua_State *L);
int   GetIntFromArray(lua_State *L, int index);
int   luaLF_FieldError(lua_State *L, const char* key, const char* expected_typename);
int   luaLF_SlotError(lua_State *L, int key, const char* expected_typename);
void  PushPanelItem(lua_State *L, const struct PluginPanelItem *PanelItem, int NoUserData);
void  PushPanelItems(lua_State *L, const struct PluginPanelItem *PanelItems, size_t ItemsNumber, int NoUserData);
void  PutMouseEvent(lua_State *L, const MOUSE_EVENT_RECORD* rec, BOOL table_exist);
unsigned __int64 GetFileSizeFromTable(lua_State *L, const char *key);
FILETIME GetFileTimeFromTable(lua_State *L, const char *key);
void  PutFileTimeToTable(lua_State *L, const char* key, FILETIME ft);
TDialogData* NewDialogData(lua_State* L, struct PluginStartupInfo *Info, HANDLE hDlg,
                           BOOL isOwned);
int GetFarColor(lua_State *L, int pos, struct FarColor* Color);
void PushFarColor(lua_State *L, const struct FarColor* Color);

typedef struct
{
	intptr_t X,Y;
	intptr_t Size;
	struct FAR_CHAR_INFO VBuf[1];
} TFarUserControl;

TFarUserControl* CheckFarUserControl(lua_State* L, int pos);

#endif
