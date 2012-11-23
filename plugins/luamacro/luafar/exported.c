//---------------------------------------------------------------------------
#include <windows.h>
#include <signal.h>
#include "luafar.h"
#include "util.h"
#include "ustring.h"
#include "compat52.h"

#define CAST(tp,expr) ((tp)(expr))
#define TRANSFORM_REF(h)        (h > 0 ? h : h - 2)
#define UNTRANSFORM_REF(h)      (int)((intptr_t)h > 0 ? (intptr_t)h : (intptr_t)h + 2)

extern int bit64_push(lua_State *L, INT64 v);
extern int bit64_pushuserdata(lua_State *L, INT64 v);
extern int bit64_getvalue(lua_State *L, int pos, INT64 *target);
extern void PutFlagsToTable(lua_State *L, const char* key, UINT64 flags);
extern UINT64 GetFlagCombination(lua_State *L, int pos, int *success);
extern UINT64 GetFlagsFromTable(lua_State *L, int pos, const char* key);

extern const char* VirtualKeyStrings[256];
extern void LF_Error(lua_State *L, const wchar_t* aMsg);
extern int pushInputRecord(lua_State *L, const INPUT_RECORD* ir);
extern void FillInputRecord(lua_State *L, int pos, INPUT_RECORD *ir);
extern int far_FreeSettings(lua_State *L);
extern HANDLE Open_Luamacro(lua_State *L, const struct OpenInfo *Info);

// "Collector" is a Lua table referenced from the Plugin Object table by name.
// Collector contains an array of lightuserdata which are pointers to malloc'ed
// memory regions.
const char COLLECTOR_FD[]  = "Collector_FindData";
const char COLLECTOR_OPI[] = "Collector_OpenPanelInfo";
const char COLLECTOR_PI[]  = "Collector_PluginInfo";
const char KEY_OBJECT[]    = "Object";

// taken from lua.c v5.1.2
int traceback(lua_State *L)
{
	lua_getglobal(L, "debug");

	if(!lua_istable(L, -1))
	{
		lua_pop(L, 1);
		return 1;
	}

	lua_getfield(L, -1, "traceback");

	if(!lua_isfunction(L, -1))
	{
		lua_pop(L, 2);
		return 1;
	}

	lua_pushvalue(L, 1);  /* pass error message */
	lua_pushinteger(L, 2);  /* skip this function and traceback */
	lua_call(L, 2, 1);  /* call debug.traceback */
	return 1;
}

// taken from lua.c v5.1.2 (modified)
int docall(lua_State *L, int narg, int nret)
{
//TODO(signal)    TPluginData *pdata = GetPluginData(L);
	int status;
	int base = lua_gettop(L) - narg;  /* function index */
	lua_pushcfunction(L, traceback);  /* push traceback function */
	lua_insert(L, base);  /* put it under chunk and args */
//TODO(signal)    pdata->old_action = signal(SIGBREAK, pdata->new_action);
	status = lua_pcall(L, narg, nret, base);
//TODO(signal)    signal(SIGBREAK, pdata->old_action);
	lua_remove(L, base);  /* remove traceback function */
	/* force a complete garbage collection in case of errors */
	if(status != 0) lua_gc(L, LUA_GCCOLLECT, 0);

	return status;
}

// if the function is successfully retrieved, it's on the stack top; 1 is returned
// else 0 returned (and the stack is unchanged)
int GetExportFunction(lua_State* L, const char* FuncName)
{
	lua_getglobal(L, "export");
	lua_getfield(L, -1, FuncName);

	if(lua_isfunction(L, -1))
		return lua_remove(L,-2), 1;
	else
		return lua_pop(L,2), 0;
}

int pcall_msg(lua_State* L, int narg, int nret)
{
	// int status = lua_pcall(L, narg, nret, 0);
	int status = docall(L, narg, nret);

	if(status != 0)
	{
		int status2 = 1;

		if(GetExportFunction(L, "OnError"))
		{
			lua_insert(L,-2);
			status2 = lua_pcall(L,1,0,0);
		}

		if(status2 != 0)
		{
			LF_Error(L, check_utf8_string(L, -1, NULL));
			lua_pop(L, 1);
		}
	}

	far_FreeSettings(L);
	return status;
}

void PushPluginTable(lua_State* L, HANDLE hPlugin)
{
	lua_rawgeti(L, LUA_REGISTRYINDEX, UNTRANSFORM_REF(hPlugin));
}

void PushPluginPair(lua_State* L, HANDLE hPlugin)
{
	lua_rawgeti(L, LUA_REGISTRYINDEX, UNTRANSFORM_REF(hPlugin));
	lua_getfield(L, -1, KEY_OBJECT);
	lua_remove(L, -2);
	lua_pushinteger(L, (intptr_t)hPlugin);
}

void ReplacePluginInfoCollector(lua_State* L)
{
	lua_newtable(L);
	lua_pushvalue(L, -1);
	lua_setfield(L, LUA_REGISTRYINDEX, COLLECTOR_PI);
}

void DestroyCollector(lua_State* L, HANDLE hPlugin, const char* Collector)
{
	PushPluginTable(L, hPlugin);      //+1: Tbl
	lua_pushnil(L);                   //+2
	lua_setfield(L, -2, Collector);   //+1
	lua_pop(L,1);                     //+0
}

// the value is on stack top (-1)
// collector table is under the index 'pos' (this index cannot be a pseudo-index)
const wchar_t* _AddStringToCollector(lua_State *L, int pos)
{
	if(lua_isstring(L,-1))
	{
		const wchar_t* s = check_utf8_string(L, -1, NULL);
		lua_rawseti(L, pos, (int)lua_objlen(L, pos) + 1);
		return s;
	}

	lua_pop(L,1);
	return NULL;
}

// input table is on stack top (-1)
// collector table is under the index 'pos' (this index cannot be a pseudo-index)
const wchar_t* AddStringToCollectorField(lua_State *L, int pos, const char* key)
{
	lua_getfield(L, -1, key);
	return _AddStringToCollector(L, pos);
}

// input table is on stack top (-1)
// collector table is under the index 'pos' (this index cannot be a pseudo-index)
const wchar_t* AddStringToCollectorSlot(lua_State *L, int pos, int key)
{
	lua_pushinteger(L, key);
	lua_gettable(L, -2);
	return _AddStringToCollector(L, pos);
}

// collector table is under the index 'pos' (this index cannot be a pseudo-index)
void* AddBufToCollector(lua_State *L, int pos, size_t size)
{
	void *t;

	if(pos < 0) --pos;

	t = lua_newuserdata(L, size);
	memset(t, 0, size);
	lua_rawseti(L, pos, (int)lua_objlen(L, pos) + 1);
	return t;
}

