call %~dp0base_64.bat
echo on
set PATH=%PATH%;c:\perl\perl\site\bin;c:\perl\perl\bin;c:\perl\c\bin;C:\unxutils\usr\local\wbin
set TERM=dumb

cd build\Release\x64

set FAR_VERSION=Far3
set PROJECT_ROOT=c:\src\Far-NetBox

set PROJECT_CONFIG=Release
set PROJECT_BUILD=Build

set PROJECT_CONF=x64
set PROJECT_PLATFORM=x64

c:\cmake\bin\cmake.exe -D PROJECT_ROOT=%PROJECT_ROOT% -D CMAKE_BUILD_TYPE=%PROJECT_CONFIG% -D CONF=%PROJECT_CONF% -D FAR_VERSION=%FAR_VERSION% %PROJECT_ROOT%\src\NetBox
nmake
