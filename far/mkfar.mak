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
.path.exe = $(FINALPATH)

!ifdef TRY
OPTEXT=-x
!else
OPTEXT=-x-
!endif

!ifdef DEBUG
OPTDEBUG=-v
CSMFILE=fard.csm
!else
OPTDEBUG=-v-
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

$(OBJPATH)\ctrlobj.obj: ctrlobj.cpp ctrlobj.hpp copyright.inc
$(OBJPATH)\syslog.obj: syslog.cpp cc.bat
$(OBJPATH)\global.obj: global.cpp global.hpp farversion.inc

FAROBJ=\
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
   $(OBJPATH)\keyboard.obj\
   $(OBJPATH)\clipboard.obj\
   $(OBJPATH)\eject.obj\
   $(OBJPATH)\xlat.obj\
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
   $(OBJPATH)\farrtl.obj\
   $(OBJPATH)\syslog.obj\
   $(OBJPATH)\fileattr.obj\
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
   @copy FarEng.hlf $(FINALPATH)\FarEng.hlf >nul
   @copy FarRus.hlf $(FINALPATH)\FarRus.hlf >nul
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
$(OPTEXT)
$(FARSYSLOG)
| $@
