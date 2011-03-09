#!/bin/sh

function bplugin2 {
PLUGIN=$1
BIT=$2
WIDE=$3
FILES=$4

if [ $WIDE -eq 1 ]; then
  FINAL=final.${BIT}W.vc
else
  FINAL=final.${BIT}.vc
fi

ADD=0
if [ "$PLUGIN" == "FTP" ]; then
  ADD=1
elif [ "$PLUGIN" == "MultiArc" ]; then
  ADD=1
fi

mkdir -p ${FINAL}/obj
if [ $ADD -eq 1 ]; then
  eval "mkdir -p ${FINAL}/$6"
fi

wine cmd /c ../../plugin.${BIT}.bat &> ../../log${PLUGIN}${BIT}

if [ "$PLUGIN" == "FExcept" ]; then
  mkdir -p ../../outfinalnew${BIT}/FExcept/
  cp -f changelog ../../outfinalnew${BIT}/FExcept/
  cd $FINAL
  cp -f $FILES ../../../outfinalnew${BIT}/FExcept/
  cd ..
elif [ $ADD -eq 1 ]; then
  mkdir -p ../../outfinalnew${BIT}/Plugins/$PLUGIN
  mkdir -p ../../outfinalnew${BIT}/Plugins/${PLUGIN}/$7

  cp -f changelog ../../outfinalnew${BIT}/Plugins/$PLUGIN/  

  cd $FINAL
  cp -f $FILES ../../../outfinalnew${BIT}/Plugins/$PLUGIN/
  cd ..

  cd $FINAL/$7
  cp -f $5 ../../../../outfinalnew${BIT}/Plugins/${PLUGIN}/$7/
  cd ../..
else
  mkdir -p ../../outfinalnew${BIT}/Plugins/$PLUGIN

  cp -f changelog ../../outfinalnew${BIT}/Plugins/$PLUGIN/  

  cd $FINAL
  cp -f $FILES ../../../outfinalnew${BIT}/Plugins/$PLUGIN/
  cd ..
fi
}

function bplugin {
PLUGIN=$1
PLDIR=$2

pushd $PLUGIN

bplugin2 "$PLDIR" 32 1 "$3"
bplugin2 "$PLDIR" 64 1 "$3"

popd
}

function bpluginftp {
PLUGIN=ftp
PLDIR=FTP

pushd $PLUGIN

FILES="FarFtp.dll FtpEng.hlf FtpRus.hlf FtpEng.lng FtpRus.lng FarFtp.map TechInfo.reg TechInfo_rus.reg FtpCmds.txt FtpCmds_rus.txt Notes.txt Notes_rus.txt"
ADDFILES="ftpDirList.fll ftpProgress.fll ftpDirList.map ftpProgress.map Progress_FarCopy.reg Progress_JM.reg Progress_Wesha.reg Progress_ZeMA.reg"
ADDBUILDDIRS="{lib,obj/DirList,obj/LibObj,obj/Notify,obj/Progress}"
ADDOUTDIR="lib"

bplugin2 "$PLDIR" 32 0 "$FILES" "$ADDFILES" $ADDBUILDDIRS $ADDOUTDIR
bplugin2 "$PLDIR" 64 0 "$FILES" "$ADDFILES" $ADDBUILDDIRS $ADDOUTDIR

popd
}

function bpluginma {
PLUGIN=multiarc
PLDIR=MultiArc

pushd $PLUGIN

FILES="MultiArc.dll arceng.hlf arcrus.hlf arceng.lng arcrus.lng MultiArc.map"
ADDFILES="Ace.fmt Arc.fmt Arj.fmt Cab.fmt Custom.fmt Ha.fmt Lzh.fmt Rar.fmt TarGz.fmt Zip.fmt custom.ini Ace.map Arc.map Arj.map Cab.map Custom.map Ha.map Lzh.map Rar.map TarGz.map Zip.map"
ADDBUILDDIRS="Formats"
ADDOUTDIR="Formats"

bplugin2 "$PLDIR" 32 0 "$FILES" "$ADDFILES" $ADDBUILDDIRS $ADDOUTDIR
bplugin2 "$PLDIR" 64 0 "$FILES" "$ADDFILES" $ADDBUILDDIRS $ADDOUTDIR

popd
}

