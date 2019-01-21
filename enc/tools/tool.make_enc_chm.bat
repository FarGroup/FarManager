@echo off
cd "%~dp0" || exit 1
python clean.py || exit 1
python tool.make_chm.py || exit 1
cd ../build/chm/ru || exit 1
rem system locate must be set to russian for this command to generate the chm file correctly
"C:\Program Files (x86)\HTML Help Workshop\hhc.exe" pluginsr.hhp
if not exist FarEncyclopedia.ru.chm (
	echo "Error: FarEncyclopedia.ru.chm wasn't created!"
	exit 1
)
exit 0
