@echo off
setlocal
:: This batch file restores Far settings from previously saved
:: files FarSettings.*.reg to the registry

if defined PROCESSOR_ARCHITEW6432 (set reg="%systemroot%\sysnative\reg.exe") else (set reg=reg)

set userfile=FarSettings.User.reg
set machinefile=FarSettings.Machine.reg
set machinewow64file=FarSettings.Machine.WOW64.reg

set savepath=%~dp0

call :restore "hkcu\software\far2" "%savepath%%userfile%"
call :restore "hklm\software\far2" "%savepath%%machinefile%"
call :restore "hklm\software\wow6432node\far2" "%savepath%%machinewow64file%"

goto :eof

:restore
if exist %2 (
%reg% query %1 >nul 2>^&1
if not errorlevel 1 (
echo.
echo Deleting %1...
%reg% delete %1 /f
)
echo.
echo Importing %2...
%reg% import %2 >nul
)