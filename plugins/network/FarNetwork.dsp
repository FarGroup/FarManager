# Microsoft Developer Studio Project File - Name="FarNetwork" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=FarNetwork - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "FarNetwork.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "FarNetwork.mak" CFG="FarNetwork - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "FarNetwork - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "FarNetwork - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "FarNetwork - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "FARNETWORK_EXPORTS" /YX /FD /c
# ADD CPP /nologo /Zp1 /MD /W3 /Zi /O1 /Op /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_USRDLL" /D "_export" /FR /YX /FD /EHa /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x419 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 Mpr.lib User32.lib Advapi32.lib kernel32.lib /nologo /dll /debug /machine:I386 /out:"H:/FARDEBUG/PLUGINS/NETWORK/NETWORK.DLL" /opt:nowin98
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=dorar.cmd
# End Special Build Tool

!ELSEIF  "$(CFG)" == "FarNetwork - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "FARNETWORK_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /Zp2 /MTd /W3 /Gm /Gi /GX /ZI /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "FARNETWORK_EXPORTS" /D "_export" /D "NETWORK_LOGGING" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "_DEBUG"
# ADD RSC /l 0x419 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 Mpr.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /out:"H:/FARDEBUG/PLUGINS/NETWORK/NETWORK.DLL" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "FarNetwork - Win32 Release"
# Name "FarNetwork - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\NetCfg.cpp
# End Source File
# Begin Source File

SOURCE=.\NetClass.cpp
# End Source File
# Begin Source File

SOURCE=.\NetCommon.cpp
# End Source File
# Begin Source File

SOURCE=.\NetFavorites.cpp
# End Source File
# Begin Source File

SOURCE=.\NetMix.cpp
# End Source File
# Begin Source File

SOURCE=.\NetNT.cpp
# End Source File
# Begin Source File

SOURCE=.\NetReg.cpp
# End Source File
# Begin Source File

SOURCE=.\Network.cpp
# End Source File
# Begin Source File

SOURCE=.\Network.def
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\NetCfg.hpp
# End Source File
# Begin Source File

SOURCE=.\NetClass.hpp
# End Source File
# Begin Source File

SOURCE=.\NetCommon.hpp
# End Source File
# Begin Source File

SOURCE=.\NetFavorites.hpp
# End Source File
# Begin Source File

SOURCE=.\NetLng.hpp
# End Source File
# Begin Source File

SOURCE=.\NetMacros.hpp
# End Source File
# Begin Source File

SOURCE=.\NetReg.hpp
# End Source File
# Begin Source File

SOURCE=.\Network.hpp
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\Network.rc

!IF  "$(CFG)" == "FarNetwork - Win32 Release"

# PROP Ignore_Default_Tool 1
USERDEP__NETWO="build.txt"	"Network.tmpl"	
# Begin Custom Build
IntDir=.\Release
InputPath=.\Network.rc

"$(IntDir)/Network.res" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	verbuilder.exe build.txt Network.tmpl 
	rc /fo "$(IntDir)/Network.res" Network.rc 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "FarNetwork - Win32 Debug"

!ENDIF 

# End Source File
# End Group
# Begin Group "Lng and hlf Files"

# PROP Default_Filter ".lng .hlf"
# Begin Source File

SOURCE=.\NetEng.hlf

!IF  "$(CFG)" == "FarNetwork - Win32 Release"

# Begin Custom Build
InputDir=.
TargetDir=\FARDEBUG\PLUGINS\NETWORK
InputPath=.\NetEng.hlf
InputName=NetEng

"$(TargetDir)\$(InputName).hlf" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputDir)\$(InputName).hlf $(TargetDir)\$(InputName).hlf

# End Custom Build

!ELSEIF  "$(CFG)" == "FarNetwork - Win32 Debug"

# Begin Custom Build
InputDir=.
TargetDir=\FARDEBUG\PLUGINS\NETWORK
InputPath=.\NetEng.hlf
InputName=NetEng

"$(TargetDir)\$(InputName).hlf" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputDir)\$(InputName).hlf $(TargetDir)\$(InputName).hlf

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetEng.lng

!IF  "$(CFG)" == "FarNetwork - Win32 Release"

# Begin Custom Build
InputDir=.
TargetDir=\FARDEBUG\PLUGINS\NETWORK
InputPath=.\NetEng.lng
InputName=NetEng

"$(TargetDir)\$(InputName).lng" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputDir)\$(InputName).lng $(TargetDir)\$(InputName).lng

# End Custom Build

!ELSEIF  "$(CFG)" == "FarNetwork - Win32 Debug"

# Begin Custom Build
InputDir=.
TargetDir=\FARDEBUG\PLUGINS\NETWORK
InputPath=.\NetEng.lng
InputName=NetEng

"$(TargetDir)\$(InputName).lng" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputDir)\$(InputName).lng $(TargetDir)\$(InputName).lng

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetRus.hlf

!IF  "$(CFG)" == "FarNetwork - Win32 Release"

# Begin Custom Build
InputDir=.
TargetDir=\FARDEBUG\PLUGINS\NETWORK
InputPath=.\NetRus.hlf
InputName=NetRus

"$(TargetDir)\$(InputName).hlf" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputDir)\$(InputName).hlf $(TargetDir)\$(InputName).hlf

# End Custom Build

!ELSEIF  "$(CFG)" == "FarNetwork - Win32 Debug"

# Begin Custom Build
InputDir=.
TargetDir=\FARDEBUG\PLUGINS\NETWORK
InputPath=.\NetRus.hlf
InputName=NetRus

"$(TargetDir)\$(InputName).hlf" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputDir)\$(InputName).hlf $(TargetDir)\$(InputName).hlf

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetRus.lng

!IF  "$(CFG)" == "FarNetwork - Win32 Release"

# Begin Custom Build
InputDir=.
TargetDir=\FARDEBUG\PLUGINS\NETWORK
InputPath=.\NetRus.lng
InputName=NetRus

"$(TargetDir)\$(InputName).lng" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputDir)\$(InputName).lng $(TargetDir)\$(InputName).lng

# End Custom Build

!ELSEIF  "$(CFG)" == "FarNetwork - Win32 Debug"

# Begin Custom Build
InputDir=.
TargetDir=\FARDEBUG\PLUGINS\NETWORK
InputPath=.\NetRus.lng
InputName=NetRus

"$(TargetDir)\$(InputName).lng" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputDir)\$(InputName).lng $(TargetDir)\$(InputName).lng

# End Custom Build

!ENDIF 

# End Source File
# End Group
# Begin Source File

SOURCE=.\build.txt
# End Source File
# Begin Source File

SOURCE=.\Network.tmpl
# End Source File
# Begin Source File

SOURCE=.\WhatsNew.Rus.txt
# End Source File
# End Target
# End Project
