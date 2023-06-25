//---------------------------------------------------------------------------
#include <windows.h>
#include <stdlib.h>
#include <signal.h>
#include "luafar.h"

#ifdef _MSC_VER
#define LUAPLUG WINAPI
#else
#define LUAPLUG WINAPI __declspec(dllexport)
#endif

#ifdef FUNC_OPENLIBS
extern int FUNC_OPENLIBS(lua_State*);
#else
#define FUNC_OPENLIBS NULL
#endif

typedef struct
{
	lua_State *LS;
	struct PluginStartupInfo *StartupInfo;
	GUID PluginId;
	TPluginData PluginData;
	wchar_t PluginDir[512];
	int InitStage;
	int Depth;
	CRITICAL_SECTION CritSection; // http://forum.farmanager.com/viewtopic.php?f=9&p=107075#p107075
} Global;

static Global G;

#define IS_PLUGIN_READY(g) (g.LS && g.InitStage==2 && TryEnterCriticalSection(&g.CritSection))
#define LEAVE_CS(g)        LeaveCriticalSection(&g.CritSection)

#define EXP_VOID(Info,Name) \
	{ if (IS_PLUGIN_READY(G)) \
	  { \
		  Name(G.LS, Info); \
		  LEAVE_CS(G); \
	  } }

#define EXP_TYPE(Info,Name,Type) \
	{ if (IS_PLUGIN_READY(G)) \
	  { \
		  Type ret = Name(G.LS, Info); \
		  LEAVE_CS(G); \
		  return ret; \
	  } \
	  return (Type)0; }

#define EXP_INTPTR(Info,Name) EXP_TYPE(Info,Name,intptr_t)
#define EXP_HANDLE(Info,Name) EXP_TYPE(Info,Name,HANDLE)

static void InitLuaState2(lua_State *L, TPluginData* PluginData);  /* forward declaration */

static void lstop(lua_State *L, lua_Debug *ar)
{
	(void)ar;  /* unused arg. */
	lua_sethook(L, NULL, 0, 0);
	luaL_error(L, "interrupted!");
}

static void laction(int i)
{
	signal(i, G.PluginData.old_action);
	lua_sethook(G.LS, lstop, LUA_MASKCALL | LUA_MASKRET | LUA_MASKCOUNT, 1);
}

static intptr_t WINAPI DlgProc(HANDLE hDlg, intptr_t Msg, intptr_t Param1, void *Param2)
{
	if (IS_PLUGIN_READY(G))
	{
		intptr_t ret = LF_DlgProc(G.LS, hDlg, Msg, Param1, Param2);
		LEAVE_CS(G);
		return ret;
	}
	return 0;
}

static void InitGlobal (Global *g, HINSTANCE hDll)
{
	size_t PluginDirSize = sizeof(g->PluginDir) / sizeof(g->PluginDir[0]);
	size_t RetSize = GetModuleFileNameW(hDll, g->PluginDir, (DWORD)PluginDirSize);
	TPluginData PD = { NULL,NULL,&g->PluginId,DlgProc,0,NULL,NULL,NULL,laction,SIG_DFL };
	g->PluginData = PD;
	g->InitStage = 0;
	g->Depth = 0;
	g->LS = NULL;
	if (RetSize && RetSize < PluginDirSize)
	{
		wcsrchr(g->PluginDir, L'\\')[1] = 0;
		g->LS = luaL_newstate();
	}
	InitializeCriticalSection(&g->CritSection);
}

static void DestroyGlobal (Global *g)
{
	if (g->LS) lua_close(g->LS);

	free(g->StartupInfo);

	DeleteCriticalSection(&g->CritSection);
}

BOOL WINAPI DllMain(HANDLE hDll, DWORD dwReason, LPVOID lpReserved)
{
	(void) lpReserved;

	if (DLL_PROCESS_ATTACH == dwReason && hDll)
		InitGlobal(&G, (HINSTANCE)hDll);
	else if (DLL_PROCESS_DETACH == dwReason)
		DestroyGlobal(&G);

	return TRUE;
}

