tool.make_chm.py
================
Makes projects files for building Far Manager Encyclopaedia in .CHM format
Files are created in "build" subdirectory in the root of Enc. source tree.

Usage:

    python tool.make_chm.py

Then change dir to /build/chm and build .hhp project file with HCW or hhc.exe
Use clean.py to remove created files.


rucheck.py
==========
Finds files with Russian letters in win-1251 encoding. Such files are
considered untranslated and reported. To skip specific line with Russian
letters, the `<!-- NLC -->` marker can be added to it.

techtonik@gmail.com

