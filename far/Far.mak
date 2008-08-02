# Microsoft Developer Studio Generated NMAKE File, Based on far.dsp
!IF "$(CFG)" == ""
CFG=far - Win32 Release
!MESSAGE No configuration specified. Defaulting to far - Win32 Release.
!ENDIF

!IF "$(CFG)" != "far - Win32 Release" && "$(CFG)" != "far - Win32 Debug" && "$(CFG)" != "far - Win64 Release" && "$(CFG)" != "far - Win64 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE
!MESSAGE NMAKE /f "far.mak" CFG="far - Win32 Debug"
!MESSAGE
!MESSAGE Possible choices for configuration are:
!MESSAGE
!MESSAGE "far - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "far - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE
!ERROR An invalid configuration is specified.
!ENDIF

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE
NULL=nul
!ENDIF

NODEFAULTLIB=

CPP=cl.exe
RSC=rc.exe
LINK32=link.exe
ULINK=ulink.exe

# Пути
FARINCLUDE=.\Include

_BUILD64=0
!IF  "$(CFG)" == "far - Win32 Release"
OUTDIR=.\Release.vc
!ELSEIF  "$(CFG)" == "far - Win32 Debug"
OUTDIR=.\Debug.vc
!ELSEIF  "$(CFG)" == "far - Win64 Release"
OUTDIR=.\Release.64.vc
_BUILD64=1
!ELSEIF  "$(CFG)" == "far - Win64 Debug"
OUTDIR=.\Debug.64.vc
_BUILD64=1
!ENDIF

!if $(_BUILD64)
ML=ml64.exe
!else
ML=ml.exe
!endif

INTDIR=$(OUTDIR)\obj
CODDIR=$(OUTDIR)\cod

!ifdef FARSYSLOG
INTDIR=$(OUTDIR)\objlog
!else
INTDIR=$(OUTDIR)\obj
!endif
CODDIR=$(OUTDIR)\cod

DEF_FILE= \
          ".\far.def"

RES_FILES= \
           "$(INTDIR)\far.res"

PCH_FILES= \
           "$(INTDIR)\headers.c.pch"\
           "$(INTDIR)\headers.cpp.pch"
           
# Сюды добавлять то, что должно быть в проекте, в смысле сорцы
LINK32_OBJS= \
!if !$(_BUILD64) && !defined(DISABLE_WOW64_HOOK)
	"$(INTDIR)\hook_wow64.obj" \
!endif
	"$(INTDIR)\cddrv.obj" \
	"$(INTDIR)\CFileMask.obj" \
	"$(INTDIR)\chgmmode.obj" \
	"$(INTDIR)\chgprior.obj" \
	"$(INTDIR)\clipboard.obj" \
	"$(INTDIR)\cmdline.obj" \
	"$(INTDIR)\cmem.obj" \
	"$(INTDIR)\config.obj" \
	"$(INTDIR)\constitle.obj" \
	"$(INTDIR)\copy.obj" \
	"$(INTDIR)\ctrlobj.obj" \
	"$(INTDIR)\cvtname.obj" \
	"$(INTDIR)\del.obj" \
	"$(INTDIR)\delete.obj" \
	"$(INTDIR)\dialog.obj" \
	"$(INTDIR)\dizlist.obj" \
	"$(INTDIR)\dlgedit.obj" \
	"$(INTDIR)\edit.obj" \
	"$(INTDIR)\editor.obj" \
	"$(INTDIR)\eject.obj" \
	"$(INTDIR)\hotplug.obj" \
	"$(INTDIR)\execute.obj" \
	"$(INTDIR)\farexcpt.obj" \
	"$(INTDIR)\farqueue.obj" \
	"$(INTDIR)\farrtl.obj" \
	"$(INTDIR)\farwinapi.obj" \
	"$(INTDIR)\ffolders.obj" \
	"$(INTDIR)\fileattr.obj" \
	"$(INTDIR)\fileedit.obj" \
	"$(INTDIR)\filefilter.obj" \
	"$(INTDIR)\filefilterparams.obj" \
	"$(INTDIR)\filelist.obj" \
	"$(INTDIR)\FileMasksProcessor.obj" \
	"$(INTDIR)\FileMasksWithExclude.obj" \
	"$(INTDIR)\fileowner.obj" \
	"$(INTDIR)\filepanels.obj" \
	"$(INTDIR)\filestr.obj" \
	"$(INTDIR)\filetype.obj" \
	"$(INTDIR)\fileview.obj" \
	"$(INTDIR)\findfile.obj" \
	"$(INTDIR)\flink.obj" \
	"$(INTDIR)\flmodes.obj" \
	"$(INTDIR)\flplugin.obj" \
	"$(INTDIR)\flshow.obj" \
	"$(INTDIR)\flupdate.obj" \
	"$(INTDIR)\fnparce.obj" \
	"$(INTDIR)\foldtree.obj" \
	"$(INTDIR)\frame.obj" \
	"$(INTDIR)\gettable.obj" \
	"$(INTDIR)\global.obj" \
	"$(INTDIR)\grabber.obj" \
	"$(INTDIR)\headers.c.obj" \
	"$(INTDIR)\headers.cpp.obj" \
	"$(INTDIR)\help.obj" \
	"$(INTDIR)\hilight.obj" \
	"$(INTDIR)\history.obj" \
	"$(INTDIR)\hmenu.obj" \
	"$(INTDIR)\infolist.obj" \
	"$(INTDIR)\interf.obj" \
	"$(INTDIR)\iswind.obj" \
	"$(INTDIR)\keybar.obj" \
	"$(INTDIR)\keyboard.obj" \
	"$(INTDIR)\language.obj" \
	"$(INTDIR)\local.obj" \
	"$(INTDIR)\lockscrn.obj" \
	"$(INTDIR)\macro.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\manager.obj" \
