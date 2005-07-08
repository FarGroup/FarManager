@echo off
if not exist OBJ mkdir OBJ
if not exist FINAL mkdir FINAL
rem :Предполагается, что все переменные окружения уже установлены.
rem :Если нет - тут надо вызвать что-то типа
if exist ..\vc-vars.bat call ..\vc-vars.bat

rc /foOBJ\Align.res Align.rc |tee  OBJ\errors
cl /FAsc /FaOBJ\Align.asm /Zp2 /Fm /O1igy /GF /Gr /W4 /GR- /GX- /LD /Gs10000 Align.cpp OBJ\Align.res /link /opt:nowin98 /noentry /def:Align.def  kernel32.lib user32.lib advapi32.lib /merge:.rdata=.text /stub:minstub.exe /FeALIGN.DLL |tee -a OBJ\errors
