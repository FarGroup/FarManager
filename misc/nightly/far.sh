#!/bin/sh

function buildfar2 {

OUTDIR=Release.$1.vc
DIRBIT=$1
BINDIR=outfinalnew${DIRBIT}

rm -fR $BINDIR

mkdir $BINDIR
mkdir -p $BINDIR/PluginSDK/Headers.c
mkdir -p $BINDIR/PluginSDK/Headers.pas

cd $2

mkdir -p $OUTDIR
mkdir -p $OUTDIR/obj
mkdir -p $OUTDIR/cod

m4 -P -DFARBIT=$DIRBIT -DHOSTTYPE=Unix farlang.templ.m4 > ${BOOTSTRAP}farlang.templ
m4 -P -DFARBIT=$DIRBIT -DHOSTTYPE=Unix far.rc.m4 > ${BOOTSTRAP}far.rc
m4 -P -DFARBIT=$DIRBIT -DHOSTTYPE=Unix Far.exe.manifest.m4 > ${BOOTSTRAP}Far.exe.manifest
m4 -P -DFARBIT=$DIRBIT -DHOSTTYPE=Unix farversion.inc.m4 > ${BOOTSTRAP}farversion.inc
m4 -P -DFARBIT=$DIRBIT -DHOSTTYPE=Unix copyright.inc.m4 > ${BOOTSTRAP}copyright.inc

m4 -P -DFARBIT=$DIRBIT -DHOSTTYPE=Unix File_id.diz.m4 > $OUTDIR/File_id.diz
dos2unix FarEng.hlf.m4
dos2unix FarRus.hlf.m4
dos2unix FarHun.hlf.m4
gawk -f ./scripts/mkhlf.awk FarEng.hlf.m4 | m4 -P -DFARBIT=$DIRBIT -DHOSTTYPE=Unix | unix2dos > $OUTDIR/FarEng.hlf
gawk -f ./scripts/mkhlf.awk FarRus.hlf.m4 | m4 -P -DFARBIT=$DIRBIT -DHOSTTYPE=Unix | unix2dos > $OUTDIR/FarRus.hlf
gawk -f ./scripts/mkhlf.awk FarHun.hlf.m4 | m4 -P -DFARBIT=$DIRBIT -DHOSTTYPE=Unix | unix2dos > $OUTDIR/FarHun.hlf

wine tools/lng.generator.exe -nc -ol $OUTDIR ${BOOTSTRAP}farlang.templ

wine cmd /c ../mysetnew.${DIRBIT}.bat

cd ..

cp $2/$OUTDIR/File_id.diz $2/$OUTDIR/Far.exe $2/$OUTDIR/*.hlf $2/$OUTDIR/Far.map $2/$OUTDIR/*.lng $BINDIR/
cp $2/Include/*.hpp $BINDIR/PluginSDK/Headers.c/
cp $2/Include/*.pas $BINDIR/PluginSDK/Headers.pas/
cp -f $2/changelog $BINDIR/
cp -f $2/changelog_eng $BINDIR/
}

function buildfar {
cd $1
mkdir -p Include
dos2unix colors.hpp
dos2unix plugin.hpp
dos2unix DlgBuilder.hpp
m4 -P -DFARBIT=32 -DHOSTTYPE=Unix -DINPUT=colors.hpp headers.m4 | unix2dos > Include/farcolor.hpp
m4 -P -DFARBIT=32 -DHOSTTYPE=Unix -DINPUT=plugin.hpp headers.m4 | unix2dos > Include/plugin.hpp
m4 -P -DFARBIT=32 -DHOSTTYPE=Unix -DINPUT=DlgBuilder.hpp headers.m4 | unix2dos > Include/DlgBuilder.hpp

BOOTSTRAP=bootstrap/
mkdir -p bootstrap
dos2unix PluginW.pas
dos2unix FarColorW.pas
dos2unix FarKeysW.pas 
m4 -P -DFARBIT=32 -DHOSTTYPE=Unix -DINPUT=PluginW.pas headers.m4 | unix2dos > Include/PluginW.pas
m4 -P -DFARBIT=32 -DHOSTTYPE=Unix -DINPUT=FarColorW.pas headers.m4 | unix2dos > Include/FarColorW.pas

dos2unix mkdep.list
gawk -f ./scripts/mkdep.awk mkdep.list | unix2dos > ${BOOTSTRAP}far.vc.dep
cd ..

buildfar2 32 $1
buildfar2 64 $1
}

rm -fR unicode_far
svn export http://localhost/svn/trunk/unicode_far unicode_far

buildfar unicode_far
