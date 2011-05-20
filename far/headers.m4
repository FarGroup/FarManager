m4_include(`farversion.m4')m4_dnl
m4_define(CMD,`CMDAWK -f CMDPLUGINS -v p1=MAJOR -v p2=MINOR -v p4=BUILD p5=STAGE INPUT')m4_dnl
m4_esyscmd(CMD)m4_dnl
