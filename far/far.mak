# Microsoft Developer Studio Generated NMAKE File, Based on far.dsp
!IF "$(CFG)" == ""
CFG=far - Win32 Debug
!MESSAGE No configuration specified. Defaulting to far - Win32 Debug.
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

CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "far - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release\obj
CODDIR=".\\Release\\cod\\"
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\far.exe"


CLEAN :
	-@erase "$(INTDIR)\checkver.obj"
	-@erase "$(INTDIR)\chgmmode.obj"
	-@erase "$(INTDIR)\chgprior.obj"
	-@erase "$(INTDIR)\cmdline.obj"
	-@erase "$(INTDIR)\config.obj"
	-@erase "$(INTDIR)\copy.obj"
	-@erase "$(INTDIR)\ctrlobj.obj"
	-@erase "$(INTDIR)\delete.obj"
	-@erase "$(INTDIR)\dialog.obj"
	-@erase "$(INTDIR)\dizlist.obj"
	-@erase "$(INTDIR)\edit.obj"
	-@erase "$(INTDIR)\editor.obj"
	-@erase "$(INTDIR)\far.res"
	-@erase "$(INTDIR)\ffolders.obj"
	-@erase "$(INTDIR)\fileedit.obj"
	-@erase "$(INTDIR)\filelist.obj"
	-@erase "$(INTDIR)\filestr.obj"
	-@erase "$(INTDIR)\filetype.obj"
	-@erase "$(INTDIR)\fileview.obj"
	-@erase "$(INTDIR)\filter.obj"
	-@erase "$(INTDIR)\findfile.obj"
	-@erase "$(INTDIR)\flmodes.obj"
	-@erase "$(INTDIR)\flplugin.obj"
	-@erase "$(INTDIR)\flshow.obj"
	-@erase "$(INTDIR)\flupdate.obj"
	-@erase "$(INTDIR)\foldtree.obj"
	-@erase "$(INTDIR)\gettable.obj"
	-@erase "$(INTDIR)\global.obj"
	-@erase "$(INTDIR)\grabber.obj"
	-@erase "$(INTDIR)\grpsort.obj"
	-@erase "$(INTDIR)\help.obj"
	-@erase "$(INTDIR)\hilight.obj"
	-@erase "$(INTDIR)\history.obj"
	-@erase "$(INTDIR)\hmenu.obj"
	-@erase "$(INTDIR)\infolist.obj"
	-@erase "$(INTDIR)\int64.obj"
	-@erase "$(INTDIR)\interf.obj"
	-@erase "$(INTDIR)\iswind.obj"
	-@erase "$(INTDIR)\keybar.obj"
	-@erase "$(INTDIR)\language.obj"
	-@erase "$(INTDIR)\local.obj"
	-@erase "$(INTDIR)\lockscrn.obj"
	-@erase "$(INTDIR)\macro.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\manager.obj"
	-@erase "$(INTDIR)\menubar.obj"
	-@erase "$(INTDIR)\message.obj"
	-@erase "$(INTDIR)\keyboard.obj"
	-@erase "$(INTDIR)\clipboard.obj"
	-@erase "$(INTDIR)\eject.obj"
	-@erase "$(INTDIR)\xlat.obj"
	-@erase "$(INTDIR)\mix.obj"
	-@erase "$(INTDIR)\syslog.obj"
	-@erase "$(INTDIR)\fileattr.obj"
	-@erase "$(INTDIR)\mkdir.obj"
	-@erase "$(INTDIR)\modal.obj"
	-@erase "$(INTDIR)\namelist.obj"
	-@erase "$(INTDIR)\options.obj"
	-@erase "$(INTDIR)\palette.obj"
	-@erase "$(INTDIR)\panel.obj"
	-@erase "$(INTDIR)\plist.obj"
	-@erase "$(INTDIR)\plognmn.obj"
	-@erase "$(INTDIR)\plugapi.obj"
	-@erase "$(INTDIR)\plugins.obj"
	-@erase "$(INTDIR)\poscache.obj"
	-@erase "$(INTDIR)\print.obj"
	-@erase "$(INTDIR)\qview.obj"
	-@erase "$(INTDIR)\rdrwdsk.obj"
	-@erase "$(INTDIR)\registry.obj"
	-@erase "$(INTDIR)\savefpos.obj"
	-@erase "$(INTDIR)\savescr.obj"
	-@erase "$(INTDIR)\scantree.obj"
	-@erase "$(INTDIR)\scrbuf.obj"
	-@erase "$(INTDIR)\scrobj.obj"
	-@erase "$(INTDIR)\scrsaver.obj"
	-@erase "$(INTDIR)\setattr.obj"
	-@erase "$(INTDIR)\setcolor.obj"
	-@erase "$(INTDIR)\treelist.obj"
	-@erase "$(INTDIR)\usermenu.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\viewer.obj"
	-@erase "$(INTDIR)\vmenu.obj"
	-@erase "$(INTDIR)\farrtl.obj"
	-@erase "$(OUTDIR)\far.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"
