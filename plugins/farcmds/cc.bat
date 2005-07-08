@echo off
if not exist OBJ mkdir OBJ
if not exist FINAL mkdir FINAL

set COMPILER=BC
if not .%1==. set COMPILER=VC

echo %COMPILER%

rem set MSVCRT=msvcrt.lib

if exist FINAL\FARCmds.dll del FINAL\FARCmds.dll > nul
set farbcc=D:\usr\bcc502
rem set farbcc=c:\Borland\bcc55
set farlib=%farbcc%\lib
set farinc=%farbcc%\include

set VSCommonDir=C:\PROGRA~1\MICROS~1\Common
set MSDevDir=C:\PROGRA~1\MICROS~1\Common\msdev98
set MSVCDir=C:\PROGRA~1\MICROS~1\VC98
set VcOsDir=WIN95
if "%OS%" == "Windows_NT" set VcOsDir=WINNT
if "%OS%" == "Windows_NT" set PATH=%MSDevDir%\BIN;%MSVCDir%\BIN;%VSCommonDir%\TOOLS\%VcOsDir%;%VSCommonDir%\TOOLS;%PATH%
if "%OS%" == "" set PATH="%MSDevDir%\BIN";"%MSVCDir%\BIN";"%VSCommonDir%\TOOLS\%VcOsDir%";"%VSCommonDir%\TOOLS";"%windir%\SYSTEM";"%PATH%"
set INCLUDE=%MSVCDir%\ATL\INCLUDE;%MSVCDir%\INCLUDE;%MSVCDir%\MFC\INCLUDE;%INCLUDE%
set LIB=%MSVCDir%\LIB;%MSVCDir%\MFC\LIB;%LIB%

goto %COMPILER%

REM set farbcc=c:\usr\bcc55
:BC
%farbcc%\bin\Brc32 -R -foobj\FARCmds.res FARCmds.rc  |tee OBJ\errors
%farbcc%\bin\bcc32 -O -Oe -Ol -Ob -Om -Op -Ov -I%FARINC% -c -a2 -x- -R- -RT- -M- -v-  -oobj\FARCmds FARCmds.cpp  |tee -a OBJ\errors
%farbcc%\bin\tlink32 -Tpd -aa -v- -P -L%FARLIB% OBJ\FARCmds, FINAL\FARCMDS.DLL, , import32 cw32 ,,obj\FARCmds.res |tee -a OBJ\errors
rem %farbcc%\bin\ilink32 -Tpd -aa -v- -P -L%FARLIB% OBJ\FARCmds, FINAL\FARCmds.dll, , import32 cw32 Far.lib,,obj\FARCmds.res >>OBJ\errors
goto next

:VC
rem cl -Gs -Ox -Zp1 /GR- /GX -c /opt:nowin98 FARCmds.cpp | tee  error
rc.exe /l 0x4E4 /fo"OBJ\FARCmds.res" /d "NDEBUG" .\FARCmds.rc
cl /FAsc /FARCmds.asm /Zp2 /Fm /O1igy /GF /Gr /W4 /GR- /GX- /LD /Gs10000 FARCmds.cpp "obj\FARCmds.res" /link /opt:nowin98 /noentry /nodefaultlib /def:FARCmds.def %MSVCRT% kernel32.lib user32.lib advapi32.lib shell32.lib /merge:.rdata=.text
rem link FARCMDS /STUB:minstub.exe /opt:nowin98 /DEF:FARCmds.def obj\FARCmds.res /DLL /NODEFAULTLIB:LIBC.LIB MSVCRT.LIB kernel32.LIB USER32.LIB ADVAPI32.LIB | tee -a error
if exist FARCmds.dll move FARCmds.dll FINAL\FARCmds.dll > nul
if exist FARCmds.map move FARCmds.map OBJ\FARCmds.map  > nul
del /q *.cod > nul
del /q *.exp > nul
del /q *.lib > nul
del /q *.obj > nul
goto next

:next
if exist final\FARCmdsEng.hlf del final\FARCmdsEng.hlf > nul
if exist final\FARCmdsEng.lng del final\FARCmdsEng.lng > nul
if exist final\FARCmdsRus.hlf del final\FARCmdsRus.hlf > nul
if exist final\FARCmdsRus.lng del final\FARCmdsRus.lng > nul

copy FARCmdsEng.hlf final\FARCmdsEng.hlf > nul
copy FARCmdsEng.lng final\FARCmdsEng.lng > nul
copy FARCmdsRus.hlf final\FARCmdsRus.hlf > nul
copy FARCmdsRus.lng final\FARCmdsRus.lng > nul