// This function must have __cdecl calling convention, it is not `LUAPLUG'.
__declspec(dllexport) int luaopen_luaplug(lua_State *L)
{
	TPluginData *pd = (TPluginData*)lua_newuserdata(L, sizeof(TPluginData));
	memcpy(pd, &G.PluginData, sizeof(TPluginData));
	LF_InitLuaState1(L, FUNC_OPENLIBS);
	InitLuaState2(L, pd);
	luaL_ref(L, LUA_REGISTRYINDEX);
	return 0;
}

static void InitLuaState2(lua_State *L, TPluginData* PluginData)
{
	LF_InitLuaState2(L, PluginData);
#ifdef RUN_LUAFAR_INIT
	LF_RunLuafarInit(L);
#endif
	lua_pushcfunction(L, luaopen_luaplug);
	lua_setglobal(L, "_luaplug");
}

__declspec(dllexport) lua_State* GetLuaState(void)
{
	if (IS_PLUGIN_READY(G))
	{
		lua_State *L = G.LS;
		LEAVE_CS(G);
		return L;
	}
	return NULL;
}

/* for other C-files of the plugin */
struct PluginStartupInfo *GetPluginStartupInfo(void)
{
	return G.StartupInfo;
}

void LUAPLUG GetGlobalInfoW(struct GlobalInfo *globalInfo)
{
	if (G.LS)
	{
		if (G.InitStage == 0)
		{
			LF_InitLuaState1(G.LS, FUNC_OPENLIBS);
			G.InitStage++;
		}

		if (LF_GetGlobalInfo(G.LS, globalInfo, G.PluginDir))
			G.PluginId = globalInfo->Guid;
		else
		{
			lua_close(G.LS);
			G.LS = NULL;
		}
	}
}
//---------------------------------------------------------------------------

void LUAPLUG SetStartupInfoW(const struct PluginStartupInfo *aInfo)
{
	if (G.LS && G.InitStage==1)
	{
		char *ptr = (char*) malloc(aInfo->StructSize + aInfo->FSF->StructSize);
		if (ptr)
		{
			memcpy(ptr, aInfo, aInfo->StructSize);
			memcpy(ptr+aInfo->StructSize, aInfo->FSF, aInfo->FSF->StructSize);
			G.StartupInfo = (struct PluginStartupInfo*) ptr;
			G.StartupInfo->FSF = (struct FarStandardFunctions*) (ptr+aInfo->StructSize);
			G.PluginData.Info = G.StartupInfo;
			G.PluginData.FSF = G.StartupInfo->FSF;

			InitLuaState2(G.LS, &G.PluginData);
			if (LF_RunDefaultScript(G.LS))
				G.InitStage++;
		}

		if (G.InitStage != 2)
		{
			lua_close(G.LS);
			G.LS = NULL;

			if (G.StartupInfo) { free(G.StartupInfo); G.StartupInfo=NULL; }
		}
	}
}
//---------------------------------------------------------------------------

void LUAPLUG GetPluginInfoW(struct PluginInfo *Info)
{
	EXP_VOID(Info, LF_GetPluginInfo)
}
//---------------------------------------------------------------------------

intptr_t LUAPLUG ProcessSynchroEventW(const struct ProcessSynchroEventInfo *Info)
{
	EXP_INTPTR(Info, LF_ProcessSynchroEvent)
}
//---------------------------------------------------------------------------

// This is exported in order not to crash when run from under Far 2.0.xxxx
// Minimal Far version = 3.0.0
int LUAPLUG GetMinFarVersionW(void)
{
	return (3<<8) | 0 | (0<<16);
}
//---------------------------------------------------------------------------