"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"
"$(CODDIR)" :
    if not exist "$(CODDIR)/$(NULL)" mkdir "$(CODDIR)"

CPP_PROJ=/nologo $(FARSYSLOG) $(FARTRY) /Zp4 /MT /Gi /O1 /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /Fp"$(INTDIR)\far.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /J /FD /c $(FARCMEM) $(FARALLOC) /FAcs /Fa"$(CODDIR)"
RSC_PROJ=/l 0x419 /fo"$(INTDIR)\far.res" /d "NDEBUG"
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\far.bsc"
BSC32_SBRS= \

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib winspool.lib advapi32.lib shell32.lib mpr.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\far.pdb" /machine:I386 /def:".\far.def" /out:"$(OUTDIR)\far.exe" /map:"$(OUTDIR)\far.map"
DEF_FILE= \
	".\far.def"
LINK32_OBJS= \
	"$(INTDIR)\checkver.obj" \
	"$(INTDIR)\chgmmode.obj" \
	"$(INTDIR)\chgprior.obj" \
	"$(INTDIR)\cmdline.obj" \
	"$(INTDIR)\config.obj" \
	"$(INTDIR)\copy.obj" \
	"$(INTDIR)\ctrlobj.obj" \
	"$(INTDIR)\delete.obj" \
	"$(INTDIR)\dialog.obj" \
	"$(INTDIR)\dizlist.obj" \
	"$(INTDIR)\edit.obj" \
	"$(INTDIR)\editor.obj" \
	"$(INTDIR)\ffolders.obj" \
	"$(INTDIR)\fileedit.obj" \
	"$(INTDIR)\filelist.obj" \
	"$(INTDIR)\filestr.obj" \
	"$(INTDIR)\filetype.obj" \
	"$(INTDIR)\fileview.obj" \
	"$(INTDIR)\filter.obj" \
	"$(INTDIR)\findfile.obj" \
	"$(INTDIR)\flmodes.obj" \
	"$(INTDIR)\flplugin.obj" \
	"$(INTDIR)\flshow.obj" \
	"$(INTDIR)\flupdate.obj" \
	"$(INTDIR)\foldtree.obj" \
	"$(INTDIR)\gettable.obj" \
	"$(INTDIR)\global.obj" \
	"$(INTDIR)\grabber.obj" \
	"$(INTDIR)\grpsort.obj" \
	"$(INTDIR)\help.obj" \
	"$(INTDIR)\hilight.obj" \
	"$(INTDIR)\history.obj" \
	"$(INTDIR)\hmenu.obj" \
	"$(INTDIR)\infolist.obj" \
	"$(INTDIR)\int64.obj" \
	"$(INTDIR)\interf.obj" \
	"$(INTDIR)\iswind.obj" \
	"$(INTDIR)\keybar.obj" \
	"$(INTDIR)\language.obj" \
	"$(INTDIR)\local.obj" \
	"$(INTDIR)\lockscrn.obj" \
	"$(INTDIR)\macro.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\manager.obj" \
	"$(INTDIR)\menubar.obj" \
	"$(INTDIR)\message.obj" \
        "$(INTDIR)\keyboard.obj" \
        "$(INTDIR)\syslog.obj" \
        "$(INTDIR)\fileattr.obj" \
	"$(INTDIR)\clipboard.obj" \
	"$(INTDIR)\eject.obj" \
	"$(INTDIR)\xlat.obj" \
	"$(INTDIR)\mix.obj" \
	"$(INTDIR)\mkdir.obj" \
	"$(INTDIR)\modal.obj" \
	"$(INTDIR)\namelist.obj" \
	"$(INTDIR)\options.obj" \
	"$(INTDIR)\palette.obj" \
	"$(INTDIR)\panel.obj" \
	"$(INTDIR)\plist.obj" \
	"$(INTDIR)\plognmn.obj" \
	"$(INTDIR)\plugapi.obj" \
	"$(INTDIR)\plugins.obj" \
	"$(INTDIR)\poscache.obj" \
	"$(INTDIR)\print.obj" \
	"$(INTDIR)\qview.obj" \
	"$(INTDIR)\rdrwdsk.obj" \
	"$(INTDIR)\registry.obj" \
	"$(INTDIR)\savefpos.obj" \
	"$(INTDIR)\savescr.obj" \
	"$(INTDIR)\scantree.obj" \
	"$(INTDIR)\scrbuf.obj" \
	"$(INTDIR)\scrobj.obj" \
	"$(INTDIR)\scrsaver.obj" \
	"$(INTDIR)\setattr.obj" \
	"$(INTDIR)\setcolor.obj" \
	"$(INTDIR)\treelist.obj" \
	"$(INTDIR)\usermenu.obj" \
	"$(INTDIR)\viewer.obj" \
	"$(INTDIR)\vmenu.obj" \
	"$(INTDIR)\farrtl.obj" \
	"$(INTDIR)\far.res"