// -- a table is on stack top
// -- its field 'field' is an array of strings
// -- 'cpos' - collector stack position
const wchar_t** CreateStringsArray(lua_State* L, int cpos, const char* field,
                                   size_t *numstrings)
{
	const wchar_t **buf = NULL;
	lua_getfield(L, -1, field);

	if(lua_istable(L, -1))
	{
		size_t n = lua_objlen(L, -1);

		if(numstrings) *numstrings = n;

		if(n > 0)
		{
			size_t i;
			buf = (const wchar_t**)AddBufToCollector(L, cpos, (n+1) * sizeof(wchar_t*));

			for(i=0; i < n; i++)
				buf[i] = AddStringToCollectorSlot(L, cpos, (int)i+1);

			buf[n] = NULL;
		}
	}

	lua_pop(L, 1);
	return buf;
}

void WINAPI FarPanelItemFreeCallback(void* UserData, const struct FarPanelItemFreeInfo* Info)
{
	FarPanelItemUserData* ud;
	(void) Info;
	ud = (FarPanelItemUserData*)UserData;
	luaL_unref(ud->L, LUA_REGISTRYINDEX, ud->ref);
	free(ud);
}

// input table is on stack top (-1)
void FillPluginPanelItem(lua_State *L, struct PluginPanelItem *pi, int CollectorPos)
{
	pi->CreationTime      = GetFileTimeFromTable(L, "CreationTime");
	pi->LastAccessTime    = GetFileTimeFromTable(L, "LastAccessTime");
	pi->LastWriteTime     = GetFileTimeFromTable(L, "LastWriteTime");
	pi->ChangeTime        = GetFileTimeFromTable(L, "ChangeTime");
	pi->FileSize          = GetFileSizeFromTable(L, "FileSize");
	pi->AllocationSize    = GetFileSizeFromTable(L, "AllocationSize");
	pi->Flags             = GetFlagsFromTable(L, -1, "Flags");
	pi->NumberOfLinks     = GetOptIntFromTable(L, "NumberOfLinks", 0);
	pi->CRC32             = GetOptIntFromTable(L, "CRC32", 0);
	pi->FileAttributes    = GetAttrFromTable(L);

  if (CollectorPos != 0)
  {
    pi->FileName          = (wchar_t*)AddStringToCollectorField(L, CollectorPos, "FileName");
    pi->AlternateFileName = (wchar_t*)AddStringToCollectorField(L, CollectorPos, "AlternateFileName");
    pi->Description       = (wchar_t*)AddStringToCollectorField(L, CollectorPos, "Description");
    pi->Owner             = (wchar_t*)AddStringToCollectorField(L, CollectorPos, "Owner");
  }
  else
  {
    lua_getfield(L, -1, "FileName");                     // +1
    pi->FileName = opt_utf8_string(L, -1, L"");
    lua_getfield(L, -2, "AlternateFileName");            // +2
    pi->AlternateFileName = opt_utf8_string(L, -1, L"");
    lua_getfield(L, -2, "Description");                  // +3
    pi->Description = opt_utf8_string(L, -1, L"");
    lua_getfield(L, -2, "Owner");                        // +4
    pi->Owner = opt_utf8_string(L, -1, L"");
  }

  lua_getfield(L, -1, "UserData");
	if(!lua_isnil(L, -1))
	{
		FarPanelItemUserData* ud = (FarPanelItemUserData*)malloc(sizeof(FarPanelItemUserData));
		ud->L = L;
		ud->ref = luaL_ref(L, LUA_REGISTRYINDEX);
		pi->UserData.Data = ud;
		pi->UserData.FreeData = FarPanelItemFreeCallback;
	}
	else
		lua_pop(L, 1);
}

// Two known values on the stack top: Tbl (at -2) and FindData (at -1).
// Both are popped off the stack on return.
void FillFindData(lua_State* L, struct PluginPanelItem **pPanelItems,
                  size_t *pItemsNumber, const char* Collector)
{
	struct PluginPanelItem *ppi;
	size_t i, num = 0;
	size_t numLines = lua_objlen(L,-1);
	lua_newtable(L);                           //+3  Tbl,FindData,Coll
	lua_pushvalue(L,-1);                       //+4: Tbl,FindData,Coll,Coll
	lua_setfield(L, -4, Collector);            //+3: Tbl,FindData,Coll
	ppi = (struct PluginPanelItem *)malloc(sizeof(struct PluginPanelItem) * numLines);
	memset(ppi, 0, numLines*sizeof(struct PluginPanelItem));

	for(i=1; i<=numLines; i++)
	{
		lua_pushinteger(L, i);                   //+4
		lua_gettable(L, -3);                     //+4: Tbl,FindData,Coll,FindData[i]

		if(lua_istable(L,-1))
		{
			FillPluginPanelItem(L, ppi+num, -2);
			++num;
		}

		lua_pop(L,1);                            //+3
	}

	lua_pop(L,3);                              //+0
	*pItemsNumber = num;
	*pPanelItems = ppi;
}

intptr_t LF_GetFindData(lua_State* L, struct GetFindDataInfo *Info)
{
	if(GetExportFunction(L, "GetFindData"))      //+1: Func
	{
		Info->StructSize = sizeof(*Info);
		PushPluginPair(L, Info->hPanel);           //+3: Func,Pair
		bit64_push(L, Info->OpMode);               //+4: Func,Pair,OpMode

		if(!pcall_msg(L, 3, 1))                    //+1: FindData
		{
			if(lua_istable(L, -1))
			{
				PushPluginTable(L, Info->hPanel);      //+2: FindData,Tbl
				lua_insert(L, -2);                     //+2: Tbl,FindData
				FillFindData(L, &Info->PanelItem, &Info->ItemsNumber, COLLECTOR_FD);
				return TRUE;
			}

			lua_pop(L,1);
		}
	}

	return FALSE;
}

void LF_FreeFindData(lua_State* L, const struct FreeFindDataInfo *Info)
{
	DestroyCollector(L, Info->hPanel, COLLECTOR_FD);
	free(Info->PanelItem);
}
//---------------------------------------------------------------------------

// PanelItem table should be on Lua stack top
void UpdateFileSelection(lua_State* L, struct PluginPanelItem *PanelItem,
                         size_t ItemsNumber)
{
	int i;

	for(i=0; i<(int)ItemsNumber; i++)
	{
		lua_rawgeti(L, -1, i+1);           //+1

		if(lua_istable(L,-1))
		{
			lua_getfield(L,-1,"Flags");      //+2

			if(lua_istable(L,-1))
			{
				lua_getfield(L,-1,"selected"); //+3

				if(lua_toboolean(L,-1))
					PanelItem[i].Flags |= PPIF_SELECTED;
				else
					PanelItem[i].Flags &= ~PPIF_SELECTED;

				lua_pop(L,1);       //+2
			}

			lua_pop(L,1);         //+1
		}

		lua_pop(L,1);           //+0
	}
}
//---------------------------------------------------------------------------

