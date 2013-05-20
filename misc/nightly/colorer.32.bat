call %~dp0base_32.bat

set PROJECT_ROOT=%~dp0FarColorer
set PROJECT_CONFIG=Release
set PROJECT_CONF=x86

c:\cmake\bin\cmake.exe -G "NMake Makefiles" -D PROJECT_ROOT=%PROJECT_ROOT% -D CMAKE_BUILD_TYPE=%PROJECT_CONFIG% -D CONF=%PROJECT_CONF% %PROJECT_ROOT%\src
nmake
