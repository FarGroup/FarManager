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
LinkerLocalOptsAtC32_filecasedlib =  -Tpd -ap -c -v
ResLocalOptsAtC32_filecasedlib = 
BLocalOptsAtC32_filecasedlib = 
CompInheritOptsAt_filecasedlib = -ID:\Lang\BC5\INCLUDE 
LinkerInheritOptsAt_filecasedlib = -x
LinkerOptsAt_filecasedlib = $(LinkerLocalOptsAtC32_filecasedlib)
ResOptsAt_filecasedlib = $(ResLocalOptsAtC32_filecasedlib)
BOptsAt_filecasedlib = $(BLocalOptsAtC32_filecasedlib)

#
# Dependency List
#
Dep_filecase = \
   filecase.lib

filecase : BccW32.cfg $(Dep_filecase)
  echo MakeNode

filecase.lib : filecase.dll
  $(IMPLIB) $@ filecase.dll


Dep_filecaseddll = \
   filecase.obj

filecase.dll : $(Dep_filecaseddll)
  $(TLINK32) @&&|
 $(IDE_LinkFLAGS32) $(LinkerOptsAt_filecasedlib) $(LinkerInheritOptsAt_filecasedlib) +
#D:\Lang\BC5\LIB\c0d32.obj+
filecase.obj
$<,$*
D:\Lang\BC5\LIB\import32.lib+
D:\Lang\BC5\LIB\cw32.lib



|
filecase.obj :  filecase.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_filecasedlib) $(CompInheritOptsAt_filecasedlib) -o$@ filecase.cpp
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
-H=filecase.csm
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


