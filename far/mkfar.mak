#
# Компиляция FAR Manager
# Modify: 25.06.2000 SVS
#
# make -fmkfar.mak [options]
#
.AUTODEPEND

# берутся из переменных среды (env)
LIBPATH=$(FARLIB)
INCLUDEPATH=$(FARINC)

# путь к каталогу с временными файлами - obj, etc
OBJPATH=OBJ

# сюда будет помещен результат
FINALPATH=final

!ifdef DEBUG
OPTDEBUG=-v
!else
OPTDEBUG=-v-
!endif

!ifdef PROCOMP
PRECOMPOPT=-H
!else
PRECOMPOPT=-H-
!endif

#
# Borland C++ tools
#
IMPLIB  = Implib
BCC32   = Bcc32 +BccW32.cfg
TLINK32 = TLink32
TLIB    = TLib
BRC32   = Brc32
#
# IDE macros
#


#
# Options
#
LINKFLAGS =  -L$(LIBPATH) -Tpe -ap -c $(OPTDEBUG) -s
CCFLAGS = -WC -WM -K -d -f- $(OPTDEBUG) -R- -k- -x- -RT -Og -Ot -Z -O -Oe -Ol -Ob -Om -Op -Ov -w-csu $(PRECOMPOPT) -I$(INCLUDEPATH)

#
# Dependency List
#
Dep_Far = \
   $(FINALPATH)\far.exe

Far : BccW32.cfg $(Dep_Far)
  echo MakeNode

Dep_fardexe = \
   $(OBJPATH)\modal.obj\
   $(OBJPATH)\help.obj\
   $(OBJPATH)\checkver.obj\
   $(OBJPATH)\plugapi.obj\
   $(OBJPATH)\language.obj\
   $(OBJPATH)\setcolor.obj\
   $(OBJPATH)\palette.obj\
   $(OBJPATH)\mkdir.obj\
   $(OBJPATH)\plugins.obj\
   $(OBJPATH)\manager.obj\
   $(OBJPATH)\poscache.obj\
   $(OBJPATH)\grabber.obj\
   $(OBJPATH)\macro.obj\
   $(OBJPATH)\scrbuf.obj\
   $(OBJPATH)\scrsaver.obj\
   $(OBJPATH)\keybar.obj\
   $(OBJPATH)\print.obj\
   $(OBJPATH)\iswind.obj\
   $(OBJPATH)\global.obj\
   $(OBJPATH)\history.obj\
   $(OBJPATH)\registry.obj\
   $(OBJPATH)\cmdline.obj\
   $(OBJPATH)\namelist.obj\
   $(OBJPATH)\dizlist.obj\
   $(OBJPATH)\grpsort.obj\
   $(OBJPATH)\gettable.obj\
   $(OBJPATH)\int64.obj\
   $(OBJPATH)\infolist.obj\
   $(OBJPATH)\hmenu.obj\
   $(OBJPATH)\vmenu.obj\
   $(OBJPATH)\qview.obj\
   $(OBJPATH)\editor.obj\
   $(OBJPATH)\options.obj\
   $(OBJPATH)\edit.obj\
   $(OBJPATH)\viewer.obj\
   $(OBJPATH)\filetype.obj\
   $(OBJPATH)\dialog.obj\
   $(OBJPATH)\findfile.obj\
   $(OBJPATH)\menubar.obj\
   $(OBJPATH)\interf.obj\
   $(OBJPATH)\usermenu.obj\
   $(OBJPATH)\chgmmode.obj\
   $(OBJPATH)\rdrwdsk.obj\
   $(OBJPATH)\copy.obj\
   $(OBJPATH)\panel.obj\
   $(OBJPATH)\scrobj.obj\
   $(OBJPATH)\savescr.obj\
   $(OBJPATH)\delete.obj\
   $(OBJPATH)\flupdate.obj\
   $(OBJPATH)\flplugin.obj\
   $(OBJPATH)\scantree.obj\
   $(OBJPATH)\mix.obj\
   $(OBJPATH)\plist.obj\
   $(OBJPATH)\hilight.obj\
   $(OBJPATH)\config.obj\
   $(OBJPATH)\message.obj\
   $(OBJPATH)\setattr.obj\
   $(OBJPATH)\plognmn.obj\
   $(OBJPATH)\filestr.obj\
   $(OBJPATH)\local.obj\
   $(OBJPATH)\filter.obj\
   $(OBJPATH)\lockscrn.obj\
   $(OBJPATH)\fileedit.obj\
   $(OBJPATH)\fileview.obj\
   $(OBJPATH)\filelist.obj\
   $(OBJPATH)\treelist.obj\
   $(OBJPATH)\savefpos.obj\
   $(OBJPATH)\chgprior.obj\
   $(OBJPATH)\foldtree.obj\
   $(OBJPATH)\ffolders.obj\
   $(OBJPATH)\ctrlobj.obj\
   $(OBJPATH)\flmodes.obj\
   $(OBJPATH)\flshow.obj\
   $(OBJPATH)\main.obj\
   far.def\
   $(OBJPATH)\far.res

