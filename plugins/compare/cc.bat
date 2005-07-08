@echo off
if not exist OBJ mkdir OBJ
if not exist FINAL mkdir FINAL
set farbcc=d:\usr\bcc502
set farlib=%farbcc%\lib
set farinc=%farbcc%\include

if exist compare.dll del compare.dll > nul
if exist compare.obj del compare.obj > nul
if exist compare.res del compare.res > nul
Brc32 -R -foobj\Compare.res Compare.rc
bcc32 -I%FARINC% -c -4 -M- -a2 -d -f- -O -Oc -OS -Ov -pr -RT- -tWD -v- -x- -wstu -wpin -wbbf -weas -wnfc -wdef -wnod -wamb -w-pia -wuse -w-par -wamp -wobs -oobj\Compare Compare.cpp
tlink32 -L%FARLIB%  -Tpd -ap -c -v- -x obj\Compare.obj, FINAL\COMPARE.DLL,, import32 cw32,, obj\Compare.res

if exist final\CmpEng.hlf   del final\CmpEng.hlf   > nul
if exist final\CmpRus.hlf   del final\CmpRus.hlf   > nul
if exist final\CompEng.lng  del final\CompEng.lng  > nul
if exist final\CompRus.lng  del final\CompRus.lng  > nul

copy CmpEng.hlf   final\CmpEng.hlf   > nul
copy CmpRus.hlf   final\CmpRus.hlf   > nul
copy CompEng.lng  final\CompEng.lng  > nul
copy CompRus.lng  final\CompRus.lng  > nul
