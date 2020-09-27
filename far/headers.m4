m4_include(`farversion.m4')m4_dnl
m4_define(CMD,`CMDAWK -f ./scripts/plugins.awk -v p1=VERSION_MAJOR -v p2=VERSION_MINOR -v p3=VERSION_REVISION p4=VERSION_BUILD p5=VERSION_TYPE INPUT')m4_dnl
m4_esyscmd(CMD)m4_dnl
