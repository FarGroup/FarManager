# Microsoft Developer Studio Project File - Name="far" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=far - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "far.mak".
!MESSAGE 
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

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/FAR", BAAAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "far - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release.vc"
# PROP BASE Intermediate_Dir "Release.vc"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release.vc"
# PROP Intermediate_Dir "Release.vc/obj"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /Gr /MT /Gi /O1 /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "FARTRY" /D "CREATE_JUNCTION" /D "USE_WFUNC" /YX /J /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib winspool.lib advapi32.lib shell32.lib mpr.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "far - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug.vc"
# PROP BASE Intermediate_Dir "Debug.vc"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug.vc"
# PROP Intermediate_Dir "Debug.vc/obj"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /Zp4 /MTd /W3 /Gm /Gi /ZI /Od /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "FARTRY" /D "CREATE_JUNCTION" /D "DIRECT_RT" /D "SYSLOG" /D "USE_WFUNC" /D "FAR_ALPHA_VERSION" /YX /J /FD /GZ /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib winspool.lib advapi32.lib shell32.lib mpr.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "far - Win32 Release"
# Name "far - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\cddrv.cpp
# End Source File
# Begin Source File

SOURCE=.\CFileMask.cpp
# End Source File
# Begin Source File

SOURCE=.\chgmmode.cpp
# End Source File
# Begin Source File

SOURCE=.\chgprior.cpp
# End Source File
# Begin Source File

SOURCE=.\clipboard.cpp
# End Source File
# Begin Source File

SOURCE=.\cmdline.cpp
# End Source File
# Begin Source File

SOURCE=.\cmem.cpp
# End Source File
# Begin Source File

SOURCE=.\config.cpp
# End Source File
# Begin Source File

SOURCE=.\constitle.cpp
# End Source File
# Begin Source File

SOURCE=.\copy.cpp
# End Source File
# Begin Source File

SOURCE=.\ctrlobj.cpp
# End Source File
# Begin Source File

SOURCE=.\cvtname.cpp
# End Source File
# Begin Source File

SOURCE=.\del.cpp
# End Source File
# Begin Source File

SOURCE=.\delete.cpp
# End Source File
# Begin Source File

SOURCE=.\dialog.cpp
# End Source File
# Begin Source File

SOURCE=.\dizlist.cpp
# End Source File
# Begin Source File

SOURCE=.\dlgedit.cpp
# End Source File
# Begin Source File

SOURCE=.\edit.cpp
# End Source File
# Begin Source File

SOURCE=.\editor.cpp
# End Source File
# Begin Source File

SOURCE=.\eject.cpp
# End Source File
# Begin Source File

SOURCE=.\execute.cpp
# End Source File
# Begin Source File

SOURCE=.\far.def
# End Source File
# Begin Source File

SOURCE=.\far.rc
# End Source File
# Begin Source File

SOURCE=.\farexcpt.cpp
# End Source File
# Begin Source File

SOURCE=.\farqueue.cpp
# End Source File
# Begin Source File

SOURCE=.\farrtl.cpp
# End Source File
# Begin Source File

SOURCE=.\farwinapi.cpp
# End Source File
# Begin Source File

SOURCE=.\ffolders.cpp
# End Source File
# Begin Source File

SOURCE=.\fileattr.cpp
# End Source File
# Begin Source File

SOURCE=.\fileedit.cpp
# End Source File
# Begin Source File

SOURCE=.\filefilter.cpp
# End Source File
# Begin Source File

SOURCE=.\filefilterparams.cpp
# End Source File
# Begin Source File

SOURCE=.\filelist.cpp
# End Source File
# Begin Source File

SOURCE=.\FileMasksProcessor.cpp
# End Source File
# Begin Source File

SOURCE=.\FileMasksWithExclude.cpp
# End Source File
# Begin Source File

SOURCE=.\fileowner.cpp
# End Source File
# Begin Source File

SOURCE=.\filepanels.cpp
# End Source File
# Begin Source File

SOURCE=.\filestr.cpp
# End Source File
# Begin Source File

SOURCE=.\filetype.cpp
# End Source File
# Begin Source File

