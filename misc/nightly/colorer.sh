#!/bin/bash

function bcolorer {
  PLUGIN=FarColorer
  COLORER_VERSION=1.4.1
  COLORER_PLATFORM=$2
  COLORER_FILE_NAME=FarColorer.${COLORER_PLATFORM}.v${COLORER_VERSION}.7z
  curl -fsLJO -o ${COLORER_FILE_NAME} https://github.com/colorer/FarColorer/releases/download/v${COLORER_VERSION}/${COLORER_FILE_NAME}
  if [ ! -e ${COLORER_FILE_NAME} ]; then
    echo "Can't find ${COLORER_FILE_NAME}"
    return 1
  fi
  mkdir ../outfinalnew${BIT}/Plugins/$PLUGIN
  7z x ${COLORER_FILE_NAME} -o../outfinalnew${BIT}/Plugins/$PLUGIN

}

( \
	bcolorer 32 x86 && \
	bcolorer 64 x64 \
) || exit 1

cd ..
