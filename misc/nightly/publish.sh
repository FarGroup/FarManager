#!/bin/sh

ARCNAME=final
NIGHTLY_WEB_ROOT=/var/www/html/nightly

#./installer.sh

#Arguments:  processFarBuild <32|64>
processFarBuild()
{
#	if [ ! -e ../outfinalnew$1/${ARCNAME}.msi ]; then
#		echo "outfinalnew$1/${ARCNAME}.msi is missing"
#		return
#	fi
	
	if [ ! -e ../outfinalnew$1/Far.exe ]; then
		echo "outfinalnew$1/Far.exe is missing"
		return
	fi

	BASE=$PWD
	
	cd ../outfinalnew$1
	if [ ! $? ]; then
		echo "cd ../outfinalnew$1 failed"
		return
	fi

	7za a -r -x!${ARCNAME}.msi ${ARCNAME}.7z *

	cd $BASE
	m4 -P -DFARBIT=$1 -DHOSTTYPE=Unix -D ARC=../outfinalnew$1/$ARCNAME -D FARVAR=new -D LASTCHANGE="$LASTCHANGE" ../pagegen.m4 > $NIGHTLY_WEB_ROOT/FarW.$1.php
}

cd unicode_far
LASTCHANGE=`head -1 changelog | dos2unix`
processFarBuild 32
processFarBuild 64
cd ..