#ifdef EXPORT_OPEN
static int RecreateLuaState (Global *g)
{
	lua_getglobal(g->LS, "RecreateLuaState");
	if (lua_toboolean(g->LS, -1))
	{
		struct ExitInfo Info = { sizeof(struct ExitInfo) };
		lua_State *newLS = g->LS;
		g->LS = NULL;
		LF_ExitFAR(newLS, &Info);
		lua_close(newLS);
		if ((newLS = luaL_newstate()) != NULL)
		{
			int OK = 0;
			struct GlobalInfo GInfo;
			LF_InitLuaState1(newLS, FUNC_OPENLIBS);
			if (LF_GetGlobalInfo(newLS, &GInfo, g->PluginDir))
			{
				InitLuaState2(newLS, &g->PluginData);
				lua_pushboolean(newLS,1);
				lua_setglobal(newLS, "IsLuaStateRecreated");
				OK = LF_RunDefaultScript(newLS);
			}
			if (OK) g->LS = newLS;
			else lua_close(newLS);
		}
		return 1;
	}
	lua_pop(g->LS,1);
	return 0;
}

HANDLE LUAPLUG OpenW(const struct OpenInfo *Info)
{
	if (IS_PLUGIN_READY(G))
	{
		HANDLE h;
		++G.Depth; // prevents crashes (this function can be called recursively)
		h = LF_Open(G.LS, Info);
		h = --G.Depth==0 && RecreateLuaState(&G) ? NULL : G.LS ? h : NULL;
		LEAVE_CS(G);
		return h;
	}
	return NULL;
}
#endif
//---------------------------------------------------------------------------

#ifdef EXPORT_GETFINDDATA
intptr_t LUAPLUG GetFindDataW(struct GetFindDataInfo *Info)
{
	EXP_INTPTR(Info, LF_GetFindData)
}

void LUAPLUG FreeFindDataW(const struct FreeFindDataInfo *Info)
{
	EXP_VOID(Info, LF_FreeFindData)
}
#endif
//---------------------------------------------------------------------------

#ifdef EXPORT_CLOSEPANEL
void LUAPLUG ClosePanelW(const struct ClosePanelInfo *Info)
{
	EXP_VOID(Info, LF_ClosePanel)
}
#endif
//---------------------------------------------------------------------------

#ifdef EXPORT_GETFILES
intptr_t LUAPLUG GetFilesW(struct GetFilesInfo *Info)
{
	EXP_INTPTR(Info, LF_GetFiles)
}
#endif
//---------------------------------------------------------------------------

#ifdef EXPORT_GETOPENPANELINFO
void LUAPLUG GetOpenPanelInfoW(struct OpenPanelInfo *Info)
{
	EXP_VOID(Info, LF_GetOpenPanelInfo)
}
#endif
//---------------------------------------------------------------------------

void LUAPLUG ExitFARW(const struct ExitInfo *Info)
{
	if (IS_PLUGIN_READY(G))
	{
		LF_ExitFAR(G.LS, Info);
		LEAVE_CS(G);
	}
}
//---------------------------------------------------------------------------

#ifdef EXPORT_COMPARE
intptr_t LUAPLUG CompareW(const struct CompareInfo *Info)
{
	EXP_INTPTR(Info, LF_Compare)
}
#endif
//---------------------------------------------------------------------------

#ifdef EXPORT_CONFIGURE
intptr_t LUAPLUG ConfigureW(const struct ConfigureInfo *Info)
{
	EXP_INTPTR(Info, LF_Configure)
}
#endif
//---------------------------------------------------------------------------

#ifdef EXPORT_DELETEFILES
intptr_t LUAPLUG DeleteFilesW(const struct DeleteFilesInfo *Info)
{
	EXP_INTPTR(Info, LF_DeleteFiles)
}
#endif
//---------------------------------------------------------------------------

#ifdef EXPORT_MAKEDIRECTORY
intptr_t LUAPLUG MakeDirectoryW(struct MakeDirectoryInfo *Info)
{
	EXP_INTPTR(Info, LF_MakeDirectory)
}
#endif
//---------------------------------------------------------------------------

#ifdef EXPORT_PROCESSPANELEVENT
intptr_t LUAPLUG ProcessPanelEventW(const struct ProcessPanelEventInfo *Info)
{
	EXP_INTPTR(Info, LF_ProcessPanelEvent)
}
#endif
//---------------------------------------------------------------------------

