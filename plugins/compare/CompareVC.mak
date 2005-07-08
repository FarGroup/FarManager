# Microsoft Developer Studio Generated NMAKE File, Based on Compare.dsp
!IF "$(CFG)" == ""
CFG=Compare - Win32 Release
!MESSAGE No configuration specified. Defaulting to Compare - Win32 Debug.
!ENDIF

!IF "$(CFG)" != "Compare - Win32 Release" && "$(CFG)" != "Compare - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE
!MESSAGE NMAKE /f "Compare.mak" CFG="Compare - Win32 Debug"
!MESSAGE
!MESSAGE Possible choices for configuration are:
!MESSAGE
!MESSAGE "Compare - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Compare - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
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

!IF  "$(CFG)" == "Compare - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\Compare.dll"


CLEAN :
	-@erase "$(INTDIR)\compare.obj"
	-@erase "$(INTDIR)\Compare.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\Compare.dll"
	-@erase "$(OUTDIR)\Compare.exp"
	-@erase "$(OUTDIR)\Compare.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "COMPARE_EXPORTS" /Fp"$(INTDIR)\Compare.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32
RSC_PROJ=/l 0x419 /fo"$(INTDIR)\Compare.res" /d "NDEBUG"
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\Compare.bsc"
BSC32_SBRS= \

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\Compare.pdb" /machine:I386 /def:".\Compare.def" /out:"$(OUTDIR)\Compare.dll" /implib:"$(OUTDIR)\Compare.lib"
DEF_FILE= \
	".\Compare.def"
LINK32_OBJS= \
	"$(INTDIR)\compare.obj" \
	"$(INTDIR)\Compare.res"

"$(OUTDIR)\Compare.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Compare - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\Compare.dll"


CLEAN :
	-@erase "$(INTDIR)\compare.obj"
	-@erase "$(INTDIR)\Compare.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\Compare.dll"
	-@erase "$(OUTDIR)\Compare.exp"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /G4 /Zp2 /ML /W3 /Oi /Ob2 /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "COMPARE_EXPORTS" /Fp"$(INTDIR)\Compare.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c
MTL_PROJ=/nologo /D "_DEBUG" /win32
RSC_PROJ=/l 0x417 /fo"$(INTDIR)\Compare.res" /d "_DEBUG"
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\Compare.bsc"
BSC32_SBRS= \

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib advapi32.lib msvcrt.lib /nologo /dll /pdb:none /machine:I386 /nodefaultlib /def:".\Compare.def" /out:"$(OUTDIR)\Compare.dll" /implib:"$(OUTDIR)\Compare.lib" /opt:nowin98
DEF_FILE= \
	".\Compare.def"
LINK32_OBJS= \
	"$(INTDIR)\compare.obj" \
	"$(INTDIR)\Compare.res"

"$(OUTDIR)\Compare.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("Compare.dep")
!INCLUDE "Compare.dep"
!ELSE
!MESSAGE Warning: cannot find "Compare.dep"
!ENDIF
!ENDIF


!IF "$(CFG)" == "Compare - Win32 Release" || "$(CFG)" == "Compare - Win32 Debug"
SOURCE=.\compare.cpp

"$(INTDIR)\compare.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Compare.rc

"$(INTDIR)\Compare.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF
