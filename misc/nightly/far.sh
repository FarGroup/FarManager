#!/bin/bash

M4CMD="m4 -P -DHOSTTYPE=Unix -DBUILD_TYPE=VS_RELEASE -DSCM_REVISION="

function buildfar2 {

OUTDIR=Release.$1.vc
export BOOTSTRAPDIR=$OUTDIR/obj/include/bootstrap/
DIRBIT=$1
BINDIR=outfinalnew${DIRBIT}

rm -fR $BINDIR

mkdir $BINDIR
mkdir -p $BINDIR/PluginSDK/Headers.c
mkdir -p $BINDIR/PluginSDK/Headers.pas

cd $2 || return 1

mkdir -p $OUTDIR
mkdir -p $OUTDIR/obj
mkdir -p $OUTDIR/cod
mkdir -p ${BOOTSTRAPDIR}

ls *.cpp *.hpp *.c *.rc | gawk -f ./scripts/mkdep.awk - | unix2dos > ${BOOTSTRAPDIR}far.dep

M4CMDP="$M4CMD -DFARBIT=$DIRBIT"

$M4CMDP farlang.templ.m4 > ${BOOTSTRAPDIR}farlang.templ
$M4CMDP far.rc.inc.m4 > ${BOOTSTRAPDIR}far.rc.inc
$M4CMDP Far.exe.manifest.m4 > ${BOOTSTRAPDIR}Far.exe.manifest
$M4CMDP farversion.inc.m4 > ${BOOTSTRAPDIR}farversion.inc
pushd ../far.git/far
$M4CMDP copyright.inc.m4 > ../../far/${BOOTSTRAPDIR}copyright.inc
popd
$M4CMDP File_id.diz.m4 | unix2dos -m > $OUTDIR/File_id.diz

dos2unix FarEng.hlf.m4
dos2unix FarRus.hlf.m4
dos2unix FarHun.hlf.m4
dos2unix FarPol.hlf.m4
dos2unix FarGer.hlf.m4
dos2unix FarUkr.hlf.m4
dos2unix FarCze.hlf.m4
dos2unix FarSky.hlf.m4
gawk -f ./scripts/mkhlf.awk FarEng.hlf.m4 | $M4CMDP | unix2dos -m > $OUTDIR/FarEng.hlf
gawk -f ./scripts/mkhlf.awk FarRus.hlf.m4 | $M4CMDP | unix2dos -m > $OUTDIR/FarRus.hlf
gawk -f ./scripts/mkhlf.awk FarHun.hlf.m4 | $M4CMDP | unix2dos -m > $OUTDIR/FarHun.hlf
gawk -f ./scripts/mkhlf.awk FarPol.hlf.m4 | $M4CMDP | unix2dos -m > $OUTDIR/FarPol.hlf
gawk -f ./scripts/mkhlf.awk FarGer.hlf.m4 | $M4CMDP | unix2dos -m > $OUTDIR/FarGer.hlf
gawk -f ./scripts/mkhlf.awk FarUkr.hlf.m4 | $M4CMDP | unix2dos -m > $OUTDIR/FarUkr.hlf
gawk -f ./scripts/mkhlf.awk FarCze.hlf.m4 | $M4CMDP | unix2dos -m > $OUTDIR/FarCze.hlf
gawk -f ./scripts/mkhlf.awk FarSky.hlf.m4 | $M4CMDP | unix2dos -m > $OUTDIR/FarSky.hlf

gawk -f ./scripts/sqlite_version.awk -v target=${BOOTSTRAPDIR}sqlite_version.h thirdparty/sqlite/sqlite3.h

wine tools/lng.generator.exe -nc -oh ${BOOTSTRAPDIR} -ol $OUTDIR ${BOOTSTRAPDIR}farlang.templ

wine cmd /c ../mysetnew.${DIRBIT}.bat

cd ..

( \
cp $2/$OUTDIR/File_id.diz $2/$OUTDIR/Far.exe $2/$OUTDIR/*.hlf $2/$OUTDIR/Far.map $2/$OUTDIR/Far.pdb $2/$OUTDIR/*.lng $BINDIR/ && \
cp $2/$OUTDIR/sqlite3.dll $2/$OUTDIR/sqlite3.map $2/$OUTDIR/sqlite3.pdb $BINDIR/ && \
cp $2/Include/*.hpp $BINDIR/PluginSDK/Headers.c/ && \
cp $2/../far.git/plugins/common/unicode/DlgBuilder.hpp $BINDIR/PluginSDK/Headers.c/ && \
cp $2/Include/*.pas $BINDIR/PluginSDK/Headers.pas/ && \
cp -f $2/changelog $BINDIR/ && \
cp -f $2/Far.exe.example.ini $BINDIR/ \
) || return 1

return 0
}

function buildfar {
cd $1 || return 1
mkdir -p Include
dos2unix farcolor.hpp
dos2unix plugin.hpp
$M4CMD -DINPUT=farcolor.hpp headers.m4 | unix2dos > Include/farcolor.hpp
$M4CMD -DINPUT=plugin.hpp headers.m4 | unix2dos > Include/plugin.hpp

dos2unix PluginW.pas
dos2unix FarColorW.pas
$M4CMD -DINPUT=PluginW.pas headers.m4 | unix2dos > Include/PluginW.pas
$M4CMD -DINPUT=FarColorW.pas headers.m4 | unix2dos > Include/FarColorW.pas

unix2dos -m changelog
unix2dos Far.exe.example.ini

cd ..

(buildfar2 32 $1 && buildfar2 64 $1 && buildfar2 ARM64 $1) || return 1

return 0
}

rm -fR far
rm -fR _build
( \
	cp -R far.git/far ./ && \
	cp -R far.git/_build ./ && \
	buildfar far \
) || exit 1


