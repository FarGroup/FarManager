#!/bin/bash

if [ ! -e plugins.common ]; then
	exit 1
fi

#include common
. plugins.common

function bpluginfe {
PLUGIN=fexcept
PLDIR=FExcept

pushd $PLUGIN || return 1

unix2dos changelog

mkdir -p execdump/final.32.vc/obj/LibObj
#mkdir -p execdump/final.64.vc/obj/LibObj

MASKS_FEXCEPT="*.dll *.farconfig"

( \
	bplugin2 "$PLDIR" 32 1 1 "$MASKS_FEXCEPT" \
#	bplugin2 "$PLDIR" 64 1 1 "$MASKS_FEXCEPT" \
) || return 1

popd
}

rm -fR plugins
rm -fR misc

( \
	cp -R far.git/plugins ./ && \
	cp -R far.git/misc ./ \
) || exit 1

cp -f far/Include/*.hpp plugins/common/unicode/

mkdir -p outfinalnew32/Plugins
mkdir -p outfinalnew64/Plugins

cd plugins/common/CRT || exit 1

mkdir -p obj.32.vc/wide
mkdir -p obj.64.vc/wide
wine cmd /c ../../../common.32.bat &> ../../../logs/CRT32
wine cmd /c ../../../common.64.bat &> ../../../logs/CRT64

cd ../..

MASKS="*.dll *.hlf *.lng *.farconfig *.lua *.map"

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

( \
	cd ../misc && \
	bpluginfe && \
	cd .. \
) || exit 1

cd ..

rm -f outfinalnew32/luafar3.exp outfinalnew32/luafar3.lib outfinalnew32/luafar3.pdb outfinalnew64/luafar3.exp outfinalnew64/luafar3.lib outfinalnew64/luafar3.pdb
