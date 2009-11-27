#!/bin/sh

ARCNAME=final
cd outfinalnew32
7za a -r ${ARCNAME}.7z *
cd ../outfinalnew64
7za a -r ${ARCNAME}.7z *
cd ../outfinalold32
7za a -r ${ARCNAME}.7z *
cd ../outfinalold64
7za a -r ${ARCNAME}.7z *
cd ..

./installer.sh

cd far
LASTCHANGE=`head -1 changelog | dos2unix`
m4 -P -DFARBIT=32 -D ARC=../outfinalnew32/$ARCNAME -D FARVAR=new -D LASTCHANGE="$LASTCHANGE" ../pagegen.m4 > /var/www/html/nightly/FarNew.32.php
m4 -P -DFARBIT=64 -D ARC=../outfinalnew64/$ARCNAME -D FARVAR=new -D LASTCHANGE="$LASTCHANGE" ../pagegen.m4 > /var/www/html/nightly/FarNew.64.php
cd ..

cd farold
LASTCHANGE=`head -1 changelog | dos2unix`
cp -f ../outfinalold32/changelog /var/www/html/nightly/changelogfar
m4 -P -DFARBIT=32 -D ARC=../outfinalold32/$ARCNAME -D FARVAR=old -D LASTCHANGE="$LASTCHANGE" ../pagegen.m4 > /var/www/html/nightly/FarOld.32.php
m4 -P -DFARBIT=64 -D ARC=../outfinalold64/$ARCNAME -D FARVAR=old -D LASTCHANGE="$LASTCHANGE" ../pagegen.m4 > /var/www/html/nightly/FarOld.64.php
cd ..
