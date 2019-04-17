#!/bin/bash

function buildfar2 {

OUTDIR=Release.$1.vc
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

m4 -P -DFARBIT=$DIRBIT -DHOSTTYPE=Unix farlang.templ.m4 > ${BOOTSTRAPDIR}farlang.templ
m4 -P -DFARBIT=$DIRBIT -DHOSTTYPE=Unix far.rc.inc.m4 > ${BOOTSTRAPDIR}far.rc.inc
m4 -P -DFARBIT=$DIRBIT -DHOSTTYPE=Unix Far.exe.manifest.m4 > ${BOOTSTRAPDIR}Far.exe.manifest
m4 -P -DFARBIT=$DIRBIT -DHOSTTYPE=Unix farversion.inc.m4 > ${BOOTSTRAPDIR}farversion.inc
m4 -P -DFARBIT=$DIRBIT -DHOSTTYPE=Unix copyright.inc.m4 > ${BOOTSTRAPDIR}copyright.inc

m4 -P -DFARBIT=$DIRBIT -DHOSTTYPE=Unix File_id.diz.m4 | unix2dos -m > $OUTDIR/File_id.diz
dos2unix FarEng.hlf.m4
dos2unix FarRus.hlf.m4
dos2unix FarHun.hlf.m4
gawk -f ./scripts/mkhlf.awk FarEng.hlf.m4 | m4 -P -DFARBIT=$DIRBIT -DHOSTTYPE=Unix | unix2dos -m > $OUTDIR/FarEng.hlf
gawk -f ./scripts/mkhlf.awk FarRus.hlf.m4 | m4 -P -DFARBIT=$DIRBIT -DHOSTTYPE=Unix | unix2dos -m > $OUTDIR/FarRus.hlf
gawk -f ./scripts/mkhlf.awk FarHun.hlf.m4 | m4 -P -DFARBIT=$DIRBIT -DHOSTTYPE=Unix | unix2dos -m > $OUTDIR/FarHun.hlf

wine tools/lng.generator.exe -nc -ol $OUTDIR ${BOOTSTRAPDIR}farlang.templ

wine cmd /c ../mysetnew.${DIRBIT}.bat

cd ..

( \
cp $2/$OUTDIR/File_id.diz $2/$OUTDIR/Far.exe $2/$OUTDIR/*.hlf $2/$OUTDIR/Far.map $2/$OUTDIR/Far.pdb $2/$OUTDIR/*.lng $BINDIR/ && \
cp $2/Include/*.hpp $BINDIR/PluginSDK/Headers.c/ && \
cp $2/Include/*.pas $BINDIR/PluginSDK/Headers.pas/ && \
cp -f $2/changelog $BINDIR/ && \
cp -f $2/changelog_eng $BINDIR/ && \
cp -f $2/Far.exe.example.ini $BINDIR/ \
) || return 1

return 0
}

function buildfar {
cd $1 || return 1
mkdir -p Include
dos2unix farcolor.hpp
dos2unix plugin.hpp
dos2unix DlgBuilder.hpp
m4 -P -DFARBIT=32 -DHOSTTYPE=Unix -DINPUT=farcolor.hpp headers.m4 | unix2dos > Include/farcolor.hpp
m4 -P -DFARBIT=32 -DHOSTTYPE=Unix -DINPUT=plugin.hpp headers.m4 | unix2dos > Include/plugin.hpp
m4 -P -DFARBIT=32 -DHOSTTYPE=Unix -DINPUT=DlgBuilder.hpp headers.m4 | unix2dos > Include/DlgBuilder.hpp

export BOOTSTRAPDIR=bootstrap/
mkdir -p ${BOOTSTRAPDIR}
dos2unix PluginW.pas
dos2unix FarColorW.pas
m4 -P -DFARBIT=32 -DHOSTTYPE=Unix -DINPUT=PluginW.pas headers.m4 | unix2dos > Include/PluginW.pas
m4 -P -DFARBIT=32 -DHOSTTYPE=Unix -DINPUT=FarColorW.pas headers.m4 | unix2dos > Include/FarColorW.pas

unix2dos -m changelog
unix2dos -m changelog_eng
unix2dos Far.exe.example.ini

ls *.cpp *.hpp *.c *.rc | gawk -f ./scripts/mkdep.awk - | unix2dos > ${BOOTSTRAPDIR}far.vc.dep
cd ..

(buildfar2 32 $1 && buildfar2 64 $1) || return 1

return 0
}

rm -fR far
( \
	cp -R far.git/far ./ && \
	buildfar far \
) || exit 1


