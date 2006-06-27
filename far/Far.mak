# Microsoft Developer Studio Generated NMAKE File, Based on far.dsp
!IF "$(CFG)" == ""
CFG=far - Win32 Release
!MESSAGE No configuration specified. Defaulting to far - Win32 Release.
!ENDIF

!IF "$(CFG)" != "far - Win32 Release" && "$(CFG)" != "far - Win32 Debug"
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

!IF "$(FAR_MSVCRT)" != "/D FAR_MSVCRT"
MT=/MT
NODEFAULTLIB=
!ELSE
MT=/MD
NODEFAULTLIB=
#/NODEFAULTLIB:
!ENDIF

CPP=cl.exe
RSC=rc.exe
LINK32=link.exe
BSC32=bscmake.exe

# ����
FARINCLUDE=.\Include

!IF  "$(CFG)" == "far - Win32 Release"
OUTDIR=.\Release.vc
!ELSEIF  "$(CFG)" == "far - Win32 Debug"
OUTDIR=.\Debug.vc
!ENDIF

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

# ���� ��������� ��, ��� ������ ���� � �������, � ������ �����
LINK32_OBJS= \
	"$(INTDIR)\cddrv.obj" \
	"$(INTDIR)\CFileMask.obj" \
	"$(INTDIR)\checkver.obj" \
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
	"$(INTDIR)\filelist.obj" \
	"$(INTDIR)\FileMasksProcessor.obj" \
	"$(INTDIR)\FileMasksWithExclude.obj" \
	"$(INTDIR)\fileowner.obj" \
	"$(INTDIR)\filepanels.obj" \
	"$(INTDIR)\filestr.obj" \
	"$(INTDIR)\filetype.obj" \
	"$(INTDIR)\fileview.obj" \
	"$(INTDIR)\filter.obj" \
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
	"$(INTDIR)\grpsort.obj" \
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
	"$(INTDIR)\strncpy.obj" \
	"$(INTDIR)\syntax.obj" \
	"$(INTDIR)\syslog.obj" \
	"$(INTDIR)\treelist.obj" \
	"$(INTDIR)\udlist.obj" \
	"$(INTDIR)\usermenu.obj" \
	"$(INTDIR)\viewer.obj" \
	"$(INTDIR)\vmenu.obj" \
	"$(INTDIR)\xlat.obj" \
	"$(INTDIR)\UnicodeString.obj" \
	"$(INTDIR)\far.res"



LINK32_LIBS=kernel32.lib user32.lib winspool.lib advapi32.lib shell32.lib mpr.lib

BSC32_FLAGS=/nologo /o"$(OUTDIR)\far.bsc"

BSC32_SBRS= \

RSC_PROJ=/l 0x409 /fo"$(INTDIR)\far.res" /d $(USEDEBUG)

!IF  "$(CFG)" == "far - Win32 Release"
!MESSAGE far - Win32 Release.

USEDEBUG=NDEBUG

CPP_PROJ=$(FAR_MSVCRT) $(USE_WFUNC) /nologo $(FAR_ANSI) $(FARSYSLOG) $(FARTRY) $(CREATE_JUNCTION) $(FAR_GR) /Zp4 $(MT) /Gi /O1 /D $(USEDEBUG) /D "WIN32" /D "_CONSOLE" /Fp"$(INTDIR)\far.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /J /FD /c $(FARCMEM) $(FARALLOC) /FAcs /Fa"$(CODDIR)\\"

LINK32_FLAGS=$(LINK32_LIBS) /nologo /fixed:no /subsystem:console /incremental:no /pdb:"$(OUTDIR)\far.pdb" /machine:I386 /def:"$(DEF_FILE)" /out:"$(OUTDIR)\Far.exe" /map:"$(OUTDIR)\far.map" /release $(NODEFAULTLIB)

!ELSE
!MESSAGE far - Win32 Debug.

USEDEBUG=_DEBUG

#CPP_PROJ=$(FAR_MSVCRT) $(USE_WFUNC) /nologo $(FAR_ANSI) $(FARSYSLOG) $(FARTRY) $(CREATE_JUNCTION) $(MT)d /W3 /Gm /Gi /ZI /Od /D $(USEDEBUG) /D "WIN32" /D "_CONSOLE" /D "_MBCS" /Fp"$(INTDIR)\far.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /J /FD /GZ /c $(FARCMEM) $(FARALLOC) /FAcs /Fa"$(CODDIR)\\"
CPP_PROJ=$(FAR_MSVCRT) $(USE_WFUNC) /nologo $(FAR_ANSI) $(FARSYSLOG) $(FARTRY) $(CREATE_JUNCTION) $(MT)d /W3 /Gm /Gi /ZI /Od /D $(USEDEBUG) /D "WIN32" /D "_CONSOLE" /D "_MBCS" /Fp"$(INTDIR)\far.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /J /FD /c $(FARCMEM) $(FARALLOC) /FAcs /Fa"$(CODDIR)\\"

