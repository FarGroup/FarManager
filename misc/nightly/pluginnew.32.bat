call c:\VC10\vcvarsall.bat x86
SET CPU=i386
SET APPVER=6.0

nmake /f makefile_vc WIDE=1
