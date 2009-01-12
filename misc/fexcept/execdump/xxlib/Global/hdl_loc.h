#ifndef __MY_DECLARATION_LOCAL
#define __MY_DECLARATION_LOCAL


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

#if defined(__VCL__) && !defined(__NOPKG__)
#if defined(__BORLANDC__)
#pragma message "Forced PKG packaging used"
#endif
  #define PRESTRUCT        PKG_PRESTRUCT
  #define PRECLASS         PKG_PRECLASS
  #define STRUCT           PKG_STRUCT
  #define STRUCTBASE       PKG_STRUCTBASE
  #define STRUCTBASE2      PKG_STRUCTBASE2
  #define CLASS            PKG_CLASS
  #define CLASSBASE        PKG_CLASSBASE
  #define CLASSBASE2       PKG_CLASSBASE2
#else
#if defined(__BORLANDC__)
#pragma message "Local packaging used"
#endif
  #define PRESTRUCT        LOCALPRESTRUCT
  #define PRECLASS         LOCALPRECLASS
  #define STRUCT           LOCALSTRUCT
  #define STRUCTBASE       LOCALSTRUCTBASE
  #define STRUCTBASE2      LOCALSTRUCTBASE2
  #define CLASS            LOCALCLASS
  #define CLASSBASE        LOCALCLASSBASE
  #define CLASSBASE2       LOCALCLASSBASE2
#endif

//
#define MYRTLEXP           DECLSPEC_LOCAL
#define MYRTLEXP_PT        DECLSPEC_LOCAL_PT
#define HDECLSPEC          EXTERN

#endif
