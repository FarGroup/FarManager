#!/bin/sh

function buildfar2 {

OUTDIR=Release.$1.vc
DIRBIT=$1
BINDIR=outfinal${3}${DIRBIT}

rm -fR $BINDIR

mkdir $BINDIR
mkdir -p $BINDIR/PluginSDK/Headers.c
mkdir -p $BINDIR/PluginSDK/Headers.pas

cd $2

mkdir -p $OUTDIR
mkdir -p $OUTDIR/obj
mkdir -p $OUTDIR/cod

m4 -P -DFARBIT=$DIRBIT farlang.templ.m4 > farlang.templ
m4 -P -DFARBIT=$DIRBIT far.rc.m4 > far.rc
m4 -P -DFARBIT=$DIRBIT Far.exe.manifest.m4 > Far.exe.manifest
m4 -P -DFARBIT=$DIRBIT farversion.inc.m4 > farversion.inc
m4 -P -DFARBIT=$DIRBIT copyright.inc.m4 | gawk -f ./scripts/enc.awk > copyright.inc

m4 -P -DFARBIT=$DIRBIT File_id.diz.m4 > $OUTDIR/File_id.diz
dos2unix FarEng.hlf.m4
dos2unix FarRus.hlf.m4
gawk -f ./scripts/mkhlf.awk FarEng.hlf.m4 | m4 -P -DFARBIT=$DIRBIT | unix2dos > $OUTDIR/FarEng.hlf
gawk -f ./scripts/mkhlf.awk FarRus.hlf.m4 | m4 -P -DFARBIT=$DIRBIT | unix2dos > $OUTDIR/FarRus.hlf
if [ $4 -eq 1 ]; then
  gawk -f ./scripts/mkhlf.awk FarHun.hlf.m4 | m4 -P -DFARBIT=$DIRBIT | unix2dos > $OUTDIR/FarHun.hlf
fi

wine tools/lng.generator.exe -nc -ol $OUTDIR farlang.templ

wine cmd /c ../myset${3}.${DIRBIT}.bat

cd ..

cp $2/$OUTDIR/File_id.diz $2/$OUTDIR/Far.exe $2/$OUTDIR/*.hlf $2/$OUTDIR/far.map $2/$OUTDIR/*.lng $BINDIR/
cp $2/Include/*.hpp $BINDIR/PluginSDK/Headers.c/
cp $2/Include/*.pas $BINDIR/PluginSDK/Headers.pas/

if [ $4 -eq 0 ]; then
  gawk -f changelog2.awk $2/changelog > $BINDIR/changelog
else
  cp -f $2/changelog $BINDIR/
fi
}

function buildfar {
cp -f tools.m4 $1/

cd $1
mkdir -p Include
dos2unix colors.hpp
dos2unix keys.hpp
dos2unix plugin.hpp
m4 -P -DFARBIT=32 -DINPUT=colors.hpp headers.m4 | unix2dos > Include/farcolor.hpp
m4 -P -DFARBIT=32 -DINPUT=keys.hpp headers.m4 | unix2dos > Include/farkeys.hpp
m4 -P -DFARBIT=32 -DINPUT=plugin.hpp headers.m4 | unix2dos > Include/plugin.hpp

if [ $3 -eq 0 ]; then
  dos2unix farcolor.pas
  dos2unix farkeys.pas
  dos2unix plugin.pas
  m4 -P -DFARBIT=32 -DINPUT=farcolor.pas headers.m4 | unix2dos > Include/farcolor.pas
  m4 -P -DFARBIT=32 -DINPUT=farkeys.pas headers.m4 | unix2dos > Include/farkeys.pas
  m4 -P -DFARBIT=32 -DINPUT=plugin.pas headers.m4 | unix2dos > Include/plugin.pas
else
  dos2unix PluginW.pas
  dos2unix FarColorW.pas
  dos2unix FarKeysW.pas 
  m4 -P -DFARBIT=32 -DINPUT=PluginW.pas headers.m4 | unix2dos > Include/PluginW.pas
  m4 -P -DFARBIT=32 -DINPUT=FarColorW.pas headers.m4 | unix2dos > Include/FarColorW.pas
  m4 -P -DFARBIT=32 -DINPUT=FarKeysW.pas headers.m4 | unix2dos > Include/FarKeysW.pas
fi

cd ..

cd $1
gawk -f ./scripts/mkdep.awk mkdep.list > far.vc.dep
cd ..

buildfar2 32 $1 $2 $3
buildfar2 64 $1 $2 $3
}

rm -fR far
svn export http://localhost/svn/trunk/unicode_far far

buildfar far new 1

rm -fR farold
svn export file:///svnroot/far/branches/171 farold

buildfar farold old 0
