call c:\VC9\vcvarsall.bat x86
SET CPU=i386
SET APPVER=6.0
SET Path=%Path%;C:\Program Files\Windows Installer XML v3\bin

cl -EHsc -DSPECIAL genscript.cpp

genscript c:\src\outfinalnew32 x86

nmake -nologo

genscript c:\src\outfinalold32 x86

nmake -nologo
