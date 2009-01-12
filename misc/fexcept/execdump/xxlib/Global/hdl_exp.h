#ifndef __MY_DECLARATION_EXPORT
#define __MY_DECLARATION_EXPORT

#if defined(__BORLANDC__)
#pragma message "Export packaging used"
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
#define PRESTRUCT                     EXP_PRESTRUCT
#define PRECLASS                      EXP_PRECLASS
#define STRUCT                        EXP_STRUCT
#define STRUCTBASE                    EXP_STRUCTBASE
#define STRUCTBASE2                   EXP_STRUCTBASE2
#define CLASS                         EXP_CLASS
#define CLASSBASE                     EXP_CLASSBASE
#define CLASSBASE2                    EXP_CLASSBASE2

//
#define MYRTLEXP                      DECLSPEC_EXP
#define MYRTLEXP_PT                   DECLSPEC_EXP_PT
#define HDECLSPEC                     EXTERN

#endif
