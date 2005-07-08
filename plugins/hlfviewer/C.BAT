@echo off
if not exist OBJ mkdir OBJ
if not exist FINAL mkdir FINAL

if exist HlfViewer.dll del HlfViewer.dll
if exist HlfViewer.obj del HlfViewer.obj
Brc32 -R HlfViewer.rc
bcc32 -c -a1 -R- -RT- -M- -v-  HlfViewer.cpp>errors
tlink32 -Tpd -aa HlfViewer.obj, HlfViewer.dll, , import32 cw32 ,,HlfViewer.res>>errors
type errors
