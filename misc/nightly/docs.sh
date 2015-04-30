#!/bin/bash

function makedocs2 {

mkdir -p $1/Documentation

( \
	svn export file://`pwd`/syncrepo/trunk/docs/ENG $1/Documentation/eng && \
	svn export file://`pwd`/syncrepo/trunk/docs/RUS $1/Documentation/rus && \
	svn export file://`pwd`/syncrepo/trunk/addons $1/Addons && \

	svn export file://`pwd`/syncrepo/trunk/docs/RestoreOldPluginSettings.cmd $1/ && \
	svn export file://`pwd`/syncrepo/trunk/docs/SaveOldPluginSettings.cmd $1/ \
) || return 1

}

( makedocs2 outfinalnew32 && makedocs2 outfinalnew64 ) || exit 1	
