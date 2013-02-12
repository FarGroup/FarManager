call %~dp0base_32.bat

SET Path=%Path%;C:\WIX

cl -nologo -EHsc -DNIGHTLY genparams.cpp

genparams c:\src\outfinalnew32 x86

nmake -nologo RELEASE=1
