#ifndef __MY_STRUCT_DEFS
#define __MY_STRUCT_DEFS

//- Simple STRUCT, CLASS, TCLASS
#define LOCALPRESTRUCT( cl )           typedef struct LOCALDECLSPEC cl *P##cl; typedef const struct LOCALDECLSPEC cl *PC##cl; struct LOCALDECLSPEC cl
#define LOCALPRECLASS( cl )            typedef class LOCALDECLSPEC cl *P##cl; typedef const class LOCALDECLSPEC cl *PC##cl; class LOCALDECLSPEC cl
#define LOCALSTRUCT( name )            LOCALPRESTRUCT(name) {
#define LOCALSTRUCTBASE( name,b )      LOCALPRESTRUCT(name) : b {
#define LOCALSTRUCTBASE2( name,b,b1 )  LOCALPRESTRUCT(name) : b,b1 {
#define LOCALCLASS( name )             LOCALPRECLASS(name) {
#define LOCALCLASSBASE( name,b )       LOCALPRECLASS(name) : b {
#define LOCALCLASSBASE2( name,b,b1 )   LOCALPRECLASS(name) : b,b1 {
#define LOCALDECLSPEC                  DECLSPEC_LOCAL

//
#define IMP_PRESTRUCT( cl )           typedef struct IMP_DECLSPEC cl *P##cl; typedef const struct IMP_DECLSPEC cl *PC##cl; struct IMP_DECLSPEC cl
#define IMP_PRECLASS( cl )            typedef class IMP_DECLSPEC cl *P##cl; typedef const class IMP_DECLSPEC cl *PC##cl; class IMP_DECLSPEC cl
#define IMP_STRUCT( name )            IMP_PRESTRUCT(name) {
#define IMP_STRUCTBASE( name,b )      IMP_PRESTRUCT(name) : b {
#define IMP_STRUCTBASE2( name,b,b1 )  IMP_PRESTRUCT(name) : b,b1 {
#define IMP_CLASS( name )             IMP_PRECLASS(name) {
#define IMP_CLASSBASE( name,b )       IMP_PRECLASS(name) : b {
#define IMP_CLASSBASE2( name,b,b1 )   IMP_PRECLASS(name) : b,b1 {
#define IMP_DECLSPEC                  DECLSPEC_IMP

//
#define EXP_PRESTRUCT( cl )           typedef struct EXP_DECLSPEC cl *P##cl; typedef const struct EXP_DECLSPEC cl *PC##cl; struct EXP_DECLSPEC cl
#define EXP_PRECLASS( cl )            typedef class  EXP_DECLSPEC cl *P##cl; typedef const class  EXP_DECLSPEC cl *PC##cl; class  EXP_DECLSPEC cl
#define EXP_STRUCT( name )            EXP_PRESTRUCT(name) {
#define EXP_STRUCTBASE( name,b )      EXP_PRESTRUCT(name) : b {
#define EXP_STRUCTBASE2( name,b,b1 )  EXP_PRESTRUCT(name) : b,b1 {
#define EXP_CLASS( name )             EXP_PRECLASS(name) {
#define EXP_CLASSBASE( name,b )       EXP_PRECLASS(name) : b {
#define EXP_CLASSBASE2( name,b,b1 )   EXP_PRECLASS(name) : b,b1 {
#define EXP_DECLSPEC                  DECLSPEC_EXP

//
#define PKG_PRESTRUCT( cl )           typedef struct PKG_DECLSPEC cl *P##cl; typedef const struct PKG_DECLSPEC cl *PC##cl; struct PKG_DECLSPEC cl
#define PKG_PRECLASS( cl )            typedef class PKG_DECLSPEC cl *P##cl; typedef const class PKG_DECLSPEC cl *PC##cl; class  PKG_DECLSPEC cl
#define PKG_STRUCT( name )            PKG_PRESTRUCT(name) {
#define PKG_STRUCTBASE( name,b )      PKG_PRESTRUCT(name) : b {
#define PKG_STRUCTBASE2( name,b,b1 )  PKG_PRESTRUCT(name) : b,b1 {
#define PKG_CLASS( name )             PKG_PRECLASS(name) {
#define PKG_CLASSBASE( name,b )       PKG_PRECLASS(name) : b {
#define PKG_CLASSBASE2( name,b,b1 )   PKG_PRECLASS(name) : b,b1 {
#define PKG_DECLSPEC                  _HPACKAGE

#endif