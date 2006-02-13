!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE
NULL=nul
!ENDIF

INCLUDE = $(INCLUDE);..\common

OUTDIR=.\Final
INTDIR=.\OBJ
CODDIR=.\OBJ\cod

CPP=cl.exe
LINK32=link.exe
RSC=rc.exe

#CPP_PROJ=/nologo /Zp2 /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PROCLIST_EXPORTS" /Fp"$(INTDIR)\Proclist.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c
CPP_PROJ=-Gs -O2 -Zp1 /MT /GR- /GX -c /opt:nowin98 /Fa"$(CODDIR)\\" /Fo"$(INTDIR)\\" /D "_WIN32_DCOM"
         #-Gs -Ox -Zp1 /GR- /GX -c /opt:nowin98 /Fa"Cod\\"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\ProcList.res" /d "NDEBUG"

LINK32_FLAGS=/STUB:..\common\minstub.exe /opt:nowin98 kernel32.lib user32.lib advapi32.lib WbemUuid.Lib Ole32.Lib OleAut32.Lib shell32.lib MPR.LIB /nologo /dll /incremental:no /pdb:none /machine:I386 /def:".\Plugin.def" /out:"$(OUTDIR)\Proclist.dll" /implib:"$(INTDIR)\Proclist.lib" /MAP

DEF_FILE=".\Plugin.def"

LINK32_OBJS= \
	"$(INTDIR)\handles.obj" \
	"$(INTDIR)\PCFG.OBJ" \
	"$(INTDIR)\Pclass.obj" \
	"$(INTDIR)\perfthread.obj" \
	"$(INTDIR)\PLIST95.OBJ" \
	"$(INTDIR)\Plistnt.obj" \
	"$(INTDIR)\Pmix.obj" \
	"$(INTDIR)\preg.obj" \
	"$(INTDIR)\WMI.obj" \
	"$(INTDIR)\Proclist.obj" \
	"$(INTDIR)\ProcList.res"



ALL : "$(OUTDIR)\PROCLIST.DLL"


CLEAN :
	-@erase "$(INTDIR)\*.obj"
	-@erase "$(INTDIR)\*.res"
	-@erase "$(INTDIR)\*.exp"
	-@erase "$(INTDIR)\*.lib"
	-@erase "$(OUTDIR)\PROCLIST.DLL"


$(OUTDIR) :
    @if not exist "$(OUTDIR)/$(NULL)"  mkdir "$(OUTDIR)"
    @if not exist "$(INTDIR)/$(NULL)"  mkdir "$(INTDIR)"
    @if not exist "$(CODDIR)/$(NULL)"  mkdir "$(CODDIR)"


$(INTDIR) :
    @if not exist "$(OUTDIR)/$(NULL)"  mkdir "$(OUTDIR)"
    @if not exist "$(INTDIR)/$(NULL)"  mkdir "$(INTDIR)"
    @if not exist "$(CODDIR)/$(NULL)"  mkdir "$(CODDIR)"



$(CODDIR) :
    @if not exist "$(OUTDIR)/$(NULL)"  mkdir "$(OUTDIR)"
    @if not exist "$(INTDIR)/$(NULL)"  mkdir "$(INTDIR)"
    @if not exist "$(CODDIR)/$(NULL)"  mkdir "$(CODDIR)"



.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $<
<<




"$(OUTDIR)\PROCLIST.DLL" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<
   @copy  ProcEng.hlf  final\ProcEng.hlf   > nul
   @copy  ProcEng.lng  final\ProcEng.lng   > nul
   @copy  ProcRus.hlf  final\ProcRus.hlf   > nul
   @copy  ProcRus.lng  final\ProcRus.lng   > nul


"$(INTDIR)\ProcList.res" : ProcList.rc "$(INTDIR)"
	$(RSC) $(RSC_PROJ) ProcList.rc

HANDLES.CPP: PERFTHREAD.HPP PROCLIST.HPP PROCLNG.HPP {$(INCLUDE)}plugin.hpp $(CODDIR)

Pcfg.cpp: PERFTHREAD.HPP PROCLIST.HPP PROCLNG.HPP {$(INCLUDE)}plugin.hpp $(CODDIR)

Pclass.cpp: PERFTHREAD.HPP PROCLIST.HPP PROCLNG.HPP {$(INCLUDE)}plugin.hpp $(CODDIR)

perfthread.cpp: PERFTHREAD.HPP PROCLIST.HPP PROCLNG.HPP {$(INCLUDE)}plugin.hpp $(CODDIR)

Plist95.cpp: PERFTHREAD.HPP PROCLIST.HPP PROCLNG.HPP {$(INCLUDE)}plugin.hpp $(CODDIR)

PlistNT.cpp: PERFTHREAD.HPP PROCLIST.HPP PROCLNG.HPP {$(INCLUDE)}plugin.hpp $(CODDIR)

Pmix.cpp: PERFTHREAD.HPP PROCLIST.HPP PROCLNG.HPP {$(INCLUDE)}plugin.hpp $(CODDIR)

Preg.cpp: PERFTHREAD.HPP PROCLIST.HPP PROCLNG.HPP {$(INCLUDE)}plugin.hpp $(CODDIR)

Proclist.cpp: PERFTHREAD.HPP PROCLIST.HPP PROCLNG.HPP {$(INCLUDE)}plugin.hpp $(CODDIR)

WMI.obj: PERFTHREAD.HPP PROCLIST.HPP PROCLNG.HPP {$(INCLUDE)}plugin.hpp $(CODDIR)
