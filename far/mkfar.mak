#
# Компиляция FAR Manager
# Modify: 25.06.2000 SVS
# Modify: 27.06.2000 tran (precompiled headers)
# Modify: 29.06.2000 SVS (добавил +BccW32.cfg -
#                         у некоторых глюки с блинной ком строки)
#                        + файл FAR.EXE создвется в текущем каталоге
#                          после чего копируется в final
# Modify: 04.07.2000 SVS +копирование hlf&lng файлов в final
#                        +вставка переопределенных функцйи запроса памяти
#
# Modify: 12.07.2000 SVS +$(FARCMEM) - для включения в тестовых целях
#                         модуля cmem
# Modify: 06.12.2000 SVS + $(OBJPATH)\mix.obj: mix.cpp cc.bat
#              чтобы ручками не удалять mix.obj, если выставлен флаг SYSLOG
#
# Modify: 30.12.2000 SVS + fileattr
#
# make -fmkfar.mak [options]
#   -DALLOC - вставляет переопределенные функции работы с памятью
# Надо, иначе не будет перекомпиляция при изменении H*-файлов
.AUTODEPEND

# берутся из переменных среды (env)
LIBPATH=$(FARLIB)
INCLUDEPATH=$(FARINC)
BCCPATH=$(FARBCC)
# bccpath нужен для того, чтобы прямо использовать его в exe файлах
# set bccpath=e:\bc5, просто в path может сидеть совсем другой борланд


# путь к каталогу с временными файлами - obj, etc
OBJPATH=OBJ

# сюда будет помещен результат
FINALPATH=final

.path.obj = $(OBJPATH)
.path.cpp = .
.path.c   = .
.path.exe = $(FINALPATH)

!ifdef TRY
OPTEXT=-x -DFARTRY
!else
OPTEXT=-x- -DNDEBUG
!endif

!ifdef DEBUG
OPTDEBUG=-v
OPTDEBUG2=
CSMFILE=fard.csm
!else
OPTDEBUG=-v-
OPTDEBUG2=-DNDEBUG
CSMFILE=Far.csm
!endif


!ifdef PRECOMP
PRECOMPOPT=-H=$(OBJPATH)\$(CSMFILE)
!else
PRECOMPOPT=-H-
!endif

#
# Borland C++ tools
#
BCC32   = $(BCCPATH)\bin\Bcc32 +BccW32.cfg
!ifdef ILINK
TLINK32 = $(BCCPATH)\bin\ilink32
BRC32   = $(BCCPATH)\bin\Brcc32
!else
TLINK32 = $(BCCPATH)\bin\Tlink32
BRC32   = $(BCCPATH)\bin\Brc32
!endif
# TLIB    = $(BCCPATH)\TLib - а зачем он тут нужен?
# IMPLIB  = $(BCCPATH)\bin\Implib - и этот тоже?


#
# Options
#
LINKFLAGS =  -L$(LIBPATH) -Tpe -ap -c $(OPTDEBUG) -s

# а зачем дублировать опции тут и в bccw32.cfg?
#CCFLAGS = -WC -WM -K -d -f- $(OPTDEBUG) -R- -k- -x- -RT -Og -Ot -Z -O -Oe -Ol -Ob -Om -Op -Ov -w-csu $(PRECOMPOPT) -I$(INCLUDEPATH)
# SVS: надо! т.к. проблемы с длиной ком. строки под масдаем
#      пока оставим пустой
CCFLAGS =

# !!! -d - merge duplicate string - вредная опция, иногда сильно мешает

#
# Dependency List
#
Dep_Far = \
   $(FINALPATH)\Far.exe

Far : BccW32.cfg $(Dep_Far)
  echo MakeNode

# все эти зависимости не нужны
# просто описываем одно правило и все %)

.cpp.obj:
  @settitle "{$.} - Compiling..."
  @$(BCC32) -c -o$@ {$. }

.c.obj:
  @settitle "{$.} - Compiling..."
  @$(BCC32) -c -o$@ {$. }

