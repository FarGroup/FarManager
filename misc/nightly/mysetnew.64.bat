call c:\VC9\vcvarsall.bat x86_amd64
SET CPU=AMD64
SET APPVER=6.0

nmake /f makefile_vc build USEDEPS=1 NO_RELEASE_PDB=1
