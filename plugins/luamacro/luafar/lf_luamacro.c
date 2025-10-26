//---------------------------------------------------------------------------
#include <stdlib.h>

#include "lf_luafar.h"
#include "lf_string.h"
#include "lf_util.h"
#include "lf_bit64.h"
#include "lf_service.h"

extern int pcall_msg(lua_State* L, int narg, int nret);
extern void PushFarMacroValue(lua_State* L, const struct FarMacroValue* val);

static const char LuamacroGuid[16]= {200,239,187,78,132,32,127,75,148,192,105,44,225,54,137,77};

static int FL_PushParams(lua_State* L, const struct FarMacroCall* Data)
{
	int ret = lua_checkstack(L, 2 + (int)Data->Count);
	if (ret)
	{
		for(size_t i=0; i < Data->Count; i++)
			PushFarMacroValue(L, Data->Values + i);
	}
	if (Data->Callback)
	{
		Data->Callback(Data->CallbackData, Data->Values, Data->Count);
	}
	return ret;
}

static void InitMPR (lua_State* L, struct MacroPluginReturn *mpr, size_t nargs, intptr_t ReturnType)
{
	if (nargs)
	{
		mpr->Values = (struct FarMacroValue*)lua_newuserdata(L, nargs*sizeof(struct FarMacroValue));
		lua_setfield(L, -2, "MacroPluginReturn");
	}
	mpr->Count = nargs;
	mpr->ReturnType = ReturnType;
}

HANDLE Open_Luamacro(lua_State* L, const struct OpenInfo *Info)
{
	struct OpenMacroPluginInfo* om_info = (struct OpenMacroPluginInfo*)Info->Data;
	int calltype = om_info->CallType;
	size_t argc = om_info->Data ? om_info->Data->Count : 0; // store Data->Count: 'Data' will be invalid after FL_PushParams()

	if (!IsEqualGUID(GetPluginData(L)->PluginId, LuamacroGuid))
	{
		lua_pop(L, 1);
		return NULL;
	}

	lua_pushinteger(L, Info->OpenFrom);            //+2
	lua_pushinteger(L, calltype);                  //+3
	if (om_info->Data && !FL_PushParams(L, om_info->Data))
	{
		lua_pop(L, 3);
		LF_Message(L, L"too many values to place onto Lua stack", L"LuaMacro", L"OK", "wl", NULL, NULL);
		return NULL;
	}

	if (pcall_msg(L, 2+(int)argc, 2) == 0)
	{
		if (!lua_toboolean(L,-2))
		{
			lua_pop(L,2);
			return NULL;
		}

		intptr_t ReturnType = lua_type(L,-2)==LUA_TNUMBER ? lua_tointeger(L,-2) : 1;

		if (lua_istable(L,-2))
		{
			lua_pop(L,1);
			lua_pushvalue(L,-1);
		}
		if (!lua_istable(L,-1))
		{
			InitMPR(L, &om_info->Ret, 0, ReturnType);
			lua_pop(L,2);
			return (HANDLE)1;
		}
		else
		{
			struct MacroPluginReturn* Ret = &om_info->Ret;

			lua_getfield(L,-1,"n");
			int nargs = lua_type(L,-1)==LUA_TNUMBER ? (int)lua_tointeger(L,-1) : (int)lua_objlen(L,-2);
			lua_pop(L,1);
			if (nargs < 0)
				nargs = 0;

			InitMPR(L, Ret, (size_t)nargs, ReturnType);

			for(int idx=0; idx<nargs; idx++)
			{
				Ret->Values[idx].Type = FMVT_NIL;
				lua_rawgeti(L,-1,idx+1);

				switch (lua_type(L, -1))
				{
					case LUA_TNUMBER:
						Ret->Values[idx].Type = FMVT_DOUBLE;
						Ret->Values[idx].Value.Double = lua_tonumber(L, -1);
						lua_pop(L,1);
						break;

					case LUA_TSTRING:
						Ret->Values[idx].Type = FMVT_STRING;
						Ret->Values[idx].Value.String = check_utf8_string(L, -1, NULL);
						lua_rawseti(L,-2,idx+1);
						break;

					case LUA_TBOOLEAN:
						Ret->Values[idx].Type = FMVT_BOOLEAN;
						Ret->Values[idx].Value.Boolean = lua_toboolean(L, -1);
						lua_pop(L,1);
						break;

					case LUA_TLIGHTUSERDATA:
						Ret->Values[idx].Type = FMVT_POINTER;
						Ret->Values[idx].Value.Pointer = lua_touserdata(L, -1);
						lua_rawseti(L,-2,idx+1);
						break;

					case LUA_TTABLE:
						lua_getfield(L, -1, TKEY_BINARY);
						if (lua_type(L, -1) == LUA_TSTRING)
						{
							Ret->Values[idx].Type = FMVT_BINARY;
							Ret->Values[idx].Value.Binary.Data = (char*)lua_tostring(L,-1);
							Ret->Values[idx].Value.Binary.Size = lua_objlen(L,-1);
							lua_rawseti(L,-3,idx+1);
						}
						lua_pop(L,1);
						break;

					default:
						INT64 val64;
						if (bit64_getvalue(L, -1, &val64))
						{
							Ret->Values[idx].Type = FMVT_INTEGER;
							Ret->Values[idx].Value.Integer = val64;
							lua_pop(L,1);
						}
						else
						{
							Ret->Values[idx].Type = FMVT_NIL;
							lua_pop(L,1);
						}
						break;
				}
			}

			lua_pop(L,2);
			return (HANDLE)1;
		}
	}

	return NULL;
}

