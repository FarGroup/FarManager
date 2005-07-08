@echo off
if not exist OBJ mkdir OBJ
if not exist FINAL mkdir FINAL
if exist FINAL\Align.dll del FINAL\Align.dll > nul
set farbcc=d:\usr\bcc502
set farlib=%farbcc%\lib
set farinc=%farbcc%\include
%farbcc%\bin\Brc32 -R -foobj\Align.res Align.rc |tee OBJ\errors
%farbcc%\bin\bcc32 -O -WD -Oe -Ol -Ob -Om -Op -Ov -d -I%FARINC% -c -a2 -x- -R- -RT- -M- -v-  -oobj\Align Align.cpp |tee -a OBJ\errors
%farbcc%\bin\tlink32 -Tpd -aa -v- -P -L%FARLIB% OBJ\Align, FINAL\ALIGN.DLL, , import32 cw32,,obj\Align.res |tee -a OBJ\errors

if exist final\AlignEng.hlf del final\AlignEng.hlf > nul
if exist final\AlignEng.lng del final\AlignEng.lng > nul
if exist final\AlignRus.hlf del final\AlignRus.hlf > nul
if exist final\AlignRus.lng del final\AlignRus.lng > nul

copy AlignEng.hlf final\AlignEng.hlf > nul
copy AlignEng.lng final\AlignEng.lng > nul
copy AlignRus.hlf final\AlignRus.hlf > nul
copy AlignRus.lng final\AlignRus.lng > nul
