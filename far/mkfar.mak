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
#   -DUSEMSRC - заставляем использовать rc из sdk
# Надо, иначе не будет перекомпиляция при изменении H*-файлов
.AUTODEPEND

# берутся из переменных среды (env)
LIBPATH=$(FARLIB)
INCLUDEPATH=$(FARINC)
BCCPATH=$(FARBCC)
# bccpath нужен для того, чтобы прямо использовать его в exe файлах
# set bccpath=e:\bc5, просто в path может сидеть совсем другой борланд

!ifdef DEBUG
OUTPATH=Debug.bcc
!else
OUTPATH=Release.bcc
!endif

# путь для публичных HPP-файлов
FARINCLUDE=Include

# сюда будет помещен результат
FINALPATH=$(OUTPATH)

# путь к каталогу с временными файлами - obj, etc
OBJPATH=$(OUTPATH)\OBJ

OUTDIR=$(OUTPATH)

.path.obj = $(OBJPATH)
.path.cpp = .
.path.c   = .
.path.exe = $(FINALPATH)

!ifdef DEBUG
OPTDEBUG=-v -R -y
OPTDEBUG2=
CSMFILE=Far.csm
OPTLINKDEBUG=-v
# FAR_STDHDR_OBJ - это как раз тот файл, в котором все говно по DEBUG инфе из стд.хёдеров
FAR_STDHDR_OBJ="$(OBJPATH)\Far.#00"
!else
OPTDEBUG=-v- -R- -y-
OPTDEBUG2=-DNDEBUG
CSMFILE=Far.csm
OPTLINKDEBUG=-v-
FAR_STDHDR_OBJ=
!endif



!ifdef PRECOMP
PRECOMPOPT=-H -H=$(OBJPATH)\$(CSMFILE)
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

!ifdef USEMSRC
BRC32   = $(BCCPATH)\bin\rc
RESFLAGS=/D__FARBIT__=32 /l 0x409 /d NDEBUG -i$(INCLUDEPATH)
!else
RESFLAGS = -i$(INCLUDEPATH) -R
!endif

#
# Options
#
!ifdef ILINK
LINKFLAGS =  -L$(LIBPATH) -Tpe -ap -c $(OPTLINKDEBUG) -s -V4.0 -j.\$(OBJPATH) -I.\$(OBJPATH) -M -Gz
!else
LINKFLAGS =  -L$(LIBPATH) -Tpe -ap -c $(OPTLINKDEBUG) -s -V4.0 -j.\$(OBJPATH)
!endif


CCFLAGS =

DEPFILE=far.bc.dep

FAROBJ=\
   $(OBJPATH)\checkver.obj\
   $(OBJPATH)\udlist.obj\
   $(OBJPATH)\FileMasksProcessor.obj\
   $(OBJPATH)\FileMasksWithExclude.obj\
   $(OBJPATH)\CFileMask.obj\
   $(OBJPATH)\chgmmode.obj\
   $(OBJPATH)\RefreshFrameManager.obj\
   $(OBJPATH)\chgprior.obj\
   $(OBJPATH)\clipboard.obj\
   $(OBJPATH)\cddrv.obj\
   $(OBJPATH)\cmdline.obj\
   $(OBJPATH)\config.obj\
   $(OBJPATH)\constitle.obj\
   $(OBJPATH)\copy.obj\
   $(OBJPATH)\ctrlobj.obj\
   $(OBJPATH)\delete.obj\
   $(OBJPATH)\dialog.obj\
   $(OBJPATH)\dizlist.obj\
   $(OBJPATH)\edit.obj\
   $(OBJPATH)\dlgedit.obj\
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
   $(OBJPATH)\findfile.obj\
   $(OBJPATH)\flink.obj\
   $(OBJPATH)\flmodes.obj\
   $(OBJPATH)\flplugin.obj\
   $(OBJPATH)\flshow.obj\
   $(OBJPATH)\flupdate.obj\
   $(OBJPATH)\foldtree.obj\
   $(OBJPATH)\gettable.obj\
   $(OBJPATH)\cvtname.obj\
   $(OBJPATH)\global.obj\
   $(OBJPATH)\grabber.obj\
   $(OBJPATH)\help.obj\
   $(OBJPATH)\hilight.obj\
   $(OBJPATH)\history.obj\
   $(OBJPATH)\hmenu.obj\
   $(OBJPATH)\hotplug.obj\
   $(OBJPATH)\infolist.obj\
   $(OBJPATH)\interf.obj\
   $(OBJPATH)\iswind.obj\
   $(OBJPATH)\keybar.obj\
   $(OBJPATH)\keyboard.obj\
   $(OBJPATH)\language.obj\
   $(OBJPATH)\local.obj\
   $(OBJPATH)\lockscrn.obj\
   $(OBJPATH)\macro.obj\
   $(OBJPATH)\manager.obj\
   $(OBJPATH)\filefilter.obj\
   $(OBJPATH)\filefilterparams.obj\
   $(OBJPATH)\menubar.obj\
   $(OBJPATH)\message.obj\
   $(OBJPATH)\strftime.obj\
   $(OBJPATH)\mix.obj\
   $(OBJPATH)\mkdir.obj\
   $(OBJPATH)\timefunc.obj\
   $(OBJPATH)\modal.obj\
   $(OBJPATH)\namelist.obj\
   $(OBJPATH)\options.obj\
   $(OBJPATH)\palette.obj\
   $(OBJPATH)\execute.obj\
   $(OBJPATH)\fnparce.obj\
   $(OBJPATH)\panel.obj\
   $(OBJPATH)\plist.obj\
   $(OBJPATH)\plognmn.obj\
   $(OBJPATH)\plugapi.obj\
   $(OBJPATH)\plugins.obj\
   $(OBJPATH)\poscache.obj\
   $(OBJPATH)\print.obj\
   $(OBJPATH)\TPreRedrawFunc.obj\
   $(OBJPATH)\mktemp.obj\
   $(OBJPATH)\qsortex.obj\
   $(OBJPATH)\qview.obj\
   $(OBJPATH)\rdrwdsk.obj\
   $(OBJPATH)\fileowner.obj\
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
   $(OBJPATH)\farwinapi.obj\
   $(OBJPATH)\vmenu.obj\
   $(OBJPATH)\frame.obj\
   $(OBJPATH)\xlat.obj\
   $(OBJPATH)\farexcpt.obj\
   $(OBJPATH)\strdup.obj\
   $(OBJPATH)\new.obj\
   $(OBJPATH)\del.obj\
   $(OBJPATH)\strncat.obj\
   $(OBJPATH)\strncpy.obj\
   $(OBJPATH)\qsort.obj\
   $(OBJPATH)\cmem.obj\
   $(OBJPATH)\syntax.obj\
   $(OBJPATH)\tvar.obj\
   $(OBJPATH)\TVMStack.obj\
   $(OBJPATH)\main.obj