intptr_t LF_GetFiles(lua_State* L, struct GetFilesInfo *Info)
{
	if(GetExportFunction(L, "GetFiles"))         //+1: Func
	{
		intptr_t ret;
		Info->StructSize = sizeof(*Info);
		PushPanelItems(L, Info->PanelItem, Info->ItemsNumber); //+2: Func,Item
		lua_insert(L,-2);                  //+2: Item,Func
		PushPluginPair(L, Info->hPanel);   //+4: Item,Func,Pair
		lua_pushvalue(L,-4);               //+5: Item,Func,Pair,Item
		lua_pushboolean(L, Info->Move);
		push_utf8_string(L, Info->DestPath, -1);
		bit64_push(L, Info->OpMode);       //+8: Item,Func,Pair,Item,Move,Dest,OpMode
		ret = pcall_msg(L, 6, 2);          //+3: Item,Res,Dest

		if(ret == 0)
		{
			if(lua_isstring(L,-1))
			{
				Info->DestPath = check_utf8_string(L,-1,NULL);
				lua_setfield(L, LUA_REGISTRYINDEX, "GetFiles.DestPath"); // protect from GC
			}
			else
				lua_pop(L,1);                  //+2: Item,Res

			ret = lua_tointeger(L,-1);
			lua_pop(L,1);                    //+1: Item
			UpdateFileSelection(L, Info->PanelItem, Info->ItemsNumber);
			return lua_pop(L,1), ret;
		}

		return lua_pop(L,1), 0;
	}

	return 0;
}
//---------------------------------------------------------------------------

// return FALSE only if error occurred
BOOL CheckReloadDefaultScript(lua_State *L)
{
	// reload default script?
	int reload;
	lua_getglobal(L, "far");
	lua_getfield(L, -1, "ReloadDefaultScript");
	reload = lua_toboolean(L, -1);
	lua_pop(L, 2);
	return !reload || LF_RunDefaultScript(L);
}

// -- an object (any non-nil value) is on stack top;
// -- a new table is created, the object is put into it under the key KEY_OBJECT;
// -- the table is put into the registry, and reference to it is obtained;
// -- the function pops the object and returns the reference;
intptr_t RegisterObject(lua_State* L)
{
	int ref;
	lua_newtable(L);               //+2: Obj,Tbl
	lua_pushvalue(L,-2);           //+3: Obj,Tbl,Obj
	lua_setfield(L,-2,KEY_OBJECT); //+2: Obj,Tbl
	ref = luaL_ref(L, LUA_REGISTRYINDEX); //+1: Obj
	lua_pop(L,1);                  //+0
	return TRANSFORM_REF(ref);
}

static void PushAnalyseInfo(lua_State* L, const struct AnalyseInfo *Info)
{
	lua_createtable(L, 0, 4);
	PutIntToTable(L, "StructSize", (int)Info->StructSize);
	PutWStrToTable(L, "FileName", Info->FileName, -1);
	PutLStrToTable(L, "Buffer", Info->Buffer, Info->BufferSize);
	PutFlagsToTable(L, "OpMode", Info->OpMode);
}

HANDLE LF_Analyse(lua_State* L, const struct AnalyseInfo *Info)
{
	HANDLE result = NULL;

	if(GetExportFunction(L, "Analyse"))    //+1
	{
		PushAnalyseInfo(L, Info);            //+2

		if(!pcall_msg(L, 1, 1))              //+1
		{
			if(lua_toboolean(L, -1))
			{
				intptr_t ref = luaL_ref(L, LUA_REGISTRYINDEX); //+0

				if(ref == 0)    /* Lua 5.1 manual doesn't guarantee ref != 0 */
				{
					lua_rawgeti(L, LUA_REGISTRYINDEX, 0); //+1
					ref = luaL_ref(L, LUA_REGISTRYINDEX); //+0
					luaL_unref(L, LUA_REGISTRYINDEX, 0);
				}

				result = CAST(HANDLE, ref);
			}
			else
				lua_pop(L, 1);                   //+0
		}
	}

	return result;
}

void LF_CloseAnalyse(lua_State* L, const struct CloseAnalyseInfo *Info)
{
	luaL_unref(L, LUA_REGISTRYINDEX, (int)(intptr_t)Info->Handle);
}
//---------------------------------------------------------------------------

