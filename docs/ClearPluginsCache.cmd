@echo off
:: This batch file cleares the plugins cache

reg query hkcu\software\far2\pluginscache /ve > nul 2>^&1
if %errorlevel% equ 0 reg delete hkcu\software\far2\pluginscache /f
