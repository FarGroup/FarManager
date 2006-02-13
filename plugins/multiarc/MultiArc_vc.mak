!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE
NULL=nul
!ENDIF

INCLUDE = $(INCLUDE);..\common;.\libpcre

FINALPATH=FINAL
INTDIR=obj\VC
CODDIR=obj\VC\cod
CC=cl.exe
RSC=rc.exe
LINK=link.exe

#CFLAGS=/nologo /Gr /Zp2 /ML /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "MULTIARC_EXPORTS"
CFLAGS  =/nologo /Gs /Ox /Zp1 /GR- /c /FD /opt:nowin98 /Fa"$(CODDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /D "WIN32" /D "NDEBUG"
RSC_PROJ=/l 0x409 /fo$(INTDIR)\MULTIARC.res /d "NDEBUG"
LINK_FLAGS=/DLL $(MSVCRT) kernel32.lib user32.lib advapi32.lib /nologo /incremental:no /pdb:none /map:$(FINALPATH)\multiarc.map /machine:I386 /def:MultiArc.def /out:"$(FINALPATH)\MULTIARC.DLL"  /opt:nowin98 /stub:..\common\minstub.exe /noentry /release
#
#"/ENTRY:DllEntryPoint"


OBJ_FILE= \
	"$(INTDIR)\ArcCfg.obj" \
	"$(INTDIR)\ArcCmd.obj" \
	"$(INTDIR)\ArcGet.obj" \
	"$(INTDIR)\ArcMix.obj" \
	"$(INTDIR)\ArcPlg.obj" \
	"$(INTDIR)\ArcProc.obj" \
	"$(INTDIR)\ArcPut.obj" \
	"$(INTDIR)\ArcRead.obj" \
	"$(INTDIR)\ArcReg.obj" \
	"$(INTDIR)\global.obj" \
	"$(INTDIR)\MultiArc.obj" \
	"$(INTDIR)\ownrtl.obj" \
	"$(INTDIR)\multiarc.res"


ALL : "$(FINALPATH)\MULTIARC.DLL" $(FINALPATH)\Formats\Custom.fmt $(FINALPATH)\Formats\Arc.fmt $(FINALPATH)\Formats\Ace.fmt $(FINALPATH)\Formats\Arj.fmt $(FINALPATH)\Formats\Cab.fmt $(FINALPATH)\Formats\Ha.fmt $(FINALPATH)\Formats\Lzh.fmt $(FINALPATH)\Formats\Rar.fmt $(FINALPATH)\Formats\TarGz.fmt $(FINALPATH)\Formats\Zip.fmt $(FINALPATH)\Formats\Arc.fmt


CLEAN :
	-@erase "$(INTDIR)\*.obj"
	-@erase "$(INTDIR)\*.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\*.exp"
	-@erase "$(INTDIR)\*.lib"
	-@erase "$(FINALPATH)\*.dll"


$(FINALPATH) :
    @if not exist "$(FINALPATH)/$(NULL)"  mkdir "$(FINALPATH)"
    @if not exist "$(INTDIR)/$(NULL)"  mkdir "$(INTDIR)"
    @if not exist "$(CODDIR)/$(NULL)"  mkdir "$(CODDIR)"


$(INTDIR) :
    @if not exist "$(FINALPATH)/$(NULL)"  mkdir "$(FINALPATH)"
    @if not exist "$(INTDIR)/$(NULL)"  mkdir "$(INTDIR)"
    @if not exist "$(CODDIR)/$(NULL)"  mkdir "$(CODDIR)"

$(CODDIR) :
    @if not exist "$(FINALPATH)/$(NULL)"  mkdir "$(FINALPATH)"
    @if not exist "$(INTDIR)/$(NULL)"  mkdir "$(INTDIR)"
    @if not exist "$(CODDIR)/$(NULL)"  mkdir "$(CODDIR)"


.c{$(INTDIR)}.obj::
   $(CC) @<<
   $(CFLAGS) $<
<<


.cpp{$(INTDIR)}.obj::
   $(CC) @<<
   $(CFLAGS) $<
<<



$(FINALPATH)\MULTIARC.DLL : "$(FINALPATH)" MultiArc.def $(OBJ_FILE)
    @$(LINK) @<<
  $(LINK_FLAGS) $(OBJ_FILE)
<<
   @copy  ArcEng.lng   $(FINALPATH)\ArcEng.lng > nul
   @copy  ArcRus.lng   $(FINALPATH)\ArcRus.lng > nul
   @copy  ArcEng.hlf   $(FINALPATH)\ArcEng.hlf > nul
   @copy  ArcRus.hlf   $(FINALPATH)\ArcRus.hlf > nul



$(INTDIR)\MULTIARC.res : multiarc.rc "$(INTDIR)" ../common/farversion.hpp multiarcversion.hpp
	$(RSC) $(RSC_PROJ) multiarc.rc



".\$(INTDIR)\ArcCfg.obj" : ArcCfg.cpp MArcLng.hpp MultiArc.hpp {$(INCLUDE)}fmt.hpp {$(INCLUDE)}plugin.hpp {$(INCLUDE)}farkeys.hpp "$(INTDIR)" "$(CODDIR)"

