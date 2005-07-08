@echo off
if not exist OBJ mkdir OBJ
if not exist FINAL mkdir FINAL
if exist FINAL\Brackets.dll del FINAL\Brackets.dll > nul
set farbcc=d:\usr\bcc502
set farlib=%farbcc%\lib
set farinc=%farbcc%\include
%farbcc%\bin\Brc32 -R -foobj\Brackets.res Brackets.rc  |tee OBJ\errors
%farbcc%\bin\bcc32 -O -Oe -Ol -Ob -Om -Op -Ov -d -I%FARINC% -c -a2 -x- -R- -RT- -M- -v-  -oobj\Brackets Brackets.cpp  |tee -a OBJ\errors
%farbcc%\bin\tlink32 -Tpd -aa -v- -P -L%FARLIB% OBJ\Brackets, FINAL\BRACKETS.DLL, , import32 cw32,,obj\Brackets.res |tee -a OBJ\errors

if exist final\BrackEng.hlf del final\BrackEng.hlf > nul
if exist final\BrackEng.lng del final\BrackEng.lng > nul
if exist final\BrackRus.hlf del final\BrackRus.hlf > nul
if exist final\BrackRus.lng del final\BrackRus.lng > nul
if exist final\BrackDel.reg del final\BrackDel.reg > nul
if exist final\BrackEng.reg del final\BrackEng.reg > nul
if exist final\BrackRus.reg del final\BrackRus.reg > nul

copy BrackEng.hlf final\BrackEng.hlf > nul
copy BrackEng.lng final\BrackEng.lng > nul
copy BrackRus.hlf final\BrackRus.hlf > nul
copy BrackRus.lng final\BrackRus.lng > nul
copy BrackDel.reg final\BrackDel.reg > nul
copy BrackEng.reg final\BrackEng.reg > nul
copy BrackRus.reg final\BrackRus.reg > nul