!if $(_BUILD64)
	"$(INTDIR)\deb64_ud2.obj" \
!endif
	"$(INTDIR)\menubar.obj" \
	"$(INTDIR)\message.obj" \
	"$(INTDIR)\mix.obj" \
	"$(INTDIR)\mkdir.obj" \
	"$(INTDIR)\mktemp.obj" \
	"$(INTDIR)\modal.obj" \
	"$(INTDIR)\namelist.obj" \
	"$(INTDIR)\new.obj" \
	"$(INTDIR)\options.obj" \
	"$(INTDIR)\palette.obj" \
	"$(INTDIR)\panel.obj" \
	"$(INTDIR)\plist.obj" \
	"$(INTDIR)\plognmn.obj" \
	"$(INTDIR)\plugapi.obj" \
	"$(INTDIR)\plugins.obj" \
	"$(INTDIR)\PluginA.obj" \
	"$(INTDIR)\PluginW.obj" \
	"$(INTDIR)\poscache.obj" \
	"$(INTDIR)\print.obj" \
	"$(INTDIR)\qsortex.obj" \
	"$(INTDIR)\qsort.obj" \
	"$(INTDIR)\qview.obj" \
	"$(INTDIR)\rdrwdsk.obj" \
	"$(INTDIR)\RefreshFrameManager.obj" \
	"$(INTDIR)\registry.obj" \
	"$(INTDIR)\savefpos.obj" \
	"$(INTDIR)\savescr.obj" \
	"$(INTDIR)\scantree.obj" \
	"$(INTDIR)\scrbuf.obj" \
	"$(INTDIR)\scrobj.obj" \
	"$(INTDIR)\scrsaver.obj" \
	"$(INTDIR)\setattr.obj" \
	"$(INTDIR)\setcolor.obj" \
	"$(INTDIR)\stddlg.obj" \
	"$(INTDIR)\strdup.obj" \
	"$(INTDIR)\strftime.obj" \
	"$(INTDIR)\strmix.obj" \
	"$(INTDIR)\strncat.obj" \
	"$(INTDIR)\strncpy.obj" \
	"$(INTDIR)\syntax.obj" \
	"$(INTDIR)\tvar.obj" \
	"$(INTDIR)\TVMStack.obj" \
	"$(INTDIR)\syslog.obj" \
	"$(INTDIR)\treelist.obj" \
	"$(INTDIR)\udlist.obj" \
	"$(INTDIR)\UnicodeString.obj" \
	"$(INTDIR)\usermenu.obj" \
	"$(INTDIR)\viewer.obj" \
	"$(INTDIR)\vmenu.obj" \
	"$(INTDIR)\xlat.obj"

!ifndef USE_VC9
COMPAT64=/Wp64
NOWIN98=/OPT:NOWIN98
!else
!undef USE_VC8_32
USE_VC8_32=1
!endif

!ifndef USE_VC8_32
CPP_ADD_32=/Gi
!else
CPP_ADD_32=/GS- /GR-
!endif

LINK32_LIBS=kernel32.lib user32.lib winspool.lib advapi32.lib shell32.lib mpr.lib

RSC_PROJ=/l 0x409 /fo"$(INTDIR)\far.res" /d $(USEDEBUG)

ULINK_MODES=-q -m- -ap -Gz -O- -o- -Gh -Gh-

DEFINES=\
	/D $(USEDEBUG)\
	/D "WIN32"\
	/D "_CONSOLE"\
	/D "_CRT_SECURE_NO_DEPRECATE"\
	/D "_CRT_NONSTDC_NO_DEPRECATE"\
	/D "_CRT_NON_CONFORMING_SWPRINTFS"