$(FINALPATH)\far.exe : $(Dep_fardexe)
  $(TLINK32) @&&|
 $(LINKFLAGS) +
$(LIBPATH)\c0x32.obj+
$(OBJPATH)\modal.obj+
$(OBJPATH)\flplugin.obj+
$(OBJPATH)\help.obj+
$(OBJPATH)\rdrwdsk.obj+
$(OBJPATH)\checkver.obj+
$(OBJPATH)\setcolor.obj+
$(OBJPATH)\mkdir.obj+
$(OBJPATH)\language.obj+
$(OBJPATH)\print.obj+
$(OBJPATH)\palette.obj+
$(OBJPATH)\plugapi.obj+
$(OBJPATH)\interf.obj+
$(OBJPATH)\dizlist.obj+
$(OBJPATH)\edit.obj+
$(OBJPATH)\plugins.obj+
$(OBJPATH)\filestr.obj+
$(OBJPATH)\mix.obj+
$(OBJPATH)\delete.obj+
$(OBJPATH)\manager.obj+
$(OBJPATH)\poscache.obj+
$(OBJPATH)\options.obj+
$(OBJPATH)\grabber.obj+
$(OBJPATH)\macro.obj+
$(OBJPATH)\scrbuf.obj+
$(OBJPATH)\scrobj.obj+
$(OBJPATH)\flshow.obj+
$(OBJPATH)\ffolders.obj+
$(OBJPATH)\setattr.obj+
$(OBJPATH)\panel.obj+
$(OBJPATH)\local.obj+
$(OBJPATH)\iswind.obj+
$(OBJPATH)\config.obj+
$(OBJPATH)\message.obj+
$(OBJPATH)\flupdate.obj+
$(OBJPATH)\ctrlobj.obj+
$(OBJPATH)\keybar.obj+
$(OBJPATH)\scrsaver.obj+
$(OBJPATH)\chgmmode.obj+
$(OBJPATH)\registry.obj+
$(OBJPATH)\global.obj+
$(OBJPATH)\grpsort.obj+
$(OBJPATH)\gettable.obj+
$(OBJPATH)\hmenu.obj+
$(OBJPATH)\vmenu.obj+
$(OBJPATH)\history.obj+
$(OBJPATH)\namelist.obj+
$(OBJPATH)\int64.obj+
$(OBJPATH)\infolist.obj+
$(OBJPATH)\plognmn.obj+
$(OBJPATH)\lockscrn.obj+
$(OBJPATH)\qview.obj+
$(OBJPATH)\editor.obj+
$(OBJPATH)\menubar.obj+
$(OBJPATH)\viewer.obj+
$(OBJPATH)\scantree.obj+
$(OBJPATH)\filetype.obj+
$(OBJPATH)\savescr.obj+
$(OBJPATH)\dialog.obj+
$(OBJPATH)\savefpos.obj+
$(OBJPATH)\chgprior.obj+
$(OBJPATH)\flmodes.obj+
$(OBJPATH)\findfile.obj+
$(OBJPATH)\usermenu.obj+
$(OBJPATH)\fileedit.obj+
$(OBJPATH)\fileview.obj+
$(OBJPATH)\foldtree.obj+
$(OBJPATH)\copy.obj+
$(OBJPATH)\plist.obj+
$(OBJPATH)\hilight.obj+
$(OBJPATH)\filter.obj+
$(OBJPATH)\filelist.obj+
$(OBJPATH)\treelist.obj+
$(OBJPATH)\cmdline.obj+
$(OBJPATH)\main.obj
$<,$*
$(LIBPATH)\import32.lib+
$(LIBPATH)\cw32mt.lib
far.def
|

   $(BRC32) $(OBJPATH)\far.res $(OBJPATH)\far.res $<