SOURCE=.\fileview.cpp
# End Source File
# Begin Source File

SOURCE=.\findfile.cpp
# End Source File
# Begin Source File

SOURCE=.\flink.cpp
# End Source File
# Begin Source File

SOURCE=.\flmodes.cpp
# End Source File
# Begin Source File

SOURCE=.\flplugin.cpp
# End Source File
# Begin Source File

SOURCE=.\flshow.cpp
# End Source File
# Begin Source File

SOURCE=.\flupdate.cpp
# End Source File
# Begin Source File

SOURCE=.\fnparce.cpp
# End Source File
# Begin Source File

SOURCE=.\foldtree.cpp
# End Source File
# Begin Source File

SOURCE=.\frame.cpp
# End Source File
# Begin Source File

SOURCE=.\gettable.cpp
# End Source File
# Begin Source File

SOURCE=.\global.cpp
# End Source File
# Begin Source File

SOURCE=.\grabber.cpp
# End Source File
# Begin Source File

SOURCE=.\help.cpp
# End Source File
# Begin Source File

SOURCE=.\hilight.cpp
# End Source File
# Begin Source File

SOURCE=.\history.cpp
# End Source File
# Begin Source File

SOURCE=.\hmenu.cpp
# End Source File
# Begin Source File

SOURCE=.\infolist.cpp
# End Source File
# Begin Source File

SOURCE=.\interf.cpp
# End Source File
# Begin Source File

SOURCE=.\iswind.cpp
# End Source File
# Begin Source File

SOURCE=.\keybar.cpp
# End Source File
# Begin Source File

SOURCE=.\keyboard.cpp
# End Source File
# Begin Source File

SOURCE=.\language.cpp
# End Source File
# Begin Source File

SOURCE=.\local.cpp
# End Source File
# Begin Source File

SOURCE=.\lockscrn.cpp
# End Source File
# Begin Source File

SOURCE=.\macro.cpp
# End Source File
# Begin Source File

SOURCE=.\main.cpp
# End Source File
# Begin Source File

SOURCE=.\manager.cpp
# End Source File
# Begin Source File

SOURCE=.\menubar.cpp
# End Source File
# Begin Source File

SOURCE=.\message.cpp
# End Source File
# Begin Source File

SOURCE=.\mix.cpp
# End Source File
# Begin Source File

SOURCE=.\mkdir.cpp
# End Source File
# Begin Source File

SOURCE=.\mktemp.c
# End Source File
# Begin Source File

SOURCE=.\modal.cpp
# End Source File
# Begin Source File

SOURCE=.\namelist.cpp
# End Source File
# Begin Source File

SOURCE=.\new.cpp
# End Source File
# Begin Source File

SOURCE=.\options.cpp
# End Source File
# Begin Source File

SOURCE=.\palette.cpp
# End Source File
# Begin Source File

SOURCE=.\panel.cpp
# End Source File
# Begin Source File

SOURCE=.\plist.cpp
# End Source File
# Begin Source File

SOURCE=.\plognmn.cpp
# End Source File
# Begin Source File

SOURCE=.\plugapi.cpp
# End Source File
# Begin Source File

SOURCE=.\plugins.cpp
# End Source File
# Begin Source File

SOURCE=.\poscache.cpp
# End Source File
# Begin Source File

SOURCE=.\print.cpp
# End Source File
# Begin Source File

SOURCE=.\qsort.c
# End Source File
# Begin Source File

SOURCE=.\qsortex.c
# End Source File
# Begin Source File

SOURCE=.\qview.cpp
# End Source File
# Begin Source File

SOURCE=.\rdrwdsk.cpp
# End Source File
# Begin Source File

SOURCE=.\RefreshFrameManager.cpp
# End Source File
# Begin Source File

SOURCE=.\registry.cpp
# End Source File
# Begin Source File

SOURCE=.\savefpos.cpp
# End Source File
# Begin Source File

SOURCE=.\savescr.cpp
# End Source File
# Begin Source File

SOURCE=.\scantree.cpp
# End Source File
# Begin Source File

SOURCE=.\scrbuf.cpp
# End Source File
# Begin Source File

SOURCE=.\scrobj.cpp
# End Source File
# Begin Source File

