@echo off
rmdir /s /q ..\build\lua
mkdir ..\build
mkdir ..\build\lua
cd ..\build\lua

if not '%1'=='' goto :make

call :make luafar_manual
call :make macroapi_manual

goto :EOF

:make
mkdir %1
cd %1
"%~dp0lua\lua.exe" "%~dp0lua\scripts\tp2hh.lua" "..\..\..\enc_lua\%1.tsi" tsi "%~dp0lua\templates\api.tem" 
cd ..
goto :EOF
