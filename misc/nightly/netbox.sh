#!/bin/bash

function bnetbox {
  BIT=$1
  PLATFORM=$2
  PLUGIN=NetBox

  rm -fR Far3_${PLATFORM}
  rm -fR build/Release/${PLATFORM}/CMakeFiles
  rm build/Release/${PLATFORM}/*.dll
  rm build/Release/${PLATFORM}/*.lib

  mkdir -p Far3_${PLATFORM}/Plugins/NetBox || return 1

  wine cmd /c ../netbox.${BIT}.bat &> ../logs/netbox${BIT}

  pushd build/${PLUGIN}/Far3/${PLATFORM} || return 1

  mkdir -p ../../../../../outfinalnew${BIT}/Plugins/${PLUGIN}
  cp -f * ../../../../../outfinalnew${BIT}/Plugins/${PLUGIN}

  popd
}

#git clone must already exist and set to far3 branch
#all build dirs with cmake cache files must also exist as cmake gets stuck under wine on first run without cache files
cd Far-NetBox || exit 1
git pull || exit 1

rm -fR build/NetBox

( \
	bnetbox 32 x86 && \
	bnetbox 64 x64 \
) || exit 1

cd ..
