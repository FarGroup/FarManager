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
	int ret;
	if (Data == NULL) return 1;

	if ((ret = lua_checkstack(L, (int)Data->Count)) != 0)
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

static struct MacroPluginReturn* CreateMPR(lua_State* L, int nargs, int ReturnType)
{
	size_t size = sizeof(struct MacroPluginReturn) + nargs*sizeof(struct FarMacroValue);
	struct MacroPluginReturn* mpr = (struct MacroPluginReturn*)lua_newuserdata(L, size);
	lua_setfield(L, -2, "MacroPluginReturn");
	memset(mpr, 0, size);
	mpr->Values = (struct FarMacroValue*)(mpr+1);
	mpr->Count = nargs;
	mpr->ReturnType = ReturnType;
	return mpr;
}

HANDLE Open_Luamacro(lua_State* L, const struct OpenInfo *Info)
{
	const struct OpenMacroPluginInfo* om_info = (const struct OpenMacroPluginInfo*)Info->Data;
	int calltype = om_info->CallType;
	size_t argc = om_info->Data ? om_info->Data->Count : 0; // store Data->Count: 'Data' will be invalid after FL_PushParams()

	if (!IsEqualGUID(GetPluginData(L)->PluginId, LuamacroGuid))
	{
		lua_pop(L, 1);
		return NULL;
	}

	lua_pushinteger(L, Info->OpenFrom);            //+2
	lua_pushinteger(L, calltype);                  //+3
	lua_pushinteger(L, (intptr_t)om_info->Handle); //+4
	if (!FL_PushParams(L, om_info->Data))
	{
		lua_pop(L, 4);
		return NULL;
	}

	if(pcall_msg(L, 3+(int)argc, 2) == 0)
	{
		int t1=lua_type(L,-2), t2=lua_type(L,-1);
		if (t1 != LUA_TNUMBER || (t2 != LUA_TNIL && t2 != LUA_TTABLE))
		{
			lua_pop(L,2);
			return NULL;
		}
		if(t2 == LUA_TNIL)
		{
			intptr_t ret = lua_tointeger(L,-2);
			lua_pop(L,2);
			return (HANDLE)ret;
		}
		else
		{
			struct MacroPluginReturn* mpr;
			int ReturnType = (int)lua_tointeger(L,-2);
			int nargs, idx;

			lua_getfield(L,-1,"n");
			nargs = lua_type(L,-1)==LUA_TNUMBER ? (int)lua_tointeger(L,-1) : (int)lua_objlen(L,-2);
			lua_pop(L,1);
			mpr = CreateMPR(L,nargs,ReturnType);

			for(idx=0; idx<nargs; idx++)
			{
				int type;
				INT64 val64;
				lua_rawgeti(L,-1,idx+1);
				type = lua_type(L, -1);

				if(type == LUA_TNUMBER)
				{
					mpr->Values[idx].Type = FMVT_DOUBLE;
					mpr->Values[idx].Value.Double = lua_tonumber(L, -1);
					lua_pop(L,1);
				}
				else if(type == LUA_TSTRING)
				{
					mpr->Values[idx].Type = FMVT_STRING;
					mpr->Values[idx].Value.String = check_utf8_string(L, -1, NULL);
					lua_rawseti(L,-2,idx+1);
				}
				else if(type == LUA_TBOOLEAN)
				{
					mpr->Values[idx].Type = FMVT_BOOLEAN;
					mpr->Values[idx].Value.Boolean = lua_toboolean(L, -1);
					lua_pop(L,1);
				}
				else if(type == LUA_TLIGHTUSERDATA)
				{
					mpr->Values[idx].Type = FMVT_POINTER;
					mpr->Values[idx].Value.Pointer = lua_touserdata(L, -1);
					lua_rawseti(L,-2,idx+1);
				}
				else if(type == LUA_TTABLE)
				{
					mpr->Values[idx].Type = FMVT_BINARY;
					lua_rawgeti(L,-1,1);
					if (lua_type(L,-1) == LUA_TSTRING)
					{
						mpr->Values[idx].Value.Binary.Data = (char*)lua_tostring(L,-1);
						mpr->Values[idx].Value.Binary.Size = lua_objlen(L,-1);
						lua_rawseti(L,-3,idx+1);
					}
					else
					{
						mpr->Values[idx].Value.Binary.Data = (char*)"";
						mpr->Values[idx].Value.Binary.Size = 0;
						lua_pop(L,1);
					}
					lua_pop(L,1);
				}
				else if(bit64_getvalue(L, -1, &val64))
				{
					mpr->Values[idx].Type = FMVT_INTEGER;
					mpr->Values[idx].Value.Integer = val64;
					lua_pop(L,1);
				}
				else
				{
					mpr->Values[idx].Type = FMVT_NIL;
					lua_pop(L,1);
				}
			}

			lua_pop(L,2);
			return (HANDLE)mpr;
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
