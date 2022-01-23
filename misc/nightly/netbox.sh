#!/bin/bash

function bnetbox {
  BIT=$1
  PLUGIN=NetBox
  NETBOX_VERSION=$(curl --silent "https://api.github.com/repos/FarGroup/Far-NetBox/releases/latest" | grep '"tag_name":' | sed -E 's/.*"v([^"]+)".*/\1/')
  if [ -z "$NETBOX_VERSION" ]; then
    echo "Failed to get NetBox version"
    return 1
  fi

  echo "Download NetBox ${NETBOX_VERSION}"
  NETBOX_PLATFORM=$2
  NETBOX_BASE_NAME=NetBox.${NETBOX_PLATFORM}.${NETBOX_VERSION}
  NETBOX_FILE_NAME=${NETBOX_BASE_NAME}.7z
  NETBOX_PDB_NAME=${NETBOX_BASE_NAME}.pdb.7z

  rm -f ${NETBOX_FILE_NAME}
  rm -f ${NETBOX_PDB_NAME}

  NETBOX_BASE_URL=https://github.com/FarGroup/Far-NetBox/releases/download/v${NETBOX_VERSION}/
  curl -fsLJO ${NETBOX_BASE_URL}${NETBOX_FILE_NAME}
  curl -fsLJO ${NETBOX_BASE_URL}${NETBOX_PDB_NAME}
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
