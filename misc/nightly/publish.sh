#!/bin/bash

ARCNAME=final
NIGHTLY_WEB_ROOT=/var/www/html/nightly

#Arguments:  processFarBuild <32|64>
processFarBuild()
{
	if [ ! -e ../outfinalnew$1/${ARCNAME}.msi ]; then
		echo "outfinalnew$1/${ARCNAME}.msi is missing"
		return 1
	fi
	
	BASE=$PWD
	
	cd ../far.git
	SCM_REVISION=`git rev-parse --short HEAD`
	cd $BASE

	if ! cd ../outfinalnew$1; then
		echo "cd ../outfinalnew$1 failed"
		return 1
	fi

	7za a -m0=LZMA -r -x!${ARCNAME}.msi -x!*.pdb ${ARCNAME}.7z *
	7za a -m0=LZMA -r ${ARCNAME}.pdb.7z *.pdb

	cd $BASE || return 1
	m4 -P -DFARBIT=$1 -DHOSTTYPE=Unix -D ARC=../outfinalnew$1/$ARCNAME -D FARVAR=new -D SCM_REVISION="$SCM_REVISION" -D LASTCHANGE="$LASTCHANGE" ../pagegen.m4 > $NIGHTLY_WEB_ROOT/FarW.$1.php
}

./installer.sh || exit 1

cd far || exit 1
LASTCHANGE=`head -1 changelog | dos2unix`
( \
	processFarBuild 32 && \
	processFarBuild 64 \
) || exit 1
cd ..
