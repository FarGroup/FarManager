m4_include(`farversion.m4')m4_dnl
m4_define(CMD,``svn copy http://farmanager.com/svn/trunk/unicode_far http://farmanager.com/svn/tags/unicode_far/'MAJOR`'MINOR`_b'BUILD -m "tag build BUILD"')m4_dnl
m4_esyscmd(CMD)m4_dnl
