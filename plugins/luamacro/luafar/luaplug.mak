# luaplug.mak

ifndef PATH_LUAFAR
    PATH_LUAFAR = ..
endif
ifndef TARGET
    TARGET = luaplug.dll
endif
ifdef FAR_EXPORTS
    EXPORTS = $(addprefix -DEXPORT_,$(FAR_EXPORTS)) 
endif

PATH_LUAFARSRC = $(PATH_LUAFAR)\src

include $(PATH_LUAFAR)/config.mak

ARCH=
CC= gcc

OBJ     = luaplug.o
CFLAGS  = -O2 -Wall -I$(INC_FAR) -I$(INC_LUA) $(EXPORTS) \
          $(ARCH) $(MYCFLAGS)
LDFLAGS = -Wl,--kill-at -shared -s $(ARCH) $(MYLDFLAGS)

vpath %.c $(PATH_LUAFARSRC)
vpath %.h $(PATH_LUAFARSRC)

$(TARGET): $(OBJ) $(PATH_LUAFARSRC)\$(LUAFARDLL)
	$(CC) -o $@ $^ $(LDFLAGS) -l$(LUADLLNAME)

$(PATH_LUAFARSRC)\$(LUAFARDLL):
	cd /D $(PATH_LUAFARSRC) && $(MAKE) PATH_LUAFAR=..

luaplug.o: luaplug.c luafar.h
# (end of Makefile)
