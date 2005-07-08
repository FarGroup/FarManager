::@echo off
if not exist OBJ mkdir OBJ
if not exist FINAL mkdir FINAL
echo Making EditCase Far Plugin with the aid of Borland C++ 5.5

d:\usr\bcc55\bin\bcc32 -WD -IC:\usr\bcc55\Include -c -a2 -RT- -K -M- -v- -u- -G EditCase.cpp
d:\usr\bcc55\bin\Brc32 -R EditCase.rc

if not errorlevel 1 d:\usr\bcc55\bin\ilink32.exe /Gn /Tpd /Ld:\usr\bcc55\Lib /x EditCase.obj,,, import32.lib, EditCase.def,EditCase.res
::%farbcc%\bin\tlink32 -Tpd -aa -v- -P -L%FARLIB% OBJ\DrawLine, FINAL\DRAWLINE.DLL, , import32 cw32 ,,obj\DrawLine.res >>OBJ\errors

if not errorlevel 1 echo Done!

if exist final\ECaseEng.hlf del final\ECaseEng.hlf
if exist final\ECaseEng.lng del final\ECaseEng.lng
if exist final\ECaseRus.hlf del final\ECaseRus.hlf
if exist final\ECaseRus.lng del final\ECaseRus.lng
if exist final\EditCase.dll del final\EditCase.dll

copy ECaseEng.hlf final\ECaseEng.hlf
copy ECaseEng.lng final\ECaseEng.lng
copy ECaseRus.hlf final\ECaseRus.hlf
copy ECaseRus.lng final\ECaseRus.lng
copy EditCase.dll final\EDITCASE.DLL
del EditCase.dll
