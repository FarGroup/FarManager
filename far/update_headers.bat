@echo off
setlocal

set param=%1

for /f "tokens=1,2,4 delims=, " %%i in ('tools\m4 -P farversion.inc.m4') do (
	set major=%%i
	set minor=%%j
	set build=%%k
)

set hdrpath=..\plugins\common\unicode
if exist %hdrpath% (
	copy Include\*.hpp %hdrpath% && copy Include\vc_crt_fix* %hdrpath%\.. && call :commit %hdrpath%\.. "Update headers to %build%"
) else (
	echo %hdrpath% directory is absent
)

goto :eof

:commit
if "nocommit" == "%param%" goto :eof
if exist ..\.git call :git %* else call :svn %*
goto :eof

:git
set remote=origin
set branch=master

for /f "tokens=2" %%i in ('git branch') do set current_branch=%%i
if %current_branch% neq %branch% echo ERROR: Must be on %branch% branch, not %current_branch%. && exit
git fetch
for /f "tokens=1" %%i in ('git rev-parse HEAD') do set head_rev=%%i
for /f "tokens=1" %%i in ('git rev-parse %remote%/%branch%') do set remote_rev=%%i
if %head_rev% neq %remote_rev% echo ERROR: Your branch and '%remote%/%branch%' have diverged. && exit
git add %1 && git commit %1 -m %2 && pause && git push %remote% %branch%
goto :eof

:svn
svn commit %1 -m %2
goto :eof

endlocal
