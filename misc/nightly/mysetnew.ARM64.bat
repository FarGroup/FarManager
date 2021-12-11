call %~dp0base_ARM64.bat

nmake /f makefile_vc build USEDEPS=1
