@echo off
if not exist OBJ mkdir OBJ
if not exist FINAL mkdir FINAL
if exist FINAL\HlfViewer.dll del FINAL\HlfViewer.dll > nul
set farbcc=d:\usr\bcc502
rem set farbcc=c:\Borland\bcc55
set farlib=%farbcc%\lib
set farinc=%farbcc%\include
%farbcc%\bin\Brc32 -R -foobj\HlfViewer.res HlfViewer.rc  |tee OBJ\errors
%farbcc%\bin\bcc32 -O -Oe -Ol -Ob -Om -Op -Ov -I%FARINC% -c -a1 -x- -R- -RT- -M- -v-  -oobj\HlfViewer HlfViewer.cpp | tee -a OBJ\errors
%farbcc%\bin\tlink32 -Tpd -aa -v- -P -L%FARLIB% OBJ\HlfViewer, FINAL\HLFVIEWER.DLL, , import32 cw32,,obj\HlfViewer.res | tee -a OBJ\errors
rem %farbcc%\bin\ilink32 -Tpd -aa -v- -P -L%FARLIB% OBJ\HlfViewer, FINAL\HlfViewer.dll, , import32 cw32,,obj\HlfViewer.res >>OBJ\errors

if exist final\HlfViewerEng.hlf del final\HlfViewerEng.hlf > nul
if exist final\HlfViewerEng.lng del final\HlfViewerEng.lng > nul
if exist final\HlfViewerRus.hlf del final\HlfViewerRus.hlf > nul
if exist final\HlfViewerRus.lng del final\HlfViewerRus.lng > nul

copy HlfViewerEng.hlf final\HlfViewerEng.hlf > nul
copy HlfViewerEng.lng final\HlfViewerEng.lng > nul
copy HlfViewerRus.hlf final\HlfViewerRus.hlf > nul
copy HlfViewerRus.lng final\HlfViewerRus.lng > nul
