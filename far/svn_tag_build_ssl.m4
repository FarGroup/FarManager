m4_include(`farversion.m4')m4_dnl
m4_define(TAG,``https://svn.code.sf.net/p/farmanager/code/tags/unicode_far/'MAJOR`'MINOR`_b'BUILD')m4_dnl
m4_define(CMD,`svn info TAG > nul 2>&1 & ( if not errorlevel 1 ( echo Error: tag TAG already exists ) else ( svn copy https://svn.code.sf.net/p/farmanager/code/trunk/unicode_far TAG -m "tag build BUILD" ) )')m4_dnl
m4_esyscmd(CMD)m4_dnl
