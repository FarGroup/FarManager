#ifndef LUAFAR_SERVICE_H
#define LUAFAR_SERVICE_H

#define TKEY_BINARY "__binary"

#define PAIR(prefix,txt) {#txt, prefix ## _ ## txt}

void     PutRECTToTable(lua_State *L, const char* key, RECT rect);
int      SetKeyBar(lua_State *L, BOOL editor);
void     GetOptGuid(lua_State *L, int pos, GUID* target, const GUID* source);
void     PushEditorSetPosition(lua_State *L, const struct EditorSetPosition *esp);
int      FillEditorSelect(lua_State *L, int pos_table, struct EditorSelect *es);
void     FillEditorSetPosition(lua_State *L, struct EditorSetPosition *esp);
void     OptInputRecord(lua_State* L, TPluginData *pd, int pos, INPUT_RECORD* ir);
void     PushFarMacroValue(lua_State* L, const struct FarMacroValue* val);
int      pcall_msg(lua_State* L, int narg, int nret);
int      Dialog_getvalue(lua_State *L, int pos, HANDLE *target);
void     PushPluginObject(lua_State* L, HANDLE hPlugin);
void     PackMacroValues(lua_State* L, size_t Count, const struct FarMacroValue* Values);

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

