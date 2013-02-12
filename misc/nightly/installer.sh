#!/bin/bash

cd misc/msi-installer

WINEDLLOVERRIDES="msi=n" wine cmd /c ../../installer.32.bat

WINEDLLOVERRIDES="msi=n" wine cmd /c ../../installer.64.bat

cd ../..
