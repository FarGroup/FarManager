@echo off
if not exist final\AutoWrap.dll call cc.bat
if not exist final\AutoWrap.dll goto done

echo Copy AutoWrap to Install\Editor...
rem Delete from Install dir
if exist %IPath%\Editor\AutoWrap\AutoWrap.dll    del %IPath%\Editor\AutoWrap\AutoWrap.dll > nul
rem if exist %IPath%\Editor\AutoWrap\AWrapEng.hlf del %IPath%\Editor\AutoWrap\AWrapEng.hlf > nul
rem if exist %IPath%\Editor\AutoWrap\AWrapRus.hlf del %IPath%\Editor\AutoWrap\AWrapRus.hlf > nul
if exist %IPath%\Editor\AutoWrap\AWrapEng.lng del %IPath%\Editor\AutoWrap\AWrapEng.lng > nul
if exist %IPath%\Editor\AutoWrap\AWrapRus.lng del %IPath%\Editor\AutoWrap\AWrapRus.lng > nul

rem Copy To Install dir
copy final\AutoWrap.dll    %IPath%\Editor\AutoWrap\AUTOWRAP.DLL > nul
rem copy final\AWrapEng.hlf %IPath%\Editor\AutoWrap\AWrapEng.hlf > nul
rem copy final\AWrapRus.hlf %IPath%\Editor\AutoWrap\AWrapRus.hlf > nul
copy final\AWrapEng.lng %IPath%\Editor\AutoWrap\AWrapEng.lng > nul
copy final\AWrapRus.lng %IPath%\Editor\AutoWrap\AWrapRus.lng > nul


echo ...copy AutoWrap to PlugDoc...
rem Delete from PlugDoc dir
if exist %EPath%\Editor\AutoWrap\AutoWrap.cpp  del %EPath%\Editor\AutoWrap\AutoWrap.cpp > nul
if exist %EPath%\Editor\AutoWrap\WrapMix.cpp   del %EPath%\Editor\AutoWrap\WrapMix.cpp  > nul
if exist %EPath%\Editor\AutoWrap\WrapReg.cpp   del %EPath%\Editor\AutoWrap\WrapReg.cpp  > nul
if exist %EPath%\Editor\AutoWrap\AutoWrap.hpp  del %EPath%\Editor\AutoWrap\AutoWrap.hpp > nul
if exist %EPath%\Editor\AutoWrap\WrapLng.hpp   del %EPath%\Editor\AutoWrap\WrapLng.hpp  > nul
if exist %EPath%\Editor\AutoWrap\AWrapEng.lng  del %EPath%\Editor\AutoWrap\AWrapEng.lng  > nul
if exist %EPath%\Editor\AutoWrap\AWrapRus.lng  del %EPath%\Editor\AutoWrap\AWrapRus.lng  > nul

rem Copy To PlugDoc dir
copy AutoWrap.cpp %EPath%\Editor\AutoWrap\AutoWrap.cpp > nul
copy WrapMix.cpp  %EPath%\Editor\AutoWrap\WrapMix.cpp  > nul
copy WrapReg.cpp  %EPath%\Editor\AutoWrap\WrapReg.cpp  > nul
copy AutoWrap.hpp %EPath%\Editor\AutoWrap\AutoWrap.hpp > nul
copy WrapLng.hpp  %EPath%\Editor\AutoWrap\WrapLng.hpp  > nul
copy AWrapEng.lng  %EPath%\Editor\AutoWrap\AWrapEng.lng  > nul
copy AWrapRus.lng  %EPath%\Editor\AutoWrap\AWrapRus.lng  > nul
echo done.
:done