void LF_GetOpenPanelInfo(lua_State* L, struct OpenPanelInfo *aInfo)
{
	int cpos;
	size_t dfn;
	struct OpenPanelInfo *Info;
	aInfo->StructSize = sizeof(struct OpenPanelInfo);

	if(!GetExportFunction(L, "GetOpenPanelInfo"))     //+1
		return;

	PushPluginPair(L, aInfo->hPanel);                 //+3

	if(pcall_msg(L, 2, 1) != 0)
		return;

	if(lua_isstring(L,-1) && !strcmp("reuse", lua_tostring(L,-1)))
	{
		struct OpenPanelInfo *Info;
		PushPluginTable(L, aInfo->hPanel);               //+2: reuse,Tbl
		lua_getfield(L, -1, COLLECTOR_OPI);              //+3: reuse,Tbl,Coll

		if(!lua_istable(L,-1))       // collector either not set, or destroyed
		{
			lua_pop(L,3);
			return;
		}

		lua_rawgeti(L,-1,1);                             //+4: reuse,Tbl,Coll,OPI
		Info = (struct OpenPanelInfo*)lua_touserdata(L,-1);
		*aInfo = *Info;
		lua_pop(L,4);
		return;
	}

	if(!lua_istable(L, -1))                            //+1: Info
	{
		lua_pop(L, 1);
		LF_Error(L, L"GetOpenPanelInfo should return a table");
		return;
	}

	DestroyCollector(L, aInfo->hPanel, COLLECTOR_OPI);
	PushPluginTable(L, aInfo->hPanel);                 //+2: Info,Tbl
	lua_newtable(L);                                   //+3: Info,Tbl,Coll
	cpos = lua_gettop(L);       // collector stack position
	lua_pushvalue(L,-1);                               //+4: Info,Tbl,Coll,Coll
	lua_setfield(L, -3, COLLECTOR_OPI);                //+3: Info,Tbl,Coll
	lua_pushvalue(L,-3);                               //+4: Info,Tbl,Coll,Info
	//---------------------------------------------------------------------------
	// First element in the collector; can be retrieved on later calls;
	Info = (struct OpenPanelInfo*) AddBufToCollector(L, cpos, sizeof(struct OpenPanelInfo));
	//---------------------------------------------------------------------------
	Info->StructSize = sizeof(struct OpenPanelInfo);
	Info->FreeSize   = CAST(unsigned __int64, GetOptNumFromTable(L, "FreeSize", 0));
	Info->Flags      = GetFlagsFromTable(L, -1, "Flags");
	Info->HostFile   = AddStringToCollectorField(L, cpos, "HostFile");
	Info->CurDir     = AddStringToCollectorField(L, cpos, "CurDir");
	Info->Format     = AddStringToCollectorField(L, cpos, "Format");
	Info->PanelTitle = AddStringToCollectorField(L, cpos, "PanelTitle");
	//---------------------------------------------------------------------------
	lua_getfield(L, -1, "InfoLines");
	lua_getfield(L, -2, "InfoLinesNumber");

	if(lua_istable(L,-2) && lua_isnumber(L,-1))
	{
		intptr_t InfoLinesNumber = lua_tointeger(L, -1);
		lua_pop(L,1);                         //+5: Info,Tbl,Coll,Info,Lines

		if(InfoLinesNumber > 0 && InfoLinesNumber <= 100)
		{
			int i;
			struct InfoPanelLine *pl = (struct InfoPanelLine*)
			                           AddBufToCollector(L, cpos, InfoLinesNumber * sizeof(struct InfoPanelLine));
			Info->InfoLines = pl;
			Info->InfoLinesNumber = InfoLinesNumber;

			for(i=0; i<InfoLinesNumber; ++i,++pl,lua_pop(L,1))
			{
				lua_pushinteger(L, i+1);
				lua_gettable(L, -2);

				if(lua_istable(L, -1))            //+6: Info,Tbl,Coll,Info,Lines,Line
				{
					pl->Text = AddStringToCollectorField(L, cpos, "Text");
					pl->Data = AddStringToCollectorField(L, cpos, "Data");
					pl->Flags = GetFlagsFromTable(L, -1, "Flags");
				}
			}
		}

		lua_pop(L,1);
	}
	else lua_pop(L, 2);

	//---------------------------------------------------------------------------
	Info->DescrFiles = CreateStringsArray(L, cpos, "DescrFiles", &dfn);
	Info->DescrFilesNumber = dfn;
	//---------------------------------------------------------------------------
	lua_getfield(L, -1, "PanelModesArray");
	lua_getfield(L, -2, "PanelModesNumber");

	if(lua_istable(L,-2) && lua_isnumber(L,-1))
	{
		intptr_t PanelModesNumber = lua_tointeger(L, -1);
		lua_pop(L,1);                               //+5: Info,Tbl,Coll,Info,Modes

		if(PanelModesNumber > 0 && PanelModesNumber <= 100)
		{
			int i;
			struct PanelMode *pm = (struct PanelMode*)
			                       AddBufToCollector(L, cpos, PanelModesNumber * sizeof(struct PanelMode));
			Info->PanelModesArray = pm;
			Info->PanelModesNumber = PanelModesNumber;

			for(i=0; i<PanelModesNumber; ++i,++pm,lua_pop(L,1))
			{
				lua_pushinteger(L, i+1);
				lua_gettable(L, -2);

				if(lua_istable(L, -1))                  //+6: Info,Tbl,Coll,Info,Modes,Mode
				{
					pm->ColumnTypes  = (wchar_t*)AddStringToCollectorField(L, cpos, "ColumnTypes");
					pm->ColumnWidths = (wchar_t*)AddStringToCollectorField(L, cpos, "ColumnWidths");
					pm->StatusColumnTypes  = (wchar_t*)AddStringToCollectorField(L, cpos, "StatusColumnTypes");
					pm->StatusColumnWidths = (wchar_t*)AddStringToCollectorField(L, cpos, "StatusColumnWidths");
					pm->ColumnTitles = (const wchar_t* const*)CreateStringsArray(L, cpos, "ColumnTitles", NULL);
					pm->Flags = GetFlagsFromTable(L, -1, "Flags");
				}
			}
		}

		lua_pop(L,1);
	}
	else lua_pop(L, 2);

	//---------------------------------------------------------------------------
	Info->StartPanelMode = GetOptIntFromTable(L, "StartPanelMode", 0);
	Info->StartSortMode  = GetOptIntFromTable(L, "StartSortMode", 0);
	Info->StartSortOrder = GetOptIntFromTable(L, "StartSortOrder", 0);
	//---------------------------------------------------------------------------
	lua_getfield(L, -1, "KeyBar");

	if(lua_istable(L, -1))
	{
		size_t size;
		int i;
		struct KeyBarTitles *kbt = (struct KeyBarTitles*)AddBufToCollector(L, cpos, sizeof(struct KeyBarTitles));
		Info->KeyBar = kbt;
		kbt->CountLabels = lua_objlen(L, -1);
		size = kbt->CountLabels * sizeof(struct KeyBarLabel);
		kbt->Labels = (struct KeyBarLabel*)AddBufToCollector(L, cpos, size);
		memset(kbt->Labels, 0, size);

		for(i=0; i < (int)kbt->CountLabels; i++)
		{
			lua_rawgeti(L, -1, i+1);

			if(!lua_istable(L, -1))
			{
				kbt->CountLabels = i;
				lua_pop(L, 1);
				break;
			}

			kbt->Labels[i].Key.VirtualKeyCode = GetOptIntFromTable(L, "VirtualKeyCode", 0);
			lua_getfield(L, -1, "ControlKeyState");
			kbt->Labels[i].Key.ControlKeyState = CAST(DWORD, GetFlagCombination(L, -1, NULL));
			lua_pop(L, 1);
			kbt->Labels[i].Text = AddStringToCollectorField(L, cpos, "Text");
			kbt->Labels[i].LongText = AddStringToCollectorField(L, cpos, "LongText");
			lua_pop(L, 1);
		}
	}

	lua_pop(L, 1);
	//---------------------------------------------------------------------------
	Info->ShortcutData = AddStringToCollectorField(L, cpos, "ShortcutData");
	//---------------------------------------------------------------------------
	lua_pop(L,4);
	*aInfo = *Info;
}
//---------------------------------------------------------------------------

void PushFarMacroValue(lua_State* L, const struct FarMacroValue* val)
{
	if (val->Type == FMVT_INTEGER)      bit64_push(L, val->Value.Integer);
	else if (val->Type == FMVT_DOUBLE)  lua_pushnumber(L, val->Value.Double);
	else if (val->Type == FMVT_STRING)  push_utf8_string(L, val->Value.String, -1);
	else if (val->Type == FMVT_BOOLEAN) lua_pushboolean(L, (int)val->Value.Boolean);
	else if (val->Type == FMVT_BINARY)
	{
		lua_createtable(L,1,0);
		lua_pushlstring(L, (char*)val->Value.Binary.Data, val->Value.Binary.Size);
		lua_rawseti(L,-2,1);
	}
	else lua_pushboolean(L, 0);
}

