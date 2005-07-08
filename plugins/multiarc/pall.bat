@echo off
set FV1=1
set FV2=70
set FV3=6
set FVB=beta

if not .%1==. goto nextD
echo Накладываем все патчи за номером number_of_patch
echo Синтаксис:
echo     pall.bat number_of_patch#1 [ number_of_patch#N]
echo Если несколько номеров, то через пробел
echo Пример:
echo     pall.bat 1
echo     pall.bat 1 2 3 4 5 6
echo     pall.bat 1 - 60 63
goto done

:begin
if not .%1==. goto nextD
echo Done.
goto done2

:nextD
if "%2"=="-" goto range

:next
echo *[%1]***************************************************************
settitle "Path [%1]"
dir /b diff\*.*.%1.diff | awk "BEGIN{FS=\".\"}{system(sprintf(\"p.bat %%s.%%s %1 1\",$1,$2))}"
awk -f ver_ma.awk -v FV1=%FV1% -v FV2=%FV2% -v FV3=%FV3%  -v FV4=%1 -v BETA=%FVB% > multiarc.rc
rem Скинем файлы в нужные каталоги для отправки.
dir /b diff\*.*.%1.diff | awk "{system(sprintf('copy diff\\%%s local\\diff\\%%s',$0,$0))}"
dir /b diff.doc\*%1.*   | awk "{system(sprintf('copy diff.doc\\%%s local\\diff.doc\\%%s',$0,$0))}"
echo %1 > vbuild

rem берем следующий
shift
goto begin

:range
echo *[%1-%3]************************************************************
settitle "Path [%1-%3]"
FOR /L %%i IN (%1,1,%3) DO dir /b diff\*.*.%%i.diff | awk "BEGIN{FS=\".\"}{system(sprintf(\"p.bat %%s.%%s %%i 1\",$1,$2))}"
awk -f ver_ma.awk -v FV1=%FV1% -v FV2=%FV2% -v FV3=%FV3%  -v FV4=%3 -v BETA=%FVB% > multiarc.rc
echo %3 > vbuild

rem берем следующий
shift
shift
shift

goto begin

:done2
rem cd save
rem jar32 ac save.j *.* -xsave.j -d -y > nul
rem cd ..

:done
