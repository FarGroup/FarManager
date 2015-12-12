m4_include(`farversion.m4')m4_dnl
m4_define(HDRPATH,`..\plugins\common\unicode')m4_dnl
m4_define(CMD,`if exist HDRPATH ( @copy Include\*.hpp HDRPATH && svn commit HDRPATH -m "update headers to BUILD" )')m4_dnl
m4_esyscmd(CMD)m4_dnl
