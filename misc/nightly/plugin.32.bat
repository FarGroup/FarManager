call %~dp0base_32.bat

nmake /f makefile_vc FAR_WORKDIR=..\..\outfinalnew32 BUILD=1 LUA=luasdk\20240301\64\lua.exe