LINK32_FLAGS=$(LINK32_LIBS) /nologo /fixed:no /subsystem:console /pdb:none /debug /debugtype:both /machine:I386 /def:"$(DEF_FILE)" /out:"$(OUTDIR)\Far.exe" /map:"$(OUTDIR)\far.map" /release $(NODEFAULTLIB)

!ENDIF


ALL : lng "far.release.dep" "far.debug.dep" "lang.hpp" "$(OUTDIR)\Far.exe" "$(FARINCLUDE)\farcolor.hpp" "$(FARINCLUDE)\farkeys.hpp" "$(FARINCLUDE)\plugin.hpp"

lng:
	@lng.generator.exe -nc -i lang.ini farlang.templ

"$(OUTDIR)\Far.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
	$(LINK32) @<<
	$(LINK32_FLAGS) $(LINK32_OBJS)
<<
	@if exist "$(OUTDIR)\FarEng.hlf" del "$(OUTDIR)\FarEng.hlf" >nul
	@if exist "$(OUTDIR)\FarRus.hlf" del "$(OUTDIR)\FarRus.hlf"  >nul
	@if exist "$(OUTDIR)\FarEng.lng" del "$(OUTDIR)\FarEng.lng"  >nul
	@if exist "$(OUTDIR)\FarRus.lng" del "$(OUTDIR)\FarRus.lng"  >nul
	@awk -f mkhlf.awk -v FV1=$(FV1) -v FV2=$(FV2) -v FV3=$(FV3) -v BETA=$(FVB) ".\FarEng.hlf" > "$(OUTDIR)\FarEng.hlf"
	@awk -f mkhlf.awk -v FV1=$(FV1) -v FV2=$(FV2) -v FV3=$(FV3) -v BETA=$(FVB) ".\FarRus.hlf" > "$(OUTDIR)\FarRus.hlf"
	@copy ".\FarEng.lng" "$(OUTDIR)\FarEng.lng" >nul
	@copy ".\FarRus.lng" "$(OUTDIR)\FarRus.lng" >nul
	@if exist ".\plugin.pas" awk -f plugins.awk -v p1=$(FV1) -v p2=$(FV2) -v Lang=pas ".\plugin.pas" > "$(FARINCLUDE)\plugin.pas"
	@if exist ".\fmt.hpp" awk -f plugins.awk -v p1=$(FV1) -v p2=$(FV2) ".\fmt.hpp" > "$(FARINCLUDE)\fmt.hpp"
	@if exist ".\fmt.pas" awk -f plugins.awk -v p1=$(FV1) -v p2=$(FV2) -v Lang=pas ".\fmt.pas" > "$(FARINCLUDE)\fmt.pas"
	@if exist ".\farcolor.pas" awk -f plugins.awk -v p1=$(FV1) -v p2=$(FV2) -v Lang=pas ".\farcolor.pas" > "$(FARINCLUDE)\farcolor.pas"
	@if exist ".\farkeys.pas" awk -f plugins.awk -v p1=$(FV1) -v p2=$(FV2) -v Lang=pas ".\farkeys.pas" > "$(FARINCLUDE)\farkeys.pas"


# ************************************************************************
# �����������
# ************************************************************************
.c{$(INTDIR)}.obj::
	$(CPP) @<<
	$(CPP_PROJ) $<
<<

.cpp{$(INTDIR)}.obj::
	$(CPP) @<<
	$(CPP_PROJ) $<
<<


"far.release.dep": mkdep.list.txt
	@awk -f mkdep.awk -v out=Release flist.txt > far.release.dep


"far.debug.dep": mkdep.list.txt
	@awk -f mkdep.awk -v out=Debug flist.txt > far.debug.dep


"$(OUTDIR)" :
	@if not exist "$(OUTDIR)\$(NULL)" mkdir "$(OUTDIR)"
	@if not exist "$(FARINCLUDE)\$(NULL)" mkdir "$(FARINCLUDE)"

"$(INTDIR)" :
	@if not exist "$(INTDIR)\$(NULL)" mkdir "$(INTDIR)"
	@if not exist "$(CODDIR)\$(NULL)" mkdir "$(CODDIR)"

"$(INTDIR)\far.res" : far.rc "$(INTDIR)"
	$(RSC) $(RSC_PROJ) far.rc

# ************************************************************************
# ����������� ��� ��������� ������
# ************************************************************************
"$(FARINCLUDE)\farcolor.hpp" : colors.hpp
	@awk -f plugins.awk -v p1=$(FV1) -v p2=$(FV2)  ".\colors.hpp" > "$(FARINCLUDE)\farcolor.hpp"

"$(FARINCLUDE)\farkeys.hpp" : keys.hpp
	@awk -f plugins.awk -v p1=$(FV1) -v p2=$(FV2)  ".\keys.hpp"   > "$(FARINCLUDE)\farkeys.hpp"

"$(FARINCLUDE)\plugin.hpp" : plugin.hpp
	@awk -f plugins.awk -v p1=$(FV1) -v p2=$(FV2)  ".\plugin.hpp" > "$(FARINCLUDE)\plugin.hpp"


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF  "$(CFG)" == "far - Win32 Release"
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
# �������
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
