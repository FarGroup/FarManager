@echo off
if not exist OBJ mkdir OBJ
if not exist FINAL mkdir FINAL
set farbcc=d:\usr\bcc502
set farlib=%farbcc%\lib
set farinc=%farbcc%\include

%farbcc%\bin\Brc32 -R -foobj\AutoWrap.res AutoWrap.rc | tee OBJ\errors
%farbcc%\bin\bcc32 -O -Oe -Ol -Ob -Om -Op -Ov -d -I%FARINC% -c -a2 -x- -R- -RT- -M- -v-  -oobj\AutoWrap AutoWrap.cpp | tee -a OBJ\errors
%farbcc%\bin\tlink32 -Tpd -aa -v- -P -L%FARLIB% OBJ\AutoWrap, FINAL\AUTOWRAP.DLL, , import32 cw32 ,,obj\AutoWrap.res | tee -a OBJ\errors

if exist final\WrapEng.lng del final\WrapEng.lng > nul
if exist final\WrapRus.lng del final\WrapRus.lng > nul

copy WrapEng.lng final\WrapEng.lng > nul
copy WrapRus.lng final\WrapRus.lng > nul
