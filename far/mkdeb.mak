#
# Borland C++ IDE generated makefile
# Generated 29/08/1996 at 20:10:50 PM
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
IDE_LinkFLAGS32 =  -LD:\LANG\BC5\LIB
CompLocalOptsAtC32_fardexe =  -WC -WM -K -d -f- -v -R- -k -x- -RT -Og -Ot -Z -O -Oe -Ol -Ob -Om -Op -Ov
LinkerLocalOptsAtC32_fardexe =  -Tpe -ap -c -v
ResLocalOptsAtC32_fardexe =
BLocalOptsAtC32_fardexe =
CompOptsAt_fardexe = $(CompLocalOptsAtC32_fardexe)
CompInheritOptsAt_fardexe = -ID:\LANG\BC5\INCLUDE
LinkerInheritOptsAt_fardexe = -s
LinkerOptsAt_fardexe = $(LinkerLocalOptsAtC32_fardexe)
ResOptsAt_fardexe = $(ResLocalOptsAtC32_fardexe)
BOptsAt_fardexe = $(BLocalOptsAtC32_fardexe)

#
# Dependency List
#
Dep_Far = \
   far.exe

Far : BccW32.cfg $(Dep_Far)
  echo MakeNode

Dep_fardexe = \
   filelist.obj\
   treelist.obj\
   infolist.obj\
   qview.obj\
   editor.obj\
   viewer.obj\
   filetype.obj\
   dialog.obj\
   findfile.obj\
   usermenu.obj\
   copy.obj\
   plist.obj\
   hilight.obj\
   filter.obj\
   far.obj\
   far.def\
   far.res

far.exe : $(Dep_fardexe)
  $(TLINK32) @&&|
 $(IDE_LinkFLAGS32) $(LinkerOptsAt_fardexe) $(LinkerInheritOptsAt_fardexe) +
D:\LANG\BC5\LIB\c0x32.obj+
filelist.obj+
treelist.obj+
infolist.obj+
qview.obj+
editor.obj+
viewer.obj+
filetype.obj+
dialog.obj+
findfile.obj+
usermenu.obj+
copy.obj+
plist.obj+
hilight.obj+
filter.obj+
far.obj
$<,$*
D:\LANG\BC5\LIB\import32.lib+
D:\LANG\BC5\LIB\cw32mt.lib
far.def
|
   $(BRC32) far.res far.res $<

filelist.obj :  filelist.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_fardexe) $(CompInheritOptsAt_fardexe) -o$@ filelist.cpp
|

treelist.obj :  treelist.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_fardexe) $(CompInheritOptsAt_fardexe) -o$@ treelist.cpp
|

infolist.obj :  infolist.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_fardexe) $(CompInheritOptsAt_fardexe) -o$@ infolist.cpp
|

qview.obj :  qview.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_fardexe) $(CompInheritOptsAt_fardexe) -o$@ qview.cpp
|

editor.obj :  editor.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_fardexe) $(CompInheritOptsAt_fardexe) -o$@ editor.cpp
|

viewer.obj :  viewer.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_fardexe) $(CompInheritOptsAt_fardexe) -o$@ viewer.cpp
|

filetype.obj :  filetype.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_fardexe) $(CompInheritOptsAt_fardexe) -o$@ filetype.cpp
|

dialog.obj :  dialog.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_fardexe) $(CompInheritOptsAt_fardexe) -o$@ dialog.cpp
|


findfile.obj :  findfile.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_fardexe) $(CompInheritOptsAt_fardexe) -o$@ findfile.cpp
|

usermenu.obj :  usermenu.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_fardexe) $(CompInheritOptsAt_fardexe) -o$@ usermenu.cpp
|

copy.obj :  copy.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_fardexe) $(CompInheritOptsAt_fardexe) -o$@ copy.cpp
|

plist.obj :  plist.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_fardexe) $(CompInheritOptsAt_fardexe) -o$@ plist.cpp
|

hilight.obj :  hilight.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_fardexe) $(CompInheritOptsAt_fardexe) -o$@ hilight.cpp
|

filter.obj :  filter.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_fardexe) $(CompInheritOptsAt_fardexe) -o$@ filter.cpp
|

far.obj :  far.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_fardexe) $(CompInheritOptsAt_fardexe) -o$@ far.cpp
|

far.res :  far.rc
  $(BRC32) -R @&&|
 $(IDE_ResFLAGS) $(ROptsAt_fardexe) $(CompInheritOptsAt_fardexe)  -FO$@ far.rc
|

# Compiler configuration file
BccW32.cfg :
   Copy &&|
-R
-v
-vi
-H
-H=far.csm
-K
-d
-f-
-R-
-k
-p-
-x-
-RT-
-O-
-w-sig
-w-big
-w-pin
-wdef
-wnod
-wuse
-wamp
| $@
