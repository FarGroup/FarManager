m4_include(`vbuild.m4')m4_dnl
m4_define(BUILDTYPE,`alpha')m4_dnl
m4_define(BUILDTYPENUM,4)m4_dnl
m4_define(CMDDATE,`.\scripts\gendate.awk')m4_dnl
m4_define(CMDPLUGINS,`.\scripts\plugins.awk')m4_dnl
m4_define(CMDAWK,`tools\gawk.exe')m4_dnl
m4_define(MAJOR,1)m4_dnl
m4_define(MINOR,71)m4_dnl
m4_define(DATE,m4_esyscmd(CMDAWK -f CMDDATE))m4_dnl
m4_define(BLD_YEAR,m4_substr(DATE,6,4))m4_dnl
m4_define(BLD_MONTH,m4_substr(DATE,3,2))m4_dnl
m4_define(BLD_DAY,m4_substr(DATE,0,2))m4_dnl
m4_define(COPYRIGHTYEARS,m4_ifelse(`2000',BLD_YEAR,`2000',`2000-'BLD_YEAR))m4_dnl
m4_define(FULLVERSION,`m4_ifelse(`',BUILDTYPE,`MAJOR.MINOR (build BUILD)',
`RC',BUILDTYPE,`MAJOR.MINOR `RC'BUILDTYPENUM (build BUILD)',
`alpha',BUILDTYPE,`MAJOR.MINOR alpha BUILDTYPENUM (build BUILD)',
`beta',BUILDTYPE,`MAJOR.MINOR beta BUILDTYPENUM (build BUILD)',
`MAJOR.MINOR alpha (BUILDTYPE based on build BUILD)')')m4_dnl
m4_define(FULLVERSIONNOBRACES,`m4_patsubst(FULLVERSION,`[\(\)]',`')')m4_dnl