static void PushParamsTable(lua_State* L, const struct OpenMacroInfo* Data)
{
	size_t i;
	lua_createtable(L, (int)Data->Count, 0);
	for(i=0; i < Data->Count; i++)
	{
		PushFarMacroValue(L, Data->Values + i);
		lua_rawseti(L, -2, i+1);
	}
}

static void WINAPI FillFarMacroCall_Callback (void *CallbackData, struct FarMacroValue *Values, size_t Count)
{
	size_t i;
	struct FarMacroCall *fmc = (struct FarMacroCall*)CallbackData;
	(void)Values; // not used
	(void)Count;  // not used
	for(i=0; i<fmc->Count; i++)
	{
		struct FarMacroValue *v = fmc->Values + i;
		if (v->Type == FMVT_STRING)
			free((void*)v->Value.String);
		else if (v->Type == FMVT_BINARY && v->Value.Binary.Size)
			free(v->Value.Binary.Data);
	}
	free(CallbackData);
}

static HANDLE FillFarMacroCall (lua_State* L, int narg)
{
	INT64 val64;
	int i;

	struct FarMacroCall *fmc = (struct FarMacroCall*)
		malloc(sizeof(struct FarMacroCall) + narg*sizeof(struct FarMacroValue));

	fmc->StructSize = sizeof(*fmc);
	fmc->Count = narg;
	fmc->Values = (struct FarMacroValue*)(fmc+1);
	fmc->Callback = FillFarMacroCall_Callback;
	fmc->CallbackData = fmc;

	for (i=0; i<narg; i++)
	{
		int type = lua_type(L, i-narg);
		if (type == LUA_TNUMBER)
		{
			fmc->Values[i].Type = FMVT_DOUBLE;
			fmc->Values[i].Value.Double = lua_tonumber(L, i-narg);
		}
		else if (type == LUA_TBOOLEAN)
		{
			fmc->Values[i].Type = FMVT_BOOLEAN;
			fmc->Values[i].Value.Boolean = lua_toboolean(L, i-narg);
		}
		else if (type == LUA_TSTRING)
		{
			fmc->Values[i].Type = FMVT_STRING;
			fmc->Values[i].Value.String = wcsdup(check_utf8_string(L, i-narg, NULL));
		}
		else if (type == LUA_TTABLE)
		{
			size_t len;
			fmc->Values[i].Type = FMVT_BINARY;
			fmc->Values[i].Value.Binary.Data = (char*)"";
			fmc->Values[i].Value.Binary.Size = 0;
			lua_rawgeti(L, i-narg, 1);
			if (lua_type(L,-1) == LUA_TSTRING && (len=lua_objlen(L,-1)) != 0)
			{
				void* arr = malloc(len);
				memcpy(arr, lua_tostring(L,-1), len);
				fmc->Values[i].Value.Binary.Data = arr;
				fmc->Values[i].Value.Binary.Size = len;
			}
			lua_pop(L,1);
		}
		else if (bit64_getvalue(L, i-narg, &val64))
		{
			fmc->Values[i].Type = FMVT_INTEGER;
			fmc->Values[i].Value.Integer = val64;
		}
		else
		{
			fmc->Values[i].Type = FMVT_BOOLEAN;
			fmc->Values[i].Value.Boolean = 0;
		}
	}

	return (HANDLE)fmc;
}

HANDLE LF_Open(lua_State* L, const struct OpenInfo *Info)
{
	if(!CheckReloadDefaultScript(L) || !GetExportFunction(L, "Open"))
		return NULL;

	if(Info->OpenFrom == OPEN_LUAMACRO)
		return Open_Luamacro(L, Info);

	lua_pushinteger(L, Info->OpenFrom); // 1-st argument
	lua_pushlstring(L, (const char*)Info->Guid, sizeof(GUID)); // 2-nd argument

	// 3-rd argument

	if(Info->OpenFrom == OPEN_FROMMACRO)
		PushParamsTable(L, (struct OpenMacroInfo*)Info->Data);
	else if(Info->OpenFrom == OPEN_SHORTCUT)
	{
		struct OpenShortcutInfo *osi = CAST(struct OpenShortcutInfo*, Info->Data);
		lua_createtable(L, 0, 3);
		PutWStrToTable(L, "HostFile", osi->HostFile, -1);
		PutWStrToTable(L, "ShortcutData", osi->ShortcutData, -1);
		PutFlagsToTable(L, "Flags", osi->Flags);
	}
	else if(Info->OpenFrom == OPEN_COMMANDLINE)
		push_utf8_string(L, CAST(struct OpenCommandLineInfo*, Info->Data)->CommandLine, -1);
	else if(Info->OpenFrom == OPEN_DIALOG)
	{
		struct OpenDlgPluginData *data = CAST(struct OpenDlgPluginData*, Info->Data);
		lua_createtable(L, 0, 1);
		NewDialogData(L, NULL, data->hDlg, FALSE);
		lua_setfield(L, -2, "hDlg");
	}
	else if(Info->OpenFrom == OPEN_ANALYSE)
	{
		struct OpenAnalyseInfo* oai = CAST(struct OpenAnalyseInfo*, Info->Data);
		PushAnalyseInfo(L, oai->Info);
		lua_rawgeti(L, LUA_REGISTRYINDEX, (int)(intptr_t)oai->Handle);
		lua_setfield(L, -2, "Handle");
		luaL_unref(L, LUA_REGISTRYINDEX, (int)(intptr_t)oai->Handle);
	}
	else
		lua_pushinteger(L, Info->Data);

	// Call export.Open()

	if(Info->OpenFrom == OPEN_FROMMACRO)
	{
		int top = lua_gettop(L);
		lua_checkstack(L, 4096);
		if (pcall_msg(L, 3, LUA_MULTRET) == 0)
		{
			HANDLE ret;
			int narg, type;

			if ((narg = lua_gettop(L) - top + 4) == 0)
				return NULL;
			else if (narg == 1 && ((type=lua_type(L,-1)) == LUA_TBOOLEAN || type == LUA_TNIL))
				ret = (HANDLE)(intptr_t)(lua_toboolean(L,-1) ? 1:0);
			else
				ret = FillFarMacroCall(L,narg);

			lua_pop(L,narg);
			return ret;
		}
	}
	else
	{
		if(pcall_msg(L, 3, 1) == 0)
		{
			if(lua_type(L,-1) == LUA_TNUMBER && lua_tonumber(L,-1) == -1)
			{
				lua_pop(L,1);
				return PANEL_STOP;
			}
			else if(lua_toboolean(L, -1))             //+1: Obj
				return CAST(HANDLE, RegisterObject(L)); //+0

			lua_pop(L,1);
		}
	}

	return NULL;
}

