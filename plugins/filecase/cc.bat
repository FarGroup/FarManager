@echo off
if not exist OBJ mkdir OBJ
if not exist FINAL mkdir FINAL
if exist FINAL\FileCase.dll del FINAL\FileCase.dll > nul
set farbcc=d:\usr\bcc502
set farlib=%farbcc%\lib
set farinc=%farbcc%\include
REM set farbcc=c:\usr\bcc55
%farbcc%\bin\Brc32 -R -foobj\FileCase.res FileCase.rc | tee OBJ\errors
%farbcc%\bin\bcc32 -K -O -Oe -Ol -Ob -Om -Op -Ov -I%FARINC% -c -a2 -x- -R- -RT- -M- -v-  -oobj\FileCase FileCase.cpp  | tee -a OBJ\errors
%farbcc%\bin\tlink32 -Tpd -aa -v- -P -L%FARLIB% OBJ\FileCase, FINAL\FILECASE.DLL, , import32 cw32,,obj\FileCase.res | tee -a OBJ\errors

if exist final\CaseEng.hlf del final\CaseEng.hlf > nul
if exist final\CaseEng.lng del final\CaseEng.lng > nul
if exist final\CaseRus.hlf del final\CaseRus.hlf > nul
if exist final\CaseRus.lng del final\CaseRus.lng > nul

copy CaseEng.hlf final\CaseEng.hlf > nul
copy CaseEng.lng final\CaseEng.lng > nul
copy CaseRus.hlf final\CaseRus.hlf > nul
copy CaseRus.lng final\CaseRus.lng > nul
