//---------------------------------------------------------------------------
#include <windows.h>
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

#ifndef ENV_PREFIX
# ifdef _WIN64
#  define ENV_PREFIX L"LUAFAR64"
# else
#  define ENV_PREFIX L"LUAFAR"
# endif
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
	CRITICAL_SECTION FindFileSection; // http://forum.farmanager.com/viewtopic.php?f=9&p=107075#p107075
} Global;

#define IsPluginReady(g) (g.LS && g.InitStage==2)

static Global G;

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
	return LF_DlgProc(G.LS, hDlg, Msg, Param1, Param2);
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
	InitializeCriticalSection(&g->FindFileSection);
}

static void DestroyGlobal (Global *g)
{
	if (g->StartupInfo) free(g->StartupInfo);

	if (g->LS) lua_close(g->LS);

	DeleteCriticalSection(&g->FindFileSection);
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
	LF_ProcessEnvVars(L, ENV_PREFIX, G.PluginDir);
	lua_pushcfunction(L, luaopen_luaplug);
	lua_setglobal(L, "_luaplug");
}

__declspec(dllexport) lua_State* GetLuaState()
{
	return IsPluginReady(G) ? G.LS : NULL;
}

/* for other C-files of the plugin */
struct PluginStartupInfo *GetPluginStartupInfo()
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
		G.StartupInfo = (struct PluginStartupInfo *) malloc(aInfo->StructSize + aInfo->FSF->StructSize);
		if (G.StartupInfo)
		{
			memcpy(G.StartupInfo, aInfo, aInfo->StructSize);
			memcpy(G.StartupInfo+1, aInfo->FSF, aInfo->FSF->StructSize);
			G.StartupInfo->FSF = (struct FarStandardFunctions *) (G.StartupInfo+1);
			G.PluginData.Info = G.StartupInfo;
			G.PluginData.FSF = G.StartupInfo->FSF;

			InitLuaState2(G.LS, &G.PluginData);
			if (LF_RunDefaultScript(G.LS))
				G.InitStage++;
		}

		if (G.InitStage != 2)
		{
			if (G.StartupInfo) { free(G.StartupInfo); G.StartupInfo=NULL; }

			lua_close(G.LS);
			G.LS = NULL;
		}
	}
}
//---------------------------------------------------------------------------

void LUAPLUG GetPluginInfoW(struct PluginInfo *Info)
{
	if (G.LS) LF_GetPluginInfo(G.LS, Info);
}
//---------------------------------------------------------------------------

intptr_t LUAPLUG ProcessSynchroEventW(const struct ProcessSynchroEventInfo *Info)
{
	return IsPluginReady(G) ? LF_ProcessSynchroEvent(G.LS, Info) : 0;
}
//---------------------------------------------------------------------------

// This is exported in order not to crash when run from under Far 2.0.xxxx
// Minimal Far version = 3.0.0
int LUAPLUG GetMinFarVersionW()
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
	if (IsPluginReady(G))
	{
		HANDLE h;
		++G.Depth; // prevents crashes (this function can be called recursively)
#ifdef EXPORT_PROCESSDIALOGEVENT
		EnterCriticalSection(&G.FindFileSection);
		h = LF_Open(G.LS, Info);
		LeaveCriticalSection(&G.FindFileSection);
#else
		h = LF_Open(G.LS, Info);
#endif
		return --G.Depth==0 && RecreateLuaState(&G) ? NULL : G.LS ? h : NULL;
	}
	return NULL;
}
#endif
//---------------------------------------------------------------------------

#ifdef EXPORT_GETFINDDATA
intptr_t LUAPLUG GetFindDataW(struct GetFindDataInfo *Info)
{
	return IsPluginReady(G) ? LF_GetFindData(G.LS, Info) : 0;
}

void LUAPLUG FreeFindDataW(const struct FreeFindDataInfo *Info)
{
	if (IsPluginReady(G)) LF_FreeFindData(G.LS, Info);
}
#endif
//---------------------------------------------------------------------------

#ifdef EXPORT_CLOSEPANEL
void LUAPLUG ClosePanelW(const struct ClosePanelInfo *Info)
{
	if (IsPluginReady(G)) LF_ClosePanel(G.LS, Info);
}
#endif
//---------------------------------------------------------------------------

#ifdef EXPORT_GETFILES
intptr_t LUAPLUG GetFilesW(struct GetFilesInfo *Info)
{
	return IsPluginReady(G) ? LF_GetFiles(G.LS, Info) : 0;
}
#endif
//---------------------------------------------------------------------------

#ifdef EXPORT_GETOPENPANELINFO
void LUAPLUG GetOpenPanelInfoW(struct OpenPanelInfo *Info)
{
	if (IsPluginReady(G)) LF_GetOpenPanelInfo(G.LS, Info);
}
#endif
//---------------------------------------------------------------------------

#ifdef EXPORT_EXITFAR
void LUAPLUG ExitFARW(const struct ExitInfo *Info)
{
	if (IsPluginReady(G))
	{
		lua_State* oldState = G.LS;
		G.LS = NULL;
		LF_ExitFAR(oldState, Info);
		lua_close(oldState);
	}
}
#endif
//---------------------------------------------------------------------------

#ifdef EXPORT_COMPARE
intptr_t LUAPLUG CompareW(const struct CompareInfo *Info)
{
	return IsPluginReady(G) ? LF_Compare(G.LS, Info) : 0;
}
#endif
//---------------------------------------------------------------------------

#ifdef EXPORT_CONFIGURE
intptr_t LUAPLUG ConfigureW(const struct ConfigureInfo *Info)
{
	return IsPluginReady(G) ? LF_Configure(G.LS, Info) : 0;
}
#endif
//---------------------------------------------------------------------------

