@echo off
  if /i "%~1" == "set_vcver" goto :set_vcver
  if /i "%~1" == "set_vcvars" call :set_vcvars %~2 & goto :EOF

  setlocal
  rem examples:
  rem ---------------------------------------------------------------
  rem   build.bat
  rem   build.bat vc
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
  set "hostbit=%~1"
  set "hostbit=%hostbit:~-2%"
  if "32" == "%hostbit%" (set gccdir=%MINGW32%) else (set gccdir=%MINGW64%)
  if "" == "%gccdir%" if "32" == "%hostbit%" (set gccdir=C:\MinGW\32\) else (set gccdir=C:\MinGW\64\)
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
  for %%p in (15 15.0 141 vc15 2017 vs2017) do if /i "%%p" == "%~1" set "vcver=15"
  for %%p in (16 16.0 142 vc16 2019 vs2019) do if /i "%%p" == "%~1" set "vcver=16"
  for %%p in (17 17.0 143 vc17 2022 vs2022) do if /i "%%p" == "%~1" set "vcver=17"
goto :EOF

:init
  set dirbit=%2
  set dirbit=%dirbit:~0,2%
  echo.
  if /i "vc" == "%1" (set x=sln=%vcsln%) else (set x=dwarf=%dwarf%)
  echo build far-%2 %1%vcver% [clean=%clean% debug=%deb_b% %x%]
  echo.
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
  for %%v in (15 16 17) do for %%a in (%*) do if /i "vc%%v" == "%%a" set "vcver=%%v"
goto :EOF

:set_vc
:set_vcvars
  if "" == "%vcver%" if not "" == "%VS160COMNTOOLS%" if exist "%VS160COMNTOOLS%\..\..\VC\Auxiliary\Build\vcvars32.bat" set "vcver=16"
  if "" == "%vcver%" if not "" == "%VS170COMNTOOLS%" if exist "%VS170COMNTOOLS%\..\..\VC\Auxiliary\Build\vcvars64.bat" set "vcver=17"
  if "" == "%vcver%" if not "" == "%VS150COMNTOOLS%" if exist "%VS150COMNTOOLS%\..\..\VC\Auxiliary\Build\vcvars32.bat" set "vcver=15"
  if "" == "%vcver%" set "vcver=16"
  set "VisualStudioVersion=%vcver%.0"
  if "" == "%~1" goto :EOF
  set "vcmod=32"
  if "%~1" == "64" set "vcmod=64"
  if "%vcver%" == "15" call "%VS150COMNTOOLS%\..\..\VC\Auxiliary\Build\vcvars%vcmod%.bat"
  if "%vcver%" == "16" call "%VS160COMNTOOLS%\..\..\VC\Auxiliary\Build\vcvars%vcmod%.bat"
  if "%vcver%" == "17" call "%VS170COMNTOOLS%\..\..\VC\Auxiliary\Build\vcvars%vcmod%.bat"
  set "vcmod="
goto :EOF