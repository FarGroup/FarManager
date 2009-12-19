#!/bin/sh

cd misc/msi-installer

WINEDLLOVERRIDES="msi=n" wine cmd /c ../../installer.bat

cd ../..
