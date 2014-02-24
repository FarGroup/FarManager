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

mkdir -p execdump/final.32.vc/obj/LibObj
#mkdir -p execdump/final.64.vc/obj/LibObj

FILES32="demangle32.dll ExcDump.dll FExcept.dll SetFarExceptionHandler.farconfig"
#FILES64="demangle64.dll ExcDump.dll FExcept.dll SetFarExceptionHandler.farconfig"

( \
	bplugin2 "$PLDIR" 32 1 "$FILES32" \
#	bplugin2 "$PLDIR" 64 1 "$FILES64" \
) || return 1

popd
}

rm -fR plugins
rm -fR misc

( \
	svn co file://`pwd`/fromgoogle/trunk/plugins plugins && \
	svn co file://`pwd`/fromgoogle/trunk/misc misc \
) || exit 1

cp -f unicode_far/Include/*.hpp plugins/common/unicode/

mkdir -p outfinalnew32/Plugins
mkdir -p outfinalnew64/Plugins

cd plugins/common/CRT || exit 1

mkdir -p obj.32.vc/wide
mkdir -p obj.64.vc/wide
wine cmd /c ../../../common.32.bat &> ../../../logs/CRT32
wine cmd /c ../../../common.64.bat &> ../../../logs/CRT64

cd ../..

( \
bplugin "align" "Align" "Align.dll AlignEng.lng AlignRus.lng AlignSky.lng AlignSpa.lng Align.map" && \
bplugin "autowrap" "AutoWrap" "AutoWrap.dll WrapEng.lng WrapRus.lng WrapSky.lng WrapSpa.lng AutoWrap.map" && \
bplugin "brackets" "Brackets" "Brackets.dll BrackEng.hlf BrackRus.hlf BrackEng.lng BrackRus.lng BrackSky.lng BrackSpa.lng Brackets.lua Brackets.map" && \
bplugin "compare" "Compare" "Compare.dll CmpEng.hlf CmpRus.hlf CompEng.lng CompRus.lng CompSky.lng CompSpa.lng Compare.map" && \
bplugin "drawline" "DrawLine" "DrawLine.dll DrawEng.hlf DrawRus.hlf DrawEng.lng DrawRus.lng DrawSky.lng DrawSpa.lng DrawLine.map" && \
bplugin "editcase" "EditCase" "EditCase.dll ECaseEng.hlf ECaseRus.hlf ECaseEng.lng ECaseRus.lng ECaseSky.lng ECaseSpa.lng EditCase.map" && \
bplugin "emenu" "EMenu" "EMenu.dll EMenuEng.hlf EMenuRus.hlf EMenuEng.lng EMenuRus.lng EMenuSky.lng EMenuSpa.lng EMenu.map HotkeyClipboard.lua HotkeyProperties.lua Hotkey.lua Hotkey.farconfig" && \
bplugin "farcmds" "FarCmds" "FARCmds.dll FARCmdsEng.hlf FARCmdsRus.hlf FARCmdsEng.lng FARCmdsRus.lng FARCmdsSky.lng FARCmdsSpa.lng FARCmds.map" && \
bplugin "filecase" "FileCase" "FileCase.dll CaseEng.hlf CaseRus.hlf CaseEng.lng CaseRus.lng CaseSpa.lng FileCase.map" && \
bplugin "hlfviewer" "HlfViewer" "HlfViewer.dll HlfViewerEng.hlf HlfViewerRus.hlf HlfViewerEng.lng HlfViewerRus.lng HlfViewerSky.lng HlfViewerSpa.lng HlfViewer.map" && \
bplugin "network" "Network" "Network.dll NetEng.hlf NetRus.hlf NetEng.lng NetRus.lng NetSky.lng NetSpa.lng Network.map" && \
bplugin "proclist" "ProcList" "Proclist.dll ProcEng.hlf ProcRus.hlf ProcEng.lng ProcRus.lng Proclist.map" && \
bplugin "tmppanel" "TmpPanel" "TmpPanel.dll TmpEng.hlf TmpRus.hlf TmpEng.lng TmpRus.lng TmpSpa.lng TmpPanel.map shortcuts.eng.lua shortcuts.rus.lua disks.eng.temp disks.rus.temp shortcuts.eng.temp shortcuts.rus.temp" && \
bplugin "arclite" "ArcLite" "7z.dll 7z.sfx 7zCon.sfx 7zS2.sfx 7zS2con.sfx 7zSD.sfx arclite.dll arclite.map arclite_eng.hlf arclite_eng.lng arclite_rus.hlf arclite_rus.lng arclite_spa.lng" && \
bplugin "luamacro" "LuaMacro" "LuaMacro.dll _globalinfo.lua api.lua luamacro.lua macrotest.lua utils.lua mbrowser.lua lang.lua panelsort.lua winapi.lua farapi.lua lm_eng.lng lm_rus.lng lm_spa.lng LuaMacro.map" \
) || exit 1

( \
	cd ../misc && \
	bpluginfe && \
	cd .. \
) || exit 1

cd ..

rm -f outfinalnew32/luafar3.exp outfinalnew32/luafar3.lib outfinalnew32/luafar3.pdb outfinalnew64/luafar3.exp outfinalnew64/luafar3.lib outfinalnew64/luafar3.pdb
