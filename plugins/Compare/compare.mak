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
LinkerLocalOptsAtC32_comparedlib =  -Tpd -ap -c -v
ResLocalOptsAtC32_comparedlib = 
BLocalOptsAtC32_comparedlib = 
CompInheritOptsAt_comparedlib = -ID:\Lang\BC5\INCLUDE 
LinkerInheritOptsAt_comparedlib = -x
LinkerOptsAt_comparedlib = $(LinkerLocalOptsAtC32_comparedlib)
ResOptsAt_comparedlib = $(ResLocalOptsAtC32_comparedlib)
BOptsAt_comparedlib = $(BLocalOptsAtC32_comparedlib)

#
# Dependency List
#
Dep_compare = \
   compare.lib

compare : BccW32.cfg $(Dep_compare)
  echo MakeNode

compare.lib : compare.dll
  $(IMPLIB) $@ compare.dll


Dep_compareddll = \
   compare.obj

compare.dll : $(Dep_compareddll)
  $(TLINK32) @&&|
 $(IDE_LinkFLAGS32) $(LinkerOptsAt_comparedlib) $(LinkerInheritOptsAt_comparedlib) +
#D:\Lang\BC5\LIB\c0d32.obj+
compare.obj
$<,$*
D:\Lang\BC5\LIB\import32.lib+
D:\Lang\BC5\LIB\cw32.lib



|
compare.obj :  compare.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_comparedlib) $(CompInheritOptsAt_comparedlib) -o$@ compare.cpp
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
-H=compare.csm
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


