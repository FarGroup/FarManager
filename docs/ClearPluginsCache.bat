@echo off
rem This batch file cleared the Plugin Cache

echo REGEDIT4 > "%TEMP%\$DelOld$.reg"
echo [-HKEY_CURRENT_USER\Software\Far\PluginsCache] >> "%TEMP%\$DelOld$.reg"
start/wait regedit -s "%TEMP%\$DelOld$.reg"
del "%TEMP%\$DelOld$.reg" > nul