# ************************************************************************
ALL :  AllDirs lng depfile $(FINALPATH)\Far.exe
	@echo MakeNode

.PHONY: lng
lng: farlang.templ
	@echo generating language files
	@tools\lng.generator.exe -nc -i lang.ini -ol "$(OUTPATH)" farlang.templ

.PHONY: depfile
depfile:
	@tools\gawk -f .\scripts\mkdep.awk mkdep.list > $(DEPFILE)

.PHONY: AllDirs
AllDirs:
	@if not exist "$(OUTPATH)\$(NULL)" mkdir "$(OUTPATH)"
	@if not exist "$(FARINCLUDE)\$(NULL)" mkdir "$(FARINCLUDE)"
	@if not exist "$(OBJPATH)\$(NULL)" mkdir "$(OBJPATH)"


# все эти зависимости не нужны
# просто описываем одно правило и все %)
.cpp.obj:
	-@settitle "{$.} - Compiling..."
	@if not exist $(OBJPATH) mkdir $(OBJPATH)
	@$(BCC32) -c -o$@ {$. }

.c.obj:
	-@settitle "{$.} - Compiling..."
	@if not exist $(OBJPATH) mkdir $(OBJPATH)
	@$(BCC32) -c -o$@ {$. }

# уточнения
$(OBJPATH)\syslog.obj: syslog.cpp cc.bat
$(OBJPATH)\flink.obj: flink.cpp cc.bat
$(OBJPATH)\copy.obj: copy.cpp cc.bat
$(OBJPATH)\global.obj: global.cpp global.hpp farversion.inc copyright.inc


copyright.inc: copyright.inc.m4 farversion.m4 tools.m4 vbuild.m4
	@echo generating $@
	@tools\m4 -P copyright.inc.m4 > copyright.inc.temp
	@tools\gawk -f .\scripts\enc.awk copyright.inc.temp > copyright.inc

farversion.inc: farversion.inc.m4 farversion.m4 tools.m4 vbuild.m4
	@echo generating $@
	@tools\m4 -P farversion.inc.m4 > farversion.inc

Far.exe.manifest: Far.exe.manifest.m4 farversion.m4 tools.m4 vbuild.m4
	@echo generating $@
	@tools\m4 -P Far.exe.manifest.m4 > Far.exe.manifest

far.rc: far.rc.m4 farversion.m4 tools.m4 vbuild.m4 Far.exe.manifest res.hpp
	@echo generating $@
	@tools\m4 -P far.rc.m4 > far.rc

farlang.templ: farlang.templ.m4 farversion.m4 tools.m4 vbuild.m4
	@echo generating $@
	@tools\m4 -P farlang.templ.m4 > farlang.templ

$(FARINCLUDE)\farcolor.hpp: colors.hpp farversion.m4 tools.m4 vbuild.m4
	@echo generating $@
	@tools\m4 -P -DINPUT=colors.hpp headers.m4 > "$(FARINCLUDE)\farcolor.hpp"

