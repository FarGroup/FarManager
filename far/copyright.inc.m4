m4_include(`farversion.m4')m4_dnl
m4_define(VERSIONFORENC,`m4_ifelse(`1',BUILDTESTONLY,
`The FAR manager, version FULLVERSION - TEST ONLY!$Copyright (C) 1996-2000 Eugene Roshal, Copyright (C) COPYRIGHTYEARS FAR Group',
`The FAR manager, version FULLVERSION$Copyright (C) 1996-2000 Eugene Roshal, Copyright (C) COPYRIGHTYEARS FAR Group')')m4_dnl
VERSIONFORENC`'m4_dnl
