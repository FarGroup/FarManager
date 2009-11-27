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

nmake /f makefile_vc build USEDEPS=1
