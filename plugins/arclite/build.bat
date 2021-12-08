@echo off
  setlocal
  pushd "%~dp0"
  rem examples:
  rem ---------------------------------------------------------------
  rem   build.bat
  rem   build.bat vc
  rem   build.bat vc 32 debug
  rem   build.bat gcc 64
  rem   build.bat clean gcc
  rem   build.bat clean gcc 32/64
  rem   build.bat 32
  rem ---------------------------------------------------------------

  set "ccomp=vc"
  set "nbits=32 64" rem ARM64 -- it's arch really
  set "deb_b=N"
  set "clean=N"& set "cleanonly=N"
  set "vcbld=msbuild"
  set "vcver=16"

  for %%a in (%*) do call :proc_param %%a

  set "opath=%PATH%"
  for %%c in (%ccomp%) do for %%b in (%nbits%) do call :init %%c %%b& call :build_%%c %%b
  popd
  endlocal
goto :EOF

:set_gcc
  set "hostbit=%~1"
  set "hostbit=%hostbit:~-2%"
  if "32" == "%hostbit%" (set gccdir=C:\MinGW\32\) else (set gccdir=C:\MinGW\64\)
  PATH=%gccdir%bin;%opath%
goto :EOF

:proc_param
  for %%p in (vc vcc msc) do if /i "%%p" == "%~1" set "ccomp=vc"
  for %%p in (gcc gnu) do if /i "%%p" == "%~1" set "ccomp=gcc"
  for %%p in (32 x86 win32) do if /i "%%p" == "%~1" set "nbits=32"
  for %%p in (64 x64 win64) do if /i "%%p" == "%~1" set "nbits=64"
  for %%p in (arm64 arm) do if /i "%%p" == "%~1" set "nbits=arm64"  rem now: ONLY vc 16 msbuild
  for %%p in (32/64 32-64 32x64) do if /i "%%p" == "%~1" set "nbits=32/64"
  for %%p in (64/32 64-32 64x32) do if /i "%%p" == "%~1" set "nbits=64/32"
  for %%p in (rebuild clean) do if /i "%%p" == "%~1" set "clean=Y"
  for %%p in (cleanonly) do if /i "%%p" == "%~1" set "clean=Y"& set "cleanonly=Y"
  for %%p in (debug dbg) do if /i "%%p" == "%~1" set "deb_b=Y"
  for %%p in (msbuild) do if /i "%%p" == "%~1" set "vcbld=Msbuild"
  for %%p in (mmake) do if /i "%%p" == "%~1" set "vcbld=nmake"
  for %%p in (15 15.0 141 vc15 2017 vs2017) do if /i "%%p" == "%~1" set "vcver=15"
  for %%p in (16 16.0 142 vc16 2019 vs2019) do if /i "%%p" == "%~1" set "vcver=16"
goto :EOF

:init
  set dirbit=%2
  if not "%2" == "arm64" set dirbit=%dirbit:~0,2%
  if "%1"=="vc" (set "cver=%vcver%") else (set "cver=")
  echo.
  echo build far-%2 %1%cver% [clean=%clean% debug=%deb_b%]
  echo.
  call :set_%1 %2
goto :EOF

:build_gcc
  set gmake=mingw32-make.exe
  set m=%gmake% --no-print-directory -f makefile_gcc DIRBIT=%dirbit%
:do_make
  if /i "Y" == "%deb_b%" set m=%m% DEBUG=1
  if /i "Y" == "%clean%" %m% clean
  if /i not "Y" == "%cleanonly%" %m%
goto :EOF

:build_vc
  if /i "Msbuild" == "%vcbld%" goto :msbuild
  set m=nmake -f makefile_vc VisualStudioVersion=%VisualStudioVersion%
  if /i "64" == "%dirbit%" (set m=%m% CPU=AMD64) else (set m=%m% CPU=X86)
goto :do_make
:msbuild
  if /i "Y"  == "%clean%"  (set b=Rebuild) else (set b=Build)
  if /i "Y"  == "%cleanonly%"                    set b=Clean
  if /i "64" == "%dirbit%" (set p=x64)    else  (set p=Win32)
  if /i "arm64" == "%dirbit%" (set p=ARM64)
  if /i "Y"  == "%deb_b%"  (set c=Debug) else (set c=Release)
  msbuild arclite.vcxproj /nologo /t:%b% /p:Configuration=%c%;Platform=%p%
goto :EOF

:set_vc
  set "PATH=%opath%"
  set "VisualStudioVersion=%vcver%.0"
  set "vcmod=32"
  if "%~1" == "64" set "vcmod=64"
  if "%~1" == "arm64" set "vcmod=amd64_arm64"
  if "%vcver%" == "15" call "%VS150COMNTOOLS%\..\..\VC\Auxiliary\Build\vcvars%vcmod%.bat"
  if "%vcver%" == "16" call "%VS160COMNTOOLS%\..\..\VC\Auxiliary\Build\vcvars%vcmod%.bat"
goto :EOF
