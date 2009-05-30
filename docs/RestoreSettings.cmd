@echo off
:: This batch file restores Far settings from previously saved
:: files FarSave1.reg and FarSave2.reg to the registry

if not exist "%~dp0FarSave1.reg" goto hklm
reg delete hkcu\software\far2 /f > nul 2>^&1
reg import "%~dp0FarSave1.reg"
reg delete hkcu\software\far2\pluginscache /f > nul 2>^&1

:hklm
if not exist "%~dp0FarSave2.reg" goto :eof
reg delete hklm\software\far2 /f > nul 2>^&1
reg import "%~dp0FarSave2.reg"