#ifdef EXPORT_DELETEFILES
intptr_t LUAPLUG DeleteFilesW(const struct DeleteFilesInfo *Info)
{
	return IsPluginReady(G) ? LF_DeleteFiles(G.LS, Info) : 0;
}
#endif
//---------------------------------------------------------------------------

#ifdef EXPORT_MAKEDIRECTORY
intptr_t LUAPLUG MakeDirectoryW(struct MakeDirectoryInfo *Info)
{
	return IsPluginReady(G) ? LF_MakeDirectory(G.LS, Info) : 0;
}
#endif
//---------------------------------------------------------------------------

#ifdef EXPORT_PROCESSPANELEVENT
intptr_t LUAPLUG ProcessPanelEventW(const struct ProcessPanelEventInfo *Info)
{
	return IsPluginReady(G) ? LF_ProcessPanelEvent(G.LS, Info) : 0;
}
#endif
//---------------------------------------------------------------------------

#ifdef EXPORT_PROCESSHOSTFILE
intptr_t LUAPLUG ProcessHostFileW(const struct ProcessHostFileInfo *Info)
{
	return IsPluginReady(G) ? LF_ProcessHostFile(G.LS, Info) : 0;
}
#endif
//---------------------------------------------------------------------------

#ifdef EXPORT_PROCESSPANELINPUT
intptr_t LUAPLUG ProcessPanelInputW(const struct ProcessPanelInputInfo *Info)
{
	return IsPluginReady(G) ? LF_ProcessPanelInput(G.LS, Info) : 0;
}
#endif
//---------------------------------------------------------------------------

#ifdef EXPORT_PUTFILES
intptr_t LUAPLUG PutFilesW(const struct PutFilesInfo *Info)
{
	return IsPluginReady(G) ? LF_PutFiles(G.LS, Info) : 0;
}
#endif
//---------------------------------------------------------------------------

#ifdef EXPORT_SETDIRECTORY
intptr_t LUAPLUG SetDirectoryW(const struct SetDirectoryInfo *Info)
{
	return IsPluginReady(G) ? LF_SetDirectory(G.LS, Info) : 0;
}
#endif
//---------------------------------------------------------------------------

#ifdef EXPORT_SETFINDLIST
intptr_t LUAPLUG SetFindListW(const struct SetFindListInfo *Info)
{
	return IsPluginReady(G) ? LF_SetFindList(G.LS, Info) : 0;
}
#endif
//---------------------------------------------------------------------------

#ifdef EXPORT_PROCESSEDITORINPUT
intptr_t LUAPLUG ProcessEditorInputW(const struct ProcessEditorInputInfo *Info)
{
	return IsPluginReady(G) ? LF_ProcessEditorInput(G.LS, Info) : 0;
}
#endif
//---------------------------------------------------------------------------

#ifdef EXPORT_PROCESSEDITOREVENT
intptr_t LUAPLUG ProcessEditorEventW(const struct ProcessEditorEventInfo *Info)
{
	return IsPluginReady(G) ? LF_ProcessEditorEvent(G.LS, Info) : 0;
}
#endif
//---------------------------------------------------------------------------

#ifdef EXPORT_PROCESSVIEWEREVENT
intptr_t LUAPLUG ProcessViewerEventW(const struct ProcessViewerEventInfo *Info)
{
	return IsPluginReady(G) ? LF_ProcessViewerEvent(G.LS, Info) : 0;
}
#endif
//---------------------------------------------------------------------------

#ifdef EXPORT_PROCESSDIALOGEVENT
intptr_t LUAPLUG ProcessDialogEventW(const struct ProcessDialogEventInfo *Info)
{
	if (IsPluginReady(G))
	{
#ifdef EXPORT_OPEN
		intptr_t r;
		EnterCriticalSection(&G.FindFileSection);
		r = LF_ProcessDialogEvent(G.LS, Info);
		LeaveCriticalSection(&G.FindFileSection);
		return r;
#else
		return LF_ProcessDialogEvent(G.LS, Info);
#endif
	}
	return 0;
}
#endif
//---------------------------------------------------------------------------

#ifdef EXPORT_GETCONTENTDATA
intptr_t LUAPLUG GetContentFieldsW(const struct GetContentFieldsInfo *Info)
{
	return IsPluginReady(G) ? LF_GetContentFields(G.LS, Info) : 0;
}

intptr_t LUAPLUG GetContentDataW(struct GetContentDataInfo *Info)
{
	return IsPluginReady(G) ? LF_GetContentData(G.LS, Info) : 0;
}

void LUAPLUG FreeContentDataW(const struct GetContentDataInfo *Info)
{
	if (IsPluginReady(G)) LF_FreeContentData(G.LS, Info);
}
#endif
//---------------------------------------------------------------------------

#ifdef EXPORT_ANALYSE
HANDLE LUAPLUG AnalyseW(const struct AnalyseInfo *Info)
{
	return IsPluginReady(G) ? LF_Analyse(G.LS, Info) : 0;
}
#endif
//---------------------------------------------------------------------------

#ifdef EXPORT_CLOSEANALYSE
void LUAPLUG CloseAnalyseW(const struct CloseAnalyseInfo *Info)
{
	if (IsPluginReady(G)) LF_CloseAnalyse(G.LS, Info);
}
#endif
//---------------------------------------------------------------------------

#ifdef EXPORT_PROCESSCONSOLEINPUT
intptr_t LUAPLUG ProcessConsoleInputW(struct ProcessConsoleInputInfo *Info)
{
	return IsPluginReady(G) ? LF_ProcessConsoleInput(G.LS, Info) : 0;
}
#endif
//---------------------------------------------------------------------------
