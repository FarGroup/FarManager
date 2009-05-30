@echo off
:: This batch file cleares the plugins cache

reg delete hkcu\software\far2\pluginscache /f > nul 2>^&1
