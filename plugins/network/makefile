#make file to make Network.dll
#Note: plugins.hpp should be in $(INCLUDE) dir
INCLUDE = $(INCLUDE);..\common

CC = CL
CCOPTS = /nologo /Zp1 /MD /W3 /GX- /Ox /Op /c /Gs10000

RCC = RC

LNK = LINK
LNK_OPTS = /nologo /dll /incremental:no /machine:I386 /def:"Network.def" /opt:nowin98

LIBS = Mpr.lib User32.lib Advapi32.lib

default: \
  Network.dll

TARGETS = \
  Network.obj \
  Memory.obj \
  NetCfg.obj \
  NetClass.obj \
  NetCommon.obj \
  NetFavorites.obj \
  NetMix.obj \
  NetNT.obj \
  NetReg.obj \
  Network.res

.cpp.obj:
  $(CC) $(CCOPTS) $<

Network.dll: $(TARGETS) Network.def
  $(LNK) $(LIBS) $(LNK_OPTS) $(TARGETS)

Network.res: Network.rc
 $(RCC) Network.rc
