#!/bin/bash

# absolute path to this script
DIR=$(dirname $(readlink -f $0))
# absolute path to source root
ROOT=$(cd $DIR/../.. && pwd)
BUILDDIR=$ROOT/_output


function benc2 {
LNG=$1
L=$2

cd ${LNG} || return 1
wine "C:/Program Files (x86)/HTML Help Workshop/hhc.exe" plugins${L}.hhp

( \
	cp -vf FarEncyclopedia.${LNG}.chm $BUILDDIR/outfinalnew32/Encyclopedia/ && \
	cp -vf FarEncyclopedia.${LNG}.chm $BUILDDIR/outfinalnew64/Encyclopedia/ \
) || return 1

cd ..

}

function blua {

mkdir $1
cd $1 || return 1

wine "C:/src/enc/tools/lua/lua.exe" "C:/src/enc/tools/lua/scripts/tp2hh.lua" "../../../enc_lua/${1}.tsi" tsi "C:/src/enc/tools/lua/templates/api.tem" 
wine "C:/Program Files (x86)/HTML Help Workshop/hhc.exe" ${1}.hhp

( \
	cp -f ${1}.chm $BUILDDIR/outfinalnew32/Encyclopedia/ && \
	cp -f ${1}.chm $BUILDDIR/outfinalnew64/Encyclopedia/ \
) || return 1

cd ..

}


echo Building in $BUILDDIR
echo Cleaning up $BUILDDIR
rm -fR $BUILDDIR
mkdir $BUILDDIR

cp -R $ROOT/enc $BUILDDIR/ || exit 1

mkdir -p $BUILDDIR/outfinalnew32/Encyclopedia
mkdir -p $BUILDDIR/outfinalnew64/Encyclopedia

pushd $BUILDDIR/enc/tools || exit 1
python tool.make_chm.py
cd ../build/chm

( \
	#benc2 en e && \
	benc2 ru r \
) || exit 1

popd

mkdir -p $BUILDDIR/enc/build/lua
pushd $BUILDDIR/enc/build/lua || exit 1

( \
	blua macroapi_manual.ru && \
	blua macroapi_manual.en && \
	blua luafar_manual \
) || exit 1

popd

#update api.farmanager.com
pushd $BUILDDIR/enc/tools || exit 1
python ./tool.make_inet.py || exit 1
popd

#TODO this should belong to deploy script
rm -Rf /var/www/api/* || exit 1
cp -Rf $BUILDDIR/enc/build/inet/* /var/www/api/ || exit 1