".\$(INTDIR)\ArcCmd.obj" : ArcCmd.cpp MArcLng.hpp MultiArc.hpp {$(INCLUDE)}fmt.hpp {$(INCLUDE)}plugin.hpp "$(INTDIR)" "$(CODDIR)"

".\$(INTDIR)\ArcGet.obj" : ArcGet.cpp MArcLng.hpp MultiArc.hpp {$(INCLUDE)}fmt.hpp {$(INCLUDE)}plugin.hpp "$(INTDIR)" "$(CODDIR)"

".\$(INTDIR)\ArcMix.obj" : ArcMix.cpp MArcLng.hpp MultiArc.hpp {$(INCLUDE)}fmt.hpp {$(INCLUDE)}plugin.hpp "$(INTDIR)" "$(CODDIR)"

".\$(INTDIR)\ArcPlg.obj" : ArcPlg.cpp MArcLng.hpp MultiArc.hpp {$(INCLUDE)}fmt.hpp {$(INCLUDE)}plugin.hpp "$(INTDIR)" "$(CODDIR)"

".\$(INTDIR)\ArcProc.obj" : ArcProc.cpp MArcLng.hpp MultiArc.hpp {$(INCLUDE)}fmt.hpp {$(INCLUDE)}plugin.hpp "$(INTDIR)" "$(CODDIR)"

".\$(INTDIR)\ArcPut.obj" : ArcPut.cpp MArcLng.hpp MultiArc.hpp {$(INCLUDE)}fmt.hpp {$(INCLUDE)}plugin.hpp "$(INTDIR)" "$(CODDIR)"

".\$(INTDIR)\ArcRead.obj" : ArcRead.cpp MArcLng.hpp MultiArc.hpp {$(INCLUDE)}fmt.hpp {$(INCLUDE)}plugin.hpp "$(INTDIR)" "$(CODDIR)"

".\$(INTDIR)\ArcReg.obj" : ArcReg.cpp MArcLng.hpp MultiArc.hpp {$(INCLUDE)}fmt.hpp {$(INCLUDE)}plugin.hpp "$(INTDIR)" "$(CODDIR)"

".\$(INTDIR)\global.obj" : global.cpp MArcLng.hpp MultiArc.hpp {$(INCLUDE)}fmt.hpp {$(INCLUDE)}plugin.hpp "$(INTDIR)" "$(CODDIR)"

".\$(INTDIR)\MultiArc.obj" : MultiArc.cpp MArcLng.hpp MultiArc.hpp {$(INCLUDE)}fmt.hpp {$(INCLUDE)}plugin.hpp "$(INTDIR)" "$(CODDIR)"


$(FINALPATH)\Formats\Ace.fmt : $(INTDIR)\Ace.obj Ace.cpp fmt.hpp Ace.def Ace.rc
	nmake /c fmt_vc.mak FMT=Ace


$(FINALPATH)\Formats\Arc.fmt : $(INTDIR)\Arc.obj Arc.cpp fmt.hpp Arc.def Arc.rc
	nmake /c fmt_vc.mak FMT=Arc


$(FINALPATH)\Formats\Arj.fmt : $(INTDIR)\Arj.obj Arj.cpp fmt.hpp Arj.def Arj.rc
	nmake /c fmt_vc.mak FMT=Arj


$(FINALPATH)\Formats\Cab.fmt : $(INTDIR)\Cab.obj Cab.cpp fmt.hpp Cab.def Cab.rc
	nmake /c fmt_vc.mak FMT=Cab


$(FINALPATH)\Formats\Custom.fmt : $(INTDIR)\Custom.obj Custom.cpp fmt.hpp Custom.def Custom.rc
	cd libpcre
	nmake /c makefile_lib_vc
	cd ..
	nmake /c fmt_vc.mak FMT=Custom


$(FINALPATH)\Formats\Ha.fmt : $(INTDIR)\Ha.obj Ha.cpp fmt.hpp Ha.def Ha.rc
	nmake /c fmt_vc.mak FMT=Ha


$(FINALPATH)\Formats\Lzh.fmt : $(INTDIR)\Lzh.obj Lzh.cpp fmt.hpp Lzh.def Lzh.rc
	nmake /c fmt_vc.mak FMT=Lzh


$(FINALPATH)\Formats\Rar.fmt : $(INTDIR)\Rar.obj Rar.cpp fmt.hpp Rar.def Rar.rc
	nmake /c fmt_vc.mak FMT=Rar


$(FINALPATH)\Formats\TarGz.fmt : $(INTDIR)\TarGz.obj TarGz.cpp fmt.hpp TarGz.def TarGz.rc
	nmake /c fmt_vc.mak FMT=TarGz


$(FINALPATH)\Formats\Zip.fmt : $(INTDIR)\Zip.obj Zip.cpp fmt.hpp Zip.def Zip.rc
	nmake /c fmt_vc.mak FMT=Zip
