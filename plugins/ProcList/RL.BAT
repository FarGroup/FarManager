bcc32 -O2 -WD -v -x- proclist.cpp
tdstrp32 -s proclist.dll proclist
@copy proclist.dll D:\Lang\BC5\FAR\Plugins\proclist>nul
@copy *.lng D:\Lang\BC5\FAR\Plugins\proclist>nul
@copy *.hlf D:\Lang\BC5\FAR\Plugins\proclist>nul
@copy proclist.dll c:\FAR\Plugins\proclist>nul
@copy *.lng c:\FAR\Plugins\proclist>nul
@copy *.hlf c:\FAR\Plugins\proclist>nul
@copy p*.cpp D:\Lang\BC5\FAR>nul
