#!/bin/bash

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

function blua {

mkdir $1
cd $1

wine "C:/src/enc/tools/lua/lua.exe" "C:/src/enc/tools/lua/scripts/tp2hh.lua" "../../../enc_lua/${1}.tsi" tsi "C:/src/enc/tools/lua/templates/api.tem" 
wine "C:/Program Files/HTML Help Workshop/hhc.exe" ${1}.hhp

cp -f ${1}.chm ../../../../outfinalnew32/Encyclopedia/ && \
cp -f ${1}.chm ../../../../outfinalnew64/Encyclopedia/
if [ "$?" != "0" ]; then exit 1
fi

cd ..

}

rm -fR enc

svn co http://localhost/svn/trunk/enc enc

mkdir -p outfinalnew32/Encyclopedia
mkdir -p outfinalnew64/Encyclopedia

pushd enc/tools
python tool.make_chm.py
cd ../build/chm

#benc2 en e
benc2 ru r

popd

mkdir -p enc/build/lua
pushd enc/build/lua

blua macroapi_manual
blua luafar_manual

popd
