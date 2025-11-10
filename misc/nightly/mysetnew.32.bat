call %~dp0base_32.bat

set ENABLE_TESTS=0
nmake /f makefile_vc build USEDEPS=1
