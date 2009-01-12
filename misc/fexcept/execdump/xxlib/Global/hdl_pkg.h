#ifndef __MY_DECLARATION_PACKAGE
#define __MY_DECLARATION_PACKAGE

#if defined(__BORLANDC__)
#pragma message "VCL Package packaging used"
#endif

//
#undef PRESTRUCT
#undef PRECLASS
#undef STRUCT
#undef STRUCTBASE
#undef STRUCTBASE2
#undef CLASS
#undef CLASSBASE
#undef CLASSBASE2

#undef HDECLSPEC
#undef MYRTLEXP
#undef MYRTLEXP_PT

//
#define PRESTRUCT        PKG_PRESTRUCT
#define PRECLASS         PKG_PRECLASS
#define STRUCT           PKG_STRUCT
#define STRUCTBASE       PKG_STRUCTBASE
#define STRUCTBASE2      PKG_STRUCTBASE2
#define CLASS            PKG_CLASS
#define CLASSBASE        PKG_CLASSBASE
#define CLASSBASE2       PKG_CLASSBASE2

//
#define MYRTLEXP         DECLSPEC_LOCAL
#define MYRTLEXP_PT      DECLSPEC_LOCAL_PT
#define HDECLSPEC        EXTERN PKG_DECLSPEC

#endif
