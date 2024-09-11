#!/bin/bash

if [ ! -e plugins.common ]; then
	exit 1
fi

#include common
. plugins.common

rm -fR plugins
rm -fR misc

( \
	cp -R far.git/plugins ./ && \
	cp -R far.git/misc ./ \
) || exit 1

cp -f far/Include/*.hpp plugins/common/unicode/

mkdir -p outfinalnew32/Plugins
mkdir -p outfinalnew64/Plugins
mkdir -p outfinalnewARM64/Plugins

#cd plugins/common/CRT || exit 1
#
#mkdir -p obj.32.vc/wide
#mkdir -p obj.64.vc/wide
#mkdir -p obj.ARM64.vc/wide
#wine cmd /c ../../../common.32.bat &> ../../../logs/CRT32
#wine cmd /c ../../../common.64.bat &> ../../../logs/CRT64
#wine cmd /c ../../../common.64.bat &> ../../../logs/CRTARM64
#
#cd ../../..

cd plugins

VERSION_7Z=24.08
curl -o arclite/7z/7z-${VERSION_7Z}.zip https://raw.githubusercontent.com/FarGroup/thirdparty/master/7z-${VERSION_7Z}.zip
unzip arclite/7z/7z-${VERSION_7Z}.zip -d arclite/7z/${VERSION_7Z}

VERSION_LUASDK=20240911
curl -o luamacro/luasdk/LuaSDK-${VERSION_LUASDK}.zip https://raw.githubusercontent.com/FarGroup/thirdparty/master/LuaSDK-${VERSION_LUASDK}.zip
unzip luamacro/luasdk/LuaSDK-${VERSION_LUASDK}.zip -d luamacro/luasdk/${VERSION_LUASDK}

MASKS="*.dll *.hlf *.lng *.farconfig *.lua *.map *.pdb"

( \
bplugin "align"      "Align"      "$MASKS" && \
bplugin "autowrap"   "AutoWrap"   "$MASKS" && \
bplugin "brackets"   "Brackets"   "$MASKS" && \
bplugin "compare"    "Compare"    "$MASKS" && \
bplugin "drawline"   "DrawLine"   "$MASKS" && \
bplugin "editcase"   "EditCase"   "$MASKS" && \
bplugin "emenu"      "EMenu"      "$MASKS" && \
bplugin "farcmds"    "FarCmds"    "$MASKS" && \
bplugin "samefolder" "SameFolder" "$MASKS" && \
bplugin "filecase"   "FileCase"   "$MASKS" && \
bplugin "hlfviewer"  "HlfViewer"  "$MASKS" && \
bplugin "network"    "Network"    "$MASKS" && \
bplugin "proclist"   "ProcList"   "$MASKS" && \
bplugin "tmppanel"   "TmpPanel"   "$MASKS *.temp" && \
bplugin "arclite"    "ArcLite"    "$MASKS *.sfx *.xml" && \
bplugin "luamacro"   "LuaMacro"   "$MASKS *.ini" \

) || exit 1

cd ..