$(OBJPATH)\lockscrn.obj :  lockscrn.cpp
  $(BCC32) -c @&&|
 $(CCFLAGS)  -o$@ lockscrn.cpp
|

$(OBJPATH)\filestr.obj :  filestr.cpp
  $(BCC32) -c @&&|
 $(CCFLAGS)  -o$@ filestr.cpp
|

$(OBJPATH)\flshow.obj :  flshow.cpp
  $(BCC32) -c @&&|
 $(CCFLAGS)  -o$@ flshow.cpp
|

$(OBJPATH)\flplugin.obj :  flplugin.cpp
  $(BCC32) -c @&&|
 $(CCFLAGS)  -o$@ flplugin.cpp
|

$(OBJPATH)\flupdate.obj :  flupdate.cpp
  $(BCC32) -c @&&|
 $(CCFLAGS)  -o$@ flupdate.cpp
|

$(OBJPATH)\plognmn.obj :  plognmn.cpp
  $(BCC32) -c @&&|
 $(CCFLAGS)  -o$@ plognmn.cpp
|

$(OBJPATH)\rdrwdsk.obj :  rdrwdsk.cpp
  $(BCC32) -c @&&|
 $(CCFLAGS)  -o$@ rdrwdsk.cpp
|

$(OBJPATH)\vmenu.obj :  vmenu.cpp
  $(BCC32) -c @&&|
 $(CCFLAGS)  -o$@ vmenu.cpp
|

$(OBJPATH)\mix.obj :  mix.cpp
  $(BCC32) -c @&&|
 $(CCFLAGS)  -o$@ mix.cpp
|

$(OBJPATH)\chgprior.obj :  chgprior.cpp
  $(BCC32) -c @&&|
 $(CCFLAGS)  -o$@ chgprior.cpp
|

$(OBJPATH)\savefpos.obj :  savefpos.cpp
  $(BCC32) -c @&&|
 $(CCFLAGS)  -o$@ savefpos.cpp
|

$(OBJPATH)\flmodes.obj :  flmodes.cpp
  $(BCC32) -c @&&|
 $(CCFLAGS)  -o$@ flmodes.cpp
|

$(OBJPATH)\chgmmode.obj :  chgmmode.cpp
  $(BCC32) -c @&&|
 $(CCFLAGS)  -o$@ chgmmode.cpp
|

$(OBJPATH)\ffolders.obj :  ffolders.cpp
  $(BCC32) -c @&&|
 $(CCFLAGS)  -o$@ ffolders.cpp
|

$(OBJPATH)\foldtree.obj :  foldtree.cpp
  $(BCC32) -c @&&|
 $(CCFLAGS)  -o$@ foldtree.cpp
|

$(OBJPATH)\fileview.obj :  fileview.cpp
  $(BCC32) -c @&&|
 $(CCFLAGS)  -o$@ fileview.cpp
|

$(OBJPATH)\fileedit.obj :  fileedit.cpp
  $(BCC32) -c @&&|
 $(CCFLAGS)  -o$@ fileedit.cpp
|

$(OBJPATH)\scantree.obj :  scantree.cpp
  $(BCC32) -c @&&|
 $(CCFLAGS)  -o$@ scantree.cpp
|

$(OBJPATH)\delete.obj :  delete.cpp
  $(BCC32) -c @&&|
 $(CCFLAGS)  -o$@ delete.cpp
|

$(OBJPATH)\savescr.obj :  savescr.cpp
  $(BCC32) -c @&&|
 $(CCFLAGS)  -o$@ savescr.cpp
|

$(OBJPATH)\interf.obj :  interf.cpp
  $(BCC32) -c @&&|
 $(CCFLAGS)  -o$@ interf.cpp
|

$(OBJPATH)\menubar.obj :  menubar.cpp
  $(BCC32) -c @&&|
 $(CCFLAGS)  -o$@ menubar.cpp
|

$(OBJPATH)\hmenu.obj :  hmenu.cpp
  $(BCC32) -c @&&|
 $(CCFLAGS)  -o$@ hmenu.cpp
|

$(OBJPATH)\gettable.obj :  gettable.cpp
  $(BCC32) -c @&&|
 $(CCFLAGS)  -o$@ gettable.cpp
|

$(OBJPATH)\grpsort.obj :  grpsort.cpp
  $(BCC32) -c @&&|
 $(CCFLAGS)  -o$@ grpsort.cpp
|