SOURCE=.\scrsaver.cpp
# End Source File
# Begin Source File

SOURCE=.\setattr.cpp
# End Source File
# Begin Source File

SOURCE=.\setcolor.cpp
# End Source File
# Begin Source File

SOURCE=.\stddlg.cpp
# End Source File
# Begin Source File

SOURCE=.\strdup.c
# End Source File
# Begin Source File

SOURCE=.\strftime.cpp
# End Source File
# Begin Source File

SOURCE=.\timefunc.cpp
# End Source File
# Begin Source File

SOURCE=.\strmix.cpp
# End Source File
# Begin Source File

SOURCE=.\strncat.c
# End Source File
# Begin Source File

SOURCE=.\strncpy.c
# End Source File
# Begin Source File

SOURCE=.\syntax.cpp
# End Source File
# Begin Source File

SOURCE=.\tvar.cpp
# End Source File
# Begin Source File

SOURCE=.\TVMStack.cpp
# End Source File
# Begin Source File

SOURCE=.\syslog.cpp
# End Source File
# Begin Source File

SOURCE=.\treelist.cpp
# End Source File
# Begin Source File

SOURCE=.\udlist.cpp
# End Source File
# Begin Source File

SOURCE=.\usbeject.cpp
# End Source File
# Begin Source File

SOURCE=.\usermenu.cpp
# End Source File
# Begin Source File

SOURCE=.\viewer.cpp
# End Source File
# Begin Source File

SOURCE=.\vmenu.cpp
# End Source File
# Begin Source File

SOURCE=.\xlat.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\BaseFileMask.hpp
# End Source File
# Begin Source File

SOURCE=.\bitflags.hpp
# End Source File
# Begin Source File

SOURCE=.\CFileMask.hpp
# End Source File
# Begin Source File

SOURCE=.\chgmmode.hpp
# End Source File
# Begin Source File

SOURCE=.\chgprior.hpp
# End Source File
# Begin Source File

SOURCE=.\cmdline.hpp
# End Source File
# Begin Source File

SOURCE=.\colors.hpp

!IF  "$(CFG)" == "far - Win32 Release"

# Begin Custom Build - Преобразование $(InputPath) в дистрибутивный вид
InputPath=.\colors.hpp

