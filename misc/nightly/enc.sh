#!/bin/sh

function benc2 {
LNG=$1
L=$2

cd ${LNG}
wine "C:/Program Files/HTML Help Workshop/hhc.exe" plugins${L}.hhp
cp -f FarEncyclopedia.${LNG}.chm ../../../../outfinalnew32/Encyclopedia/ && \
cp -f FarEncyclopedia.${LNG}.chm ../../../../outfinalnew64/Encyclopedia/
if [ "$?" != "0" ]; then exit 1
fi
cd ..

}

rm -fR enc

svn co http://localhost/svn/trunk/enc enc

mkdir -p outfinalnew32/Encyclopedia
mkdir -p outfinalnew64/Encyclopedia

svn export --force http://localhost/svn/trunk/docs/misc outfinalnew32/Encyclopedia
svn export --force http://localhost/svn/trunk/docs/misc outfinalnew64/Encyclopedia

pushd enc/tools
python tool.make_chm.py
cd ../build/chm

#benc2 en e
benc2 ru r

popd
