//---------------------------------------------------------------------------
#include "luafar.h"
#include "ustring.h"
#include "compat52.h"

extern int bit64_getvalue(lua_State *L, int pos, INT64 *target);
extern int pcall_msg(lua_State* L, int narg, int nret);
extern void PushFarMacroValue(lua_State* L, const struct FarMacroValue* val);
extern void ConvertLuaValue (lua_State *L, int pos, struct FarMacroValue *target);

static const char LuamacroGuid[16]= {200,239,187,78,132,32,127,75,148,192,105,44,225,54,137,77};

static int FL_PushParams(lua_State* L, const struct FarMacroCall* Data)
{
	int ret = lua_checkstack(L, (int)Data->Count);
	if (ret)
	{
		size_t i;
		for(i=0; i < Data->Count; i++)
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
	lua_pushinteger(L, om_info->Handle);           //+4
	if (om_info->Data && !FL_PushParams(L, om_info->Data))
	{
		lua_pop(L, 4);
		return NULL;
	}

	if(pcall_msg(L, 3+(int)argc, 2) == 0)
	{
		intptr_t ReturnType;
		if (!lua_toboolean(L,-2))
		{
			lua_pop(L,2);
			return NULL;
		}
		ReturnType = lua_type(L,-2)==LUA_TNUMBER ? lua_tointeger(L,-2) : 1;

		if(lua_type(L,-1) != LUA_TTABLE)
		{
			InitMPR(L, &om_info->Ret, 0, ReturnType);
			lua_pop(L,2);
			return (HANDLE)1;
		}
		else
		{
			struct MacroPluginReturn* Ret = &om_info->Ret;
			int nargs, idx;

			lua_getfield(L,-1,"n");
			nargs = lua_type(L,-1)==LUA_TNUMBER ? (int)lua_tointeger(L,-1) : (int)lua_objlen(L,-2);
			lua_pop(L,1);
			if (nargs < 0) nargs = 0;

			InitMPR(L, Ret, (size_t)nargs, ReturnType);

			for(idx=0; idx<nargs; idx++)
			{
				int type;
				INT64 val64;
				lua_rawgeti(L,-1,idx+1);
				type = lua_type(L, -1);

				if(type == LUA_TNUMBER)
				{
					Ret->Values[idx].Type = FMVT_DOUBLE;
					Ret->Values[idx].Value.Double = lua_tonumber(L, -1);
					lua_pop(L,1);
				}
				else if(type == LUA_TSTRING)
				{
					Ret->Values[idx].Type = FMVT_STRING;
					Ret->Values[idx].Value.String = check_utf8_string(L, -1, NULL);
					lua_rawseti(L,-2,idx+1);
				}
				else if(type == LUA_TBOOLEAN)
				{
					Ret->Values[idx].Type = FMVT_BOOLEAN;
					Ret->Values[idx].Value.Boolean = lua_toboolean(L, -1);
					lua_pop(L,1);
				}
				else if(type == LUA_TLIGHTUSERDATA)
				{
					Ret->Values[idx].Type = FMVT_POINTER;
					Ret->Values[idx].Value.Pointer = lua_touserdata(L, -1);
					lua_rawseti(L,-2,idx+1);
				}
				else if(type == LUA_TTABLE)
				{
					Ret->Values[idx].Type = FMVT_BINARY;
					lua_rawgeti(L,-1,1);
					if (lua_type(L,-1) == LUA_TSTRING)
					{
						Ret->Values[idx].Value.Binary.Data = (char*)lua_tostring(L,-1);
						Ret->Values[idx].Value.Binary.Size = lua_objlen(L,-1);
						lua_rawseti(L,-3,idx+1);
					}
					else
					{
						Ret->Values[idx].Value.Binary.Data = (char*)"";
						Ret->Values[idx].Value.Binary.Size = 0;
						lua_pop(L,1);
					}
					lua_pop(L,1);
				}
				else if(bit64_getvalue(L, -1, &val64))
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
			}

			lua_pop(L,2);
			return (HANDLE)1;
		}

		lua_pop(L,2);
	}

	return NULL;
}

typedef struct
{
	lua_State *L;
	int ret_avail;
} mcfc_data;

static void WINAPI MacroCallFarCallback(void *Data, struct FarMacroValue *Val, size_t Count)
{
	mcfc_data *cbdata = (mcfc_data*)Data;
	(void) Count;
	if(cbdata->ret_avail > 0)
	{
		--cbdata->ret_avail;
		PushFarMacroValue(cbdata->L, Val);
	}
}

int far_MacroCallFar(lua_State *L)
{
	enum { MAXARG=32, MAXRET=32 };
	struct FarMacroValue args[MAXARG];
	struct FarMacroCall fmc;
	int idx, ret, pushed;
	mcfc_data cbdata = { L, MAXRET };
	TPluginData *pd = GetPluginData(L);
	struct MacroPrivateInfo *privateInfo = (struct MacroPrivateInfo*)pd->Info->Private;
	int opcode = (int)luaL_checkinteger(L, 1);
	fmc.Values = args;
	fmc.Count = lua_gettop(L) - 1;
	fmc.Callback = MacroCallFarCallback;
	fmc.CallbackData = &cbdata;
	luaL_argcheck(L, fmc.Count<=MAXARG, MAXARG+2, "too many arguments");

	for(idx=0; idx<(int)fmc.Count; idx++)
	{
		ConvertLuaValue(L, idx+2, args+idx);
		if (args[idx].Type == FMVT_UNKNOWN)
		{
			lua_Debug ar;
			if (lua_getstack(L,1,&ar) && lua_getinfo(L,"n",&ar) && ar.name)
				luaL_error(L, "invalid argument #%d to '%s'", idx+1, ar.name);
			else
				luaL_argerror(L, idx+1, "invalid argument");
		}
	}

	lua_checkstack(L, MAXRET);
	ret = (int) privateInfo->CallFar(opcode, &fmc);
	pushed = MAXRET - cbdata.ret_avail;
	return pushed ? pushed : (lua_pushnumber(L, ret), 1);
}

int far_MacroCallPlugin(lua_State *L)
{
	int idx;
	struct MacroPrivateInfo *privateInfo = (struct MacroPrivateInfo*)GetPluginData(L)->Info->Private;
	struct MacroPluginReturn Params = {0,0,NULL};
	struct FarMacroCall Ret;
	size_t nargs = lua_gettop(L);

	lua_createtable(L,0,1);
	InitMPR(L, &Params, nargs, 0);
	for(idx=0; idx<nargs; idx++)
	{
		ConvertLuaValue(L, idx+1, idx+Params.Values);
	}
	privateInfo->CallPlugin(&Params, &Ret);
	return FL_PushParams(L, &Ret) ? Ret.Count : 0;
}