#ifdef EXPORT_PROCESSHOSTFILE
intptr_t LUAPLUG ProcessHostFileW(const struct ProcessHostFileInfo *Info)
{
	EXP_INTPTR(Info, LF_ProcessHostFile)
}
#endif
//---------------------------------------------------------------------------

#ifdef EXPORT_PROCESSPANELINPUT
intptr_t LUAPLUG ProcessPanelInputW(const struct ProcessPanelInputInfo *Info)
{
	EXP_INTPTR(Info, LF_ProcessPanelInput)
}
#endif
//---------------------------------------------------------------------------

#ifdef EXPORT_PUTFILES
intptr_t LUAPLUG PutFilesW(const struct PutFilesInfo *Info)
{
	EXP_INTPTR(Info, LF_PutFiles)
}
#endif
//---------------------------------------------------------------------------

#ifdef EXPORT_SETDIRECTORY
intptr_t LUAPLUG SetDirectoryW(const struct SetDirectoryInfo *Info)
{
	EXP_INTPTR(Info, LF_SetDirectory)
}
#endif
//---------------------------------------------------------------------------

#ifdef EXPORT_SETFINDLIST
intptr_t LUAPLUG SetFindListW(const struct SetFindListInfo *Info)
{
	EXP_INTPTR(Info, LF_SetFindList)
}
#endif
//---------------------------------------------------------------------------

#ifdef EXPORT_PROCESSEDITORINPUT
intptr_t LUAPLUG ProcessEditorInputW(const struct ProcessEditorInputInfo *Info)
{
	EXP_INTPTR(Info, LF_ProcessEditorInput)
}
#endif
//---------------------------------------------------------------------------

#ifdef EXPORT_PROCESSEDITOREVENT
intptr_t LUAPLUG ProcessEditorEventW(const struct ProcessEditorEventInfo *Info)
{
	EXP_INTPTR(Info, LF_ProcessEditorEvent)
}
#endif
//---------------------------------------------------------------------------

#ifdef EXPORT_PROCESSVIEWEREVENT
intptr_t LUAPLUG ProcessViewerEventW(const struct ProcessViewerEventInfo *Info)
{
	EXP_INTPTR(Info, LF_ProcessViewerEvent)
}
#endif
//---------------------------------------------------------------------------

#ifdef EXPORT_PROCESSDIALOGEVENT
intptr_t LUAPLUG ProcessDialogEventW(const struct ProcessDialogEventInfo *Info)
{
	EXP_INTPTR(Info, LF_ProcessDialogEvent)
}
#endif
//---------------------------------------------------------------------------

#ifdef EXPORT_GETCONTENTDATA
intptr_t LUAPLUG GetContentFieldsW(const struct GetContentFieldsInfo *Info)
{
	EXP_INTPTR(Info, LF_GetContentFields)
}

intptr_t LUAPLUG GetContentDataW(struct GetContentDataInfo *Info)
{
	static int nest = 0; //prevent stack overflow on message/error box display
	intptr_t ret = 0;
	if (0==nest++ && IS_PLUGIN_READY(G))
	{
		ret = LF_GetContentData(G.LS, Info);
		LEAVE_CS(G);
	}
	nest--;
	return ret;
}

void LUAPLUG FreeContentDataW(const struct GetContentDataInfo *Info)
{
	EXP_VOID(Info, LF_FreeContentData)
}
#endif
//---------------------------------------------------------------------------

#ifdef EXPORT_ANALYSE
HANDLE LUAPLUG AnalyseW(const struct AnalyseInfo *Info)
{
	EXP_HANDLE(Info, LF_Analyse)
}

void LUAPLUG CloseAnalyseW(const struct CloseAnalyseInfo *Info)
{
	EXP_VOID(Info, LF_CloseAnalyse)
}
#endif
//---------------------------------------------------------------------------

#ifdef EXPORT_PROCESSCONSOLEINPUT
intptr_t LUAPLUG ProcessConsoleInputW(struct ProcessConsoleInputInfo *Info)
{
	EXP_INTPTR(Info, LF_ProcessConsoleInput)
}
#endif
//---------------------------------------------------------------------------
