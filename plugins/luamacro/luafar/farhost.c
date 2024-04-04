#include <stdlib.h>
#include <lua.h>
#include <lauxlib.h>

#include <plugin.hpp>

#include "luafar.h"
#include "ustring.h"
#include "util.h"

typedef struct PluginStartupInfo PSInfo;

static HMODULE GetPluginModuleHandle(const PSInfo *psInfo, GUID* PluginGuid)
{
	HMODULE dll_handle = NULL;
	intptr_t plug_handle;
	if (0 != (plug_handle = psInfo->PluginsControl(NULL, PCTL_FINDPLUGIN, PFM_GUID, PluginGuid)))
	{
		size_t size = psInfo->PluginsControl((HANDLE)plug_handle, PCTL_GETPLUGININFORMATION, 0, NULL);
		if (size != 0)
		{
			struct FarGetPluginInformation *piInfo = (struct FarGetPluginInformation *) malloc(size);
			if (piInfo != NULL)
			{
				piInfo->StructSize = sizeof(*piInfo);
				if (psInfo->PluginsControl((HANDLE)plug_handle, PCTL_GETPLUGININFORMATION, size, piInfo))
					dll_handle = GetModuleHandleW(piInfo->ModuleName);
				free(piInfo);
			}
		}
	}
	return dll_handle;
}

static int far_host_GetFiles(lua_State *L)
{
	typedef intptr_t (WINAPI * T_GetFilesW)(struct GetFilesInfo *);
	T_GetFilesW getfiles;
	struct PanelInfo panInfo;
	struct GetFilesInfo gfInfo;
	HMODULE dll_handle;
	PSInfo *psInfo = GetPluginData(L)->Info;
	HANDLE panHandle = OptHandle(L); //1-st argument
	int collectorPos;
	struct PluginPanelItem *ppi, *ppi_curr;
	size_t i, numLines;

	luaL_checktype(L, 2, LUA_TTABLE);  //2-nd argument
	numLines = lua_objlen(L, 2);
	memset(&gfInfo, 0, sizeof(gfInfo));
	gfInfo.StructSize = sizeof(gfInfo);
	gfInfo.Move = lua_toboolean(L, 3); //3-rd argument
	gfInfo.DestPath = check_utf8_string(L, 4, NULL); //4-th argument
	gfInfo.OpMode = luaL_optinteger(L, 5, (lua_Integer)(OPM_FIND|OPM_SILENT)); //5-th argument

	lua_pushinteger(L,0);  //prepare to return 0

	panInfo.StructSize = sizeof(panInfo);
	if (! (panHandle && psInfo->PanelControl(panHandle,FCTL_GETPANELINFO,0,&panInfo) && panInfo.PluginHandle) )
		return 1;
	gfInfo.hPanel = panInfo.PluginHandle;

	if (NULL == (dll_handle = GetPluginModuleHandle(psInfo, &panInfo.OwnerGuid)))
		return 1;

	if (NULL == (getfiles = (T_GetFilesW)GetProcAddress(dll_handle, "GetFilesW")))
		return 1;

	ppi = (struct PluginPanelItem *)malloc(sizeof(struct PluginPanelItem) * numLines);
	if (ppi == NULL)
		return luaL_error(L, "insufficient memory");

	lua_newtable(L);
	collectorPos = lua_gettop(L);
	for(i=1,ppi_curr=ppi; i<=numLines; i++)
	{
		lua_pushinteger(L, i);
		lua_gettable(L, 2);
		if (lua_istable(L,-1))
			FillPluginPanelItem(L, ppi_curr++, collectorPos);
		lua_pop(L,1);
	}
	gfInfo.ItemsNumber = ppi_curr - ppi;
	gfInfo.PanelItem = ppi;

	lua_pushinteger(L, getfiles(&gfInfo));
	free(ppi);
	return 1;
}