"INCLUDE/$(InputPath)" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	awk -f plugins.awk -v p1=1 -v p2=70  $(InputPath) > INCLUDE/$(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "far - Win32 Debug"

# Begin Custom Build - Преобразование $(InputPath) в дистрибутивный вид
InputPath=.\colors.hpp

"INCLUDE/$(InputPath)" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	awk -f plugins.awk -v p1=1 -v p2=70  $(InputPath) > INCLUDE/$(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\common.hpp
# End Source File
# Begin Source File

SOURCE=.\const.hpp
# End Source File
# Begin Source File

SOURCE=.\constitle.hpp
# End Source File
# Begin Source File

SOURCE=.\copy.hpp
# End Source File
# Begin Source File

SOURCE=.\ctrlobj.hpp
# End Source File
# Begin Source File

SOURCE=.\dialog.hpp
# End Source File
# Begin Source File

SOURCE=.\dizlist.hpp
# End Source File
# Begin Source File

SOURCE=.\dlgedit.hpp
# End Source File
# Begin Source File

SOURCE=.\edit.hpp
# End Source File
# Begin Source File

SOURCE=.\editor.hpp
# End Source File
# Begin Source File

SOURCE=.\farconst.hpp
# End Source File
# Begin Source File

SOURCE=.\farexcpt.hpp
# End Source File
# Begin Source File

SOURCE=.\farftp.hpp
# End Source File
# Begin Source File

SOURCE=.\farqueue.hpp
# End Source File
# Begin Source File

SOURCE=.\farversion.inc
# End Source File
# Begin Source File

SOURCE=.\fileedit.hpp
# End Source File
# Begin Source File

SOURCE=.\filefilter.hpp
# End Source File
# Begin Source File

SOURCE=.\filefilterparams.hpp
# End Source File
# Begin Source File

SOURCE=.\filelist.hpp
# End Source File
# Begin Source File

SOURCE=.\FileMasksProcessor.hpp
# End Source File
# Begin Source File

SOURCE=.\FileMasksWithExclude.hpp
# End Source File
# Begin Source File

SOURCE=.\filepanels.hpp
# End Source File
# Begin Source File

SOURCE=.\filestr.hpp
# End Source File
# Begin Source File

SOURCE=.\fileview.hpp
# End Source File
# Begin Source File

SOURCE=.\findfile.hpp
# End Source File
# Begin Source File

SOURCE=.\flink.hpp
# End Source File
# Begin Source File

SOURCE=.\fn.hpp
# End Source File
# Begin Source File

SOURCE=.\foldtree.hpp
# End Source File
# Begin Source File

SOURCE=.\frame.hpp
# End Source File
# Begin Source File

SOURCE=.\global.hpp
# End Source File
# Begin Source File

SOURCE=.\grabber.hpp
# End Source File
# Begin Source File

SOURCE=.\headers.hpp
# End Source File
# Begin Source File

SOURCE=.\help.hpp
# End Source File
# Begin Source File

SOURCE=.\hilight.hpp
# End Source File
# Begin Source File

SOURCE=.\history.hpp
# End Source File
# Begin Source File

SOURCE=.\hmenu.hpp
# End Source File
# Begin Source File

SOURCE=.\infolist.hpp
# End Source File
# Begin Source File

SOURCE=.\int64.hpp
# End Source File
# Begin Source File

SOURCE=.\keybar.hpp
# End Source File
# Begin Source File

SOURCE=.\keys.hpp

!IF  "$(CFG)" == "far - Win32 Release"

# Begin Custom Build - Преобразование $(InputPath) в дистрибутивный вид
InputPath=.\keys.hpp

"INCLUDE/$(InputPath)" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	awk -f plugins.awk -v p1=1 -v p2=70  $(InputPath) > INCLUDE/$(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "far - Win32 Debug"

# Begin Custom Build - Преобразование $(InputPath) в дистрибутивный вид
InputPath=.\keys.hpp

"INCLUDE/$(InputPath)" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	awk -f plugins.awk -v p1=1 -v p2=70  $(InputPath) > INCLUDE/$(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\lang.hpp
# End Source File
# Begin Source File

SOURCE=.\language.hpp
# End Source File
# Begin Source File

SOURCE=.\lasterror.hpp
# End Source File
# Begin Source File

SOURCE=.\lockscrn.hpp
# End Source File
# Begin Source File

SOURCE=.\macro.hpp
# End Source File
# Begin Source File

SOURCE=.\macroopcode.hpp
# End Source File
# Begin Source File

SOURCE=.\manager.hpp
# End Source File
# Begin Source File

SOURCE=.\menubar.hpp
# End Source File
# Begin Source File

SOURCE=.\modal.hpp
# End Source File
# Begin Source File

SOURCE=.\namelist.hpp
# End Source File
# Begin Source File

SOURCE=.\panel.hpp
# End Source File
# Begin Source File

SOURCE=.\plognmn.hpp
# End Source File
# Begin Source File

SOURCE=.\plugin.hpp

!IF  "$(CFG)" == "far - Win32 Release"

# Begin Custom Build - Преобразование $(InputPath) в дистрибутивный вид
InputPath=.\plugin.hpp

"INCLUDE/$(InputPath)" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	awk -f plugins.awk -v p1=1 -v p2=70  $(InputPath) > INCLUDE/$(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "far - Win32 Debug"

# Begin Custom Build - Преобразование $(InputPath) в дистрибутивный вид
InputPath=.\plugin.hpp

"INCLUDE/$(InputPath)" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	awk -f plugins.awk -v p1=1 -v p2=70  $(InputPath) > INCLUDE/$(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\plugins.hpp
# End Source File
# Begin Source File

SOURCE=.\poscache.hpp
# End Source File
# Begin Source File

SOURCE=.\qview.hpp
# End Source File
# Begin Source File

SOURCE=.\rdrwdsk.hpp
# End Source File
# Begin Source File

SOURCE=.\RefreshFrameManager.hpp
# End Source File
# Begin Source File

SOURCE=.\res.hpp
# End Source File
# Begin Source File

SOURCE=.\savefpos.hpp
# End Source File
# Begin Source File

SOURCE=.\savescr.hpp
# End Source File
# Begin Source File

SOURCE=.\scantree.hpp
# End Source File
# Begin Source File

SOURCE=.\scrbuf.hpp
# End Source File
# Begin Source File

SOURCE=.\scrobj.hpp
# End Source File
# Begin Source File

SOURCE=.\struct.hpp
# End Source File
# Begin Source File

SOURCE=.\syntax.hpp
# End Source File
# Begin Source File

SOURCE=.\tvar.hpp
# End Source File
# Begin Source File

SOURCE=.\TVMStack.hpp
# End Source File
# Begin Source File

SOURCE=.\syslog.hpp
# End Source File
# Begin Source File

SOURCE=.\TList.hpp
# End Source File
# Begin Source File

SOURCE=.\treelist.hpp
# End Source File
# Begin Source File

SOURCE=.\udlist.hpp
# End Source File
# Begin Source File

SOURCE=.\viewer.hpp
# End Source File
# Begin Source File

SOURCE=.\vmenu.hpp
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\Far.ico
# End Source File
# Begin Source File

SOURCE=.\FarEng.hlf

!IF  "$(CFG)" == "far - Win32 Release"

# Begin Custom Build
OutDir=.\Release
InputPath=.\FarEng.hlf
InputName=FarEng

"$(OutDir)\$(InputName).hlf" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	awk -f mkhlf.awk -v FV1=1 -v FV2=70 -v FV3=6 -v BETA=" alpha " $(InputName).hlf > $(OutDir)\$(InputName).hlf

# End Custom Build

!ELSEIF  "$(CFG)" == "far - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
InputPath=.\FarEng.hlf
InputName=FarEng

"$(OutDir)\$(InputName).hlf" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	awk -f mkhlf.awk -v FV1=1 -v FV2=70 -v FV3=6 -v BETA=" alpha "  $(InputName).hlf > $(OutDir)\$(InputName).hlf

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\FarEng.lng

!IF  "$(CFG)" == "far - Win32 Release"

# Begin Custom Build
OutDir=.\Release
InputPath=.\FarEng.lng
InputName=FarEng

"$(OutDir)\$(InputName).lng" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	echo f|xcopy $(Inputpath) $(OutDir)\$(InputName).lng /Y /R

# End Custom Build

!ELSEIF  "$(CFG)" == "far - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
InputPath=.\FarEng.lng
InputName=FarEng

"$(OutDir)\$(InputName).lng" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	echo f|xcopy $(Inputpath) $(OutDir)\$(InputName).lng /Y /R

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\FarRus.hlf

!IF  "$(CFG)" == "far - Win32 Release"

# Begin Custom Build
OutDir=.\Release
InputPath=.\FarRus.hlf
InputName=FarRus

"$(OutDir)\$(InputName).hlf" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	awk -f mkhlf.awk -v FV1=1 -v FV2=70 -v FV3=6 -v BETA=" alpha "  $(InputName).hlf > $(OutDir)\$(InputName).hlf

# End Custom Build

!ELSEIF  "$(CFG)" == "far - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
InputPath=.\FarRus.hlf
InputName=FarRus

"$(OutDir)\$(InputName).hlf" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	awk -f mkhlf.awk -v FV1=1 -v FV2=70 -v FV3=6 -v BETA=" alpha "  $(InputName).hlf > $(OutDir)\$(InputName).hlf

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\FarRus.lng

!IF  "$(CFG)" == "far - Win32 Release"

# Begin Custom Build
OutDir=.\Release
InputPath=.\FarRus.lng
InputName=FarRus

"$(OutDir)\$(InputName).lng" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	echo f|xcopy $(Inputpath) $(OutDir)\$(InputName).lng /Y /R

# End Custom Build

!ELSEIF  "$(CFG)" == "far - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
InputPath=.\FarRus.lng
InputName=FarRus

"$(OutDir)\$(InputName).lng" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	echo f|xcopy $(Inputpath) $(OutDir)\$(InputName).lng /Y /R

# End Custom Build

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
