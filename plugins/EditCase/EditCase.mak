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
LinkerLocalOptsAtC32_editcasedlib =  -Tpd -ap -c -v
ResLocalOptsAtC32_editcasedlib = 
BLocalOptsAtC32_editcasedlib = 
CompInheritOptsAt_editcasedlib = -ID:\Lang\BC5\INCLUDE 
LinkerInheritOptsAt_editcasedlib = -x
LinkerOptsAt_editcasedlib = $(LinkerLocalOptsAtC32_editcasedlib)
ResOptsAt_editcasedlib = $(ResLocalOptsAtC32_editcasedlib)
BOptsAt_editcasedlib = $(BLocalOptsAtC32_editcasedlib)

#
# Dependency List
#
Dep_editcase = \
   editcase.lib

editcase : BccW32.cfg $(Dep_editcase)
  echo MakeNode

editcase.lib : editcase.dll
  $(IMPLIB) $@ editcase.dll


Dep_editcaseddll = \
   editcase.obj

editcase.dll : $(Dep_editcaseddll)
  $(TLINK32) @&&|
  $(IDE_LinkFLAGS32) $(LinkerOptsAt_editcasedlib) $(LinkerInheritOptsAt_editcasedlib) +
#D:\Lang\BC5\LIB\c0d32.obj+
editcase.obj
$<,$*
D:\Lang\BC5\LIB\import32.lib+
D:\Lang\BC5\LIB\cw32.lib



|
editcase.obj :  editcase.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_editcasedlib) $(CompInheritOptsAt_editcasedlib) -o$@ editcase.cpp
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
-H=editcase.csm
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