"$(OUTDIR)\far.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<
   @if exist "$(OUTDIR)\FarEng.hlf" del "$(OUTDIR)\FarEng.hlf" >nul
   @if exist "$(OUTDIR)\FarRus.hlf" del "$(OUTDIR)\FarRus.hlf"  >nul
   @if exist "$(OUTDIR)\FarEng.lng" del "$(OUTDIR)\FarEng.lng"  >nul
   @if exist "$(OUTDIR)\FarRus.lng" del "$(OUTDIR)\FarRus.lng"  >nul
   @copy ".\FarEng.hlf" "$(OUTDIR)\FarEng.hlf" >nul
   @copy ".\FarRus.hlf" "$(OUTDIR)\FarRus.hlf" >nul
   @copy ".\FarEng.lng" "$(OUTDIR)\FarEng.lng" >nul
   @copy ".\FarRus.lng" "$(OUTDIR)\FarRus.lng" >nul

!ELSEIF  "$(CFG)" == "far - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug\obj
CODDIR=".\\Debug\\cod\\"
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\far.exe"


CLEAN :
	-@erase "$(INTDIR)\checkver.obj"
	-@erase "$(INTDIR)\chgmmode.obj"
	-@erase "$(INTDIR)\chgprior.obj"
	-@erase "$(INTDIR)\cmdline.obj"
	-@erase "$(INTDIR)\config.obj"
	-@erase "$(INTDIR)\copy.obj"
	-@erase "$(INTDIR)\ctrlobj.obj"
	-@erase "$(INTDIR)\delete.obj"
	-@erase "$(INTDIR)\dialog.obj"
	-@erase "$(INTDIR)\dizlist.obj"
	-@erase "$(INTDIR)\edit.obj"
	-@erase "$(INTDIR)\editor.obj"
	-@erase "$(INTDIR)\far.res"
	-@erase "$(INTDIR)\ffolders.obj"
	-@erase "$(INTDIR)\fileedit.obj"
	-@erase "$(INTDIR)\filelist.obj"
	-@erase "$(INTDIR)\filestr.obj"
	-@erase "$(INTDIR)\filetype.obj"
	-@erase "$(INTDIR)\fileview.obj"
	-@erase "$(INTDIR)\filter.obj"
	-@erase "$(INTDIR)\findfile.obj"
	-@erase "$(INTDIR)\flmodes.obj"
	-@erase "$(INTDIR)\flplugin.obj"
	-@erase "$(INTDIR)\flshow.obj"
	-@erase "$(INTDIR)\flupdate.obj"
	-@erase "$(INTDIR)\foldtree.obj"
	-@erase "$(INTDIR)\gettable.obj"
	-@erase "$(INTDIR)\global.obj"
	-@erase "$(INTDIR)\grabber.obj"
	-@erase "$(INTDIR)\grpsort.obj"
	-@erase "$(INTDIR)\help.obj"
	-@erase "$(INTDIR)\hilight.obj"
	-@erase "$(INTDIR)\history.obj"
	-@erase "$(INTDIR)\hmenu.obj"
	-@erase "$(INTDIR)\infolist.obj"
	-@erase "$(INTDIR)\int64.obj"
	-@erase "$(INTDIR)\interf.obj"
	-@erase "$(INTDIR)\iswind.obj"
	-@erase "$(INTDIR)\keybar.obj"
	-@erase "$(INTDIR)\language.obj"
	-@erase "$(INTDIR)\local.obj"
	-@erase "$(INTDIR)\lockscrn.obj"
	-@erase "$(INTDIR)\macro.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\manager.obj"
	-@erase "$(INTDIR)\menubar.obj"
	-@erase "$(INTDIR)\message.obj"
	-@erase "$(INTDIR)\keyboard.obj"
	-@erase "$(INTDIR)\syslog.obj"
	-@erase "$(INTDIR)\fileattr.obj"
	-@erase "$(INTDIR)\clipboard.obj"
	-@erase "$(INTDIR)\eject.obj"
	-@erase "$(INTDIR)\xlat.obj"
	-@erase "$(INTDIR)\mix.obj"
	-@erase "$(INTDIR)\mkdir.obj"
	-@erase "$(INTDIR)\modal.obj"
	-@erase "$(INTDIR)\namelist.obj"
	-@erase "$(INTDIR)\options.obj"
	-@erase "$(INTDIR)\palette.obj"
	-@erase "$(INTDIR)\panel.obj"
	-@erase "$(INTDIR)\plist.obj"
	-@erase "$(INTDIR)\plognmn.obj"
	-@erase "$(INTDIR)\plugapi.obj"
	-@erase "$(INTDIR)\plugins.obj"
	-@erase "$(INTDIR)\poscache.obj"
	-@erase "$(INTDIR)\print.obj"
	-@erase "$(INTDIR)\qview.obj"
	-@erase "$(INTDIR)\rdrwdsk.obj"
	-@erase "$(INTDIR)\registry.obj"
	-@erase "$(INTDIR)\savefpos.obj"
	-@erase "$(INTDIR)\savescr.obj"
	-@erase "$(INTDIR)\scantree.obj"
	-@erase "$(INTDIR)\scrbuf.obj"
	-@erase "$(INTDIR)\scrobj.obj"
	-@erase "$(INTDIR)\scrsaver.obj"
	-@erase "$(INTDIR)\setattr.obj"
	-@erase "$(INTDIR)\setcolor.obj"
	-@erase "$(INTDIR)\treelist.obj"
	-@erase "$(INTDIR)\usermenu.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\viewer.obj"
	-@erase "$(INTDIR)\vmenu.obj"
	-@erase "$(INTDIR)\farrtl.obj"
	-@erase "$(OUTDIR)\far.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"
