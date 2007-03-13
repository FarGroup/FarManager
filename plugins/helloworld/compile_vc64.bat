@echo off
call VCVARS64.BAT
set INCLUDE=%INCLUDE%;..\common\ascii;..\common
if exist HelloWorld.dll del HelloWorld.dll>nul
cl /Wp64 /Zp8 /O1iy /GF /Gr /GR- /EHs-c- /LD /MT /Gs10000 HelloWorld.c /link /subsystem:console /machine:AMD64 /opt:nowin98 /noentry /nodefaultlib /def:HelloWorld.vc.def kernel32.lib advapi32.lib user32.lib LIBCMT.LIB /map:HelloWorld.map /merge:.rdata=.text
if exist HelloWorld.exp del HelloWorld.exp>nul
if exist HelloWorld.obj del HelloWorld.obj>nul
if exist HelloWorld.lib del HelloWorld.lib>nul
