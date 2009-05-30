@echo off
:: This batch file saves Far settings from the registry
:: to files FarSave1.reg and FarSave2.reg

reg query hkcu\software\far2 /ve > nul 2>^&1 
IF %errorlevel% equ 0 reg export hkcu\software\far2 %~dp0FarSave1.reg /y
reg query hklm\software\far2 /ve > nul 2>^&1
IF %errorlevel% equ 0 reg export hklm\software\far2 %~dp0FarSave2.reg /y