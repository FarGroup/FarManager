@echo off
  if /i "%~1" == "set_vcver" goto :set_vcver
  if /i "%~1" == "set_vcvars" call :set_vcvars %~2 & goto :EOF

  setlocal
  rem examples:
  rem ---------------------------------------------------------------
  rem   build.bat
  rem   build.bat vc
  rem   build.bat vc10
  rem   build.bat vc12
  rem   build.bat vc 32 debug sln
  rem   build.bat gcc 64
  rem   build.bat clean gcc
  rem   build.bat clean gcc 32/64
  rem   build.bat 32
  rem ---------------------------------------------------------------

  set ccomp=vc gcc
  set nbits=32 64
  set deb_b=N
  set clean=N& set cleanonly=N
  set "vcsln=N" & set "vcver=" & rem -- vc  only
  set dwarf=N& rem -- gcc only
  rem if auto-detection doesn't work - change/uncomment line(s) below
  rem ---------------------------------------------------------------
  rem set MINGW32=Your_MinGW32_Path\
  rem set MINGW64=Your_MinGW64_Path\
  rem set GMAKE=gmake.exe
  rem ---------------------------------------------------------------
  rem or create 'build_settings.bat' file
  if exist "%~dpn0_settings.bat" call "%~dpn0_settings.bat"
  rem ---------------------------------------------------------------

  call :set_vcver %*
  for %%a in (%*) do call :proc_param %%a
  if not "%vcver%" == "" set "ccomp=vc"

  set opath=%PATH%
  for %%c in (%ccomp%) do for %%b in (%nbits%) do call :init %%c %%b& call :build_%%c %%b
  endlocal
goto :EOF

:set_gcc
  if "32" == "%~1" (set gccdir=%MINGW32%) else (set gccdir=%MINGW64%)
  if "" == "%gccdir%" if "32" == "%~1" (set gccdir=C:\MinGW\32\) else (set gccdir=C:\MinGW\64\)
  PATH=%gccdir%bin;%opath%
goto :EOF

:proc_param
  for %%p in (vc vcc msc) do if /i "%%p" == "%~1" set ccomp=vc
  for %%p in (gcc gnu) do if /i "%%p" == "%~1" set ccomp=gcc
  for %%p in (32 x86 win32) do if /i "%%p" == "%~1" set nbits=32
  for %%p in (64 x64 win64) do if /i "%%p" == "%~1" set nbits=64
  for %%p in (32/64 32-64 32x64) do if /i "%%p" == "%~1" set nbits=32/64
  for %%p in (64/32 64-32 64x32) do if /i "%%p" == "%~1" set nbits=64/32
  for %%p in (rebuild clean) do if /i "%%p" == "%~1" set clean=Y
  for %%p in (cleanonly) do if /i "%%p" == "%~1" set clean=Y& set cleanonly=Y
  for %%p in (debug dbg) do if /i "%%p" == "%~1" set deb_b=Y
  for %%p in (dev devenv) do if /i "%%p" == "%~1" set vcsln=Devenv
  for %%p in (msbuild) do if /i "%%p" == "%~1" set vcsln=Msbuild
  for %%p in (dwarf dw2) do if /i "%%p" == "%~1" set dwarf=Y
goto :EOF

:init
  set dirbit=%2
  set dirbit=%dirbit:~0,2%
  echo.
  if /i "vc" == "%1" (set x=sln=%vcsln%) else (set x=dwarf=%dwarf%)
  echo build far-%2 %1%vcver% [clean=%clean% debug=%deb_b% %x%]
  echo.
  for %%f in (copyright.inc far.rc) do if exist bootsrap\%%f del /q bootstrap\%%f >NUL
  call :set_%1 %2
goto :EOF

:build_gcc
  if "" == "%gmake%" set gmake=mingw32-make.exe
  set m=%gmake% --no-print-directory -f makefile_gcc DIRBIT=%dirbit%
  if /i "Y" == "%dwarf%" set m=%m% DW2=-dw2
:do_make
  if /i "Y" == "%deb_b%" set m=%m% DEBUG=1
  if /i "Y" == "%clean%" %m% clean
  if /i not "Y" == "%cleanonly%" %m%
goto :EOF

:build_vc
  if /i "Devenv"  == "%vcsln%" goto :solution
  if /i "Msbuild" == "%vcsln%" goto :solution
  set m=nmake -f makefile_vc
  if /i "64" == "%dirbit%" (set m=%m% CPU=AMD64) else (set m=%m% CPU=X86)
goto :do_make
:solution
  if /i "Y"  == "%clean%"  (set b=Rebuild) else (set b=Build)
  if /i "Y"  == "%cleanonly%"                    set b=Clean
  if /i "64" == "%dirbit%" (set p=x64)    else  (set p=Win32)
  if /i "Y"  == "%deb_b%"  (set c=Debug) else (set c=Release)
  if /i "Devenv" == "%vcsln%" devenv far.vcxproj /%b% "%c%^|%p%"
  if /i "Msbuild" == "%vcsln%" msbuild far.vcxproj /nologo /t:%b% /p:Configuration=%c%;Platform=%p%
goto :EOF

:set_vcver
  set "vcver="
  for %%v in (10 11 12 14) do for %%a in (%*) do if /i "vc%%v" == "%%a" set "vcver=%%v"
goto :EOF

:set_vc
:set_vcvars
  if "" == "%vcver%" if not "" == "%VS120COMNTOOLS%" if exist "%VS120COMNTOOLS%\..\..vc\vcvarsall.bat" set "vcver=12"
  if "" == "%vcver%" if not "" == "%VS100COMNTOOLS%" if exist "%VS100COMNTOOLS%\..\..vc\vcvarsall.bat" set "vcver=10"
  if "" == "%vcver%" if not "" == "%VS140COMNTOOLS%" if exist "%VS140COMNTOOLS%\..\..vc\vcvarsall.bat" set "vcver=14"
  if "" == "%vcver%" if not "" == "%VS110COMNTOOLS%" if exist "%VS110COMNTOOLS%\..\..vc\vcvarsall.bat" set "vcver=11"
  if "" == "%vcver%" set "vcver=12"
  set "VisualStudioVersion=%vcver%.0"
  if "" == "%~1" goto :EOF
  set "vcmod=%~1"
  if "%vcmod:~0,2%" == "32" set "vcmod=x86"
  if "%vcmod:~0,2%" == "64" set "vcmod=x86_amd64"
  if "%vcver%" == "10" call "%VS100COMNTOOLS%\..\..\vc\vcvarsall.bat" %vcmod%
  if "%vcver%" == "11" call "%VS110COMNTOOLS%\..\..\vc\vcvarsall.bat" %vcmod%
  if "%vcver%" == "12" call "%VS120COMNTOOLS%\..\..\vc\vcvarsall.bat" %vcmod%
  if "%vcver%" == "14" call "%VS140COMNTOOLS%\..\..\vc\vcvarsall.bat" %vcmod%
  set "vcmod="
goto :EOF