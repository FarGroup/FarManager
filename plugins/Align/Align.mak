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
LinkerLocalOptsAtC32_Aligndlib =  -Tpd -ap -c -v
ResLocalOptsAtC32_Aligndlib = 
BLocalOptsAtC32_Aligndlib = 
CompInheritOptsAt_Aligndlib = -ID:\Lang\BC5\INCLUDE 
LinkerInheritOptsAt_Aligndlib = -x
LinkerOptsAt_Aligndlib = $(LinkerLocalOptsAtC32_Aligndlib)
ResOptsAt_Aligndlib = $(ResLocalOptsAtC32_Aligndlib)
BOptsAt_Aligndlib = $(BLocalOptsAtC32_Aligndlib)

#
# Dependency List
#
Dep_Align = \
   Align.lib

Align : BccW32.cfg $(Dep_Align)
  echo MakeNode

Align.lib : Align.dll
  $(IMPLIB) $@ Align.dll


Dep_Alignddll = \
   Align.obj

Align.dll : $(Dep_Alignddll)
  $(TLINK32) @&&|
  $(IDE_LinkFLAGS32) $(LinkerOptsAt_Aligndlib) $(LinkerInheritOptsAt_Aligndlib) +
#D:\Lang\BC5\LIB\c0d32.obj+
Align.obj
$<,$*
D:\Lang\BC5\LIB\import32.lib+
D:\Lang\BC5\LIB\cw32.lib



|
Align.obj :  Align.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_Aligndlib) $(CompInheritOptsAt_Aligndlib) -o$@ Align.cpp
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
-H=Align.csm
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


