call %~dp0base_64.bat

nmake /f makefile_vc FAR_WORKDIR=..\..\outfinalnew64 BUILD=1 LUA=luasdk\64\lua.exe