static int far_host_PutFiles(lua_State *L)
{
	typedef intptr_t (WINAPI * T_PutFilesW)(const struct PutFilesInfo *);
	T_PutFilesW putfiles;
	struct PanelInfo panInfo;
	struct PutFilesInfo pfInfo;
	HMODULE dll_handle;
	PSInfo *psInfo = GetPluginData(L)->Info;
	HANDLE panHandle = OptHandle(L); //1-st argument
	int collectorPos;
	struct PluginPanelItem *ppi, *ppi_curr;
	size_t i, numLines;

	luaL_checktype(L, 2, LUA_TTABLE);  //2-nd argument
	numLines = lua_objlen(L, 2);
	memset(&pfInfo, 0, sizeof(pfInfo));
	pfInfo.StructSize = sizeof(pfInfo);
	pfInfo.Move = lua_toboolean(L, 3); //3-rd argument
	pfInfo.SrcPath = check_utf8_string(L, 4, NULL); //4-th argument
	pfInfo.OpMode = luaL_optinteger(L, 5, (lua_Integer)(OPM_SILENT)); //5-th argument

	lua_pushinteger(L,0);  //prepare to return 0

	panInfo.StructSize = sizeof(panInfo);
	if (! (panHandle && psInfo->PanelControl(panHandle,FCTL_GETPANELINFO,0,&panInfo) && panInfo.PluginHandle) )
		return 1;
	pfInfo.hPanel = panInfo.PluginHandle;

	if (NULL == (dll_handle = GetPluginModuleHandle(psInfo, &panInfo.OwnerGuid)))
		return 1;

	if (NULL == (putfiles = (T_PutFilesW)GetProcAddress(dll_handle, "PutFilesW")))
		return 1;

	ppi = (struct PluginPanelItem *)malloc(sizeof(struct PluginPanelItem) * numLines);
	if (ppi == NULL)
		return luaL_error(L, "insufficient memory");

	lua_newtable(L);
	collectorPos = lua_gettop(L);
	for(i=1,ppi_curr=ppi; i<=numLines; i++)
	{
		lua_pushinteger(L, i);
		lua_gettable(L, 2);
		if (lua_istable(L,-1))
			FillPluginPanelItem(L, ppi_curr++, collectorPos);
		lua_pop(L,1);
	}
	pfInfo.ItemsNumber = ppi_curr - ppi;
	pfInfo.PanelItem = ppi;

	lua_pushinteger(L, putfiles(&pfInfo));
	free(ppi);
	return 1;
}

static int far_host_GetFindData(lua_State *L)
{
	typedef intptr_t (WINAPI * T_GetFindDataW)(const struct GetFindDataInfo *);
	typedef void     (WINAPI * T_FreeFindDataW)(const struct FreeFindDataInfo *);
	T_GetFindDataW getfinddata;
	T_FreeFindDataW freefinddata;
	struct PanelInfo panInfo;
	struct GetFindDataInfo gfdInfo;
	HMODULE dll_handle;
	PSInfo *psInfo = GetPluginData(L)->Info;
	HANDLE panHandle = OptHandle(L); //1-st argument

	lua_settop(L, 2); //2 arguments at most
	lua_pushnil(L);  //prepare to return nil
	panInfo.StructSize = sizeof(panInfo);
	if (! (panHandle && psInfo->PanelControl(panHandle,FCTL_GETPANELINFO,0,&panInfo) && panInfo.PluginHandle) )
		return 1;

	if (NULL == (dll_handle = GetPluginModuleHandle(psInfo, &panInfo.OwnerGuid)))
		return 1;

	getfinddata = (T_GetFindDataW)(intptr_t)GetProcAddress(dll_handle, "GetFindDataW");
	memset(&gfdInfo, 0, sizeof(gfdInfo));
	gfdInfo.StructSize = sizeof(gfdInfo);
	gfdInfo.OpMode = luaL_optinteger(L, 2, (lua_Integer)(OPM_FIND | OPM_SILENT)); //2-nd argument
	gfdInfo.hPanel = panInfo.PluginHandle;
	if (! (getfinddata && getfinddata(&gfdInfo)))
		return 1;

	PushPanelItems(L, gfdInfo.PanelItem, gfdInfo.ItemsNumber, 1); //this will be returned

	//as the panel items have been copied (internalized) they should be freed
	freefinddata = (T_FreeFindDataW)(intptr_t)GetProcAddress(dll_handle, "FreeFindDataW");
	if (freefinddata)
	{
		struct FreeFindDataInfo ffdInfo;
		ffdInfo.StructSize = sizeof(ffdInfo);
		ffdInfo.hPanel = panInfo.PluginHandle;
		ffdInfo.PanelItem = gfdInfo.PanelItem;
		ffdInfo.ItemsNumber = gfdInfo.ItemsNumber;
		freefinddata(&ffdInfo);
	}
	return 1;
}

