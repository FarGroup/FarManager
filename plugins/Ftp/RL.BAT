if %1.==. goto m1
make -ffarftprl
goto m2
:m1
make -B -ffarftprl
:m2
tdstrp32 -s farftp.dll farftp
@copy farftp.dll D:\Lang\BC5\FAR\Plugins\FTP
@copy *.lng D:\Lang\BC5\FAR\Plugins\FTP
@copy *.hlf D:\Lang\BC5\FAR\Plugins\FTP
@copy farftp.dll c:\far\Plugins\FTP
@copy *.lng c:\far\Plugins\FTP
@copy *.hlf c:\far\Plugins\FTP
@copy farftp.dll d:\lang\bc5\far
@copy ftpmain.cpp d:\lang\bc5\far
@copy ftpclass.cpp d:\lang\bc5\far
@copy farftp.cpp d:\lang\bc5\far
@copy main.cpp d:\lang\bc5\far
@copy cmds.cpp d:\lang\bc5\far
@copy ftpfn.cpp d:\lang\bc5\far
@copy connect.cpp d:\lang\bc5\far
@copy farftp.hpp d:\lang\bc5\far

