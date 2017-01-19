@echo off
setlocal

for /f "tokens=1,2,4 delims=," %%i in ('tools\m4 -P farversion.inc.m4') do (
	set major=%%i
	set minor=%%j
	set build=%%k
)

set hdrpath=..\plugins\common\unicode
if exist %hdrpath% (
	@copy Include\*.hpp %hdrpath% && svn commit %hdrpath% -m "update headers to %build%"
) else (
	echo %hdrpath% directory is absent
)

endlocal
