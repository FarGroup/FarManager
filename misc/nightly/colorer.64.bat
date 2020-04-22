call %~dp0base_64.bat

set PROJECT_ROOT=%~dp0FarColorer
set PROJECT_CONFIG=Release
set PROJECT_CONF=x64

c:\cmake\bin\cmake.exe -G "NMake Makefiles" -D CMAKE_BUILD_TYPE=%PROJECT_CONFIG% -D COLORER_WIN64_BUILD=ON %PROJECT_ROOT%
nmake
