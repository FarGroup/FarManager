#!/bin/sh

function makedocs {

mkdir -p $1/Documentation/eng
mkdir -p $1/Documentation/rus

cp docs/ClearPluginsCache.bat docs/RestoreSettings.bat docs/SaveSettings.bat docs/Descript.ion $1/
cp docs/ENG/Descript.ion docs/ENG/Arc.Support.txt docs/ENG/Bug.Report.txt docs/ENG/Far.FAQ.txt docs/ENG/Plugins.Install.txt docs/ENG/Plugins.Review.txt docs/ENG/TechInfo.txt $1/Documentation/eng/
cp docs/RUS/Descript.ion docs/RUS/Arc.Support.txt docs/RUS/Bug.Report.txt docs/RUS/Far.FAQ.txt docs/RUS/Plugins.Install.txt docs/RUS/Plugins.Review.txt docs/RUS/TechInfo.txt $1/Documentation/rus/
cp docs/PluginSDK/Descript.ion docs/PluginSDK/GCCReadme.txt docs/PluginSDK/README.TXT docs/PluginSDK/VCReadme.txt $1/PluginSDK/

svn export http://localhost/svn/trunk/addons $1/Addons

rm $1/Addons/XLat/Russian/Qwerty2.reg

}

function makedocs2 {

mkdir -p $1/Documentation/eng
mkdir -p $1/Documentation/rus

mkdir -p $1/Addons/Archivers
mkdir -p "$1/Addons/Colors/Custom Highlighting"
mkdir -p "$1/Addons/Colors/Default Highlighting"
mkdir -p $1/Addons/Macros
mkdir -p $1/Addons/SetUp
mkdir -p $1/Addons/Shell
mkdir -p $1/Addons/XLat/Russian

wine cmd /c docs.bat $1

cp docs/ClearPluginsCache.cmd docs/RestoreSettings.cmd docs/SaveSettings.cmd $1/

cp addons/XLat/Russian/Qwerty2.reg $1/Addons/XLat/Russian/Qwerty.reg

}

rm -fR addons
rm -fR docs

svn co http://localhost/svn/trunk/addons addons
svn co http://localhost/svn/trunk/docs docs

makedocs outfinalold32
makedocs outfinalold64

makedocs2 outfinalnew32
makedocs2 outfinalnew64
