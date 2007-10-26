m4_include(`farversion.m4')m4_dnl
m4_define(CMD,``svn copy http://farmanager.com/svn/unicode_far/trunk http://farmanager.com/svn/unicode_far/tags/'MAJOR`'MINOR`_b'BUILD -m "tag build BUILD"')m4_dnl
m4_esyscmd(CMD)m4_dnl
