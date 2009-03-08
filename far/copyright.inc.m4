m4_include(`farversion.m4')m4_dnl
m4_define(COPYVAR,`char *Copyright="Far Manager, version $1 $Copyright (C) 1996-2000 Eugene Roshal, Copyright (C) COPYRIGHTYEARS Far Group";')m4_dnl
`#ifdef _M_IA64'
COPYVAR(FULLVERSIONIA64)
`#elif defined(_WIN64)'
COPYVAR(FULLVERSION64)
`#else'
COPYVAR(FULLVERSION32)
`#endif'
