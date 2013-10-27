#!/bin/bash

function benc2 {
LNG=$1
L=$2

cd ${LNG} || return 1
wine "C:/Program Files (x86)/HTML Help Workshop/hhc.exe" plugins${L}.hhp

( \
	cp -f FarEncyclopedia.${LNG}.chm ../../../../outfinalnew32/Encyclopedia/ && \
	cp -f FarEncyclopedia.${LNG}.chm ../../../../outfinalnew64/Encyclopedia/ \
) || return 1

cd ..

}

function blua {

mkdir $1
cd $1 || return 1

wine "C:/src/enc/tools/lua/lua.exe" "C:/src/enc/tools/lua/scripts/tp2hh.lua" "../../../enc_lua/${1}.tsi" tsi "C:/src/enc/tools/lua/templates/api.tem" 
wine "C:/Program Files (x86)/HTML Help Workshop/hhc.exe" ${1}.hhp

( \
	cp -f ${1}.chm ../../../../outfinalnew32/Encyclopedia/ && \
	cp -f ${1}.chm ../../../../outfinalnew64/Encyclopedia/ \
) || return 1

cd ..

}

rm -fR enc

svn co file://`pwd`/fromgoogle/trunk/enc enc || exit 1

mkdir -p outfinalnew32/Encyclopedia
mkdir -p outfinalnew64/Encyclopedia

pushd enc/tools || exit 1
python tool.make_chm.py
cd ../build/chm

( \
	#benc2 en e && \
	benc2 ru r \
) || exit 1

popd

mkdir -p enc/build/lua
pushd enc/build/lua || exit 1

( \
	blua macroapi_manual && \
	blua luafar_manual \
) || exit 1

popd
