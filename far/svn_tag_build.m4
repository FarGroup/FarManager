m4_include(`farversion.m4')m4_dnl
m4_define(TAG,``http://farmanager.com/svn/tags/unicode_far/'MAJOR`'MINOR`_b'BUILD')m4_dnl
m4_define(CMD,`svn info TAG > nul 2>&1 & ( if not errorlevel 1 ( echo Error: tag TAG already exists ) else ( svn copy http://farmanager.com/svn/trunk/unicode_far TAG -m "tag build BUILD" ) )')m4_dnl
m4_esyscmd(CMD)m4_dnl
