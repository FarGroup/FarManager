@echo off
if not .%1==. goto next
if not .%2==. goto next
echo Накладываем патч number_of_patch на файл filename.ext
echo Синтаксис:
echo     p.bat filename.ext number_of_patch
echo Пример:
echo     p.bat checkver.cpp 1
goto done

:next
if not exist patchs goto next_path
grep "%1.%2.diff" patchs > nul
if not errorlevel 1 goto error

:next_path
rem сохраним копию
rem copy %1 save\%1

rem пропатчим
rem коррекция от AT
rem patch.exe -c %1 diff\%1.%2.diff
settitle "Path [%2]"
patch.exe -c --fuzz=10 --forward --ignore-whitespace %1 diff\%1.%2.diff

rem сохраняем имя патча
echo %1.%2.diff >> patchs

:next2
goto done

:error
echo Этот патч уже накладывался!
goto done

:done
exit
