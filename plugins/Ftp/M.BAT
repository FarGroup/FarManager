if %1.==. goto m1
make -B -ffarftp
goto m2
:m1
make -ffarftp
:m2
@copy farftp.dll D:\Lang\BC5\FAR\Plugins\FTP>nul
@copy *.lng D:\Lang\BC5\FAR\Plugins\FTP>nul
@copy ftpmain.cpp d:\lang\bc5\far>nul
@copy ftpclass.cpp d:\lang\bc5\far>nul
@copy ftpsmall.cpp d:\lang\bc5\far>nul
@copy ftpmix.cpp d:\lang\bc5\far>nul
@copy farftp.cpp d:\lang\bc5\far>nul
@copy main.cpp d:\lang\bc5\far>nul
@copy cmds.cpp d:\lang\bc5\far>nul
@copy ftpfn.cpp d:\lang\bc5\far>nul
@copy connect.cpp d:\lang\bc5\far>nul
@copy farftp.hpp d:\lang\bc5\far>nul
