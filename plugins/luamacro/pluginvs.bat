@rem Script to build a plugin under "Visual Studio .NET Command Prompt".
@rem It creates <plugin>.dll.

@setlocal
@set PLUGNAME=luamacro.dll
@set DEFNAME=luamacro.vc.def
@set INC_FAR=S:\Progr\work\system\include\far\unicode
@set INC_LUA=S:\Progr\work\system\include\lua51
@set PATH_LUAFAR=S:\Progr\work\luafar\luafar_unicode
@set DEFINES=/DEXPORT_OPEN

@set MYCOMPILE=cl /nologo /MT /O2 /W3 /c /D_CRT_SECURE_NO_DEPRECATE^
  /I%INC_FAR% /I%INC_LUA% %DEFINES%
@set MYLINK=link /nologo

if exist *.obj del *.obj
%MYCOMPILE% %PATH_LUAFAR%\src\luaplug.c
%MYLINK% /DLL /out:%PLUGNAME% luaplug.obj lua5.1_x86.lib luafar3.lib^
  /def:%DEFNAME%
if exist *.obj del *.obj
