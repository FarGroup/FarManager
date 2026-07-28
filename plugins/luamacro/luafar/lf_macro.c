#include <windows.h>
#include <stdlib.h>

#include "lf_bit64.h"
#include "lf_flags.h"
#include "lf_luafar.h"
#include "lf_service.h"
#include "lf_string.h"
#include "lf_util.h"

const char AddMacroDataType[] = "FarAddMacroData";

void ConvertLuaValue(lua_State *L, int pos, struct FarMacroValue *target)
{
	INT64 val64;
	HANDLE val_handle;
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
		lua_getfield(L, pos, TKEY_BINARY);
		if (lua_type(L, -1) == LUA_TSTRING)
		{
			target->Type = FMVT_BINARY;
			target->Value.Binary.Data = (void*)lua_tolstring(L, -1, &target->Value.Binary.Size);
		}
		else
		{
			target->Type = FMVT_TABLE;
			target->Value.Integer = pos;
		}
		lua_pop(L, 1);
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
	else if (Dialog_getvalue(L, pos, &val_handle))
	{
		target->Type = FMVT_DIALOG;
		target->Value.Pointer = val_handle;
	}
}

void PackMacroValues(lua_State* L, size_t Count, const struct FarMacroValue* Values)
{
	lua_createtable(L, (int)Count, 1);
	for(size_t i=0; i < Count; i++)
	{
		PushFarMacroValue(L, Values + i);
		lua_rawseti(L, -2, (int)i+1);
	}
	lua_pushinteger(L, Count);
	lua_setfield(L, -2, "n");
}

static int far_MacroLoadAll(lua_State* L)
{
	TPluginData *pd = GetPluginData(L);
	struct FarMacroLoad Data = { sizeof(Data) };
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
	struct MacroSendMacroText smt = { sizeof(smt) };
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
	struct MacroAddMacro data = { sizeof(data) };
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
	int result = FALSE;

	MacroAddData *Id = (MacroAddData*)luaL_checkudata(L, 1, AddMacroDataType);
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

	struct MacroExecuteString Data = { sizeof(Data) };
	Data.SequenceText = check_utf8_string(L, 1, NULL);
	Data.Flags = OptFlags(L,2,0);
	Data.InCount = 0;

	if (top > 2)
	{
		Data.InCount = top-2;
		Data.InValues = (struct FarMacroValue*)lua_newuserdata(L, Data.InCount*sizeof(struct FarMacroValue));
		memset(Data.InValues, 0, Data.InCount*sizeof(struct FarMacroValue));
		for (size_t i=0; i<Data.InCount; i++)
			ConvertLuaValue(L, (int)i+3, Data.InValues+i);
	}

	if (pd->Info->MacroControl(pd->PluginId, MCTL_EXECSTRING, 0, &Data))
		PackMacroValues(L, Data.OutCount, Data.OutValues);
	else
		lua_pushnil(L);

	return 1;
}

static const luaL_Reg far_funcs[] =
{
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

	{NULL, NULL},
};

int luaopen_macro(lua_State *L)
{
	luaL_register(L, "far", far_funcs);

	luaL_newmetatable(L, AddMacroDataType);
	lua_pushcfunction(L, AddMacroData_gc);
	lua_setfield(L, -2, "__gc");

	return 0;
}
