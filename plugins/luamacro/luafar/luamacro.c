//---------------------------------------------------------------------------
#include "luafar.h"
#include "ustring.h"

extern int bit64_getvalue(lua_State *L, int pos, INT64 *target);
extern int pcall_msg(lua_State* L, int narg, int nret);
extern void PushFarMacroValue(lua_State* L, const struct FarMacroValue* val);

static const char LuamacroGuid[16]= {200,239,187,78,132,32,127,75,148,192,105,44,225,54,137,77};

static void FL_PushParamsTable(lua_State* L, const struct FarMacroCall* Data)
{
	size_t i;
	lua_createtable(L, (int)Data->Count, 0);
	for(i=0; i < Data->Count; i++)
	{
		PushFarMacroValue(L, Data->Values + i);
		lua_rawseti(L, -2, (int)i+1);
	}
	if (Data->Callback)
		Data->Callback(Data->CallbackData, Data->Values, Data->Count);
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

	if (!IsEqualGUID(GetPluginData(L)->PluginId, LuamacroGuid))
		return NULL;

	lua_pushinteger(L, Info->OpenFrom);
	lua_pushinteger(L, calltype);
	lua_pushinteger(L, (intptr_t)om_info->Handle);
	FL_PushParamsTable(L, om_info->Data);

	if(pcall_msg(L, 4, 2) == 0)
	{
		if(calltype == MCT_MACROINIT || calltype == MCT_MACROFINAL)
		{
			if(lua_type(L,-2) == LUA_TNUMBER)
			{
				intptr_t ret = lua_tointeger(L,-2);
				lua_pop(L,2);
				return (HANDLE)ret;
			}
		}
		else
		{
			struct MacroPluginReturn* mpr;
			int ReturnType;

			if(lua_type(L,-2) != LUA_TNUMBER || lua_type(L,-1) != LUA_TTABLE)
			{
				lua_pop(L,2); return NULL;
			}

			switch((ReturnType=(int)lua_tointeger(L,-2)))
			{
				case MPRT_NORMALFINISH:
				{
					mpr = CreateMPR(L,0,ReturnType);
					lua_pop(L,2);
					return (HANDLE)mpr;
				}

				case MPRT_ERRORFINISH:
				case MPRT_KEYS:
				case MPRT_PRINT:
				{
					lua_rawgeti(L,-1,1);

					if(lua_type(L,-1) == LUA_TSTRING)
					{
						wchar_t *s = check_utf8_string(L,-1,NULL);
						lua_rawseti(L,-2,1);
						mpr = CreateMPR(L,1,ReturnType);
						mpr->Values[0].Type = FMVT_STRING;
						mpr->Values[0].Value.String = s;
						lua_pop(L,2);
						return (HANDLE)mpr;
					}
					else
					{
						lua_pop(L,3); return NULL;
					}
				}

				default:
				{
					int nargs, type, idx;
					INT64 val64;
					lua_getfield(L,-1,"n");
					nargs=(int)lua_tointeger(L,-1);

					lua_pop(L,1);
					mpr = CreateMPR(L,nargs,ReturnType);

					for(idx=0; idx<nargs; idx++)
					{
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
						else if(type == LUA_TBOOLEAN || type == LUA_TNIL)
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
							mpr->Values[idx].Type = FMVT_BOOLEAN;
							mpr->Values[idx].Value.Boolean = 0;
							lua_pop(L,1);
						}
					}

					lua_pop(L,2);
					return (HANDLE)mpr;
				}
			}
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
	int idx, ret, pushed, stackpos=0, success=1;
	INT64 val64;
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
		int type;
		stackpos = idx + 2;
		type = lua_type(L, stackpos);

		if(type == LUA_TNUMBER)
		{
			args[idx].Type = FMVT_DOUBLE;
			args[idx].Value.Double = lua_tonumber(L, stackpos);
		}
		else if(type == LUA_TSTRING)
		{
			args[idx].Type = FMVT_STRING;
			args[idx].Value.String = check_utf8_string(L, stackpos, NULL);
		}
		else if(type == LUA_TBOOLEAN || type == LUA_TNIL)
		{
			args[idx].Type = FMVT_BOOLEAN;
			args[idx].Value.Boolean = lua_toboolean(L, stackpos);
		}
		else if(type == LUA_TLIGHTUSERDATA)
		{
			args[idx].Type = FMVT_POINTER;
			args[idx].Value.Pointer = lua_touserdata(L, stackpos);
		}
		else if(bit64_getvalue(L, stackpos, &val64))
		{
			args[idx].Type = FMVT_INTEGER;
			args[idx].Value.Integer = val64;
		}
		else
		{
			success = 0;
			break;
		}
	}

	if(!success)
	{
		lua_Debug ar;
		if (lua_getstack(L,1,&ar) && lua_getinfo(L,"n",&ar) && ar.name)
			luaL_error(L, "invalid argument #%d to '%s'", stackpos-1, ar.name);
		else
			luaL_argerror(L, stackpos, "invalid argument");
	}

	lua_checkstack(L, MAXRET);
	ret = (int) privateInfo->CallFar(opcode, &fmc);
	pushed = MAXRET - cbdata.ret_avail;
	return pushed ? pushed : (lua_pushnumber(L, ret), 1);
}
