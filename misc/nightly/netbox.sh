#!/bin/bash

function bnetbox {
  CURL="curl --fail --silent --retry 50 --connect-timeout 10 --retry-delay 10"
  BIT=$1
  PLUGIN=NetBox
  NETBOX_PLATFORM=$2
  NETBOX_VERSION=$($CURL "https://api.github.com/repos/michaellukashov/Far-NetBox/releases/latest" | grep '"tag_name":' | sed -E 's/.*"v([^"]+)".*/\1/')
  NETBOX_FILE_VERSION=$($CURL "https://api.github.com/repos/michaellukashov/Far-NetBox/releases/latest" | grep -E '"browser_download_url.+'${NETBOX_PLATFORM}'.+[0-9]\.7z\"' | sed -E 's/.+NetBox\.'${NETBOX_PLATFORM}'\.(.+)\.7z.+/\1/')

  if [ -z "$NETBOX_VERSION" ]; then
    echo "Failed to get NetBox version"
    return 1
  fi

  if [ -z "NETBOX_FILE_VERSION" ]; then
    echo "Failed to get NetBox file version"
    return 1
  fi

  echo "Download NetBox ${NETBOX_FILE_VERSION}"
  NETBOX_BASE_NAME=NetBox.${NETBOX_PLATFORM}.${NETBOX_FILE_VERSION}
  NETBOX_FILE_NAME=${NETBOX_BASE_NAME}.7z
  NETBOX_PDB_NAME=${NETBOX_BASE_NAME}.pdb.7z

  rm -f ${NETBOX_FILE_NAME}
  rm -f ${NETBOX_PDB_NAME}

  NETBOX_BASE_URL=https://github.com/michaellukashov/Far-NetBox/releases/download/v${NETBOX_VERSION}/
  $CURL -LJO ${NETBOX_BASE_URL}${NETBOX_FILE_NAME}
  $CURL -LJO ${NETBOX_BASE_URL}${NETBOX_PDB_NAME}
  if [ ! -e ${NETBOX_FILE_NAME} ]; then
    echo "Can't find ${NETBOX_FILE_NAME}"
    return 1
  fi
  if [ ! -e ${NETBOX_PDB_NAME} ]; then
    echo "Can't find ${NETBOX_PDB_NAME}"
    return 1
  fi
  NETBOX_DIR=outfinalnew${BIT}/Plugins
  mkdir ${NETBOX_DIR}
  7z x ${NETBOX_FILE_NAME} -o${NETBOX_DIR}
  7z x ${NETBOX_PDB_NAME} -o${NETBOX_DIR}/${PLUGIN}
  rm -f ${NETBOX_FILE_NAME}
  rm -f ${NETBOX_PDB_NAME}
}

( \
	bnetbox 32 x86 && \
	bnetbox 64 x64 && \
	bnetbox ARM64 ARM64 \
) || exit 1

cd ..
