ifeq ($(LUAVERSION),52)
  PATH_LUASRC = ../../../../lua_thirdparty/lua-5.2/src
  LUADLLNAME = lua52
else
  PATH_LUASRC = ../../../../lua_thirdparty/lua/src
  LUADLLNAME = lua5.1
endif

TARGET = lflua.exe

vpath %.c $(PATH_LUASRC)
ARCH = -m32
CFLAGS = -O2 -W -Wall -I$(PATH_LUASRC) $(ARCH)
LIBS = -lluafar3 -l$(LUADLLNAME)

$(TARGET): linit.o lua.o
	gcc -o $@ $^ $(ARCH) $(LIBS) -s
