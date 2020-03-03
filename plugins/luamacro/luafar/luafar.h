#ifndef LUAFAR_H
#define LUAFAR_H

#include <plugin.hpp>

#ifdef __cplusplus
extern "C" {
#endif

#include <lua.h>
#include <lauxlib.h>

#if defined BUILD_DLL
#  define DLLFUNC __declspec(dllexport)
#elif defined BUILD_STATIC
#  define DLLFUNC
#else
#  define DLLFUNC __declspec(dllimport)
#endif

typedef intptr_t PLUGINDATAFLAGS;
static const PLUGINDATAFLAGS
	PDF_DIALOGEVENTDRAWENABLE = 0x00000001,
	PDF_PROCESSINGERROR       = 0x00000002,
	PDF_FULL_TIME_RESOLUTION  = 0x00000004;

typedef struct
{
	struct PluginStartupInfo *Info;
	struct FarStandardFunctions *FSF;
	GUID *PluginId;
	FARWINDOWPROC DlgProc;
	PLUGINDATAFLAGS Flags;
	lua_Alloc origAlloc;
	void *origUserdata;
	lua_State *MainLuaState;
	void (*new_action)(int i);
	void (*old_action)(int i);
} TPluginData;
extern TPluginData* (*GetPluginData)(lua_State* L);

/*---------------------------------*/
/* DO NOT CHANGE THESE SIGNATURES. */
/*---------------------------------*/
typedef wchar_t*       (*LUAFARAPI_CHECKUTF8STRING)    (lua_State *L, int Pos, size_t *TrgSize);
typedef const wchar_t* (*LUAFARAPI_OPTUTF8STRING)      (lua_State *L, int Pos, const wchar_t *Dflt);
typedef char*          (*LUAFARAPI_PUSHUTF8STRING)     (lua_State *L, const wchar_t *Str, intptr_t NumChars);
typedef const wchar_t* (*LUAFARAPI_CHECK_UTF16_STRING) (lua_State *L, int pos, size_t *len);
typedef const wchar_t* (*LUAFARAPI_OPT_UTF16_STRING)   (lua_State *L, int pos, const wchar_t *dflt);
typedef void           (*LUAFARAPI_PUSH_UTF16_STRING)  (lua_State* L, const wchar_t* str, intptr_t numchars);
typedef wchar_t*       (*LUAFARAPI_UTF8_TO_UTF16)      (lua_State *L, int pos, size_t* pTrgSize);

typedef BOOL           (*LUAFARAPI_GETBOOLFROMTABLE)   (lua_State *L, const char* key);
typedef BOOL           (*LUAFARAPI_GETOPTBOOLFROMTABLE)(lua_State *L, const char* key, BOOL dflt);
typedef int            (*LUAFARAPI_GETOPTINTFROMARRAY) (lua_State *L, int key, int dflt);
typedef int            (*LUAFARAPI_GETOPTINTFROMTABLE) (lua_State *L, const char* key, int dflt);
typedef double         (*LUAFARAPI_GETOPTNUMFROMTABLE) (lua_State *L, const char* key, double dflt);
typedef void           (*LUAFARAPI_PUTBOOLTOTABLE)     (lua_State *L, const char* key, int num);
typedef void           (*LUAFARAPI_PUTINTTOARRAY)      (lua_State *L, int key, intptr_t val);
typedef void           (*LUAFARAPI_PUTINTTOTABLE)      (lua_State *L, const char *key, intptr_t val);
typedef void           (*LUAFARAPI_PUTLSTRTOTABLE)     (lua_State *L, const char* key, const void* str, size_t len);
typedef void           (*LUAFARAPI_PUTNUMTOTABLE)      (lua_State *L, const char* key, double num);
typedef void           (*LUAFARAPI_PUTSTRTOARRAY)      (lua_State *L, int key, const char* str);
typedef void           (*LUAFARAPI_PUTSTRTOTABLE)      (lua_State *L, const char* key, const char* str);
typedef void           (*LUAFARAPI_PUTWSTRTOARRAY)     (lua_State *L, int key, const wchar_t* str, intptr_t numchars);
typedef void           (*LUAFARAPI_PUTWSTRTOTABLE)     (lua_State *L, const char* key, const wchar_t* str, intptr_t numchars);

typedef int            (*LUAFARAPI_GETEXPORTFUNCTION)  (lua_State* L, const char* FuncName);
typedef int            (*LUAFARAPI_PCALLMSG)           (lua_State* L, int narg, int nret);

typedef int            (*LUAFARAPI_BIT64_PUSHUSERDATA) (lua_State *L, __int64 v);
typedef int            (*LUAFARAPI_BIT64_PUSH)         (lua_State *L, __int64 v);
typedef int            (*LUAFARAPI_BIT64_GETVALUE)     (lua_State *L, int pos, __int64 *target);


/*---------------------------------------------------------------------------*/
/* DO NOT CHANGE THE ORDER OR THE CONTENTS. ADD NEW MEMBERS AT THE END ONLY. */
/*---------------------------------------------------------------------------*/
typedef struct
{
	size_t StructSize;

	LUAFARAPI_CHECKUTF8STRING      check_utf8_string;
	LUAFARAPI_OPTUTF8STRING        opt_utf8_string;
	LUAFARAPI_PUSHUTF8STRING       push_utf8_string;
	LUAFARAPI_CHECK_UTF16_STRING   check_utf16_string;
	LUAFARAPI_OPT_UTF16_STRING     opt_utf16_string;
	LUAFARAPI_PUSH_UTF16_STRING    push_utf16_string;
	LUAFARAPI_UTF8_TO_UTF16        utf8_to_utf16;

	LUAFARAPI_GETBOOLFROMTABLE     GetBoolFromTable;
	LUAFARAPI_GETOPTBOOLFROMTABLE  GetOptBoolFromTable;
	LUAFARAPI_GETOPTINTFROMARRAY   GetOptIntFromArray;
	LUAFARAPI_GETOPTINTFROMTABLE   GetOptIntFromTable;
	LUAFARAPI_GETOPTNUMFROMTABLE   GetOptNumFromTable;
	LUAFARAPI_PUTBOOLTOTABLE       PutBoolToTable;
	LUAFARAPI_PUTINTTOARRAY        PutIntToArray;
	LUAFARAPI_PUTINTTOTABLE        PutIntToTable;
	LUAFARAPI_PUTLSTRTOTABLE       PutLStrToTable;
	LUAFARAPI_PUTNUMTOTABLE        PutNumToTable;
	LUAFARAPI_PUTSTRTOARRAY        PutStrToArray;
	LUAFARAPI_PUTSTRTOTABLE        PutStrToTable;
	LUAFARAPI_PUTWSTRTOARRAY       PutWStrToArray;
	LUAFARAPI_PUTWSTRTOTABLE       PutWStrToTable;

	LUAFARAPI_GETEXPORTFUNCTION    GetExportFunction;
	LUAFARAPI_PCALLMSG             pcall_msg;

	LUAFARAPI_BIT64_PUSHUSERDATA   bit64_pushuserdata;
	LUAFARAPI_BIT64_PUSH           bit64_push;
	LUAFARAPI_BIT64_GETVALUE       bit64_getvalue;
} LuafarAPI;

DLLFUNC void     LF_GetLuafarAPI (LuafarAPI *target);
DLLFUNC intptr_t LF_DlgProc(lua_State *L, HANDLE hDlg, intptr_t  Msg, intptr_t  Param1, void *Param2);
DLLFUNC intptr_t LF_MacroCallback(lua_State* L, void* Id, FARADDKEYMACROFLAGS Flags);
DLLFUNC const wchar_t *LF_Gsub(lua_State *L, const wchar_t *s, const wchar_t *p, const wchar_t *r);
DLLFUNC void    LF_InitLuaState1(lua_State *L, lua_CFunction aOpenLibs);
DLLFUNC void    LF_InitLuaState2(lua_State *L, TPluginData *aData);
DLLFUNC int     LF_LoadFile(lua_State *L, const wchar_t* filename);
DLLFUNC int     LF_DoFile(lua_State *L, const wchar_t *fname, int argc, wchar_t* argv[]);
DLLFUNC int     LF_Message(lua_State *L, const wchar_t* aMsg, const wchar_t* aTitle, const wchar_t* aButtons, const char* aFlags, const wchar_t* aHelpTopic, const GUID* aId);
DLLFUNC void    LF_ProcessEnvVars(lua_State *L, const wchar_t* aEnvPrefix, const wchar_t* PluginDir);
DLLFUNC void    LF_RunLuafarInit(lua_State *L);
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
DLLFUNC intptr_t LF_GetContentFields(lua_State* L, const struct GetContentFieldsInfo *Info);
DLLFUNC intptr_t LF_GetContentData(lua_State* L, struct GetContentDataInfo *Info);
DLLFUNC void     LF_FreeContentData(lua_State* L, const struct GetContentDataInfo *Info);

#ifdef __cplusplus
}
#endif

#endif // LUAFAR_H
