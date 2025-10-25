#ifndef LUAFAR_SERVICE_H
#define LUAFAR_SERVICE_H

#define TKEY_BINARY "__binary"

UINT64   OptFlags(lua_State* L, int pos, UINT64 dflt);
UINT64   GetFlagCombination(lua_State *L, int pos, int *success);
UINT64   GetFlagsFromTable(lua_State *L, int pos, const char* key);
void     PutFlagsToTable(lua_State *L, const char* key, UINT64 flags);

int      PushDMParams (lua_State *L, intptr_t Msg, intptr_t Param1);
int      PushDNParams (lua_State *L, intptr_t Msg, intptr_t Param1, void *Param2);
intptr_t ProcessDNResult(lua_State *L, intptr_t Msg, void *Param2);

void     FillInputRecord(lua_State *L, int pos, INPUT_RECORD *ir);
void     PushInputRecord(lua_State *L, const INPUT_RECORD* ir);

void     DeleteLuaStateTimerQueue(lua_State *L);
HANDLE   GetLuaStateTimerQueue(lua_State *L);

int      IsLuaJIT();
void     LF_Error(lua_State *L, const wchar_t* aMsg);
void     NewVirtualKeyTable(lua_State* L, BOOL twoways);
void     pushFileTime(lua_State *L, const FILETIME *ft);
void     ConvertLuaValue(lua_State *L, int pos, struct FarMacroValue *target);

extern const char* VirtualKeyStrings[256];

#endif