"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"
"$(CODDIR)" :
    if not exist "$(CODDIR)/$(NULL)" mkdir "$(CODDIR)"

CPP_PROJ=/nologo $(FARSYSLOG) $(FARTRY) /MTd /W3 /Gm /Gi /ZI /Od /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /Fp"$(INTDIR)\far.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /J /FD /GZ /c $(FARCMEM) $(FARALLOC) /FAcs /Fa"$(CODDIR)"
RSC_PROJ=/l 0x419 /fo"$(INTDIR)\far.res" /d "_DEBUG"
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\far.bsc"
BSC32_SBRS= \

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib winspool.lib advapi32.lib shell32.lib mpr.lib /nologo /subsystem:console /pdb:none /debug /debugtype:both /machine:I386 /def:".\far.def" /out:"$(OUTDIR)\far.exe" /map:"$(OUTDIR)\far.map"
DEF_FILE= \
	".\far.def"
LINK32_OBJS= \
	"$(INTDIR)\checkver.obj" \
	"$(INTDIR)\chgmmode.obj" \
	"$(INTDIR)\chgprior.obj" \
	"$(INTDIR)\cmdline.obj" \
	"$(INTDIR)\config.obj" \
	"$(INTDIR)\copy.obj" \
	"$(INTDIR)\ctrlobj.obj" \
	"$(INTDIR)\delete.obj" \
	"$(INTDIR)\dialog.obj" \
	"$(INTDIR)\dizlist.obj" \
	"$(INTDIR)\edit.obj" \
	"$(INTDIR)\editor.obj" \
	"$(INTDIR)\ffolders.obj" \
	"$(INTDIR)\fileedit.obj" \
	"$(INTDIR)\filelist.obj" \
	"$(INTDIR)\filestr.obj" \
	"$(INTDIR)\filetype.obj" \
	"$(INTDIR)\fileview.obj" \
	"$(INTDIR)\filter.obj" \
	"$(INTDIR)\findfile.obj" \
	"$(INTDIR)\flmodes.obj" \
	"$(INTDIR)\flplugin.obj" \
	"$(INTDIR)\flshow.obj" \
	"$(INTDIR)\flupdate.obj" \
	"$(INTDIR)\foldtree.obj" \
	"$(INTDIR)\gettable.obj" \
	"$(INTDIR)\global.obj" \
	"$(INTDIR)\grabber.obj" \
	"$(INTDIR)\grpsort.obj" \
	"$(INTDIR)\help.obj" \
	"$(INTDIR)\hilight.obj" \
	"$(INTDIR)\history.obj" \
	"$(INTDIR)\hmenu.obj" \
	"$(INTDIR)\infolist.obj" \
	"$(INTDIR)\int64.obj" \
	"$(INTDIR)\interf.obj" \
	"$(INTDIR)\iswind.obj" \
	"$(INTDIR)\keybar.obj" \
	"$(INTDIR)\language.obj" \
	"$(INTDIR)\local.obj" \
	"$(INTDIR)\lockscrn.obj" \
	"$(INTDIR)\macro.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\manager.obj" \
	"$(INTDIR)\menubar.obj" \
	"$(INTDIR)\message.obj" \
        "$(INTDIR)\keyboard.obj" \
        "$(INTDIR)\syslog.obj" \
        "$(INTDIR)\fileattr.obj" \
	"$(INTDIR)\clipboard.obj" \
	"$(INTDIR)\eject.obj" \
	"$(INTDIR)\xlat.obj" \
	"$(INTDIR)\mix.obj" \
	"$(INTDIR)\mkdir.obj" \
	"$(INTDIR)\modal.obj" \
	"$(INTDIR)\namelist.obj" \
	"$(INTDIR)\options.obj" \
	"$(INTDIR)\palette.obj" \
	"$(INTDIR)\panel.obj" \
	"$(INTDIR)\plist.obj" \
	"$(INTDIR)\plognmn.obj" \
	"$(INTDIR)\plugapi.obj" \
	"$(INTDIR)\plugins.obj" \
	"$(INTDIR)\poscache.obj" \
	"$(INTDIR)\print.obj" \
	"$(INTDIR)\qview.obj" \
	"$(INTDIR)\rdrwdsk.obj" \
	"$(INTDIR)\registry.obj" \
	"$(INTDIR)\savefpos.obj" \
	"$(INTDIR)\savescr.obj" \
	"$(INTDIR)\scantree.obj" \
	"$(INTDIR)\scrbuf.obj" \
	"$(INTDIR)\scrobj.obj" \
	"$(INTDIR)\scrsaver.obj" \
	"$(INTDIR)\setattr.obj" \
	"$(INTDIR)\setcolor.obj" \
	"$(INTDIR)\treelist.obj" \
	"$(INTDIR)\usermenu.obj" \
	"$(INTDIR)\viewer.obj" \
	"$(INTDIR)\vmenu.obj" \
	"$(INTDIR)\farrtl.obj" \
	"$(INTDIR)\far.res"