function bpluginfe {
PLUGIN=fexcept
PLDIR=FExcept

pushd $PLUGIN

mkdir -p execdump/final.32.vc/obj/LibObj
#mkdir -p execdump/final.64.vc/obj/LibObj

FILES32="demangle32.dll ExcDump.dll FExcept.dll SetFarExceptionHandler.reg"
#FILES64="demangle64.dll ExcDump.dll FExcept.dll SetFarExceptionHandler.reg"

bplugin2 "$PLDIR" 32 1 "$FILES32"
#bplugin2 "$PLDIR" 64 1 "$FILES64"

popd
}

rm -fR plugins
rm -fR misc

svn co http://localhost/svn/trunk/plugins plugins
svn co http://localhost/svn/trunk/misc misc

cp -f far/Include/*.hpp plugins/common/unicode/

mkdir -p outfinalnew32/Plugins
mkdir -p outfinalnew64/Plugins

cd plugins/common/CRT

mkdir -p obj.32.vc/wide
mkdir -p obj.64.vc/wide
wine cmd /c ../../../common.32.bat &> ../../../logCRT32
wine cmd /c ../../../common.64.bat &> ../../../logCRT64

cd ../..

bplugin "align" "Align" "Align.dll AlignEng.lng AlignRus.lng Align.map"
bplugin "autowrap" "AutoWrap" "AutoWrap.dll WrapEng.lng WrapRus.lng AutoWrap.map"
bplugin "brackets" "Brackets" "Brackets.dll BrackEng.hlf BrackRus.hlf BrackEng.lng BrackRus.lng BrackDel.reg BrackEng.reg BrackRus.reg Brackets.map"
bplugin "compare" "Compare" "Compare.dll CmpEng.hlf CmpRus.hlf CompEng.lng CompRus.lng Compare.map"
bplugin "drawline" "DrawLine" "DrawLine.dll DrawEng.hlf DrawRus.hlf DrawEng.lng DrawRus.lng DrawLine.map"
bplugin "editcase" "EditCase" "EditCase.dll ECaseEng.hlf ECaseRus.hlf ECaseEng.lng ECaseRus.lng EditCase.map"
bplugin "emenu" "EMenu" "EMenu.dll EMenuEng.hlf EMenuRus.hlf EMenuEng.lng EMenuRus.lng EMenu.map EMenuDel.reg HotkeyClipboard.reg HotkeyProperties.reg Hotkey.reg"
bplugin "farcmds" "FarCmds" "FARCmds.dll FARCmdsEng.hlf FARCmdsRus.hlf FARCmdsEng.lng FARCmdsRus.lng FARCmds.map"
bplugin "filecase" "FileCase" "FileCase.dll CaseEng.hlf CaseRus.hlf CaseEng.lng CaseRus.lng FileCase.map"
bplugin "hlfviewer" "HlfViewer" "HlfViewer.dll HlfViewerEng.hlf HlfViewerRus.hlf HlfViewerEng.lng HlfViewerRus.lng HlfViewer.map"
bplugin "macroview" "MacroView" "MacroView.dll MacroEng.hlf MacroRus.hlf MacroEng.lng MacroRus.lng MacroView.map"
bplugin "network" "Network" "Network.dll NetEng.hlf NetRus.hlf NetEng.lng NetRus.lng Network.map"
bplugin "proclist" "ProcList" "Proclist.dll ProcEng.hlf ProcRus.hlf ProcEng.lng ProcRus.lng Proclist.map"
bplugin "tmppanel" "TmpPanel" "TmpPanel.dll TmpEng.hlf TmpRus.hlf TmpEng.lng TmpRus.lng TmpPanel.map shortcuts.eng.reg shortcuts.rus.reg disks.eng.temp disks.rus.temp shortcuts.eng.temp shortcuts.rus.temp"
bplugin "arclite" "ArcLite" "7z.dll 7z.sfx 7zCon.sfx 7zS2.sfx 7zS2con.sfx 7zSD.sfx arclite.dll arclite.map arclite_eng.hlf arclite_eng.lng arclite_rus.hlf arclite_rus.lng"

bpluginftp
#bpluginma

cd ../misc

bpluginfe

cd ..
