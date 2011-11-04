#! /bin/sh
#-----------------------------------------------------------------
#examples:
#  build.sh
#  build.sh 32
#  build.sh clean 64
#  build.sh clean 32/64
#-----------------------------------------------------------------
nbits='32 64'
deb_b=N
clean=N
while [ $# -ne 0 ] ; do
  case "${1}" in
    debug|dbg|-debug|-dbg)         deb_b=Y     ;;
    32|x86|win32|-32)              nbits=32    ;;
    64|x64|win64|-64)              nbits=64    ;;
    32/64|32x64|32-64|-32/64)      nbits=32/64 ;;
    clean|rebuild|-clean|-rebuild) clean=Y     ;;
  esac
  shift
done
#-----------------------------------------------------------------
[ -f ./build_settings.sh ] && source ./build_settings.sh
[ -z "${GMAKE}" ] && GMAKE=make
case `uname -o` in
  Msys) ;;
  *)
    [ -z "${GCC_PREFIX_32}" ] && GCC_PREFIX_32=i686-w64-mingw32-
    [ -z "${GCC_PREFIX_64}" ] && GCC_PREFIX_64=x86_64-w64-mingw32-
  ;;
esac
#-----------------------------------------------------------------
m="${GMAKE} --no-print-directory -f makefile_gcc"
if [ "Y" = "${deb_b}" ]; then m="${m} DEBUG=1" ; fi

for dirbit in $nbits
do
  case "${dirbit}" in
    32)    pref=${GCC_PREFIX_32} ; nbit=32 ;;
    64)    pref=${GCC_PREFIX_64} ; nbit=64 ;;
    32/64) pref=${GCC_PREFIX_64} ; nbit=32 ;;
  esac
  [ -z "${pref}" ] || m="${m} GCC_PREFIX=${pref}"
  [ "Y" = "${clean}" ] && $m DIRBIT=${nbit} clean
  ${m} DIRBIT=${nbit}
done