"$(OUTDIR)\far.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<
   @if exist "$(OUTDIR)\FarEng.hlf" del "$(OUTDIR)\FarEng.hlf" >nul
   @if exist "$(OUTDIR)\FarRus.hlf" del "$(OUTDIR)\FarRus.hlf"  >nul
   @if exist "$(OUTDIR)\FarEng.lng" del "$(OUTDIR)\FarEng.lng"  >nul
   @if exist "$(OUTDIR)\FarRus.lng" del "$(OUTDIR)\FarRus.lng"  >nul
   @copy ".\FarEng.hlf" "$(OUTDIR)\FarEng.hlf" >nul
   @copy ".\FarRus.hlf" "$(OUTDIR)\FarRus.hlf" >nul
   @copy ".\FarEng.lng" "$(OUTDIR)\FarEng.lng" >nul
   @copy ".\FarRus.lng" "$(OUTDIR)\FarRus.lng" >nul

!ENDIF

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $<
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $<
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $<
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $<
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $<
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $<
<<


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("far.dep")
!INCLUDE "far.dep"
!ELSE
!MESSAGE Warning: cannot find "far.dep"
!ENDIF
!ENDIF


!IF "$(CFG)" == "far - Win32 Release" || "$(CFG)" == "far - Win32 Debug"
SOURCE=.\checkver.cpp

