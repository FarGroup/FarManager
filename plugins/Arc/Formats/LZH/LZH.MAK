#
# Borland C++ IDE generated makefile
#
.AUTODEPEND


#
# Borland C++ tools
#
IMPLIB  = Implib
BCC32   = Bcc32 +BccW32.cfg
TLINK32 = TLink32
TLIB    = TLib
BRC32   = Brc32
TASM32  = Tasm32
#
# IDE macros
#


#
# Options
#
IDE_LFLAGS32 =  -LD:\LANG\BC5\LIB
LLATC32_hadlib =  -Tpd -ap -c -v
RLATC32_hadlib =  -w32
BLATC32_hadlib =
CNIEAT_hadlib = -ID:\LANG\BC5\INCLUDE -D
LNIEAT_hadlib = -x
LEAT_hadlib = $(LLATC32_hadlib)
REAT_hadlib = $(RLATC32_hadlib)
BEAT_hadlib = $(BLATC32_hadlib)

#
# Dependency List
#
Dep_lzh = \
   lzh.lib

lzh : BccW32.cfg $(Dep_lzh)
  echo MakeNode

lzh.lib : lzh.dll
  $(IMPLIB) $@ lzh.dll


Dep_haddll = \
   lzh.obj

lzh.dll : $(Dep_haddll)
  $(TLINK32) @&&|
 $(IDE_LFLAGS32) $(LEAT_hadlib) $(LNIEAT_hadlib) +
# D:\LANG\BC5\LIB\c0d32.obj+
lzh.obj
$<,$*
D:\LANG\BC5\LIB\import32.lib+
D:\LANG\BC5\LIB\cw32.lib

|

lzh.obj :  lzh.cpp
  $(BCC32) -c @&&|
 $(CEAT_hadlib) $(CNIEAT_hadlib) -o$@ lzh.cpp
|

# Compiler configuration file
BccW32.cfg :
   Copy &&|
-v
-R
-O1
-H-
-WCD
| $@


