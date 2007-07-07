@echo off
call VCVARS32.BAT
set INCLUDE=%INCLUDE%;..\common\ascii;..\common
if exist HelloWorld.dll del HelloWorld.dll>nul
cl /Zp2 /O1igy /GF /Gr /GR- /GX- /LD HelloWorld.c /link /subsystem:console /machine:I386 /opt:nowin98 /noentry /nodefaultlib /def:HelloWorld.vc.def kernel32.lib advapi32.lib user32.lib /map:HelloWorld.map /merge:.rdata=.text
if exist HelloWorld.exp del HelloWorld.exp>nul
if exist HelloWorld.obj del HelloWorld.obj>nul
if exist HelloWorld.lib del HelloWorld.lib>nul
