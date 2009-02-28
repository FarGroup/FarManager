tool.make_chm.py
================
Makes projects files for building Far Manager Encyclopaedia in .CHM format
Files are created in "build" subdirectory in the root of Enc. source tree.

Usage:

python tool.make_chm.py

Then change dir to /build/chm and build .hhp project file with HCW or hhc.exe
Use clean.py to remove created files.


untranslated.py
===============
Finds files with russian letters in specified directory

If files contain Russian letters they are considered untranslated
unless <!-- NLC --> marker is present at the same line in HTML code


techtonik // rainforce.org