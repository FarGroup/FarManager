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
  COLORER_BASE_NAME=FarColorer.${COLORER_PLATFORM}.v${COLORER_VERSION}
  COLORER_FILE_NAME=${COLORER_BASE_NAME}.wobase.7z
  COLORER_PDB_NAME=${COLORER_BASE_NAME}.pdb.7z

  rm -f ${COLORER_FILE_NAME}
  rm -f ${COLORER_PDB_NAME}

  COLORER_BASE_URL=https://github.com/colorer/FarColorer/releases/download/v${COLORER_VERSION}/
  curl -fsLJO ${COLORER_BASE_URL}${COLORER_FILE_NAME}
  curl -fsLJO ${COLORER_BASE_URL}${COLORER_PDB_NAME}
  if [ ! -e ${COLORER_FILE_NAME} ]; then
    echo "Can't find ${COLORER_FILE_NAME}"
    return 1
  fi
  if [ ! -e ${COLORER_PDB_NAME} ]; then
    echo "Can't find ${COLORER_PDB_NAME}"
    return 1
  fi

  COLORER_SCHEMES_VERSION=$(curl --silent "https://api.github.com/repos/colorer/Colorer-schemes/releases/latest" | grep '"tag_name":' | sed -E 's/.*"v([^"]+)".*/\1/')
  if [ -z "$COLORER_SCHEMES_VERSION" ]; then
    echo "Failed to get Colorer schemes version"
    return 1
  fi
  echo "Download Colorer schemes ${COLORER_SCHEMES_VERSION}"
  COLORER_SCHEMES_FILE_NAME=colorer-base.allpacked.${COLORER_SCHEMES_VERSION}.zip

  rm -f ${COLORER_SCHEMES_FILE_NAME}

  COLORER_SCHEMES_BASE_URL=https://github.com/colorer/Colorer-schemes/releases/download/v${COLORER_SCHEMES_VERSION}/
  curl -fsLJO ${COLORER_SCHEMES_BASE_URL}${COLORER_SCHEMES_FILE_NAME}
  if [ ! -e ${COLORER_SCHEMES_FILE_NAME} ]; then
    echo "Can't find ${COLORER_SCHEMES_FILE_NAME}"
    return 1
  fi

  COLORER_DIR=outfinalnew${BIT}/Plugins/$PLUGIN
  mkdir ${COLORER_DIR}
  7z x ${COLORER_FILE_NAME} -o${COLORER_DIR}
  7z x ${COLORER_PDB_NAME} -o${COLORER_DIR}/bin
  7z x ${COLORER_SCHEMES_FILE_NAME} -o${COLORER_DIR}/base
  rm -f ${COLORER_FILE_NAME}
  rm -f ${COLORER_PDB_NAME}
  rm -f ${COLORER_SCHEMES_FILE_NAME}

}

( \
	bcolorer 32 x86 && \
	bcolorer 64 x64 \
) || exit 1

cd ..
