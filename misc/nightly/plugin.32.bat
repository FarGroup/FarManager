call c:\VC10\vcvarsall.bat x86
SET CPU=i386
SET APPVER=6.0

nmake /f makefile_vc NO_RELEASE_PDB=1
