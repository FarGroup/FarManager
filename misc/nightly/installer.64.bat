call %~dp0base_32.bat

SET Path=%Path%;C:\WIX

cl -nologo -EHsc -DNIGHTLY genparams.cpp

call %~dp0base_64.bat

genparams c:\src\outfinalnew64 x64

nmake -nologo RELEASE=1 NO_LOCALISED_MSI=1
