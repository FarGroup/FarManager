@echo off
if .%1==. goto noname
set FILENAME=%1
goto export
:noname
set FILENAME=FarColors.reg
:export
regedit /ea %FILENAME% HKEY_CURRENT_USER\Software\Far\Colors
echo FAR color settings saved to file %FILENAME%
