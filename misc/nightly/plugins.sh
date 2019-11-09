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

( \
bplugin "align" "Align" "Align.dll AlignEng.lng AlignRus.lng AlignSky.lng AlignSpa.lng AlignIta.lng AlignPol.lng AlignBel.lng Align.map" && \
bplugin "autowrap" "AutoWrap" "AutoWrap.dll WrapEng.lng WrapRus.lng WrapSky.lng WrapSpa.lng WrapIta.lng WrapPol.lng WrapBel.lng AutoWrap.map" && \
bplugin "brackets" "Brackets" "Brackets.dll BrackEng.hlf BrackRus.hlf BrackPol.hlf BrackEng.lng BrackRus.lng BrackSky.lng BrackSpa.lng BrackIta.lng BrackPol.lng BrackBel.lng Brackets.lua Brackets.map" && \
bplugin "compare" "Compare" "Compare.dll CmpEng.hlf CmpRus.hlf CmpPol.hlf CompEng.lng CompRus.lng CompSky.lng CompSpa.lng CompIta.lng CompPol.lng CompBel.lng Compare.map AdvCompare.CompareBufferSize.farconfig" && \
bplugin "drawline" "DrawLine" "DrawLine.dll DrawEng.hlf DrawRus.hlf DrawPol.hlf DrawEng.lng DrawRus.lng DrawSky.lng DrawSpa.lng DrawIta.lng DrawPol.lng DrawBel.lng DrawLine.map" && \
bplugin "editcase" "EditCase" "EditCase.dll ECaseEng.hlf ECaseRus.hlf ECasePol.hlf ECaseEng.lng ECaseRus.lng ECaseSky.lng ECaseSpa.lng ECaseIta.lng ECasePol.lng ECaseBel.lng EditCase.map" && \
bplugin "emenu" "EMenu" "EMenu.dll EMenuEng.hlf EMenuRus.hlf EMenuPol.hlf EMenuEng.lng EMenuRus.lng EMenuSky.lng EMenuSpa.lng EMenuIta.lng EMenuPol.lng EMenuBel.lng EMenu.map HotkeyClipboard.lua HotkeyProperties.lua Hotkey.lua Hotkey.farconfig" && \
bplugin "farcmds" "FarCmds" "FarCmds.dll FARCmdsEng.hlf FARCmdsRus.hlf FARCmdsPol.hlf FARCmdsEng.lng FARCmdsRus.lng FARCmdsSky.lng FARCmdsSpa.lng FARCmdsIta.lng FARCmdsBel.lng FARCmdsPol.lng FarCmds.map" && \
bplugin "samefolder" "SameFolder" "SameFolder.dll SameFolderEng.hlf SameFolderRus.hlf SameFolderPol.hlf SameFolderEng.lng SameFolderRus.lng SameFolderSky.lng SameFolderSpa.lng SameFolderIta.lng SameFolderPol.lng SameFolderBel.lng SameFolder.map" && \
bplugin "filecase" "FileCase" "FileCase.dll CaseEng.hlf CaseRus.hlf CasePol.hlf CaseEng.lng CaseRus.lng CaseSky.lng CaseSpa.lng CaseIta.lng CasePol.lng CaseBel.lng FileCase.map" && \
bplugin "hlfviewer" "HlfViewer" "HlfViewer.dll HlfViewerEng.hlf HlfViewerRus.hlf HlfViewerPol.hlf HlfViewerEng.lng HlfViewerRus.lng HlfViewerSky.lng HlfViewerSpa.lng HlfViewerIta.lng HlfViewerPol.lng HlfViewerBel.lng HlfViewer.map" && \
bplugin "network" "Network" "Network.dll NetEng.hlf NetRus.hlf NetPol.hlf NetEng.lng NetRus.lng NetSky.lng NetSpa.lng NetIta.lng NetBel.lng NetPol.lng Network.map" && \
bplugin "proclist" "ProcList" "ProcList.dll ProcEng.hlf ProcRus.hlf ProcPol.hlf ProcEng.lng ProcRus.lng ProcIta.lng ProcBel.lng ProcPol.lng ProcList.map" && \
bplugin "tmppanel" "TmpPanel" "TmpPanel.dll TmpEng.hlf TmpRus.hlf TmpPol.hlf TmpEng.lng TmpRus.lng TmpSky.lng TmpSpa.lng TmpIta.lng TmpPol.lng TmpBel.lng TmpPanel.map shortcuts.eng.lua shortcuts.rus.lua shortcuts.pol.lua shortcuts.bel.lua disks.eng.temp disks.rus.temp disks.pol.temp disks.bel.temp shortcuts.eng.temp shortcuts.rus.temp shortcuts.bel.temp TmpPanel.ListUTF8.farconfig" && \
bplugin "arclite" "ArcLite" "7z.dll 7z.sfx 7zCon.sfx 7zS2.sfx 7zS2con.sfx 7zSD.sfx arclite.dll arclite.map arclite_eng.hlf arclite_eng.lng arclite_rus.hlf arclite_rus.lng arclite_spa.lng arclite_ita.lng arclite_pol.hlf arclite_pol.lng arclite_bel.lng arclite.xml" && \
bplugin "luamacro" "LuaMacro" "LuaMacro.dll _globalinfo.lua api.lua luamacro.lua macrotest.lua utils.lua mbrowser.lua lang.lua panelsort.lua winapi.lua farapi.lua moonscript.lua keymacro.lua lm_eng.lng lm_rus.lng lm_sky.lng lm_spa.lng lm_ita.lng lm_bel.lng lm_pol.lng LuaMacro.map luamacro.example.ini" \
) || exit 1

( \
	cd ../misc && \
	bpluginfe && \
	cd .. \
) || exit 1

cd ..

rm -f outfinalnew32/luafar3.exp outfinalnew32/luafar3.lib outfinalnew32/luafar3.pdb outfinalnew64/luafar3.exp outfinalnew64/luafar3.lib outfinalnew64/luafar3.pdb