void LF_ClosePanel(lua_State* L, const struct ClosePanelInfo *Info)
{
	if(GetExportFunction(L, "ClosePanel"))    //+1: Func
	{
		PushPluginPair(L, Info->hPanel);        //+3: Func,Pair
		pcall_msg(L, 2, 0);
	}

	DestroyCollector(L, Info->hPanel, COLLECTOR_OPI);
	luaL_unref(L, LUA_REGISTRYINDEX, UNTRANSFORM_REF(Info->hPanel));
}

intptr_t LF_Compare(lua_State* L, const struct CompareInfo *Info)
{
	intptr_t res = -2; // default FAR compare function should be used

	if(GetExportFunction(L, "Compare"))    //+1: Func
	{
		PushPluginPair(L, Info->hPanel);     //+3: Func,Pair
		PushPanelItem(L, Info->Item1);       //+4
		PushPanelItem(L, Info->Item2);       //+5
		lua_pushinteger(L, Info->Mode);      //+6

		if(0 == pcall_msg(L, 5, 1))          //+1
		{
			res = lua_tointeger(L,-1);
			lua_pop(L,1);
		}
	}

	return res;
}

intptr_t LF_Configure(lua_State* L, const struct ConfigureInfo *Info)
{
	int res = FALSE;

	if(GetExportFunction(L, "Configure"))    //+1: Func
	{
		lua_pushlstring(L, (const char*)Info->Guid, sizeof(GUID));

		if(0 == pcall_msg(L, 1, 1))          //+1
		{
			res = lua_toboolean(L,-1);
			lua_pop(L,1);
		}
	}

	return res;
}

intptr_t LF_DeleteFiles(lua_State* L, const struct DeleteFilesInfo *Info)
{
	int res = FALSE;

	if(GetExportFunction(L, "DeleteFiles"))      //+1: Func
	{
		PushPluginPair(L, Info->hPanel);           //+3: Func,Pair
		PushPanelItems(L, Info->PanelItem, Info->ItemsNumber); //+4
		bit64_push(L, Info->OpMode);               //+5

		if(0 == pcall_msg(L, 4, 1))                //+1
		{
			res = lua_toboolean(L,-1);
			lua_pop(L,1);
		}
	}

	return res;
}

// far.MakeDirectory returns 2 values:
//    a) status (an integer; in accordance to FAR API), and
//    b) new directory name (a string; optional)
intptr_t LF_MakeDirectory(lua_State* L, struct MakeDirectoryInfo *Info)
{
	intptr_t res = 0;

	if(GetExportFunction(L, "MakeDirectory"))    //+1: Func
	{
		Info->StructSize = sizeof(*Info);
		PushPluginPair(L, Info->hPanel);           //+3: Func,Pair
		push_utf8_string(L, Info->Name, -1);       //+4
		bit64_push(L, Info->OpMode);               //+5

		if(0 == pcall_msg(L, 4, 2))                //+2
		{
			res = lua_tointeger(L,-2);

			if(res == 1 && lua_isstring(L,-1))
			{
				Info->Name = check_utf8_string(L,-1,NULL);
				lua_pushvalue(L, -1);
				lua_setfield(L, LUA_REGISTRYINDEX, "MakeDirectory.Name"); // protect from GC
			}
			else if(res != -1)
				res = 0;

			lua_pop(L,2);
		}
	}

	return res;
}

intptr_t LF_ProcessPanelEvent(lua_State* L, const struct ProcessPanelEventInfo *Info)
{
	int res = FALSE;

	if(GetExportFunction(L, "ProcessPanelEvent"))    //+1: Func
	{
		PushPluginPair(L, Info->hPanel);   //+3
		lua_pushinteger(L, Info->Event);   //+4
		lua_pushnil(L);                    //+5

		if(0 == pcall_msg(L, 4, 1))        //+1
		{
			res = lua_toboolean(L,-1);
			lua_pop(L,1);                    //+0
		}
	}

	return res;
}

intptr_t LF_ProcessHostFile(lua_State* L, const struct ProcessHostFileInfo *Info)
{
	if(GetExportFunction(L, "ProcessHostFile"))      //+1: Func
	{
		intptr_t ret;
		PushPanelItems(L, Info->PanelItem, Info->ItemsNumber); //+2: Func,Item
		lua_insert(L,-2);                  //+2: Item,Func
		PushPluginPair(L, Info->hPanel);   //+4: Item,Func,Pair
		lua_pushvalue(L,-4);               //+5: Item,Func,Pair,Item
		bit64_push(L, Info->OpMode);       //+6: Item,Func,Pair,Item,OpMode
		ret = pcall_msg(L, 4, 1);          //+2: Item,Res

		if(ret == 0)
		{
			ret = lua_toboolean(L,-1);
			lua_pop(L,1);                    //+1: Item
			UpdateFileSelection(L, Info->PanelItem, Info->ItemsNumber);
			return lua_pop(L,1), ret;
		}

		lua_pop(L,1);
	}

	return FALSE;
}

intptr_t LF_ProcessPanelInput(lua_State* L, const struct ProcessPanelInputInfo *Info)
{
	if(GetExportFunction(L, "ProcessPanelInput"))      //+1: Func
	{
		PushPluginPair(L, Info->hPanel);                 //+3: Func,Pair
		pushInputRecord(L, &Info->Rec);                  //+4

		if(pcall_msg(L, 3, 1) == 0)                      //+1: Res
		{
			intptr_t ret = lua_toboolean(L,-1);
			return lua_pop(L,1), ret;
		}
	}

	return FALSE;
}

intptr_t LF_PutFiles(lua_State* L, const struct PutFilesInfo *Info)
{
	if(GetExportFunction(L, "PutFiles"))       //+1: Func
	{
		intptr_t ret;
		PushPanelItems(L, Info->PanelItem, Info->ItemsNumber); //+2: Func,Items
		lua_insert(L,-2);                        //+2: Items,Func
		PushPluginPair(L, Info->hPanel);         //+4: Items,Func,Pair
		lua_pushvalue(L,-4);                     //+5: Items,Func,Pair,Items
		lua_pushboolean(L, Info->Move);          //+6: Items,Func,Pair,Items,Move
		push_utf8_string(L, Info->SrcPath, -1);  //+7: Items,Func,Pair,Items,Move,SrcPath
		bit64_push(L, Info->OpMode);             //+8: Items,Func,Pair,Items,Move,SrcPath,OpMode
		ret = pcall_msg(L, 6, 1);                //+2: Items,Res

		if(ret == 0)
		{
			ret = lua_tointeger(L,-1);
			lua_pop(L,1);                    //+1: Items
			UpdateFileSelection(L, Info->PanelItem, Info->ItemsNumber);
			return lua_pop(L,1), ret;
		}

		lua_pop(L,1);
	}

	return 0;
}

