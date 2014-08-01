#ifndef LUAFAR_H
#define LUAFAR_H

#include <plugin.hpp>

#ifdef __cplusplus
extern "C" {
#endif

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#ifdef __GNUC__ //FIXME: #include <float.h> работает с MinGW64, но не с MinGW.
/* Control word masks for unMask */
#define	_MCW_EM		0x0008001F	/* Error masks */
_CRTIMP unsigned int __cdecl __MINGW_NOTHROW _control87 (unsigned int unNew, unsigned int unMask);
#else
#include <float.h>
#endif //__GNUC__

#if defined BUILD_DLL
#  define DLLFUNC __declspec(dllexport)
#elif defined BUILD_STATIC
#  define DLLFUNC
#else
#  define DLLFUNC __declspec(dllimport)
#endif

	typedef struct
	{
		struct PluginStartupInfo *Info;
		struct FarStandardFunctions *FSF;
		GUID *PluginId;
		FARWINDOWPROC DlgProc;
		intptr_t DialogEventDrawGroup;
		lua_Alloc origAlloc;
		void *origUserdata;
		lua_State *MainLuaState;
		void (*new_action)(int i);
		void (*old_action)(int i);
	} TPluginData;
	TPluginData* GetPluginData(lua_State* L);

	typedef struct
	{
		lua_State *L;
		int ref;
	} FarPanelItemUserData;

	DLLFUNC intptr_t LF_DlgProc(lua_State *L, HANDLE hDlg, intptr_t  Msg, intptr_t  Param1, void *Param2);
	DLLFUNC intptr_t LF_MacroCallback(lua_State* L, void* Id, FARADDKEYMACROFLAGS Flags);
	DLLFUNC const wchar_t *LF_Gsub(lua_State *L, const wchar_t *s, const wchar_t *p, const wchar_t *r);
	DLLFUNC void    LF_InitLuaState1(lua_State *L, lua_CFunction aOpenLibs);
	DLLFUNC void    LF_InitLuaState2(lua_State *L, TPluginData *aData);
	DLLFUNC int     LF_LoadFile(lua_State *L, const wchar_t* filename);
	DLLFUNC int     LF_DoFile(lua_State *L, const wchar_t *fname, int argc, wchar_t* argv[]);
	DLLFUNC int     LF_Message(lua_State *L, const wchar_t* aMsg, const wchar_t* aTitle, const wchar_t* aButtons, const char* aFlags, const wchar_t* aHelpTopic, const GUID* aId);
	DLLFUNC void    LF_ProcessEnvVars(lua_State *L, const wchar_t* aEnvPrefix, const wchar_t* PluginDir);
	DLLFUNC BOOL    LF_RunDefaultScript(lua_State* L);

	DLLFUNC HANDLE   LF_Analyse(lua_State* L, const struct AnalyseInfo *Info);
	DLLFUNC void     LF_CloseAnalyse(lua_State* L, const struct CloseAnalyseInfo *Info);
	DLLFUNC void     LF_ClosePanel(lua_State* L, const struct ClosePanelInfo *Info);
	DLLFUNC intptr_t LF_Compare(lua_State* L, const struct CompareInfo *Info);
	DLLFUNC intptr_t LF_Configure(lua_State* L, const struct ConfigureInfo *Info);
	DLLFUNC intptr_t LF_DeleteFiles(lua_State* L, const struct DeleteFilesInfo *Info);
	DLLFUNC void     LF_ExitFAR(lua_State* L, const struct ExitInfo *Info);
	DLLFUNC void     LF_FreeFindData(lua_State* L, const struct FreeFindDataInfo *Info);
	DLLFUNC intptr_t LF_GetFiles(lua_State* L, struct GetFilesInfo *Info);
	DLLFUNC intptr_t LF_GetFindData(lua_State* L, struct GetFindDataInfo *Info);
	DLLFUNC int      LF_GetGlobalInfo(lua_State* L, struct GlobalInfo *Info, const wchar_t *PluginDir);
	DLLFUNC void     LF_GetOpenPanelInfo(lua_State* L, struct OpenPanelInfo *Info);
	DLLFUNC void     LF_GetPluginInfo(lua_State* L, struct PluginInfo *Info);
	DLLFUNC intptr_t LF_MakeDirectory(lua_State* L, struct MakeDirectoryInfo *Info);
	DLLFUNC HANDLE   LF_Open(lua_State* L, const struct OpenInfo *Info);
	DLLFUNC intptr_t LF_ProcessDialogEvent(lua_State* L, const struct ProcessDialogEventInfo *Info);
	DLLFUNC intptr_t LF_ProcessEditorEvent(lua_State* L, const struct ProcessEditorEventInfo *Info);
	DLLFUNC intptr_t LF_ProcessEditorInput(lua_State* L, const struct ProcessEditorInputInfo *Info);
	DLLFUNC intptr_t LF_ProcessHostFile(lua_State* L, const struct ProcessHostFileInfo *Info);
	DLLFUNC intptr_t LF_ProcessPanelEvent(lua_State* L, const struct ProcessPanelEventInfo *Info);
	DLLFUNC intptr_t LF_ProcessPanelInput(lua_State* L, const struct ProcessPanelInputInfo *Info);
	DLLFUNC	intptr_t LF_ProcessConsoleInput(lua_State* L, struct ProcessConsoleInputInfo *Info);
	DLLFUNC intptr_t LF_ProcessSynchroEvent(lua_State* L, const struct ProcessSynchroEventInfo *Info);
	DLLFUNC intptr_t LF_ProcessViewerEvent(lua_State* L, const struct ProcessViewerEventInfo *Info);
	DLLFUNC intptr_t LF_PutFiles(lua_State* L, const struct PutFilesInfo *Info);
	DLLFUNC intptr_t LF_SetDirectory(lua_State* L, const struct SetDirectoryInfo *Info);
	DLLFUNC intptr_t LF_SetFindList(lua_State* L, const struct SetFindListInfo *Info);
	DLLFUNC intptr_t LF_GetCustomData(lua_State* L, const wchar_t *FilePath, wchar_t **CustomData);
	DLLFUNC void     LF_FreeCustomData(lua_State* L, wchar_t *CustomData);

#ifdef __cplusplus
}
#endif

#endif // LUAFAR_H
