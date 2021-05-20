#!/bin/bash

function benc2 {
LNG=$1
L=$2
CP=$3

cd ${LNG} || return 1
wine "../../../tools/hh_compiler/hh_compiler.exe" ${CP} plugins${L}.hhp

( \
	cp -f FarEncyclopedia.${LNG}.chm ../../../../outfinalnew32/Encyclopedia/ && \
	cp -f FarEncyclopedia.${LNG}.chm ../../../../outfinalnew64/Encyclopedia/ \
) || return 1

cd ..

}

function blua {

mkdir $1
cd $1 || return 1

python ../../../tools/convert.py "../../../enc_lua/${1}.tsi" ${2} "${1}.tsi" windows-${3}
wine "C:/src/enc/tools/lua/lua.exe" "C:/src/enc/tools/lua/scripts/tp2hh.lua" "${1}.tsi" tsi "C:/src/enc/tools/lua/templates/api.tem"
wine "../../../tools/hh_compiler/hh_compiler.exe" ${3} ${1}.hhp

( \
	cp -f ${1}.chm ../../../../outfinalnew32/Encyclopedia/ && \
	cp -f ${1}.chm ../../../../outfinalnew64/Encyclopedia/ \
) || return 1

cd ..

}

rm -fR enc

cp -R far.git/enc ./ || exit 1

mkdir -p outfinalnew32/Encyclopedia
mkdir -p outfinalnew64/Encyclopedia

pushd enc/tools || exit 1
python tool.make_chm.py
cd ../build/chm

( \
	#benc2 en e 1252 && \
	benc2 ru r 1251 \
) || exit 1

popd

mkdir -p enc/build/lua
pushd enc/build/lua || exit 1

( \
	blua macroapi_manual.ru utf-8-sig 1251 && \
	blua macroapi_manual.en utf-8-sig 1252 && \
	blua luafar_manual      utf-8-sig 1252 \
) || exit 1

popd

#update api.farmanager.com
pushd enc/tools || exit 1
python ./tool.make_inet.py || exit 1
popd 
rm -Rf /var/www/api/* || exit 1
cp -Rf enc/build/inet/* /var/www/api/ || exit 1
