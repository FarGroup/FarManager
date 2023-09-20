@echo off
cd "%~dp0" || exit 1
if exist ..\build\lua rd ..\build\lua || exit 1
mkdir ..\build
mkdir ..\build\lua
cd ..\build\lua || exit 1

if not '%1'=='' goto :make

call :make luafar_manual      1252
call :make macroapi_manual.ru 1251
call :make macroapi_manual.en 1252

goto :end

:make
mkdir %1
cd %1 || exit 1
python %~dp0convert.py "..\..\..\enc_lua\%1.tsi" utf-8-sig "%1.tsi" windows-%2
"%~dp0lua\lua.exe" "%~dp0lua\scripts\tp2hh.lua" "%1.tsi" tsi "%~dp0lua\templates\api.tem"
"%~dp0hh_compiler\hh_compiler.exe" %2 %1.hhp
if not exist %1.chm (
	echo "Error: %1.chm wasn't created!"
	exit 1
)
cd ..
goto :EOF

:end
exit 0
