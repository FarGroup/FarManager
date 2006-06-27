@echo off
set farbcc=D:\usr\bcc502
set farlib=%farbcc%\lib
set farinc=%farbcc%\include
set MSVCRT=/NODEFAULTLIB:LIBC.LIB MSVCRT.LIB

rem %farbcc%\bin\brcc32 -R multiarc.rc
rem %farbcc%\bin\bcc32 -4 -I%FARINC% -c -a2 -R- -x- -RT- -K -M- -v- -G -oobj\MULTIARC MULTIARC.cpp | tee error
rem %farbcc%\bin\tlink32 -OS -m -Tpd -aa -v- -P -L%FARLIB% OBJ\MULTIARC, MULTIARC.DLL,  , import32 cw32,,multiarc.res | tee -a error
set VC=D:\usr\vc
::set VC=D:\usr\vc.2003
set PATH=%VC%\bin;%PATH%
set INCLUDE=%VC%\INCLUDE
set LIB=%VC%\LIB

d:\usr\bcc55\bin\make.bc.exe -s -fmultiarc_bc.mak
::nmake /c MultiArc_vc.mak
if errorlevel 1 goto done

:done
