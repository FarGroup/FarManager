@echo off

rem set misc far compilation options
set FAR_MSVCRT=
set USE_WFUNC=/D "USE_WFUNC" /D "USE_WFUNC_ALW" /D "USE_WFUNC_IN" /D "FAR_ALPHA_VERSION"
set FARTRY=/EHs-c-
set FARSYSLOG=-DSYSLOG_FARSYSLOG -DSYSLOG
set CFG_0=Debug

rem call a user defined batch file for custom setting
rem you can set there all the paths for VC and etc.
if exist custom.vc.debug.64.bat call custom.vc.debug.64.bat

rem generate/update lng files and lang.hpp
tools\lng.generator.exe -nc -i lang.ini farlang.templ

rem rebuild dependencies
tools\gawk -f .\scripts\mkdep.awk -v out=Debug.64.vc mkdep.list > far.debug.dep

nmake /f "FAR.mak" CFG="far - Win64 %CFG_0%" | tools\tee !Error.vc.64.%CFG_0%
