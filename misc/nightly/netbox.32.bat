call %~dp0base_32.bat
echo on
set PATH=%PATH%;c:\perl\perl\site\bin;c:\perl\perl\bin;c:\perl\c\bin;C:\unxutils\usr\local\wbin
set TERM=dumb

cd libs\openssl
rm -rf out32dll tmp32dll tmp32 inc32 out32
rm -rf x86
perl Configure VC-WIN32 no-asm enable-static-engine no-shared no-hw no-camellia no-seed no-mdc2 no-rc4 no-rc5 no-cms no-krb5 no-ec no-whirlpool no-srp no-gost no-idea no-ripemd no-des -Ox -Ob1 -Oi -Os -Oy -GF -GS- -Gy -DNDEBUG;BN_ASM;OPENSSL_NO_CAPIENG;NO_CHMOD;OPENSSL_NO_DGRAM;OPENSSL_NO_RIJNDAEL;DSO_WIN32
call ms\do_ms
nmake -f ms\nt.mak
mkdir x86
cp out32/ssleay32.lib out32/libeay32.lib x86
cp -R inc32 x86

cd ..\..\build\Release\x86

set FAR_VERSION=Far3
set PROJECT_ROOT=c:\src\Far-NetBox

set PROJECT_CONFIG=Release
set PROJECT_BUILD=Build

set PROJECT_CONF=x86
set PROJECT_PLATFORM=Win32

c:\cmake\bin\cmake.exe -D PROJECT_ROOT=%PROJECT_ROOT% -D CMAKE_BUILD_TYPE=%PROJECT_CONFIG% -D CONF=%PROJECT_CONF% -D FAR_VERSION=%FAR_VERSION% %PROJECT_ROOT%\src\NetBox
nmake
