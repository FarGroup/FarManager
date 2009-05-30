@echo off
:: This batch file restores Far settings from previously saved
:: files FarSave1.reg and FarSave2.reg to the registry
 
if not exist %~dp0FarSave1.reg goto hklm
reg query hkcu\software\far2 /ve > nul 2>^&1
if %errorlevel% equ 0 reg delete hkcu\software\far2 /f
reg import %~dp0FarSave1.reg
reg query hkcu\software\far2\pluginscache /ve > nul 2>^&1
if %errorlevel% equ 0 reg delete hkcu\software\far2\pluginscache /f

:hklm
if not exist %~dp0FarSave2.reg goto :eof
reg query delete hklm\software\far2 /ve > nul 2>^&1
if %errorlevel% equ 0 reg delete hklm\software\far2 /f
reg import %~dp0FarSave2.reg
