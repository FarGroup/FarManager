#ifndef LUAFAR_BIT64_H
#define LUAFAR_BIT64_H

int bit64_getvalue(lua_State *L, int pos, INT64 *target);
int bit64_push(lua_State *L, INT64 v);
int bit64_pushuserdata(lua_State *L, INT64 v);
INT64 check64(lua_State *L, int pos, int* success);
int luaopen_bit64(lua_State *L);

#endif