intptr_t LF_SetDirectory(lua_State* L, const struct SetDirectoryInfo *Info) //TODO: Info->UserData
{
	if(GetExportFunction(L, "SetDirectory"))      //+1: Func
	{
		intptr_t ret;
		PushPluginPair(L, Info->hPanel);     //+3: Func,Pair
		push_utf8_string(L, Info->Dir, -1);  //+4: Func,Pair,Dir
		bit64_push(L, Info->OpMode);         //+5: Func,Pair,Dir,OpMode
		ret = pcall_msg(L, 4, 1);            //+1: Res

		if(ret == 0)
		{
			ret = lua_toboolean(L,-1);
			return lua_pop(L,1), ret;
		}
	}

	return FALSE;
}

intptr_t LF_SetFindList(lua_State* L, const struct SetFindListInfo *Info)
{
	if(GetExportFunction(L, "SetFindList"))                  //+1: Func
	{
		intptr_t ret;
		PushPluginPair(L, Info->hPanel);                       //+3: Func,Pair
		PushPanelItems(L, Info->PanelItem, Info->ItemsNumber); //+4: Func,Pair,Items
		ret = pcall_msg(L, 3, 1);                              //+1: Res

		if(ret == 0)
		{
			ret = lua_toboolean(L,-1);
			return lua_pop(L,1), ret;
		}
	}

	return FALSE;
}

void LF_ExitFAR(lua_State* L, const struct ExitInfo *Info)
{
	(void)Info;

	if(GetExportFunction(L, "ExitFAR"))    //+1: Func
		pcall_msg(L, 0, 0);                  //+0
}

void getPluginMenuItems(lua_State* L, struct PluginMenuItem *pmi, const char* namestrings,
                        const char* nameguids, int cpos)
{
	size_t count=0;
	pmi->Strings = CreateStringsArray(L, cpos, namestrings, &pmi->Count);
	lua_getfield(L, -1, nameguids);

	if(lua_type(L, -1) == LUA_TSTRING)
	{
		pmi->Guids = (GUID*)lua_tostring(L,-1);
		count = lua_objlen(L, -1) / sizeof(GUID);
		lua_rawseti(L, cpos, (int)lua_objlen(L, cpos) + 1);
	}
	else
		lua_pop(L, 1);

	if(pmi->Count > count)
		pmi->Count = count;
}

void LF_GetPluginInfo(lua_State* L, struct PluginInfo *PI)
{
	int cpos;

	if(!GetExportFunction(L, "GetPluginInfo"))     //+1
		return;

	if(pcall_msg(L, 0, 1) != 0)
		return;

	if(!lua_istable(L, -1))
	{
		lua_pop(L,1);
		return;
	}

	ReplacePluginInfoCollector(L);                     //+2: Info,Coll
	cpos = lua_gettop(L);  // collector position
	lua_pushvalue(L, -2);                              //+3: Info,Coll,Info
	//--------------------------------------------------------------------------
	PI->StructSize = sizeof(*PI);
	PI->Flags = GetFlagsFromTable(L, -1, "Flags");
	PI->CommandPrefix = AddStringToCollectorField(L, cpos, "CommandPrefix");
	//--------------------------------------------------------------------------
	getPluginMenuItems(L, &PI->DiskMenu, "DiskMenuStrings", "DiskMenuGuids", cpos);
	getPluginMenuItems(L, &PI->PluginMenu, "PluginMenuStrings", "PluginMenuGuids", cpos);
	getPluginMenuItems(L, &PI->PluginConfig, "PluginConfigStrings", "PluginConfigGuids", cpos);
	//--------------------------------------------------------------------------
	lua_pop(L, 3);
}

intptr_t LF_ProcessEditorInput(lua_State* L, const struct ProcessEditorInputInfo *Info)
{
	if(!GetExportFunction(L, "ProcessEditorInput"))    //+1: Func
		return 0;

	pushInputRecord(L, &Info->Rec);

	if(pcall_msg(L, 1, 1) == 0)        //+1: Res
	{
		intptr_t ret = lua_toboolean(L,-1);
		return lua_pop(L,1), ret;
	}

	return 0;
}

intptr_t LF_ProcessEditorEvent(lua_State* L, const struct ProcessEditorEventInfo *Info)
{
	intptr_t ret = 0;

	if(GetExportFunction(L, "ProcessEditorEvent"))     //+1: Func
	{
		lua_pushinteger(L, Info->EditorID); //+2;
		lua_pushinteger(L, Info->Event);    //+3;

		switch(Info->Event)
		{
			case EE_REDRAW:
				lua_pushinteger(L, (intptr_t)Info->Param); break;
			case EE_CHANGE:
			{
				const struct EditorChange *ec = (const struct EditorChange*) Info->Param;
				lua_createtable(L, 0, 2);
				PutNumToTable(L, "Type", ec->Type);
				PutNumToTable(L, "StringNumber", (double)ec->StringNumber);
				break;
			}
			default:
				lua_pushnil(L); break;
		}

		if(pcall_msg(L, 3, 1) == 0)       //+1
		{
			if(lua_isnumber(L,-1)) ret = lua_tointeger(L,-1);

			lua_pop(L,1);
		}
	}

	return ret;
}

intptr_t LF_ProcessViewerEvent(lua_State* L, const struct ProcessViewerEventInfo *Info)
{
	intptr_t ret = 0;

	if(GetExportFunction(L, "ProcessViewerEvent"))     //+1: Func
	{
		lua_pushinteger(L, Info->ViewerID);
		lua_pushinteger(L, Info->Event);
		lua_pushnil(L);

		if(pcall_msg(L, 3, 1) == 0)         //+1
		{
			if(lua_isnumber(L,-1)) ret = lua_tointeger(L,-1);

			lua_pop(L,1);
		}
	}

	return ret;
}

intptr_t LF_ProcessDialogEvent(lua_State* L, const struct ProcessDialogEventInfo *Info)
{
	intptr_t ret = 0;

	if(GetExportFunction(L, "ProcessDialogEvent"))     //+1: Func
	{
		struct FarDialogEvent *fde = Info->Param;
		lua_pushinteger(L, Info->Event); //+2
		lua_createtable(L, 0, 5);        //+3
		NewDialogData(L, NULL, fde->hDlg, FALSE);
		lua_setfield(L, -2, "hDlg"); //+3
		PutIntToTable(L, "Msg", fde->Msg);
		PutIntToTable(L, "Param1", fde->Param1);
		PutIntToTable(L, "Param2", (intptr_t)fde->Param2);
		PutIntToTable(L, "Result", fde->Result);

		if(pcall_msg(L, 2, 2) == 0)     //+2
		{
			ret = lua_isnumber(L,-2) ? lua_tointeger(L,-2) : lua_toboolean(L,-2);

			if(ret != 0)
				fde->Result = lua_tointeger(L,-1);

			lua_pop(L,2);
		}
	}

	return ret;
}

