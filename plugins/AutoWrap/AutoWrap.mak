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
LinkerLocalOptsAtC32_AutoWrapdlib =  -Tpd -ap -c -v
ResLocalOptsAtC32_AutoWrapdlib = 
BLocalOptsAtC32_AutoWrapdlib = 
CompInheritOptsAt_AutoWrapdlib = -ID:\Lang\BC5\INCLUDE 
LinkerInheritOptsAt_AutoWrapdlib = -x
LinkerOptsAt_AutoWrapdlib = $(LinkerLocalOptsAtC32_AutoWrapdlib)
ResOptsAt_AutoWrapdlib = $(ResLocalOptsAtC32_AutoWrapdlib)
BOptsAt_AutoWrapdlib = $(BLocalOptsAtC32_AutoWrapdlib)

#
# Dependency List
#
Dep_AutoWrap = \
   AutoWrap.lib

AutoWrap : BccW32.cfg $(Dep_AutoWrap)
  echo MakeNode

AutoWrap.lib : AutoWrap.dll
  $(IMPLIB) $@ AutoWrap.dll


Dep_AutoWrapddll = \
   AutoWrap.obj

AutoWrap.dll : $(Dep_AutoWrapddll)
  $(TLINK32) @&&|
  $(IDE_LinkFLAGS32) $(LinkerOptsAt_AutoWrapdlib) $(LinkerInheritOptsAt_AutoWrapdlib) +
#D:\Lang\BC5\LIB\c0d32.obj+
AutoWrap.obj
$<,$*
D:\Lang\BC5\LIB\import32.lib+
D:\Lang\BC5\LIB\cw32.lib



|
AutoWrap.obj :  AutoWrap.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_AutoWrapdlib) $(CompInheritOptsAt_AutoWrapdlib) -o$@ AutoWrap.cpp
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
-H=AutoWrap.csm
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