"$(INTDIR)\checkver.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\chgmmode.cpp

"$(INTDIR)\chgmmode.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\chgprior.cpp

"$(INTDIR)\chgprior.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\cmdline.cpp

"$(INTDIR)\cmdline.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\config.cpp

"$(INTDIR)\config.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\copy.cpp

"$(INTDIR)\copy.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ctrlobj.cpp

"$(INTDIR)\ctrlobj.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\delete.cpp

"$(INTDIR)\delete.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\dialog.cpp

"$(INTDIR)\dialog.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\dizlist.cpp

"$(INTDIR)\dizlist.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\edit.cpp

"$(INTDIR)\edit.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\editor.cpp

"$(INTDIR)\editor.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\far.rc

"$(INTDIR)\far.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=.\ffolders.cpp

"$(INTDIR)\ffolders.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\fileedit.cpp

"$(INTDIR)\fileedit.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\filelist.cpp

"$(INTDIR)\filelist.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\filestr.cpp

"$(INTDIR)\filestr.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\filetype.cpp

"$(INTDIR)\filetype.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\fileview.cpp

"$(INTDIR)\fileview.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\filter.cpp

"$(INTDIR)\filter.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\findfile.cpp

"$(INTDIR)\findfile.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\flmodes.cpp

"$(INTDIR)\flmodes.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\flplugin.cpp

"$(INTDIR)\flplugin.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\flshow.cpp

"$(INTDIR)\flshow.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\flupdate.cpp

"$(INTDIR)\flupdate.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\foldtree.cpp

"$(INTDIR)\foldtree.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\gettable.cpp

"$(INTDIR)\gettable.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\global.cpp

"$(INTDIR)\global.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\grabber.cpp

"$(INTDIR)\grabber.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\grpsort.cpp