typedef struct
{
	lua_State *L;
	int start_stack;
	int max_stack;
	int error;
} mcfc_data;

static void WINAPI MacroCallFarCallback(void *Data, struct FarMacroValue *Val, size_t Count)
{
	mcfc_data *cbdata = (mcfc_data*)Data;
	if (cbdata->error)
		return;

	(void) Count;
	lua_State *L = cbdata->L;
	int top = lua_gettop(L);
	int stack_avail = cbdata->max_stack - top;
	int param = (int)Val->Value.Integer;
	int pos = param >= 0 ? param : top + 1 + param;

	switch(Val->Type)
	{
		case FMVT_NEWTABLE:
			if (stack_avail > 0)
				lua_newtable(L);
			break;

		case FMVT_SETTABLE:
			if (pos >= 1 && pos <= top-2 && lua_istable(L, pos))
				lua_settable(L, pos);
			break;

		case FMVT_GETTABLE:
			if (pos >= 1 && pos <= top-1 && lua_istable(L, pos))
			{
				lua_gettable(L, pos);
				ConvertLuaValue(L, -1, Val);
			}
			break;

		case FMVT_STACKPOP:
			if (param > 0 && (top - param) >= cbdata->start_stack)
				lua_pop(L, param);
			break;

		case FMVT_STACKGETTOP:
			Val->Type = FMVT_INTEGER;
			Val->Value.Integer = top;
			break;

		case FMVT_STACKSETTOP:
			if (pos >= cbdata->start_stack && pos <= cbdata->max_stack)
				lua_settop(L, pos);
			break;

		case FMVT_STACKPUSHVALUE:
			if (stack_avail > 0 && pos >= 1 && pos <= top)
				lua_pushvalue(L, pos);
			break;

		default:
			if (stack_avail > 0)
			{
				cbdata->error = (Val->Type == FMVT_ERROR);
				PushFarMacroValue(L, Val);
			}
			break;
	}
}

int far_MacroCallFar(lua_State *L)
{
	enum { MAXARG=32, MAXRET=32 };
	struct FarMacroValue args[MAXARG];
	struct FarMacroCall fmc;
	int top = lua_gettop(L);
	mcfc_data cbdata = { L, top, top + MAXRET, 0 };
	TPluginData *pd = GetPluginData(L);
	struct MacroPrivateInfo *privateInfo = (struct MacroPrivateInfo*)pd->Info->Private;

	int opcode = (int)luaL_checkinteger(L, 1);
	fmc.Count = top - 1;
	fmc.Values = fmc.Count<=MAXARG ? args:(struct FarMacroValue*)malloc(fmc.Count*sizeof(struct FarMacroValue));
	fmc.Callback = MacroCallFarCallback;
	fmc.CallbackData = &cbdata;

	for(int idx=0; idx<(int)fmc.Count; idx++)
	{
		ConvertLuaValue(L, idx+2, fmc.Values+idx);
		if (fmc.Values[idx].Type == FMVT_UNKNOWN)
		{
			lua_Debug ar;
			if (fmc.Values != args)
				free(fmc.Values);
			if (lua_getstack(L,1,&ar) && lua_getinfo(L,"n",&ar) && ar.name)
				luaL_error(L, "invalid argument #%d to '%s'", idx+1, ar.name);
			else
				luaL_argerror(L, idx+1, "invalid argument");
		}
	}

	lua_checkstack(L, MAXRET);
	int ret = (int) privateInfo->CallFar(opcode, &fmc);
	FP_PROTECT(); // protect from plugins activating FPU exceptions
	int pushed = lua_gettop(L) - top;
	if (fmc.Values != args)
		free(fmc.Values);
	if (cbdata.error)
		return luaL_error(L, lua_tostring(L, -1));
	return pushed ? pushed : (lua_pushnumber(L, ret), 1);
}

int far_MacroCallToLua(lua_State *L)
{
	if (lua_type(L,1) == LUA_TLIGHTUSERDATA)
	{
		const struct FarMacroCall* Data = (struct FarMacroCall*)lua_touserdata(L, 1);
		lua_settop(L, 0);
		if (Data && !FL_PushParams(L, Data))
		{
			LF_Message(L, L"too many values to place onto Lua stack", L"LuaMacro", L"OK", "wl", NULL, NULL);
		}
		return lua_gettop(L);
	}
	return 0;
}
