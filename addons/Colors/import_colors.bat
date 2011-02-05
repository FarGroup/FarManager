@echo off
if .%1==. goto usage
echo FAR color settings are about to be changed. Please, close
echo all other FAR instances, because they may interfere with
echo the process, then press any key to proceed or Ctrl-Break
echo (Ctrl-C) to cancel.
pause>nul
echo.
echo Backing up current color settings...
set FILENAME=before_%1
if not exist %FILENAME% goto export
echo The backup file already exists: %FILENAME%
echo Can't proceed with backup. Please delete or rename/move
echo existing file, then run this batch job again.
goto end
:export
call export_colors.bat %FILENAME%
echo.
echo Deleting old color settings from the registry...
echo REGEDIT4 > $DelOld$.reg
echo [-HKEY_CURRENT_USER\Software\Far2\Colors] >> $DelOld$.reg
start /wait regedit -s $DelOld$.reg
del $DelOld$.reg
start /wait regedit -s %1
echo.
echo New color settings from file %1 were imported to the
echo registry. Now ensure that FAR 'Auto save setup' System Option
echo is disabled and restart FAR.
goto end
:usage
echo ******************************************************************
echo This batch job imports custom FAR color settings into registry.
echo Close all other FAR instances before starting this batch
echo (it's recommended to start it in another window using Shift-Enter,
echo so you will be able to close current FAR session, too).
echo.
echo Usage: import_colors.bat filename.reg
echo.
echo where 'filename.reg' contains new color settings
echo.
echo Current color settings will be backed up into file
echo 'before_filename.reg' in case you will want to restore them later.
echo ******************************************************************
:end