$(OBJPATH)\cmdline.obj :  cmdline.cpp
  $(BCC32) -c @&&|
 $(CCFLAGS)  -o$@ cmdline.cpp
|

$(OBJPATH)\registry.obj :  registry.cpp
  $(BCC32) -c @&&|
 $(CCFLAGS)  -o$@ registry.cpp
|

$(OBJPATH)\ctrlobj.obj :  ctrlobj.cpp
  $(BCC32) -c @&&|
 $(CCFLAGS)  -o$@ ctrlobj.cpp
|

$(OBJPATH)\keybar.obj :  keybar.cpp
  $(BCC32) -c @&&|
 $(CCFLAGS)  -o$@ keybar.cpp
|

$(OBJPATH)\scrsaver.obj :  scrsaver.cpp
  $(BCC32) -c @&&|
 $(CCFLAGS)  -o$@ scrsaver.cpp
|

$(OBJPATH)\setattr.obj :  setattr.cpp
  $(BCC32) -c @&&|
 $(CCFLAGS)  -o$@ setattr.cpp
|

$(OBJPATH)\message.obj :  message.cpp
  $(BCC32) -c @&&|
 $(CCFLAGS)  -o$@ message.cpp
|

$(OBJPATH)\local.obj :  local.cpp
  $(BCC32) -c @&&|
 $(CCFLAGS)  -o$@ local.cpp
|

$(OBJPATH)\config.obj :  config.cpp
  $(BCC32) -c @&&|
 $(CCFLAGS)  -o$@ config.cpp
|

$(OBJPATH)\scrobj.obj :  scrobj.cpp
  $(BCC32) -c @&&|
 $(CCFLAGS)  -o$@ scrobj.cpp
|

$(OBJPATH)\panel.obj :  panel.cpp
  $(BCC32) -c @&&|
 $(CCFLAGS)  -o$@ panel.cpp
|

$(OBJPATH)\edit.obj :  edit.cpp
  $(BCC32) -c @&&|
 $(CCFLAGS)  -o$@ edit.cpp
|

$(OBJPATH)\options.obj :  options.cpp
  $(BCC32) -c @&&|
 $(CCFLAGS)  -o$@ options.cpp
|

$(OBJPATH)\modal.obj :  modal.cpp
  $(BCC32) -c @&&|
 $(CCFLAGS)  -o$@ modal.cpp
|

$(OBJPATH)\help.obj :  help.cpp
  $(BCC32) -c @&&|
 $(CCFLAGS)  -o$@ help.cpp
|

$(OBJPATH)\setcolor.obj :  setcolor.cpp
  $(BCC32) -c @&&|
 $(CCFLAGS)  -o$@ setcolor.cpp
|

$(OBJPATH)\dizlist.obj :  dizlist.cpp
  $(BCC32) -c @&&|
 $(CCFLAGS)  -o$@ dizlist.cpp
|

$(OBJPATH)\print.obj :  print.cpp
  $(BCC32) -c @&&|
 $(CCFLAGS)  -o$@ print.cpp
|

$(OBJPATH)\mkdir.obj :  mkdir.cpp
  $(BCC32) -c @&&|
 $(CCFLAGS)  -o$@ mkdir.cpp
|

$(OBJPATH)\palette.obj :  palette.cpp
  $(BCC32) -c @&&|
 $(CCFLAGS)  -o$@ palette.cpp
|

$(OBJPATH)\language.obj :  language.cpp
  $(BCC32) -c @&&|
 $(CCFLAGS)  -o$@ language.cpp
|

$(OBJPATH)\plugapi.obj :  plugapi.cpp
  $(BCC32) -c @&&|
 $(CCFLAGS)  -o$@ plugapi.cpp
|

$(OBJPATH)\plugins.obj :  plugins.cpp
  $(BCC32) -c @&&|
 $(CCFLAGS)  -o$@ plugins.cpp
|

$(OBJPATH)\manager.obj :  manager.cpp
  $(BCC32) -c @&&|
 $(CCFLAGS)  -o$@ manager.cpp
|

$(OBJPATH)\poscache.obj :  poscache.cpp
  $(BCC32) -c @&&|
 $(CCFLAGS)  -o$@ poscache.cpp
|

$(OBJPATH)\grabber.obj :  grabber.cpp
  $(BCC32) -c @&&|
 $(CCFLAGS)  -o$@ grabber.cpp
|

$(OBJPATH)\scrbuf.obj :  scrbuf.cpp
  $(BCC32) -c @&&|
 $(CCFLAGS)  -o$@ scrbuf.cpp
