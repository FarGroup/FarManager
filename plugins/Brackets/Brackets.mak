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
LinkerLocalOptsAtC32_Bracketsdlib =  -Tpd -ap -c -v-
ResLocalOptsAtC32_Bracketsdlib = 
BLocalOptsAtC32_Bracketsdlib = 
CompInheritOptsAt_Bracketsdlib = -ID:\Lang\BC5\INCLUDE 
LinkerInheritOptsAt_Bracketsdlib = -x
LinkerOptsAt_Bracketsdlib = $(LinkerLocalOptsAtC32_Bracketsdlib)
ResOptsAt_Bracketsdlib = $(ResLocalOptsAtC32_Bracketsdlib)
BOptsAt_Bracketsdlib = $(BLocalOptsAtC32_Bracketsdlib)

#
# Dependency List
#
Dep_Brackets = \
   Brackets.lib

Brackets : BccW32.cfg $(Dep_Brackets)
  echo MakeNode

Brackets.lib : Brackets.dll
  $(IMPLIB) $@ Brackets.dll


Dep_Bracketsddll = \
   Brackets.obj

Brackets.dll : $(Dep_Bracketsddll)
  $(TLINK32) @&&|
  $(IDE_LinkFLAGS32) $(LinkerOptsAt_Bracketsdlib) $(LinkerInheritOptsAt_Bracketsdlib) +
#D:\Lang\BC5\LIB\c0d32.obj+
Brackets.obj
$<,$*
D:\Lang\BC5\LIB\import32.lib+
D:\Lang\BC5\LIB\cw32.lib



|
Brackets.obj :  Brackets.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_Bracketsdlib) $(CompInheritOptsAt_Bracketsdlib) -o$@ Brackets.cpp
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
-H=Brackets.csm
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


