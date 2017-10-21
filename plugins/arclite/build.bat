@echo off
  rem =======================================================
  rem build [Debug|Release] [nmake|msbuild] [clean] [vcNN] ==
  rem build 7z [overwrite] [vcNN]                          ==
  rem =======================================================

  set "c=Release" & set "c1=" & set "clean=N" & set "over=N"
  set "m=nmake" & rem msbuild

  call "%~dp0..\..\far\build.bat" set_vcver %*
  for %%a in (%*) do call :opts %%a

  call :copy_7z "%~dp0" 7z\dll\final.32W.vc 7z\src\CPP\7zip\Bundles\Format7zF\Debug
  call :copy_7z "%~dp0" 7z\dll\final.64W.vc 7z\src\CPP\7zip\Bundles\Format7zF\x64\Debug
  if /i "%m%" == "7z" goto :EOF

:cont
  pushd "%~dp0"
  call :build x86       X86   Win32
  call :build x86_amd64 AMD64 x64
  popd
goto :EOF

:build
  call :set_vc %1
  if /i "%clean%" == "Y" goto :clean
  if /i "%m%"=="nmake"   %m% -nologo -f makefile_vc CPU=%2 VisualStudioVersion=%vcver%.0 %c1%
  if /i "%m%"=="msbuild" %m% arclite.vcxproj /nologo /t:Build /p:Configuration=%c%;Platform=%3
goto :EOF
:clean
  if /i "%m%"=="nmake"   %m% -nologo -f makefile_vc CPU=%2 VisualStudioVersion=%vcver%.0 %c1% clean
  if /i "%m%"=="msbuild" %m% arclite.vcxproj /nologo /t:Clean /p:Configuration=%c%;Platform=%3
goto :EOF

:copy_7z
  if /i not "%over%" == "Y" if exist "%~1%~3\7z.dll"   goto :EOF
  if not exist "%~1%~3\*" echo md "%~3"
  if not exist "%~1%~3\*" md "%~1%~3"
  echo copy /y "%~2\7z.dll" "%~3\7z.dll"
  copy 1>nul /y "%~1%~2\7z.dll" "%~1%~3\7z.dll"
goto :EOF

:opts
  if /i "%~1" == "Debug"     set "c=Debug"   & set "c1=DEBUG=1"
  if /i "%~1" == "Release"   set "c=Release" & set "c1="
  if /i "%~1" == "nmake"     set "m=nmake"
  if /i "%~1" == "msbuild"   set "m=msbuild"
  if /i "%~1" == "clean"     set "clean=Y"
  if /i "%~1" == "clear"     set "clean=Y"
  if /i "%~1" == "7z"        set "m=7z"
  if /i "%~1" == "over"      set "over=Y"
  if /i "%~1" == "overwrite" set "over=Y"
  if /i "%~1" == "copy"      set "over=Y"
goto :EOF

:set_vc
  call "%~dp0..\..\far\build.bat" set_vcvars %1
goto :EOF