CPP_PROJ_COMMON=$(MP) $(COMPAT64) /W3 /nologo $(FAR_ALPHA_VERSION) $(FARSYSLOG) $(FARTRY) $(DEFINES) /Gy /GF /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /J /c $(FARCMEM) $(FARALLOC) /FAcs /Fa"$(CODDIR)\\"
CPP_PROJ_RELEASE=/MT /O1
CPP_PROJ_DEBUG=/MTd /Od /Zi 

LINK_COMMON=$(LINK32_LIBS) /OPT:REF /OPT:ICF $(NOWIN98) /nologo /subsystem:console /def:"$(DEF_FILE)" /out:"$(OUTDIR)\Far.exe" /map:"$(OUTDIR)\far.map" $(NODEFAULTLIB) /pdb:"$(OUTDIR)\far.pdb" /release

!MESSAGE $(CFG).

!IF  "$(CFG)" == "far - Win32 Release"

USEDEBUG=NDEBUG

CPP_PROJ=$(CPP_PROJ_COMMON) $(CPP_PROJ_RELEASE) $(CPP_ADD_32) /Zp4
LINK32_FLAGS=$(LINK_COMMON) /incremental:no /machine:i386

!ELSEIF "$(CFG)" == "far - Win64 Release"

USEDEBUG=NDEBUG

CPP_PROJ=$(CPP_PROJ_COMMON) $(CPP_PROJ_RELEASE) /GS- /GR- /Zp8
LINK32_FLAGS=$(LINK_COMMON) /incremental:no /machine:amd64
ULINK_FLAGS=-Tpe+

!ELSEIF "$(CFG)" == "far - Win64 Debug"

USEDEBUG=_DEBUG

CPP_PROJ=$(CPP_PROJ_COMMON) $(CPP_PROJ_DEBUG) /GS- /GR-
LINK32_FLAGS=$(LINK_COMMON) /debug /machine:amd64
ULINK_FLAGS=-Tpe+ -v

!ELSE

USEDEBUG=_DEBUG

CPP_PROJ=$(CPP_PROJ_COMMON) $(CPP_PROJ_DEBUG) $(CPP_ADD_32)
LINK32_FLAGS=$(LINK_COMMON) /debug /machine:I386
ULINK_FLAGS=-v

!ENDIF

ALL : AllDirs \
      "$(OUTDIR)\Far.exe" "$(FARINCLUDE)\farcolor.hpp" "$(FARINCLUDE)\farkeys.hpp" "$(FARINCLUDE)\plugin.hpp"

.PHONY: AllDirs

AllDirs:
        @if not exist "$(OUTDIR)\$(NULL)" mkdir "$(OUTDIR)"
	@if not exist "$(FARINCLUDE)\$(NULL)" mkdir "$(FARINCLUDE)"
	@if not exist "$(INTDIR)\$(NULL)" mkdir "$(INTDIR)"
	@if not exist "$(CODDIR)\$(NULL)" mkdir "$(CODDIR)"
	
"$(OUTDIR)\Far.exe" : $(DEF_FILE) $(PCH_FILES) $(LINK32_OBJS) $(RES_FILES)
!IFNDEF LINK_ULINK
	$(LINK32) @<<
	$(LINK32_FLAGS) $(LINK32_OBJS) $(RES_FILES)
<<
!ELSE
        $(ULINK) +- @+<<
        $(ULINK_MODES) $(ULINK_FLAGS) $(LINK32_OBJS)
        ,"$(OUTDIR)\Far.exe","$(OUTDIR)\Far.map"
        ,$(LINK32_LIBS)
        ,"$(DEF_FILE)","$(RES_FILES)"
<<
!ENDIF
	@if exist "$(OUTDIR)\FarEng.hlf" del "$(OUTDIR)\FarEng.hlf" >nul
	@if exist "$(OUTDIR)\FarRus.hlf" del "$(OUTDIR)\FarRus.hlf"  >nul
	@if exist "$(OUTDIR)\FarEng.lng" del "$(OUTDIR)\FarEng.lng"  >nul
	@if exist "$(OUTDIR)\FarRus.lng" del "$(OUTDIR)\FarRus.lng"  >nul
	@if exist "$(OUTDIR)\File_id.diz" del "$(OUTDIR)\File_id.diz"  >nul
	@tools\gawk -f .\scripts\mkhlf.awk FarEng.hlf.m4 | tools\m4 -P > "$(OUTDIR)\FarEng.hlf"
	@tools\gawk -f .\scripts\mkhlf.awk FarRus.hlf.m4 | tools\m4 -P > "$(OUTDIR)\FarRus.hlf"
	@copy ".\FarEng.lng" "$(OUTDIR)\FarEng.lng" >nul
	@copy ".\FarRus.lng" "$(OUTDIR)\FarRus.lng" >nul
	@tools\m4 -P File_id.diz.m4 > "$(OUTDIR)\File_id.diz"
	@if exist ".\plugin.pas" tools\m4 -P -DINPUT=plugin.pas headers.m4 > "$(FARINCLUDE)\plugin.pas"
	@if exist ".\farcolor.pas" tools\m4 -P -DINPUT=farcolor.pas headers.m4 > "$(FARINCLUDE)\farcolor.pas"
	@if exist ".\farkeys.pas" tools\m4 -P -DINPUT=farkeys.pas headers.m4 > "$(FARINCLUDE)\farkeys.pas"


