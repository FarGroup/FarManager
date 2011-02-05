call c:\VC10\vcvarsall.bat x86
SET CPU=i386
SET APPVER=6.0
SET Path=%Path%;C:\Program Files\Windows Installer XML v3\bin

cl -nologo -EHsc -DNIGHTLY genparams.cpp

genparams c:\src\outfinalnew32 x86

nmake -nologo RELEASE=1
