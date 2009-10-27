@echo off

set TOOLSDIR=tools\
set EXEDIR=.bin\
set INCDIR=Include\

if "%1"=="release" set EXEDIR=.bin\
if "%1"=="debug"   set EXEDIR=.bind\

if not exist %EXEDIR% md %EXEDIR%
if not exist %INCDIR% md %INCDIR%

set M4=%TOOLSDIR%m4 -P -DFARBIT=32

echo *********************************************
echo ***** Far Manager Preprocessing started *****

echo Building language files...
rem Cleanup old files (to ensure it's always re-created)
if exist %EXEDIR%FarEng.lng del %EXEDIR%FarEng.lng
if exist %EXEDIR%FarRus.lng del %EXEDIR%FarRus.lng
if exist FarEng.lng         del FarEng.lng
if exist FarRus.lng         del FarRus.lng
if exist farlang.templ      del farlang.templ
if exist lang.hpp           del lang.hpp
if exist lang.ini           del lang.ini
rem Create and copy new files
%M4% farlang.templ.m4 > farlang.templ
%TOOLSDIR%lng.generator.exe -nc -i lang.ini -ol %EXEDIR% farlang.templ
if exist FarEng.lng copy FarEng.lng %EXEDIR%
if exist FarRus.lng copy FarRus.lng %EXEDIR%

echo Creating resource files...
rem Cleanup old files
if exist copyright.inc    del copyright.inc
if exist farversion.inc   del farversion.inc
if exist Far.exe.manifest del Far.exe.manifest
if exist far.rc           del far.rc
rem Create and copy new files
%M4% copyright.inc.m4 | %TOOLSDIR%gawk -f scripts\enc.awk > copyright.inc
%M4% farversion.inc.m4                                    > farversion.inc
%M4% Far.exe.manifest.m4                                  > Far.exe.manifest
%M4% far.rc.m4                                            > far.rc

echo Creating dep files...
rem Cleanup old files
if exist far.gcc.dep del far.gcc.dep
rem Create and copy new files
%TOOLSDIR%gawk -f scripts\mkdep.awk -v compiler=gcc mkdep.list > far.gcc.dep

echo Generating color,keys,plugin
rem Cleanup old files
if exist %INCDIR%farcolor.hpp  del %INCDIR%farcolor.hpp
if exist %INCDIR%farkeys.hpp   del %INCDIR%farkeys.hpp
if exist %INCDIR%plugin.hpp    del %INCDIR%plugin.hpp
if exist %INCDIR%FarColorW.pas del %INCDIR%FarColorW.pas
if exist %INCDIR%FarKeysW.pas  del %INCDIR%FarKeysW.pas
if exist %INCDIR%PluginW.pas   del %INCDIR%PluginW.pas
rem Create and copy new files
%M4% -DINPUT=colors.hpp    headers.m4 > %INCDIR%farcolor.hpp
%M4% -DINPUT=keys.hpp      headers.m4 > %INCDIR%farkeys.hpp
%M4% -DINPUT=plugin.hpp    headers.m4 > %INCDIR%plugin.hpp
%M4% -DINPUT=FarColorW.pas headers.m4 > %INCDIR%FarColorW.pas
%M4% -DINPUT=FarKeysW.pas  headers.m4 > %INCDIR%FarKeysW.pas
%M4% -DINPUT=PluginW.pas   headers.m4 > %INCDIR%PluginW.pas

echo Creating help files...
rem Cleanup old files
if exist %EXEDIR%FarEng.hlf del %EXEDIR%FarEng.hlf
if exist %EXEDIR%FarRus.hlf del %EXEDIR%FarRus.hlf
if exist %EXEDIR%FarHun.hlf del %EXEDIR%FarHun.hlf
rem Create and copy new files
%TOOLSDIR%gawk -f scripts\mkhlf.awk FarEng.hlf.m4 | %TOOLSDIR%m4 -P > %EXEDIR%FarEng.hlf
%TOOLSDIR%gawk -f scripts\mkhlf.awk FarRus.hlf.m4 | %TOOLSDIR%m4 -P > %EXEDIR%FarRus.hlf
%TOOLSDIR%gawk -f scripts\mkhlf.awk FarHun.hlf.m4 | %TOOLSDIR%m4 -P > %EXEDIR%FarHun.hlf

echo Creating info file...
rem Cleanup old file
if exist %EXEDIR%File_id.diz del %EXEDIR%File_id.diz
rem Create and copy new file
%M4% File_id.diz.m4 > %EXEDIR%File_id.diz

echo ***** Far Manager Preprocessing finished *****
echo **********************************************