"$(INTDIR)\grpsort.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\help.cpp

"$(INTDIR)\help.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\hilight.cpp

"$(INTDIR)\hilight.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\history.cpp

"$(INTDIR)\history.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\hmenu.cpp

"$(INTDIR)\hmenu.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\infolist.cpp

"$(INTDIR)\infolist.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\int64.cpp

"$(INTDIR)\int64.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\interf.cpp

"$(INTDIR)\interf.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\iswind.cpp

"$(INTDIR)\iswind.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\keybar.cpp

"$(INTDIR)\keybar.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\lang.cpp
SOURCE=.\language.cpp

"$(INTDIR)\language.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\local.cpp

"$(INTDIR)\local.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\lockscrn.cpp

"$(INTDIR)\lockscrn.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\macro.cpp

"$(INTDIR)\macro.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\main.cpp

"$(INTDIR)\main.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\manager.cpp

"$(INTDIR)\manager.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\menubar.cpp

"$(INTDIR)\menubar.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\message.cpp

"$(INTDIR)\message.obj" : $(SOURCE) "$(INTDIR)"

SOURCE=.\keyboard.cpp

"$(INTDIR)\keyboard.obj" : $(SOURCE) "$(INTDIR)"

SOURCE=.\syslog.cpp

"$(INTDIR)\syslog.obj" : $(SOURCE) "$(INTDIR)"

SOURCE=.\fileattr.cpp

"$(INTDIR)\fileattr.obj" : $(SOURCE) "$(INTDIR)"

SOURCE=.\mix.cpp

"$(INTDIR)\mix.obj" : $(SOURCE) "$(INTDIR)"

SOURCE=.\clipboard.cpp

"$(INTDIR)\clipboard.obj" : $(SOURCE) "$(INTDIR)"

SOURCE=.\eject.cpp

"$(INTDIR)\eject.obj" : $(SOURCE) "$(INTDIR)"

SOURCE=.\xlat.cpp

"$(INTDIR)\xlat.obj" : $(SOURCE) "$(INTDIR)"

SOURCE=.\mkdir.cpp

"$(INTDIR)\mkdir.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\modal.cpp

"$(INTDIR)\modal.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\namelist.cpp

"$(INTDIR)\namelist.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\options.cpp

"$(INTDIR)\options.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\palette.cpp

"$(INTDIR)\palette.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\panel.cpp

"$(INTDIR)\panel.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\plist.cpp

"$(INTDIR)\plist.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\plognmn.cpp

"$(INTDIR)\plognmn.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\plugapi.cpp

"$(INTDIR)\plugapi.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\plugins.cpp

"$(INTDIR)\plugins.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\poscache.cpp

"$(INTDIR)\poscache.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\print.cpp

"$(INTDIR)\print.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\qview.cpp

"$(INTDIR)\qview.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\rdrwdsk.cpp

"$(INTDIR)\rdrwdsk.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\registry.cpp

"$(INTDIR)\registry.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\savefpos.cpp

"$(INTDIR)\savefpos.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\savescr.cpp

"$(INTDIR)\savescr.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\scantree.cpp

"$(INTDIR)\scantree.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\scrbuf.cpp

"$(INTDIR)\scrbuf.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\scrobj.cpp

"$(INTDIR)\scrobj.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\scrsaver.cpp

"$(INTDIR)\scrsaver.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\setattr.cpp

"$(INTDIR)\setattr.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\setcolor.cpp

"$(INTDIR)\setcolor.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\smallobj.cpp
SOURCE=.\treelist.cpp

"$(INTDIR)\treelist.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\usermenu.cpp

"$(INTDIR)\usermenu.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\viewer.cpp

"$(INTDIR)\viewer.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\vmenu.cpp

"$(INTDIR)\vmenu.obj" : $(SOURCE) "$(INTDIR)"

SOURCE=.\farrtl.cpp

"$(INTDIR)\farrtl.obj" : $(SOURCE) "$(INTDIR)"


!ENDIF
