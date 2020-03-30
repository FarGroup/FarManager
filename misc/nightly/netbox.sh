#!/bin/bash

function bnetbox {
  BIT=$1
  PLATFORM=$2
  PLUGIN=NetBox

  rm -fR Far3_${PLATFORM}
  mkdir -p Far3_${PLATFORM}/Plugins/NetBox || return 1
  mkdir -p build/Release/${PLATFORM} || return 1

  wine cmd /c ../netbox.${BIT}.bat &> ../logs/netbox${BIT}

  mkdir -p ../outfinalnew${BIT}/Plugins/${PLUGIN}
  cp -f build/Release/${PLATFORM}/NetBox.dll ../outfinalnew${BIT}/Plugins/${PLUGIN}
  cp -f build/Release/${PLATFORM}/NetBox.map ../outfinalnew${BIT}/Plugins/${PLUGIN}
  cp -f src/NetBox/*.lng ../outfinalnew${BIT}/Plugins/${PLUGIN}
  cp -f ChangeLog ../outfinalnew${BIT}/Plugins/${PLUGIN}
  cp -f *.md ../outfinalnew${BIT}/Plugins/${PLUGIN}
  cp -f LICENSE.txt ../outfinalnew${BIT}/Plugins/${PLUGIN}
}

#git clone must already exist
cd Far-NetBox || exit 1
rm -fR build
git pull || exit 1

( \
	bnetbox 32 x86 && \
	bnetbox 64 x64 \
) || exit 1

cd ..
