#!/bin/bash

function bcolorer {
  BIT=$1
  PLATFORM=$2
  PLUGIN=FarColorer

  mkdir -p build/Release/${PLATFORM} || return 1

  pushd build/Release/${PLATFORM} || return 1

  wine cmd /c ../../../../colorer.${BIT}.bat &> ../../../../logs/colorer${BIT}

  if [ ! -e src/colorer.dll ]; then
    echo "Can't find colorer.dll"
    return 1
  fi

  mkdir -p ../../../../outfinalnew${BIT}/Plugins/${PLUGIN}/bin

  cp -f src/colorer.dll src/colorer.map ../../../../outfinalnew${BIT}/Plugins/$PLUGIN/bin

  popd

  cp -f docs/history.ru.txt LICENSE README.md ../outfinalnew${BIT}/Plugins/$PLUGIN/
  cp -f misc/* ../outfinalnew${BIT}/Plugins/$PLUGIN/bin

  pushd ../Colorer-schemes || return 1
  mkdir -p ../outfinalnew${BIT}/Plugins/$PLUGIN/base
  cp -Rf build/basefar/* ../outfinalnew${BIT}/Plugins/$PLUGIN/base
  popd
}

#git clone must already exist
cd Colorer-schemes || exit 1
git pull || exit 1

#neweset ubuntu ant 1.8.2/1.9.3 has a bug and can't find the resolver, 1.8.4 works fine
PATH=~/apache-ant-1.8.4/bin:$PATH
export PATH
rm -fR build
./build.sh base.far.clean
./build.sh base.far &> ../logs/colorerschemes || exit 1

cd ..

#git clone must already exist
cd FarColorer || exit 1
rm -fR build
git pull || exit 1
git submodule update --recursive || exit 1

( \
	bcolorer 32 x86 && \
	bcolorer 64 x64 \
) || exit 1

cd ..