intptr_t LF_ProcessSynchroEvent(lua_State* L, const struct ProcessSynchroEventInfo *Info)
{
	intptr_t ret = 0;

	if(Info->Event == SE_COMMONSYNCHRO)
	{
		TSynchroData sd = *(TSynchroData*)Info->Param; // copy
		free(Info->Param);

		if(sd.regAction != 0)
		{
			if(sd.regAction & LUAFAR_TIMER_CALL)
			{
				lua_rawgeti(L, LUA_REGISTRYINDEX, sd.funcRef); //+1: Func

				if(lua_type(L, -1) == LUA_TFUNCTION)
				{
					lua_rawgeti(L, LUA_REGISTRYINDEX, sd.objRef); //+2: Obj

					if(pcall_msg(L, 1, 1) == 0)     //+1
					{
						if(lua_isnumber(L,-1)) ret = lua_tointeger(L,-1);

						lua_pop(L,1);
					}
				}
				else lua_pop(L, 1);
			}

			if(sd.regAction & LUAFAR_TIMER_UNREF)
			{
				luaL_unref(L, LUA_REGISTRYINDEX, sd.objRef);
				luaL_unref(L, LUA_REGISTRYINDEX, sd.funcRef);
			}
		}
		else if(GetExportFunction(L, "ProcessSynchroEvent"))     //+1: Func
		{
			lua_pushinteger(L, Info->Event); //+2
			lua_pushinteger(L, sd.data);     //+3

			if(pcall_msg(L, 2, 1) == 0)      //+1
			{
				if(lua_isnumber(L,-1)) ret = lua_tointeger(L,-1);

				lua_pop(L,1);
			}
		}
	}

	return ret;
}

intptr_t LF_GetCustomData(lua_State* L, const wchar_t *FilePath, wchar_t **CustomData)
{
	if(GetExportFunction(L, "GetCustomData"))     //+1: Func
	{
		push_utf8_string(L, FilePath, -1);  //+2

		if(pcall_msg(L, 1, 1) == 0)     //+1
		{
			if(lua_isstring(L, -1))
			{
				const wchar_t* p = utf8_to_utf16(L, -1, NULL);

				if(p)
				{
					*CustomData = _wcsdup(p);
					lua_pop(L, 1);
					return TRUE;
				}
			}

			lua_pop(L, 1);
		}
	}

	return FALSE;
}

void LF_FreeCustomData(lua_State* L, wchar_t *CustomData)
{
	(void) L;

	if(CustomData) free(CustomData);
}

int LF_GetGlobalInfo(lua_State* L, struct GlobalInfo *Info, const wchar_t *PluginDir)
{
	int cpos = lua_gettop(L);
	const char *name = "<_globalinfo";
	lua_getglobal(L, "export");

	if(!lua_istable(L, -1))
	{
		lua_createtable(L, 0, 10);
		lua_setglobal(L, "export");
	}

	lua_getglobal(L, "package");
	lua_getfield(L, -1, "preload");
	lua_getfield(L, -1, name);

	if(lua_isfunction(L, -1))
	{
		lua_pushstring(L, name);

		if(lua_pcall(L, 1, 1, 0))
		{
			lua_settop(L, cpos);
			return FALSE;
		}
	}
	else
	{
		wchar_t buf[512];
		wcscpy(buf, PluginDir);
		wcscat(buf, L"_globalinfo.lua");

		if(LF_LoadFile(L, buf))
		{
			lua_settop(L, cpos);
			return FALSE;
		}
	}

	//--------------------------------------------------------------------------
	if(lua_pcall(L, 0, 0, 0))
	{
		lua_settop(L, cpos);
		return FALSE;
	}

	lua_settop(L, cpos);

	if(!GetExportFunction(L, "GetGlobalInfo"))
		return FALSE;

	if(lua_pcall(L, 0, 1, 0) || !lua_istable(L, -1))    //+1
	{
		lua_pop(L,1);
		return FALSE;
	}

	//--------------------------------------------------------------------------
	ReplacePluginInfoCollector(L);                     //+2: Info,Coll
	cpos = lua_gettop(L);  // collector position
	lua_pushvalue(L, -2);                              //+3: Info,Coll,Info
	//--------------------------------------------------------------------------
	Info->StructSize = sizeof(*Info);
	//--------------------------------------------------------------------------
	lua_getfield(L, -1, "MinFarVersion");

	if(lua_istable(L, -1))
		Info->MinFarVersion = MAKEFARVERSION(GetOptIntFromArray(L,1,0),GetOptIntFromArray(L,2,0),
		                                     GetOptIntFromArray(L,3,0),GetOptIntFromArray(L,4,0),GetOptIntFromArray(L,5,0));

	lua_pop(L, 1);
	lua_getfield(L, -1, "Version");

	if(lua_istable(L, -1))
		Info->Version = MAKEFARVERSION(GetOptIntFromArray(L,1,0),GetOptIntFromArray(L,2,0),
		                               GetOptIntFromArray(L,3,0),GetOptIntFromArray(L,4,0),GetOptIntFromArray(L,5,0));

	lua_pop(L, 1);
	//--------------------------------------------------------------------------
	lua_getfield(L, -1, "Guid");

	if(lua_type(L,-1) == LUA_TSTRING) Info->Guid = *(GUID*)lua_tostring(L,-1);

	lua_pop(L, 1);
	//--------------------------------------------------------------------------
	Info->Title = AddStringToCollectorField(L, cpos, "Title");
	Info->Description = AddStringToCollectorField(L, cpos, "Description");
	Info->Author = AddStringToCollectorField(L, cpos, "Author");
	//--------------------------------------------------------------------------
	lua_pop(L, 3);
	return TRUE;
}

intptr_t LF_ProcessConsoleInput(lua_State* L, struct ProcessConsoleInputInfo *Info)
{
	intptr_t ret = 0;

	if(GetExportFunction(L, "ProcessConsoleInput"))    //+1: Func
	{
		pushInputRecord(L, &Info->Rec);                  //+2
		bit64_push(L, Info->Flags);                      //+3

		if(pcall_msg(L, 2, 1) == 0)                      //+1: Res
		{
			if(lua_istable(L,-1))
			{
				FillInputRecord(L, -1, &Info->Rec);
				ret = 2;
			}
			else
			{
				ret = lua_tointeger(L,-1);
			}

			lua_pop(L,1);
		}
	}

	return ret;
}
