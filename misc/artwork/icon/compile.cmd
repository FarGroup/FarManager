@echo off

goto :main

:make_impl
echo %3.svg
%XSL% %OUTDIR%\svg\%1.svg src\xslt\filter_%2.xslt -o %OUTDIR%\svg\%3.svg

echo %3.png
%INKSCAPE% -z -e %OUTDIR%\png\%3.png -w %size% -h %size% %OUTDIR%\svg\%3.svg
goto :eof

:make_one
call :make_impl %1 %2 %3
if not %size% gtr 48 (
call :make_impl %3 no_fx %3_index
)
goto :eof

:make
set size=%2

copy /b src\svg\%1.svg %OUTDIR%\svg\msg_%2.svg > nul

if not %size% gtr 48 (
call :make_one msg_%2  none        msg_%2
) else (
call :make_one msg_%2  make_shadow msg_%2
)

call :make_one msg_%2  no_message  blue_%2
call :make_one blue_%2 red         red_%2
call :make_one blue_%2 black       black_%2
call :make_one blue_%2 hack        hack_%2
call :make_one blue_%2 mono        mono_%2

goto :eof

:make_all
call :make 16 16
call :make 24 24
call :make 32 32
call :make 48 48
call :make 32 256
goto :eof

:help
echo Usage:
echo arg1 arg2
echo where: arg1 - source size, arg2 - destination size.
echo Run without args to generate all sizes.
goto :eof

:main
setlocal

set XSL=msxsl.exe
set INKSCAPE=inkscape.exe
set OUTDIR=%~dp0_artefacts

mkdir %OUTDIR%
mkdir %OUTDIR%\svg
mkdir %OUTDIR%\png

if "%2"== "" if "%1"== "" (call :make_all) else (call :help) else (call :make %1 %2)

endlocal
goto :eof