static int far_host_FreeUserData(lua_State *L)
{
	struct FarPanelItemFreeInfo freeInfo;
	size_t ItemsNumber, idx;

	luaL_checktype(L, 1, LUA_TLIGHTUSERDATA);
	freeInfo.hPlugin = lua_touserdata(L, 1);

	luaL_checktype(L, 2, LUA_TTABLE);
	ItemsNumber = lua_objlen(L, 2);

	freeInfo.StructSize = sizeof(freeInfo);
	for (idx=0; idx < ItemsNumber; idx++)
	{
		lua_rawgeti(L, 2, (int)idx+1);
		if (lua_istable(L, -1))
		{
			void *UserData;
			FARPANELITEMFREECALLBACK FreeData;

			lua_getfield(L, -1, "ExtUserData");
			UserData = lua_touserdata(L, -1);
			lua_pop(L, 1);

			lua_getfield(L, -1, "FreeUserData");
			FreeData = (FARPANELITEMFREECALLBACK)(intptr_t)lua_touserdata(L, -1);
			lua_pop(L, 1);

			if (UserData && FreeData)
				FreeData(UserData, &freeInfo);
		}
		lua_pop(L, 1);
	}
	return 0;
}

static int far_host_SetDirectory(lua_State *L)
{
	typedef intptr_t (WINAPI * T_SetDirectoryW)(const struct SetDirectoryInfo *);
	T_SetDirectoryW setdirectory;
	struct PanelInfo panInfo;
	struct SetDirectoryInfo sdInfo;
	HMODULE dll_handle;
	PSInfo *psInfo = GetPluginData(L)->Info;
	HANDLE panHandle = OptHandle(L); //1-st argument
	const wchar_t *dir_name = check_utf8_string(L, 2, NULL); //2-nd argument

	lua_settop(L, 3); //3 arguments at most
	lua_pushboolean(L,0);  //prepare to return false
	panInfo.StructSize = sizeof(panInfo);
	if (! (panHandle && psInfo->PanelControl(panHandle, FCTL_GETPANELINFO, 0, &panInfo) && panInfo.PluginHandle) )
		return 1;

	if (NULL == (dll_handle = GetPluginModuleHandle(psInfo, &panInfo.OwnerGuid)))
		return 1;

	memset(&sdInfo, 0, sizeof(sdInfo));
	sdInfo.StructSize = sizeof(sdInfo);
	sdInfo.Dir = dir_name;
	sdInfo.OpMode = luaL_optinteger(L, 3, (lua_Integer)(OPM_FIND | OPM_SILENT)); //3-rd argument
	sdInfo.hPanel = panInfo.PluginHandle;

	setdirectory = (T_SetDirectoryW)GetProcAddress(dll_handle, "SetDirectoryW");
	if (setdirectory && setdirectory(&sdInfo))
	{
		if (sdInfo.UserData.FreeData)
		{
			struct FarPanelItemFreeInfo fInfo = { sizeof(struct FarPanelItemFreeInfo), panInfo.PluginHandle };
			sdInfo.UserData.FreeData(sdInfo.UserData.Data, &fInfo);
		}
		lua_pushboolean(L,1);  //prepare to return true
	}
	return 1;
}

#define PAIR(prefix,txt) {#txt, prefix ## _ ## txt}

const luaL_Reg far_host_funcs[] =
{
	PAIR( far_host, FreeUserData),
	PAIR( far_host, GetFiles),
	PAIR( far_host, GetFindData),
	PAIR( far_host, PutFiles),
	PAIR( far_host, SetDirectory),

	{NULL, NULL}
};

int luaopen_far_host(lua_State *L)
{
	lua_newtable(L);
	luaL_register(L, NULL, far_host_funcs);
	return 1;
}
