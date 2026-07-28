#ifndef LF_FLAGS_H
#define LF_FLAGS_H

#include <stdint.h>
#include <lua.h>

extern const char FAR_FLAGSTABLE[];

typedef int64_t flags_t;

flags_t check_env_flag(lua_State *L, int pos);
flags_t CheckFlags(lua_State* L, int pos);
flags_t CheckFlagsFromTable(lua_State *L, int pos, const char* key);
flags_t get_env_flag(lua_State *L, int pos, int *success);
flags_t GetFlagCombination(lua_State *L, int pos, int *success);
flags_t GetFlagsFromTable(lua_State *L, int pos, const char* key);
flags_t OptFlags(lua_State* L, int pos, flags_t dflt);
void PutFlagsToArray(lua_State *L, int index, flags_t flags);
void PutFlagsToTable(lua_State *L, const char* key, flags_t flags);

#endif
