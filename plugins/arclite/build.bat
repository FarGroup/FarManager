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

  set "ccomp=vc gcc"
  set "nbits=32 64"
  set "deb_b=N"
  set "clean=N"& set "cleanonly=N"
  set "vcbld=nmake"
  set "vcbld=msbuild"
  set "vcver=15"

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
  for %%p in (32/64 32-64 32x64) do if /i "%%p" == "%~1" set "nbits=32/64"
  for %%p in (64/32 64-32 64x32) do if /i "%%p" == "%~1" set "nbits=64/32"
  for %%p in (rebuild clean) do if /i "%%p" == "%~1" set "clean=Y"
  for %%p in (cleanonly) do if /i "%%p" == "%~1" set "clean=Y"& set "cleanonly=Y"
  for %%p in (debug dbg) do if /i "%%p" == "%~1" set "deb_b=Y"
  for %%p in (msbuild) do if /i "%%p" == "%~1" set "vcbld=Msbuild"
  for %%p in (mmake) do if /i "%%p" == "%~1" set "vcbld=nmake"
  for %%p in (15 15.0 141 vc15 2017 vs2017) do if /i "%%p" == "%~1" set "vcver=15"
goto :EOF

:init
  set dirbit=%2
  set dirbit=%dirbit:~0,2%
  if "%1"=="vc" (set "cver=%vcver%") else (set "cver=")
  echo.
  echo build far-%2 %1%cver% [clean=%clean% debug=%deb_b%]
  echo.
  for %%f in (copyright.inc far.rc) do if exist bootsrap\%%f del /q bootstrap\%%f >NUL
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
  set m=nmake -f makefile_vc
  if /i "64" == "%dirbit%" (set m=%m% CPU=AMD64) else (set m=%m% CPU=X86)
goto :do_make
:msbuild
  if /i "Y"  == "%clean%"  (set b=Rebuild) else (set b=Build)
  if /i "Y"  == "%cleanonly%"                    set b=Clean
  if /i "64" == "%dirbit%" (set p=x64)    else  (set p=Win32)
  if /i "Y"  == "%deb_b%"  (set c=Debug) else (set c=Release)
  msbuild arclite.vcxproj /nologo /t:%b% /p:Configuration=%c%;Platform=%p%
goto :EOF

:set_vc
  set "PATH=%opath%"
  set "VisualStudioVersion=%vcver%.0"
  set "vcmod=%~1"
  if "%vcmod:~0,2%" == "32" set "vcmod=x86"
  if "%vcmod:~0,2%" == "64" set "vcmod=x86_amd64"
  if "%vcver%" == "14" call "%VS140COMNTOOLS%\..\..\vc\vcvarsall.bat" %vcmod%
  if "%vcver%" == "15" pushd .
  if "%vcver%" == "15" call "%VS150COMNTOOLS%\..\..\VC\Auxiliary\Build\vcvarsall.bat" %vcmod%
  if "%vcver%" == "15" popd
goto :EOF
