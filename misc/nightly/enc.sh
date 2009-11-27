#!/bin/sh

function benc {
LNG=$1
L=$2

cd $LNG
wine "C:/Program Files/HTML Help Workshop/hhc.exe" plugins${L}.hhp
cp -f distr_chm_plugins${L}/FarEncyclopedia.${LNG}.chm ../../../../outfinalnew32/Encyclopedia/
cp -f distr_chm_plugins${L}/FarEncyclopedia.${LNG}.chm ../../../../outfinalnew64/Encyclopedia/
cp -f distr_chm_plugins${L}/FarEncyclopedia.${LNG}.chm ../../../../outfinalold32/Encyclopedia/
cp -f distr_chm_plugins${L}/FarEncyclopedia.${LNG}.chm ../../../../outfinalold64/Encyclopedia/
COPYOK=$?
cd ..

}

rm -fR enc

svn co http://localhost/svn/trunk/enc enc

mkdir -p outfinalnew32/Encyclopedia
mkdir -p outfinalnew64/Encyclopedia
mkdir -p outfinalold32/Encyclopedia
mkdir -p outfinalold64/Encyclopedia

pushd enc/tools
chmod +x tool.make_chm.pl
./tool.make_chm.pl
cd ../enc/chm

benc en e
benc ru r

popd
