#!/bin/sh

function bcolorer {
  BIT=$1
  PLUGIN=FarColorer

  cd farcolorer/
  rm -fR bin
  cd src
  cp -f ../../../pcolorer3.rc ./pcolorer3.rc
  wine cmd /c ../../../colorer.${BIT}.bat &> ../../../logs/colorer${BIT}
  cd ..

  mkdir -p ../../outfinalnew${BIT}/Plugins/${PLUGIN}

  cp -f history.ru.txt LICENSE README ../../outfinalnew${BIT}/Plugins/$PLUGIN/

  if [ ! -e bin ]; then
    return
  fi

  if [ "$BIT" == "64" ]; then
    cd bin
    mv colorer_x64.dll colorer.dll
    mv colorer_x64.map colorer.map
    cd ..
  fi

  cp -Rf bin ../../outfinalnew${BIT}/Plugins/$PLUGIN/

  cd ../schemes
  cp -Rf base ../../outfinalnew${BIT}/Plugins/$PLUGIN/
  cd ..
}

mkdir farcolorer
cd farcolorer

rm -fR farcolorer
rm -fR colorer
#rm -fR schemes - will not delete, lots of traffic and slow, will just pull updates

svn co https://colorer.svn.sourceforge.net/svnroot/colorer/trunk/far3colorer farcolorer
svn co https://colorer.svn.sourceforge.net/svnroot/colorer/trunk/colorer/src/shared colorer/src/shared
svn co https://colorer.svn.sourceforge.net/svnroot/colorer/trunk/colorer/src/zlib colorer/src/zlib
svn co https://colorer.svn.sourceforge.net/svnroot/colorer/trunk/schemes schemes

cd schemes

chmod +x ./build.sh

./build.sh farbase.clean
./build.sh farbase &> ../../logs/colorerschemes

cd ..

bcolorer 32
bcolorer 64

cd ..
