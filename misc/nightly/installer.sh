#!/bin/bash

cd misc/msi-installer || exit 1

WINEDLLOVERRIDES="msi=n" wine cmd /c ../../installer.32.bat

WINEDLLOVERRIDES="msi=n" wine cmd /c ../../installer.64.bat

WINEDLLOVERRIDES="msi=n" wine cmd /c ../../installer.ARM64.bat

cd ../..

chmod +rw outfinalnew32/final.msi
chmod +rw outfinalnew64/final.msi
chmod +rw outfinalnewARM64/final.msi
