@echo off

cd ..
call vcvars32.bat >nul
set projname=TmpPanel
del %projname%.dll

set cppfiles=TmpPanel.cpp TmpCfg.cpp TmpClass.cpp TmpMix.cpp TmpReg.cpp Memory.cpp 

rc %projname%.rc
%comspec% /c cl /Gs10000 /O1i /GF /Gr /GR- /GX- /LD %cppfiles% %projname%.res /link /opt:nowin98 /nodefaultlib /def:%projname%.vc.def /OUT:"./VisualC/%projname%.dll" kernel32.lib user32.lib advapi32.lib shell32.lib /merge:.rdata=.text
rem copy %projname%.dll D:\TOOLS\FAR\Plugins\TmpPanel\%projname%.dll /Y >nul

del *.exp >nul
del *.obj >nul
del *.lib >nul

