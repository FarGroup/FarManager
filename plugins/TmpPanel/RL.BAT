bcc32 -O2 -WD -v -x- tmppanel.cpp
tdstrp32 -s tmppanel.dll tmppanel
@copy tmppanel.dll D:\Lang\BC5\FAR\Plugins\tmppanel>nul
@copy *.lng D:\Lang\BC5\FAR\Plugins\tmppanel>nul
@copy *.hlf D:\Lang\BC5\FAR\Plugins\tmppanel>nul
@copy tmppanel.dll c:\FAR\Plugins\tmppanel>nul
@copy *.lng c:\FAR\Plugins\tmppanel>nul
@copy *.hlf c:\FAR\Plugins\tmppanel>nul
@copy tmp*.cpp D:\Lang\BC5\FAR>nul