|

$(OBJPATH)\macro.obj :  macro.cpp
  $(BCC32) -c @&&|
 $(CCFLAGS)  -o$@ macro.cpp
|

$(OBJPATH)\checkver.obj :  checkver.cpp
  $(BCC32) -c @&&|
 $(CCFLAGS)  -o$@ checkver.cpp
|

$(OBJPATH)\namelist.obj :  namelist.cpp
  $(BCC32) -c @&&|
 $(CCFLAGS)  -o$@ namelist.cpp
|

$(OBJPATH)\global.obj :  global.cpp
  $(BCC32) -c @&&|
 $(CCFLAGS)  -o$@ global.cpp
|

$(OBJPATH)\history.obj :  history.cpp
  $(BCC32) -c @&&|
 $(CCFLAGS)  -o$@ history.cpp
|

$(OBJPATH)\iswind.obj :  iswind.cpp
  $(BCC32) -c @&&|
 $(CCFLAGS)  -o$@ iswind.cpp
|

$(OBJPATH)\int64.obj :  int64.cpp
  $(BCC32) -c @&&|
 $(CCFLAGS)  -o$@ int64.cpp
|

$(OBJPATH)\filelist.obj :  filelist.cpp
  $(BCC32) -c @&&|
 $(CCFLAGS)  -o$@ filelist.cpp
|

$(OBJPATH)\treelist.obj :  treelist.cpp
  $(BCC32) -c @&&|
 $(CCFLAGS)  -o$@ treelist.cpp
|

$(OBJPATH)\infolist.obj :  infolist.cpp
  $(BCC32) -c @&&|
 $(CCFLAGS)  -o$@ infolist.cpp
|

$(OBJPATH)\qview.obj :  qview.cpp
  $(BCC32) -c @&&|
 $(CCFLAGS)  -o$@ qview.cpp
|

$(OBJPATH)\editor.obj :  editor.cpp
  $(BCC32) -c @&&|
 $(CCFLAGS)  -o$@ editor.cpp
|

$(OBJPATH)\viewer.obj :  viewer.cpp
  $(BCC32) -c @&&|
 $(CCFLAGS)  -o$@ viewer.cpp
|

$(OBJPATH)\filetype.obj :  filetype.cpp
  $(BCC32) -c @&&|
 $(CCFLAGS)  -o$@ filetype.cpp
|

$(OBJPATH)\dialog.obj :  dialog.cpp
  $(BCC32) -c @&&|
 $(CCFLAGS)  -o$@ dialog.cpp
|

$(OBJPATH)\findfile.obj :  findfile.cpp
  $(BCC32) -c @&&|
 $(CCFLAGS)  -o$@ findfile.cpp
|

$(OBJPATH)\usermenu.obj :  usermenu.cpp
  $(BCC32) -c @&&|
 $(CCFLAGS)  -o$@ usermenu.cpp
|

$(OBJPATH)\copy.obj :  copy.cpp
  $(BCC32) -c @&&|
 $(CCFLAGS)  -o$@ copy.cpp
|

$(OBJPATH)\plist.obj :  plist.cpp
  $(BCC32) -c @&&|
 $(CCFLAGS)  -o$@ plist.cpp
|

$(OBJPATH)\hilight.obj :  hilight.cpp
  $(BCC32) -c @&&|
 $(CCFLAGS)  -o$@ hilight.cpp
|

$(OBJPATH)\filter.obj :  filter.cpp
  $(BCC32) -c @&&|
 $(CCFLAGS)  -o$@ filter.cpp
|

$(OBJPATH)\main.obj :  main.cpp
  $(BCC32) -c @&&|
 $(CCFLAGS)  -o$@ main.cpp
|

$(OBJPATH)\far.res :  far.rc
  $(BRC32) -R @&&|
 $(RESFLAGS)  -FO$@ far.rc
|

# Compiler configuration file
BccW32.cfg :
   Copy &&|
-D$(BETA)=$(BETANUM)
-R
$(OPTDEBUG)
-vi
-K
-d
-f-
-R-
-k-
-pr
-x-
-RT-
-Od
-Og
-Ov
-w-csu
-w-sig
-w-big
-w-pin
-wdef
-wnod
-wuse
-wamp
!ifdef PROCOMP
-Hc
-Hu
-H=$(OBJPATH)\far.csm
!else
-H-
!endif
| $@
