@echo off
if not exist OBJ mkdir OBJ
if not exist FINAL mkdir FINAL

set COMPILER=BC
if not .%1==. set COMPILER=VC

set farbcc=d:\usr\bcc502
set farlib=%farbcc%\lib
set farinc=%farbcc%\include
REM set farbcc=c:\usr\bcc55

if not ."%MSDevDir%"=="". goto dontsets
set VSCommonDir=C:\PROGRA~1\MICROS~1\Common
set MSDevDir=C:\PROGRA~1\MICROS~1\Common\msdev98
set MSVCDir=C:\PROGRA~1\MICROS~1\VC98
set VcOsDir=WIN95
if "%OS%" == "Windows_NT" set VcOsDir=WINNT
if "%OS%" == "Windows_NT" set PATH=%MSDevDir%\BIN;%MSVCDir%\BIN;%VSCommonDir%\TOOLS\%VcOsDir%;%VSCommonDir%\TOOLS;%PATH%
if "%OS%" == "" set PATH="%MSDevDir%\BIN";"%MSVCDir%\BIN";"%VSCommonDir%\TOOLS\%VcOsDir%";"%VSCommonDir%\TOOLS";"%windir%\SYSTEM";"%PATH%"
set INCLUDE=%MSVCDir%\ATL\INCLUDE;%MSVCDir%\INCLUDE;%MSVCDir%\MFC\INCLUDE;%INCLUDE%
set LIB=%MSVCDir%\LIB;%MSVCDir%\MFC\LIB;%LIB%
:dontsets

if exist FINAL\DrawLine.dll del /q FINAL\DrawLine.dll > nul
goto %COMPILER%


:BC
%farbcc%\bin\Brc32 -R -foobj\DrawLine.res DrawLine.rc  >OBJ\errors
%farbcc%\bin\bcc32 -O -Oe -Ol -Ob -Om -Op -Ov -d -I%FARINC% -c -a2 -x- -R- -RT- -M- -v-  -oobj\DrawLine DrawLine.cpp  >>OBJ\errors
%farbcc%\bin\tlink32 -Tpd -aa -v- -P -L%FARLIB% OBJ\DrawLine, FINAL\DRAWLINE.DLL, , import32 cw32 ,,obj\DrawLine.res >>OBJ\errors
type OBJ\errors

goto next

:VC
rem cl -Gs -Ox -Zp1 /GR- /GX -c /opt:nowin98 FARCmds.cpp | tee  error
rc.exe /l 0x4E4 /fo"OBJ\DrawLine.res" /d "NDEBUG" .\DrawLine.rc
rem cl -Gs -Ox -Zp1 /GR- /GX -c /opt:nowin98 /Fa"Cod\\" MULTIARC.cpp | tee  error
cl /FAsc /DrawLine.asm /Zp2 /Fm /O1igy /GF /Gr /W4 /GR- /GX- /LD /Gs10000 DrawLine.cpp "obj\DrawLine.res" /link /opt:nowin98 /noentry /nodefaultlib /def:DrawLine.def msvcrt.lib kernel32.lib user32.lib advapi32.lib shell32.lib /merge:.rdata=.text /stub:minstub.exe
rem link FARCMDS /STUB:minstub.exe /opt:nowin98 /DEF:DrawLine.def obj\FARCmds.res /DLL /NODEFAULTLIB:LIBC.LIB MSVCRT.LIB kernel32.LIB USER32.LIB ADVAPI32.LIB | tee -a error
if exist DrawLine.dll move DrawLine.dll FINAL\DrawLine.dll > nul
if exist DrawLine.map move DrawLine.map OBJ\DrawLine.map  > nul
del /q *.cod > nul
del /q *.exp > nul
del /q *.lib > nul
del /q *.obj > nul
goto next

:next
if exist final\DrawEng.hlf del final\DrawEng.hlf > nul
if exist final\DrawEng.lng del final\DrawEng.lng > nul
if exist final\DrawRus.hlf del final\DrawRus.hlf > nul
if exist final\DrawRus.lng del final\DrawRus.lng > nul

copy DrawEng.hlf final\DrawEng.hlf > nul
copy DrawEng.lng final\DrawEng.lng > nul
copy DrawRus.hlf final\DrawRus.hlf > nul
copy DrawRus.lng final\DrawRus.lng > nul

type OBJ\errors
