#ifndef LUAFAR_SERVICE_H
#define LUAFAR_SERVICE_H

#define TKEY_BINARY "__binary"

typedef int64_t flags_t;

flags_t  OptFlags(lua_State* L, int pos, flags_t dflt);
flags_t  GetFlagCombination(lua_State *L, int pos, int *success);
flags_t  GetFlagsFromTable(lua_State *L, int pos, const char* key);
void     PutFlagsToTable(lua_State *L, const char* key, flags_t flags);

int      PushDMParams (lua_State *L, intptr_t Msg, intptr_t Param1);
int      PushDNParams (lua_State *L, intptr_t Msg, intptr_t Param1, void *Param2);
intptr_t ProcessDNResult(lua_State *L, intptr_t Msg, void *Param2);

void     FillInputRecord(lua_State *L, int pos, INPUT_RECORD *ir);
void     PushInputRecord(lua_State *L, const INPUT_RECORD* ir);

void     DeleteLuaStateTimerQueue(lua_State *L);
HANDLE   GetLuaStateTimerQueue(lua_State *L);

int      IsLuaJIT(void);
void     LF_Error(lua_State *L, const wchar_t* aMsg);
void     NewVirtualKeyTable(lua_State* L, BOOL twoways);
void     pushFileTime(lua_State *L, const FILETIME *ft);
void     ConvertLuaValue(lua_State *L, int pos, struct FarMacroValue *target);

extern const char* VirtualKeyStrings[256];

#endif

