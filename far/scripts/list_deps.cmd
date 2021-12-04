@echo off

setlocal enabledelayedexpansion

pushd

set src=%~dp0..\

cd %src%
call :list
cd %src%common
call :list /S common
cd %src%platform.sdk
call :list /S platform.sdk
cd %src%thirdparty
call :list /S thirdparty

popd
endlocal
goto :eof

:list
(for /f "delims=" %%a in ('dir /B %1 /A:-D *.cpp *.hpp *.c *.h *.rc') do (
	set "dep_path=%%a"
	set dep_path=!dep_path:%cd%=!
	echo %2!dep_path!)
)
