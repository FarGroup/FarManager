SET MSSdk=C:\VC8
Set SdkTools=%MSSdk%\Bin
Set OSLibraries=%MSSdk%\Lib
Set OSIncludes=%MSSdk%\Include;%MSSdk%\Include\gl
Set VCTools=%MSSdk%\Bin
Set VCLibraries=%MSSdk%\Lib
Set VCIncludes=%MSSdk%\Include;%MSSdk%\Include\Sys
Set ReferenceAssemblies=%ProgramFiles%\Reference Assemblies\Microsoft\WinFX\v3.0
:: Setting the path
SET Path=%VCTools%;%SdkTools%;%Path%
SET Lib=%VCLibraries%;%OSLibraries%;%Lib%
SET Include=%VCIncludes%;%OSIncludes%;%Include%
SET CPU=i386
SET APPVER=6.0
SET Path=%Path%;C:\Program Files\Windows Installer XML v3\bin

cl -nologo -EHsc -DNIGHTLY genscript.cpp

SET MSSdk=C:\VC8
Set SdkTools=%MSSdk%\Bin
Set OSLibraries=%MSSdk%\Lib\x64
Set OSIncludes=%MSSdk%\Include;%MSSdk%\Include\gl
Set VCTools=%MSSdk%\Bin\x86_x64
Set VCLibraries=%MSSdk%\Lib\x64
Set VCIncludes=%MSSdk%\Include;%MSSdk%\Include\Sys
Set ReferenceAssemblies=%ProgramFiles%\Reference Assemblies\Microsoft\WinFX\v3.0
:: Setting the path
SET Path=%VCTools%;%SdkTools%;%Path%
SET Lib=%VCLibraries%;%OSLibraries%;%Lib%
SET Include=%VCIncludes%;%OSIncludes%;%Include%
SET CPU=AMD64
SET APPVER=6.0

genscript c:\src\outfinalnew64 x64

nmake -nologo

genscript c:\src\outfinalold64 x64

nmake -nologo
