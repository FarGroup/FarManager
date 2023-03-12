#!/bin/bash

function makedocs2 {

cp -R far.git/extra/* $1/ || return 1

find $1/Documentation -type f -exec unix2dos {} \;
find $1/Addons -type f -exec unix2dos {} \;
unix2dos $1/RestoreOldPluginSettings.cmd
unix2dos $1/SaveOldPluginSettings.cmd
unix2dos $1/Far.VisualElementsManifest.xml

}

( makedocs2 outfinalnew32 && makedocs2 outfinalnew64 && makedocs2 outfinalnewARM64 ) || exit 1