$(OBJPATH)\syslog.obj: syslog.cpp cc.bat
$(OBJPATH)\flink.obj: flink.cpp cc.bat
$(OBJPATH)\copy.obj: copy.cpp cc.bat
$(OBJPATH)\global.obj: global.cpp global.hpp farversion.inc copyright.inc

FAROBJ=\
   $(OBJPATH)\checkver.obj\
   $(OBJPATH)\udlist.obj\
   $(OBJPATH)\FileMasksProcessor.obj\
   $(OBJPATH)\FileMasksWithExclude.obj\
   $(OBJPATH)\CFileMask.obj\
   $(OBJPATH)\chgmmode.obj\
   $(OBJPATH)\chgprior.obj\
   $(OBJPATH)\clipboard.obj\
   $(OBJPATH)\cmdline.obj\
   $(OBJPATH)\config.obj\
   $(OBJPATH)\constitle.obj\
   $(OBJPATH)\copy.obj\
   $(OBJPATH)\ctrlobj.obj\
   $(OBJPATH)\delete.obj\
   $(OBJPATH)\dialog.obj\
   $(OBJPATH)\dizlist.obj\
   $(OBJPATH)\edit.obj\
   $(OBJPATH)\editor.obj\
   $(OBJPATH)\eject.obj\
   $(OBJPATH)\farqueue.obj\
   $(OBJPATH)\farrtl.obj\
   $(OBJPATH)\ffolders.obj\
   $(OBJPATH)\fileattr.obj\
   $(OBJPATH)\fileedit.obj\
   $(OBJPATH)\filelist.obj\
   $(OBJPATH)\filepanels.obj\
   $(OBJPATH)\filestr.obj\
   $(OBJPATH)\filetype.obj\
   $(OBJPATH)\fileview.obj\
   $(OBJPATH)\filter.obj\
   $(OBJPATH)\findfile.obj\
   $(OBJPATH)\flink.obj\
   $(OBJPATH)\flmodes.obj\
   $(OBJPATH)\flplugin.obj\
   $(OBJPATH)\flshow.obj\
   $(OBJPATH)\flupdate.obj\
   $(OBJPATH)\foldtree.obj\
   $(OBJPATH)\gettable.obj\
   $(OBJPATH)\global.obj\
   $(OBJPATH)\grabber.obj\
   $(OBJPATH)\grpsort.obj\
   $(OBJPATH)\help.obj\
   $(OBJPATH)\hilight.obj\
   $(OBJPATH)\history.obj\
   $(OBJPATH)\hmenu.obj\
   $(OBJPATH)\infolist.obj\
   $(OBJPATH)\int64.obj\
   $(OBJPATH)\interf.obj\
   $(OBJPATH)\iswind.obj\
   $(OBJPATH)\keybar.obj\
   $(OBJPATH)\keyboard.obj\
   $(OBJPATH)\language.obj\
   $(OBJPATH)\local.obj\
   $(OBJPATH)\lockscrn.obj\
   $(OBJPATH)\macro.obj\
   $(OBJPATH)\manager.obj\
   $(OBJPATH)\menubar.obj\
   $(OBJPATH)\message.obj\
   $(OBJPATH)\strftime.obj\
   $(OBJPATH)\mix.obj\
   $(OBJPATH)\mkdir.obj\
   $(OBJPATH)\modal.obj\
   $(OBJPATH)\namelist.obj\
   $(OBJPATH)\options.obj\
   $(OBJPATH)\palette.obj\
   $(OBJPATH)\fnparce.obj\
   $(OBJPATH)\panel.obj\
   $(OBJPATH)\plist.obj\
   $(OBJPATH)\plognmn.obj\
   $(OBJPATH)\plugapi.obj\
   $(OBJPATH)\plugins.obj\
   $(OBJPATH)\poscache.obj\
   $(OBJPATH)\print.obj\
   $(OBJPATH)\mktemp.obj\
   $(OBJPATH)\qsortex.obj\
   $(OBJPATH)\qview.obj\
   $(OBJPATH)\rdrwdsk.obj\
   $(OBJPATH)\registry.obj\
   $(OBJPATH)\savefpos.obj\
   $(OBJPATH)\savescr.obj\
   $(OBJPATH)\scantree.obj\
   $(OBJPATH)\scrbuf.obj\
   $(OBJPATH)\scrobj.obj\
   $(OBJPATH)\scrsaver.obj\
   $(OBJPATH)\setattr.obj\
   $(OBJPATH)\setcolor.obj\
   $(OBJPATH)\stddlg.obj\
   $(OBJPATH)\strmix.obj\
   $(OBJPATH)\syslog.obj\
   $(OBJPATH)\treelist.obj\
   $(OBJPATH)\usermenu.obj\
   $(OBJPATH)\viewer.obj\
   $(OBJPATH)\vmenu.obj\
   $(OBJPATH)\frame.obj\
   $(OBJPATH)\xlat.obj\
   $(OBJPATH)\farexcpt.obj\
   $(OBJPATH)\strncpy.obj\
   $(OBJPATH)\main.obj

