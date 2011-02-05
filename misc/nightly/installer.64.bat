call c:\VC10\vcvarsall.bat x86
SET CPU=i386
SET APPVER=6.0
SET Path=%Path%;C:\Program Files\Windows Installer XML v3\bin

cl -nologo -EHsc -DNIGHTLY genparams.cpp

call c:\VC10\vcvarsall.bat x86_amd64
SET CPU=AMD64
SET APPVER=6.0

genparams c:\src\outfinalnew64 x64

nmake -nologo RELEASE=1

genparams c:\src\outfinalold64 x64

nmake -nologo RELEASE=1
