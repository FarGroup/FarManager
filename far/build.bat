@echo off
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
  set clean=N
  set vcsln=N& rem -- vc  only
  set dwarf=N& rem -- gcc only
  rem if auto-detection doesn't work - change/uncomment line(s) below
  rem ---------------------------------------------------------------
  rem set VSDIR=Your_VS2010_Path\
  rem set MSSDK=Your_W7Sdk_Path\ 
  rem set MINGW32=Your_MinGW32_Path\
  rem set MINGW64=Your_MinGW64_Path\
  rem set GMAKE=gmake.exe
  rem ---------------------------------------------------------------
  rem or create 'build_settings.bat' file
  if exist "%~dpn0_settings.bat" call "%~dpn0_settings.bat"
  rem ---------------------------------------------------------------

  for %%a in (%*) do call :proc_param %%a
  set opath=%PATH%
  for %%c in (%ccomp%) do for %%b in (%nbits%) do call :init %%c %%b& call :build_%%c %%b
goto :EOF

:set_gcc
  if "32" == "%~1" (set gccdir=%MINGW32%) else (set gccdir=%MINGW64%)
  if "" == "%gccdir%" if "32" == "%~1" (set gccdir=C:\MinGW\32\) else (set gccdir=C:\MinGW\64\)
  PATH=%gccdir%bin;%opath%
goto :EOF

:set_vc
  if "" == "%vsdir%" set vsdir=%VS100COMNTOOLS:\Common7\Tools\=\%
  if "" == "%mssdk%" call :try_msdk Windows       CurrentInstallFolder
  if "" == "%mssdk%" call :try_msdk Windows\v7.1  InstallationFolder
  if "" == "%mssdk%" call :try_msdk Windows\v7.0A InstallationFolder
  set INCLUDE=%vsdir%VC\Include;%mssdk%Include
  if "32" == "%~1" set LIB=%vsdir%VC\Lib;%mssdk%Lib
  if "32" == "%~1" set pth=& rem
  if "64" == "%~1" set LIB=%vsdir%VC\Lib\amd64;%mssdk%Lib\x64
  if "64" == "%~1" set pth=%vsdir%VC\Bin\x86_amd64;
  PATH=%pth%%vsdir%VC\Bin;%vsdir%VC\VCPackages;%vsdir%Common7\Tools;%vsdir%Common7\IDE;%mssdk%Bin;%opath%
goto :EOF
:try_msdk
  for /F "tokens=1,2*" %%i in ('reg query "HKLM\SOFTWARE\Microsoft\Microsoft SDKs\%1" /v "%2" 2^>NUL') do (
    if "%%i" == "%2" if exist "%%kInclude\*" if exist "%%kLib\*" if exist "%%kBin\*" set mssdk=%%k
  )
goto :EOF

:proc_param
  for %%p in (vc vcc msc) do if /i "%%p" == "%~1" set ccomp=vc
  for %%p in (gcc gnu) do if /i "%%p" == "%~1" set ccomp=gcc
  for %%p in (32 x86 win32) do if /i "%%p" == "%~1" set nbits=32
  for %%p in (64 x64 win64) do if /i "%%p" == "%~1" set nbits=64
  for %%p in (32/64 32-64 32x64) do if /i "%%p" == "%~1" set nbits=32/64
  for %%p in (64/32 64-32 64x32) do if /i "%%p" == "%~1" set nbits=64/32
  for %%p in (rebuild clean) do if /i "%%p" == "%~1" set clean=Y
  for %%p in (debug dbg) do if /i "%%p" == "%~1" set deb_b=Y
  for %%p in (sln solution) do if /i "%%p" == "%~1" set vcsln=Y
  for %%p in (dwarf dw2) do if /i "%%p" == "%~1" set dwarf=Y
goto :EOF

:init
  set dirbit=%2
  set dirbit=%dirbit:~0,2%
  echo.
  if /i "vc" == "%1" (set x=sln=%vcsln%) else (set x=dwarf=%dwarf%)
  echo build far-%2 %1 [clean=%clean% debug=%deb_b% %x%]
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
  %m%
goto :EOF

:build_vc
  if /i "Y" == "%vcsln%" goto :devenv
  set m=nmake -f makefile_vc
  if /i "64" == "%dirbit%" (set m=%m% CPU=AMD64) else (set m=%m% CPU=X86)
goto :do_make
:devenv
  if /i "Y"  == "%clean%"  (set b=Rebuild) else (set b=Build)
  if /i "64" == "%dirbit%" (set p=x64)    else  (set p=Win32)
  if /i "Y"  == "%deb_b%"  (set c=Debug) else (set c=Release)
  devenv far.sln /%b% "%c%^|%p%"
goto :EOF
