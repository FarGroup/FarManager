#
# Borland C++ IDE generated makefile
# Generated 09.05.97 at 18:20:09 
#
.AUTODEPEND


#
# Borland C++ tools
#
IMPLIB  = Implib
BCC32   = Bcc32 +BccW32.cfg 
BCC32I  = Bcc32i +BccW32.cfg 
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
IDE_LinkFLAGS32 =  -LD:\Lang\BC5\LIB
LinkerLocalOptsAtC32_drawlinedlib =  -Tpd -ap -c -v
ResLocalOptsAtC32_drawlinedlib = 
BLocalOptsAtC32_drawlinedlib = 
CompInheritOptsAt_drawlinedlib = -ID:\Lang\BC5\INCLUDE 
LinkerInheritOptsAt_drawlinedlib = -x
LinkerOptsAt_drawlinedlib = $(LinkerLocalOptsAtC32_drawlinedlib)
ResOptsAt_drawlinedlib = $(ResLocalOptsAtC32_drawlinedlib)
BOptsAt_drawlinedlib = $(BLocalOptsAtC32_drawlinedlib)

#
# Dependency List
#
Dep_drawline = \
   drawline.lib

drawline : BccW32.cfg $(Dep_drawline)
  echo MakeNode

drawline.lib : drawline.dll
  $(IMPLIB) $@ drawline.dll


Dep_drawlineddll = \
   drawline.obj

drawline.dll : $(Dep_drawlineddll)
  $(TLINK32) @&&|
 $(IDE_LinkFLAGS32) $(LinkerOptsAt_drawlinedlib) $(LinkerInheritOptsAt_drawlinedlib) +
#D:\Lang\BC5\LIB\c0d32.obj+
drawline.obj
$<,$*
D:\Lang\BC5\LIB\import32.lib+
D:\Lang\BC5\LIB\cw32.lib



|
drawline.obj :  drawline.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_drawlinedlib) $(CompInheritOptsAt_drawlinedlib) -o$@ drawline.cpp
|

# Compiler configuration file
BccW32.cfg : 
   Copy &&|
-w
-w-pin
-R
-v
-vi
-H
-H=drawline.csm
-WCD
-K
-f-
-R-
-k-
-3
-ps
-Oc
-O1
-H-
| $@


