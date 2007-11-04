# Microsoft Developer Studio Generated NMAKE File, Based on MacroView.dsp
!IF "$(CFG)" == ""
#CFG=MacroView - Win32 Release
CFG=MacroView - Win32 Debug
!MESSAGE No configuration specified. Defaulting to MacroView - Win32 Release.
!ENDIF

!IF "$(CFG)" != "MacroView - Win32 Release" && "$(CFG)" != "MacroView - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "MacroView.mak" CFG="MacroView - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "MacroView - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "MacroView - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "MacroView - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\MacroView.dll"


CLEAN :
	-@erase "$(INTDIR)\MacroView.obj"
	-@erase "$(INTDIR)\MacroView.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\MacroView.dll"
	-@erase "$(OUTDIR)\MacroView.exp"
	-@erase "$(OUTDIR)\MacroView.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MT /W3 /GX /O2 /Os /Fo"$(INTDIR)\\" /c 
#CPP_PROJ=/nologo /MD /W3 /GX /O2 /Fo"$(INTDIR)\\" /c
MTL_PROJ=/win32 
RSC_PROJ=/l 0x419 /fo"$(INTDIR)\MacroView.res" 
BSC32=bscmake.exe
BSC32_FLAGS=/o"$(OUTDIR)\MacroView.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib version.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /pdb:none /machine:I386 /out:"$(OUTDIR)\MacroView.dll" /implib:"$(OUTDIR)\MacroView.lib" /DEF:MacroView.def /filealign:512
LINK32_OBJS= \
	"$(INTDIR)\MacroView.res" \
	"$(INTDIR)\MacroView.obj"

"$(OUTDIR)\MacroView.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "MacroView - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\MacroView.dll"


CLEAN :
	-@erase "$(INTDIR)\MacroView.obj"
	-@erase "$(INTDIR)\MacroView.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\MacroView.dll"
	-@erase "$(OUTDIR)\MacroView.exp"
	-@erase "$(OUTDIR)\MacroView.ilk"
	-@erase "$(OUTDIR)\MacroView.lib"
	-@erase "$(OUTDIR)\MacroView.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MTd /W3 /Gm /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MACROVIEW_EXPORTS" /Fp"$(INTDIR)\MacroView.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ  /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x419 /fo"$(INTDIR)\MacroView.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\MacroView.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib version.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /incremental:yes /pdb:"$(OUTDIR)\MacroView.pdb" /debug /machine:I386 /out:"$(OUTDIR)\MacroView.dll" /implib:"$(OUTDIR)\MacroView.lib" /pdbtype:con /def:MacroView.def
LINK32_OBJS= \
	"$(INTDIR)\MacroView.res" \
	"$(INTDIR)\MacroView.obj"

"$(OUTDIR)\MacroView.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

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
!IF EXISTS("MacroView.dep")
!INCLUDE "MacroView.dep"
!ELSE 
!MESSAGE Warning: cannot find "MacroView.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "MacroView - Win32 Release" || "$(CFG)" == "MacroView - Win32 Debug"
SOURCE=.\MacroView.cpp

"$(INTDIR)\MacroView.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\MacroView.rc

"$(INTDIR)\MacroView.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF 

