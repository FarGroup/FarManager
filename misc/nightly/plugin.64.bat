call %~dp0base_64.bat

nmake /f makefile_vc NO_RELEASE_PDB=1 FAR_WORKDIR=../../outfinalnew64
