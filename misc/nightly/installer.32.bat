call %~dp0base_32.bat

SET Path=%Path%;C:\WIX

set SOURCE_DIR=c:\src\outfinalnew32
pushd && cd ..\..\far && for /f "tokens=1,2,4 delims=, " %%i in ('tools\m4 -P farversion.inc.m4') do set VER_MAJOR=%%i && set VER_MINOR=%%j && set VER_BUILD=%%k && popd
set NIGHTLY=1

nmake -nologo RELEASE=1 NO_LOCALISED_MSI=1
