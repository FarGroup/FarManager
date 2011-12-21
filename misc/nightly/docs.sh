#!/bin/sh

function makedocs2 {

mkdir -p $1/Documentation

svn export http://localhost/svn/trunk/docs/ENG $1/Documentation/eng
svn export http://localhost/svn/trunk/docs/RUS $1/Documentation/rus

svn export http://localhost/svn/trunk/addons $1/Addons

cp docs/RestoreOldPluginSettings.cmd docs/SaveOldPluginSettings.cmd $1/

}

rm -fR docs

svn co http://localhost/svn/trunk/docs docs

makedocs2 outfinalnew32
makedocs2 outfinalnew64
