@echo off
setlocal

for /f "tokens=1,2,4 delims=, " %%i in ('tools\m4 -P farversion.inc.m4') do (
	set major=%%i
	set minor=%%j
	set build=%%k
)

set tag=builds/%major%.%minor%.%build%

for /f "tokens=2 delims=:" %%f in ('chcp') do set current_cp=%%f > nul
chcp 65001 > nul

echo --------------------------------------------------------------------
echo Continue only if you are sure that you have set the correct
echo build and commited the changes.
echo This command will tag the current state of the repository as:
echo %tag%
echo --------------------------------------------------------------------
echo If you're not sure press Ctrl+C
echo --------------------------------------------------------------------
echo --------------------------------------------------------------------
echo Продолжайте только если вы уверены, что вы выставили правильный
echo номер билда и закоммитили изменения.
echo Эта команда пометит текущее состояние репозитория как:
echo %tag%
echo --------------------------------------------------------------------
echo Если вы не уверены, нажмите Ctrl+C
echo --------------------------------------------------------------------

chcp %current_cp% > nul

pause
echo.

if exist ..\.git goto git

for /f "tokens=3" %%f in ('svn info ^| find "Root:"') do set repo=%%f
if [%repo%] == [] goto error

set tag_path=%repo%/tags/%tag%

svn info %tag_path% > nul 2>&1 & (
	if not errorlevel 1 (
		echo Error: tag %tag% already exists
	) else (
		svn copy %repo%/trunk %tag_path% -m "tag build %build%"
	)
)
goto finish

:git
git fetch && git tag %tag% origin/master && git push origin %tag%
goto finish

:error
echo "Something went wrong"

:finish
endlocal
