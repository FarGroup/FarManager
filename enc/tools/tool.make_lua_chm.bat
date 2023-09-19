@echo off
cd "%~dp0" || exit 1
if exist ..\build\lua rd ..\build\lua || exit 1
mkdir ..\build
mkdir ..\build\lua
cd ..\build\lua || exit 1

if not '%1'=='' goto :make

call :make luafar_manual      1252
call :make macroapi_manual.en 1252
call :make macroapi_manual.ru 1251

goto :end

:make
mkdir %1
cd %1 || exit 1
"%~dp0lua\lua.exe" "%~dp0lua\scripts\tp2hh.lua" "..\..\..\enc_lua\%1.tsi" tsi "%~dp0lua\templates\api.tem"
python %~dp0convert.py %1.hhc utf-8-sig %1.hhc windows-%2
python %~dp0convert.py %1.hhp utf-8-sig %1.hhp windows-%2
"%~dp0hh_compiler\hh_compiler.exe" %2 %1.hhp
if not exist %1.chm (
	echo "Error: %1.chm wasn't created!"
	exit 1
)
cd ..
goto :EOF

:end
exit 0
