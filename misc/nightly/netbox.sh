#!/bin/bash

function bnetbox {
  BIT=$1
  PLATFORM=$2
  PLUGIN=NetBox

  rm -fR Far3_${PLATFORM}
  mkdir -p Far3_${PLATFORM}/Plugins/NetBox || return 1
  mkdir -p build/Release/${PLATFORM} || return 1

  wine cmd /c ../netbox.${BIT}.bat &> ../logs/netbox${BIT}

  pushd Far3_${PLATFORM}/Plugins/${PLUGIN} || return 1

  mkdir -p ../../../../../outfinalnew${BIT}/Plugins/${PLUGIN}
  cp -f * ../../../../../outfinalnew${BIT}/Plugins/${PLUGIN}

  popd
  cp -f ChangeLog ../outfinalnew${BIT}/Plugins/${PLUGIN}
}

#git clone must already exist and set to far3 branch
cd Far-NetBox || exit 1
rm -fR build
git pull || exit 1

( \
	bnetbox 32 x86 && \
	bnetbox 64 x64 \
) || exit 1

cd ..
