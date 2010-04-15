#!/bin/sh

ARCNAME=final
NIGHTLY_WEB_ROOT=/var/www/html/nightly

./installer.sh

#Arguments:  processFarBuild <new|old> <32|64> <New|Old>
processFarBuild()
{
	if [! -e ../outfinal$1$2/${ARCNAME}.msi]; then
		echo "outfinal$1$2/${ARCNAME}.msi is missing"
		return
	fi
	
	BASE = $PWD
	
	cd ../outfinal$1$2
	7za a -r -x${ARCNAME}.msi ${ARCNAME}.7z *
	
	cd BASE
	m4 -P -DFARBIT=$2 -D ARC=../outfinal$1$2/$ARCNAME -D FARVAR=$1 -D LASTCHANGE="$LASTCHANGE" ../pagegen.m4 > $NIGHTLY_WEB_ROOT/Far$3.$2.php
}

cd far
LASTCHANGE=`head -1 changelog | dos2unix`
processFarBuild new 32 New
processFarBuild new 64 New
cd ..

cd farold
LASTCHANGE=`head -1 changelog | dos2unix`
cp -f ../outfinalold32/changelog $NIGHTLY_WEB_ROOT/changelogfar
processFarBuild old 32 Old
processFarBuild old 64 Old
cd ..
