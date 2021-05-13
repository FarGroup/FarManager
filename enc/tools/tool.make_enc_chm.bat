@echo off
cd "%~dp0" || exit 1
python clean.py || exit 1
python tool.make_chm.py || exit 1
cd ../build/chm/ru || exit 1
"%~dp0hh_compiler\hh_compiler.exe" 1251 pluginsr.hhp
if not exist FarEncyclopedia.ru.chm (
	echo "Error: FarEncyclopedia.ru.chm wasn't created!"
	exit 1
)
exit 0