Dep_fardexe = BccW32.cfg\
   Far.def\
   $(OBJPATH)\Far.res \
   $(FAROBJ)


!ifdef ILINK
$(FINALPATH)\Far.exe : $(Dep_fardexe)
  @settitle "Linking..."
  @$(TLINK32)  $(LINKFLAGS) @&&|
$(LIBPATH)\c0x32.obj $(FAROBJ)
$<,$*
$(LIBPATH)\import32.lib $(LIBPATH)\cw32mt.lib
Far.def
$(OBJPATH)\Far.res
|
!else
$(FINALPATH)\Far.exe : $(Dep_fardexe)
  @settitle "Linking..."
  @$(TLINK32)  $(LINKFLAGS) @&&|
$(LIBPATH)\c0x32.obj $(FAROBJ)
$<,$*
$(LIBPATH)\import32.lib $(LIBPATH)\cw32mt.lib
Far.def
|
   @copy Far.map $(FINALPATH)\Far.map
   @$(BRC32) $(OBJPATH)\Far.res $(OBJPATH)\Far.res $<
!endif

# обязательно! Что бы в ручную не делать...
   @del $(FINALPATH)\FarEng.hlf  >nul
   @del $(FINALPATH)\FarRus.hlf  >nul
   @del $(FINALPATH)\FarEng.lng  >nul
   @del $(FINALPATH)\FarRus.lng  >nul
!ifdef ILINK
   @del $(FINALPATH)\Far.ilc  >nul
   @del $(FINALPATH)\Far.ild  >nul
   @del $(FINALPATH)\Far.ilf  >nul
   @del $(FINALPATH)\Far.ils  >nul
   @del $(FINALPATH)\Far.tds  >nul
!endif
   @awk -f mkhlf.awk -v FV1=$(FV1) -v FV2=$(FV2) -v FV3=$(FV3) FarEng.hlf > $(FINALPATH)\FarEng.hlf
   @awk -f mkhlf.awk -v FV1=$(FV1) -v FV2=$(FV2) -v FV3=$(FV3) FarRus.hlf > $(FINALPATH)\FarRus.hlf
   @copy FarEng.lng $(FINALPATH)\FarEng.lng >nul
   @copy FarRus.lng $(FINALPATH)\FarRus.lng >nul

$(OBJPATH)\Far.res :  Far.rc
  @settitle "Compiling resource..."
  $(BRC32) -R @&&|
 $(RESFLAGS)  -FO$@ Far.rc
|

# Compiler configuration file
# Для тех, у кого длина ком. строки ограничена
BccW32.cfg : mkfar.mak cc.bat
   Copy &&|
-WC
-WM
-K
-d
-f-
-R-
-k-
-RT
-Og
-Ot
-Z
-a8
-O
-Oe
-Ol
-Ob
-Om
-Op
-Ov
-w-csu
-I$(INCLUDEPATH)
$(FARCMEM)
$(FARALLOC)
$(PRECOMPOPT)
$(OPTDEBUG)
$(OPTDEBUG2)
$(OPTEXT)
$(FARSYSLOG)
$(FARADDMACRO)
$(CREATE_JUNCTION)
| $@
