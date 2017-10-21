@echo off
setlocal

for /f "tokens=1,2,4 delims=," %%i in ('tools\m4 -P farversion.inc.m4') do (
	set major=%%i
	set minor=%%j
	set build=%%k
)

set tag=%major%%minor%_b%build%

echo --------------------------------------------------------------------
echo Continue only if you are sure that you have set the correct
echo build and commited the changes.
echo This command will tag the trunk under tag/%tag%.
echo --------------------------------------------------------------------
echo If you're not sure press CtrlC.
echo --------------------------------------------------------------------
echo --------------------------------------------------------------------
echo Продолжайте только если вы уверены, что вы выставили правильный
echo номер билда и закоммитили изменения.
echo Эта команда пометит текущий trunk в tags/%tag%.
echo --------------------------------------------------------------------
echo Если вы не уверены, то нажмите CtrlC
echo --------------------------------------------------------------------
pause
echo.

for /f "tokens=3" %%f in ('svn info ^| find "Root:"') do set repo=%%f

set tag_path=%repo%/tags/builds/%tag%

svn info %tag_path% > nul 2>&1 & (
	if not errorlevel 1 (
		echo Error: tag %tag% already exists
	) else (
		svn copy %repo%/trunk %tag_path% -m "tag build %build%"
	)
)

endlocal
