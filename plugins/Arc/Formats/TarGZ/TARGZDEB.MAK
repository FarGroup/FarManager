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
LLATC32_targzdlib =  -Tpd -ap -c
RLATC32_targzdlib =  -w32
BLATC32_targzdlib =
CNIEAT_targzdlib = -ID:\LANG\BC5\INCLUDE -D
LNIEAT_targzdlib = -x
LEAT_targzdlib = $(LLATC32_targzdlib)
REAT_targzdlib = $(RLATC32_targzdlib)
BEAT_targzdlib = $(BLATC32_targzdlib)

#
# Dependency List
#
Dep_targz = \
   targz.lib

targz : BccW32.cfg $(Dep_targz)
  echo MakeNode

targz.lib : targz.dll
  $(IMPLIB) $@ targz.dll


Dep_targzddll = \
   targz.obj

targz.dll : $(Dep_targzddll)
  $(TLINK32) @&&|
 /v $(IDE_LFLAGS32) $(LEAT_targzdlib) $(LNIEAT_targzdlib) +
# D:\LANG\BC5\LIB\c0d32.obj+
targz.obj
$<,$*
D:\LANG\BC5\LIB\import32.lib+
D:\LANG\BC5\LIB\cw32.lib

|

targz.obj :  targz.cpp
  $(BCC32) -c @&&|
 $(CEAT_targzdlib) $(CNIEAT_targzdlib) -o$@ targz.cpp
|

# Compiler configuration file
BccW32.cfg :
   Copy &&|
-R
-v
-vi
-H-
-WCD
| $@