$(FARINCLUDE)\farkeys.hpp: keys.hpp farversion.m4 tools.m4 vbuild.m4
	@echo generating $@
	@tools\m4 -P -DINPUT=keys.hpp headers.m4   > "$(FARINCLUDE)\farkeys.hpp"

$(FARINCLUDE)\plugin.hpp: plugin.hpp farversion.m4 tools.m4 vbuild.m4
	@echo generating $@
	@tools\m4 -P -DINPUT=plugin.hpp headers.m4 > "$(FARINCLUDE)\plugin.hpp"

"$(OUTDIR)\FarEng.hlf": FarEng.hlf.m4 farversion.m4 tools.m4 vbuild.m4
	@echo generating $@
	@tools\gawk -f .\scripts\mkhlf.awk FarEng.hlf.m4 | tools\m4 -P > "$(OUTDIR)\FarEng.hlf"

"$(OUTDIR)\FarRus.hlf": FarRus.hlf.m4 farversion.m4 tools.m4 vbuild.m4
	@echo generating $@
	@tools\gawk -f .\scripts\mkhlf.awk FarRus.hlf.m4 | tools\m4 -P > "$(OUTDIR)\FarRus.hlf"



# ************************************************************************
$(OBJPATH)\Far.res :  far.rc res.hpp Far.ico
	@echo Compiling resource...
	@if not exist $(OBJPATH) mkdir $(OBJPATH)
	$(BRC32) $(RESFLAGS) /fo$@ far.rc

!ifdef ILINK
$(FINALPATH)\Far.exe : BccW32.cfg Far.def $(OBJPATH)\Far.res $(FAROBJ) Far.exe.manifest copyright.inc farversion.inc "$(OUTDIR)\FarEng.hlf" "$(OUTDIR)\FarRus.hlf"
	-@settitle "Linking..."
	@if not exist $(OUTPATH) mkdir $(OUTPATH)
	@if not exist $(FINALPATH) mkdir $(FINALPATH)
	@if not exist $(FARINCLUDE) mkdir $(FARINCLUDE)
	@$(TLINK32)  $(LINKFLAGS) @&&|
$(LIBPATH)\c0x32.obj $(FAROBJ) $(FAR_STDHDR_OBJ)
$<,$*
$(LIBPATH)\import32.lib $(LIBPATH)\cw32mt.lib
Far.def
$(OBJPATH)\Far.res
|
!else
$(FINALPATH)\Far.exe : BccW32.cfg Far.def $(OBJPATH)\Far.res $(FAROBJ) Far.exe.manifest far.rc copyright.inc farversion.inc "$(OUTDIR)\FarEng.hlf" "$(OUTDIR)\FarRus.hlf"
	-@settitle "Linking..."
	@if not exist $(OUTPATH) mkdir $(OUTPATH)
	@if not exist $(FINALPATH) mkdir $(FINALPATH)
	@if not exist $(FARINCLUDE) mkdir $(FARINCLUDE)
	@$(TLINK32)  $(LINKFLAGS) @&&|
$(LIBPATH)\c0x32.obj $(FAROBJ) $(FAR_STDHDR_OBJ)
$<,$*
$(LIBPATH)\import32.lib $(LIBPATH)\cw32mt.lib
Far.def
|
	@copy Far.map $(FINALPATH)\Far.map
	@$(BRC32) $(OBJPATH)\Far.res $(OBJPATH)\Far.res $<
!endif

# обязательно! Что бы в ручную не делать...
	-@if not exist $(FINALPATH)\FarEng.lng del $(FINALPATH)\FarEng.lng  >nul
	-@if not exist $(FINALPATH)\FarRus.lng del $(FINALPATH)\FarRus.lng  >nul
	-@copy FarEng.lng $(FINALPATH)\FarEng.lng >nul
	-@copy FarRus.lng $(FINALPATH)\FarRus.lng >nul


# Compiler configuration file
# Для тех, у кого длина ком. строки ограничена
BccW32.cfg : mkfar.mak cc.bat
	Copy &&|
-WC
-WM
-K
-d
-f-
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
-w-inl
-x
-I$(INCLUDEPATH)
$(USE_WFUNC)
$(FARCMEM)
$(FARALLOC)
$(PRECOMPOPT)
$(OPTDEBUG)
$(OPTDEBUG2)
$(FARSYSLOG)
| $@

# ************************************************************************
# Очистка
# ************************************************************************
CLEAN :
	-@del /q /f $(OBJPATH)\*.*           > nul
	-@del /q /f lang.hpp                 > nul
	-@del /q /f FarEng.lng               > nul
	-@del /q /f FarRus.lng               > nul
	-@del /q /f $(FINALPATH)\Far.exe     > nul
	-@del /q /f $(FINALPATH)\Far.map     > nul
	-@del /q /f $(FINALPATH)\FarEng.hlf  > nul
	-@del /q /f $(FINALPATH)\FarEng.lng  > nul
	-@del /q /f $(FINALPATH)\FarRus.hlf  > nul
	-@del /q /f $(FINALPATH)\FarRus.lng  > nul
