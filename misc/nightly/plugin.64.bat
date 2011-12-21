call c:\VC10\vcvarsall.bat x86_amd64
SET CPU=AMD64
SET APPVER=6.0

nmake /f makefile_vc NO_RELEASE_PDB=1
