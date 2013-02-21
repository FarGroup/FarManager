# luaplug.mak

ifndef FARDIR
  FARDIR = ..\..\..\unicode_far
endif
ifndef TARGET
  TARGET = luaplug.dll
endif
ifdef FAR_EXPORTS
  EXPORTS = $(addprefix -DEXPORT_,$(FAR_EXPORTS)) 
endif

PATH_LUAFAR = $(FARDIR)\..\plugins\luamacro\luafar

ifeq ($(LUAVERSION),52)
  INC_LUA =
  LUADLLNAME = lua52
else
  INC_LUA = $(PATH_LUAFAR)\..\luasdk\include
  LUADLLNAME = lua5.1
endif

DIRBIT = 32
INC_FAR = $(FARDIR)\..\plugins\common\unicode
LUAFARDLLNAME = luafar3

ARCH= -m$(DIRBIT)
CC= gcc

OBJ = luaplug.o $(MYOBJ)

CFLAGS = -O2 -Wall -I$(INC_FAR) -I$(INC_LUA) $(EXPORTS) $(ARCH) $(MYCFLAGS)

LDFLAGS = -Wl,--kill-at -shared -s $(ARCH) $(MYLDFLAGS) \
  -L$(FARDIR)\Release.$(DIRBIT).gcc

vpath %.c $(PATH_LUAFAR)
vpath %.h $(PATH_LUAFAR)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS) -l$(LUADLLNAME) -l$(LUAFARDLLNAME)

luaplug.o: luaplug.c luafar.h
# (end of Makefile)
