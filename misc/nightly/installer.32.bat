call ../nightly/base_32.bat

SET Path=%Path%;C:\Program Files\Windows Installer XML v3.5\bin

cl -nologo -EHsc -DNIGHTLY genparams.cpp

genparams c:\src\outfinalnew32 x86

nmake -nologo RELEASE=1
