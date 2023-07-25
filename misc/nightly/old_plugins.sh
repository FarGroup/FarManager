#!/bin/bash

if [ ! -e plugins.common ]; then
	exit 1
fi

#include common
. plugins.common

function bpluginftp {
PLUGIN=ftp
PLDIR=FTP

pushd $PLUGIN || return 1

unix2dos changelog

FILES="FarFtp.dll FtpEng.hlf FtpRus.hlf FtpPol.hlf FtpGer.hlf FtpEng.lng FtpRus.lng FtpIta.lng FtpPol.lng FtpGer.lng FarFtp.map TechInfo.reg TechInfo_rus.reg FtpCmds.txt FtpCmds_rus.txt Notes.txt Notes_rus.txt"
ADDFILES="ftpDirList.fll ftpProgress.fll ftpDirList.map ftpProgress.map Progress_FarCopy.reg Progress_JM.reg Progress_Wesha.reg Progress_ZeMA.reg"
ADDBUILDDIRS="{lib,obj/DirList,obj/LibObj,obj/Notify,obj/Progress}"
ADDOUTDIR="lib"

( \
	bplugin2 "$PLDIR" 32 0 "$FILES" "$ADDFILES" $ADDBUILDDIRS $ADDOUTDIR && \
	bplugin2 "$PLDIR" 64 0 "$FILES" "$ADDFILES" $ADDBUILDDIRS $ADDOUTDIR && \
	bplugin2 "$PLDIR" ARM64 0 "$FILES" "$ADDFILES" $ADDBUILDDIRS $ADDOUTDIR \
) || return 1

popd
}

function bpluginma {
PLUGIN=multiarc
PLDIR=MultiArc

pushd $PLUGIN || return 1

unix2dos changelog

FILES="MultiArc.dll arceng.hlf arcrus.hlf arcger.hlf arceng.lng arcrus.lng arcspa.lng arcita.lng arcbel.lng arcger.lng MultiArc.map"
ADDFILES="ace.fmt arc.fmt arj.fmt cab.fmt custom.fmt ha.fmt lzh.fmt rar.fmt targz.fmt zip.fmt custom.ini ace.map arc.map arj.map cab.map custom.map ha.map lzh.map rar.map targz.map zip.map"
ADDBUILDDIRS="Formats"
ADDOUTDIR="Formats"

( \
	bplugin2 "$PLDIR" 32 0 "$FILES" "$ADDFILES" $ADDBUILDDIRS $ADDOUTDIR && \
	bplugin2 "$PLDIR" 64 0 "$FILES" "$ADDFILES" $ADDBUILDDIRS $ADDOUTDIR && \
	bplugin2 "$PLDIR" ARM64 0 "$FILES" "$ADDFILES" $ADDBUILDDIRS $ADDOUTDIR \
) || return 1

popd
}

rm -fR outfinalnew32
rm -fR outfinalnew64
rm -fR outfinalnewARM64
rm -fR plugins

cp -R far.git/plugins ./ || exit 1

mkdir -p outfinalnew32/Plugins
mkdir -p outfinalnew64/Plugins
mkdir -p outfinalnewARM64/Plugins

cd plugins/common/CRT || exit 1

mkdir -p obj.32.vc/wide
mkdir -p obj.64.vc/wide
mkdir -p obj.ARM64.vc/wide
wine cmd /c ../../../common.32.bat &> ../../../logs/CRT32
wine cmd /c ../../../common.64.bat &> ../../../logs/CRT64
wine cmd /c ../../../common.ARM64.bat &> ../../../logs/CRTARM64

cd ../..

( \
#bplugin "macroview" "MacroView" "MacroView.dll MacroEng.hlf MacroRus.hlf MacroEng.lng MacroRus.lng MacroView.map" && \
bpluginftp && \
bpluginma \
) || exit 1

cd ..

cd outfinalnew32/Plugins || exit 1
cd FTP || exit 1
7z a -m0=LZMA -mf=BCJ2 -mx9 ../../../FarFtp.x86.7z
cd ../MultiArc || exit 1
7z a -m0=LZMA -mf=BCJ2 -mx9 ../../../MultiArc.x86.7z
cd ../../../
cd outfinalnew64/Plugins || exit 1
cd FTP || exit 1
7z a -m0=LZMA -mf=BCJ2 -mx9 ../../../FarFtp.x64.7z
cd ../MultiArc || exit 1
7z a -m0=LZMA -mf=BCJ2 -mx9 ../../../MultiArc.x64.7z
cd ../../../
cd outfinalnewARM64/Plugins || exit 1
cd FTP || exit 1
7z a -m0=LZMA -mf=BCJ2 -mx9 ../../../FarFtp.ARM64.7z
cd ../MultiArc || exit 1
7z a -m0=LZMA -mf=BCJ2 -mx9 ../../../MultiArc.ARM64.7z
cd ../../../
