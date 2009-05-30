@echo off
:: This batch file saves Far settings from the registry
:: to files FarSave1.reg and FarSave2.reg

del /Q /F "%~dp0FarSave1.reg" > nul 2>^&1
del /Q /F "%~dp0FarSave2.reg" > nul 2>^&1
reg export hkcu\software\far2 "%~dp0FarSave1.reg" > nul 2>^&1
reg export hklm\software\far2 "%~dp0FarSave2.reg" > nul 2>^&1
