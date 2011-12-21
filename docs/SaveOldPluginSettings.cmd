@echo off
setlocal
:: This batch file saves Far Old Plugin settings from the registry
:: to files OldPluginSettings.*.reg

if defined PROCESSOR_ARCHITEW6432 (set reg="%systemroot%\sysnative\reg.exe") else (set reg=reg)

set userfile=OldPluginSettings.User.reg
set machinefile=OldPluginSettings.Machine.reg
set machinewow64file=OldPluginSettings.Machine.WoW64.reg

set savepath=%~dp0

call :save "hkcu\software\far manager" "%savepath%%userfile%"
call :save "hklm\software\far manager" "%savepath%%machinefile%"
call :save "hklm\software\wow6432node\far manager" "%savepath%%machinewow64file%"

goto :eof

:save
%reg% query %1 >nul 2>^&1
if not errorlevel 1 (
if exist %2 (
echo.
echo Deleting %2...
del /f %2
)
echo.
echo Exporting %1...
%reg% export %1 %2
)
