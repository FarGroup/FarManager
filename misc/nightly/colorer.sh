#!/bin/bash

function bcolorer {
  BIT=$1
  PLUGIN=FarColorer
  COLORER_VERSION=$(curl -fsLJ 'https://raw.githubusercontent.com/colorer/FarColorer/master/version4far.txt')
  if [ -z "$COLORER_VERSION" ]; then
    echo "Failed to get Colorer version"
    return 1
  fi

  echo "Download FarColorer ${COLORER_VERSION}"
  COLORER_PLATFORM=$2
  COLORER_FILE_NAME=FarColorer.${COLORER_PLATFORM}.v${COLORER_VERSION}.7z
  
  rm -f ${COLORER_FILE_NAME}
  curl -fsLJO -o ${COLORER_FILE_NAME} "https://github.com/colorer/FarColorer/releases/download/v${COLORER_VERSION}/${COLORER_FILE_NAME}"
  if [ ! -e ${COLORER_FILE_NAME} ]; then
    echo "Can't find ${COLORER_FILE_NAME}"
    return 1
  fi
  mkdir outfinalnew${BIT}/Plugins/$PLUGIN
  7z x ${COLORER_FILE_NAME} -ooutfinalnew${BIT}/Plugins/$PLUGIN
  rm -f ${COLORER_FILE_NAME}

}

( \
	bcolorer 32 x86 && \
	bcolorer 64 x64 \
) || exit 1

cd ..
