#!/bin/bash

function makedocs2 {

mkdir -p $1/Documentation/eng
mkdir -p $1/Documentation/rus
mkdir -p $1/Addons

( \
	cp -R far.git/docs/ENG/*  $1/Documentation/eng/ && \
	cp -R far.git/docs/RUS/* $1/Documentation/rus/ && \
	cp -R far.git/addons/* $1/Addons/ && \

	cp -R far.git/docs/RestoreOldPluginSettings.cmd $1/ && \
	cp -R far.git/docs/SaveOldPluginSettings.cmd $1/ \
) || return 1

find $1/Documentation -type f -exec unix2dos {} \;
find $1/Addons -type f -exec unix2dos {} \;
unix2dos $1/RestoreOldPluginSettings.cmd
unix2dos $1/SaveOldPluginSettings.cmd

}

( makedocs2 outfinalnew32 && makedocs2 outfinalnew64 ) || exit 1	