# ************************************************************************
# зависимости
# ************************************************************************
"$(INTDIR)\headers.c.pch" : headers.c.c
	@echo making precompiled headers for C
	@$(CPP) $(CPP_PROJ) headers.c.c /Yc /Fp"$(INTDIR)\headers.c.pch" > nul

"$(INTDIR)\headers.cpp.pch" : headers.cpp.cpp
	@echo making precompiled headers for C++
	@$(CPP) $(CPP_PROJ) headers.cpp.cpp /Yc /Fp"$(INTDIR)\headers.cpp.pch" > nul

.c{$(INTDIR)}.obj::
	$(CPP) @<<
	$(CPP_PROJ) /Yuheaders.hpp /Fp"$(INTDIR)\headers.c.pch" $<
<<

.cpp{$(INTDIR)}.obj::
	$(CPP) @<<
	$(CPP_PROJ) /Yuheaders.hpp /Fp"$(INTDIR)\headers.cpp.pch" $<
<<


.asm{$(INTDIR)}.obj:
!if $(_BUILD64)
	$(ML) /c /Fo$@ $<
!else
	$(ML) /c /coff /Fo$@ $<
!endif

"$(INTDIR)\far.res" : far.rc
	$(RSC) $(RSC_PROJ) far.rc

".\copyright.inc": copyright.inc.m4 farversion.m4 vbuild.m4
	@tools\m4 -P copyright.inc.m4 | tools\gawk -f .\scripts\enc.awk > copyright.inc

".\farversion.inc": farversion.inc.m4 farversion.m4 vbuild.m4
	@tools\m4 -P farversion.inc.m4 > farversion.inc

"Far.exe.manifest": Far.exe.manifest.m4 farversion.m4 vbuild.m4
	@tools\m4 -P Far.exe.manifest.m4 > Far.exe.manifest

"far.rc": far.rc.m4 farversion.m4 vbuild.m4 Far.exe.manifest
	@tools\m4 -P far.rc.m4 > far.rc

# ************************************************************************
# Зависимости для публичных файлов
# ************************************************************************
"$(FARINCLUDE)\farcolor.hpp" : colors.hpp farversion.m4 vbuild.m4
	@tools\m4 -P -DINPUT=colors.hpp headers.m4 > "$(FARINCLUDE)\farcolor.hpp"

"$(FARINCLUDE)\farkeys.hpp" : keys.hpp farversion.m4 vbuild.m4
	@tools\m4 -P -DINPUT=keys.hpp headers.m4   > "$(FARINCLUDE)\farkeys.hpp"

"$(FARINCLUDE)\plugin.hpp" : plugin.hpp farversion.m4 vbuild.m4
	@tools\m4 -P -DINPUT=plugin.hpp headers.m4 > "$(FARINCLUDE)\plugin.hpp"


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF  "$(CFG)" == "far - Win32 Release" || "$(CFG)" == "far - Win64 Release"
!IF EXISTS("far.release.dep")
!INCLUDE "far.release.dep"
!ELSE
!MESSAGE Warning: cannot find "far.release.dep", run mkdep.cmd
!ENDIF
!ELSE
!IF EXISTS("far.debug.dep")
!INCLUDE "far.debug.dep"
!ELSE
!MESSAGE Warning: cannot find "far.debug.dep", run mkdep.cmd
!ENDIF
!ENDIF
!ENDIF

# ************************************************************************
# Очистка
# ************************************************************************
CLEAN :
	-@del /q /f "$(INTDIR)\*.*"          > nul
	-@del /q /f "$(CODDIR)\*.*"          > nul
	-@del /q /f ".\lang.hpp"             > nul
	-@del /q /f ".\FarEng.lng"           > nul
	-@del /q /f ".\FarRus.lng"           > nul
	-@del /q /f "$(OUTDIR)\Far.exe"      > nul
	-@del /q /f "$(OUTDIR)\Far.exp"      > nul
	-@del /q /f "$(OUTDIR)\Far.lib"      > nul
	-@del /q /f "$(OUTDIR)\Far.map"      > nul
	-@del /q /f "$(OUTDIR)\FarEng.hlf"   > nul
	-@del /q /f "$(OUTDIR)\FarEng.lng"   > nul
	-@del /q /f "$(OUTDIR)\FarRus.hlf"   > nul
	-@del /q /f "$(OUTDIR)\FarRus.lng"   > nul
