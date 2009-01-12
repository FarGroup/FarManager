#ifndef __MY_DECLARATION_IMPORT
#define __MY_DECLARATION_IMPORT

#if defined(__BORLANDC__)
#pragma message "Import packaging used"
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
#define PRESTRUCT                     IMP_PRESTRUCT
#define PRECLASS                      IMP_PRECLASS
#define STRUCT                        IMP_STRUCT
#define STRUCTBASE                    IMP_STRUCTBASE
#define STRUCTBASE2                   IMP_STRUCTBASE2
#define CLASS                         IMP_CLASS
#define CLASSBASE                     IMP_CLASSBASE
#define CLASSBASE2                    IMP_CLASSBASE2

//
#define MYRTLEXP                      DECLSPEC_IMP
#define MYRTLEXP_PT                   DECLSPEC_IMP_PT
#define HDECLSPEC                     EXTERN

#endif
