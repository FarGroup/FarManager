@echo off
rem    This batch file restores Far settings from previously saved
rem    files FarSave1.reg and FarSave2.reg to the registry

if not exist FarSave?.reg goto import

echo REGEDIT4 > "%TEMP%\$DelOld$.reg"
echo [-HKEY_CURRENT_USER\Software\Far] >> "%TEMP%\$DelOld$.reg"
echo [-HKEY_LOCAL_MACHINE\Software\Far] >> "%TEMP%\$DelOld$.reg"

start/wait regedit -s "%TEMP%\$DelOld$.reg"
del "%TEMP%\$DelOld$.reg" > nul

:import
echo REGEDIT4 > "%TEMP%\$DelCache$.reg"
echo [-HKEY_CURRENT_USER\Software\Far\PluginsCache] >> "%TEMP%\$DelCache$.reg"
start/wait regedit -s FarSave1.reg FarSave2.reg "%TEMP%\$DelCache$.reg"
del "%TEMP%\$DelCache$.reg" > nul
