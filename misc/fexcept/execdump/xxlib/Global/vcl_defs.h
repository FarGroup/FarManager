#ifndef __MY_VCL_EXTENSIONS
#define __MY_VCL_EXTENSIONS

/*******************************************************************
 Macros:
   DEF_PROP_xx          - VCL property declaration
   all_DEF_PROP_xx      - property declaration for NON-VCL classes
   DECL_PROP_GET        - declare function for GET property outside class
   DECL_PROP_SET        - declare function for SET property outside class
   DECL_NOAPPWINDOW     - hide Applcation main window (usefull for
                          console application)
 Classes:
   VCLObject - holder for VCL objects with optional auto-delete it
               Used for make/send VCL object as automated const
               objects to functions.
   HBitmap   - descendant of VCLObject for TBitmap holder
 *******************************************************************/

//Declare name of property variable
#define PROP_VAR(nm)                  F##nm

//------------------------------
//-------------- VCL classes properties
//------------------------------
//Define property   tp nm = { read = get, write = set,nodefault };
#define DEF_PROP_PP( tp,nm,gt,st )                protected:                                                         \
                                                    tp   __fastcall Get##nm( void ) gt                               \
                                                    void __fastcall Set##nm( tp val ) st                             \
                                                  public:                                                            \
                                                    __property tp nm = { read = Get##nm, write = Set##nm, nodefault };

//Define property   tp nm = { read = F##nm, write = set,nodefault };
#define DEF_PROP_VP( tp,nm,st )                          protected:                                                     \
                                                        tp PROP_VAR(nm);                                                \
                                                        void __fastcall Set##nm( tp val ) st                            \
                                                      public:                                                           \
                                                        __property tp nm = { read = F##nm, write = Set##nm, nodefault };
//Define property   tp nm = { read = get, write = F##nm,nodefault };
#define DEF_PROP_PV( tp,nm,gt )                          protected:                                                     \
                                                        tp PROP_VAR(nm);                                                 \
                                                        tp __fastcall Get##nm( void ) gt                                 \
                                                      public:                                                            \
                                                        __property tp nm = { read = Get##nm, write = F##nm, nodefault };
//Define property   tp nm = { read = F##nm, nodefault };
#define DEF_PROP_V( tp,nm )                               protected:                                                     \
                                                            tp PROP_VAR(nm);                                             \
                                                          public:                                                        \
                                                            __property tp nm = { read = F##nm, nodefault };
//Define property   tp nm = { read = F##nm, write = F##nm, nodefault };
#define DEF_PROP_VV( tp,nm )                              protected:                                                     \
                                                            tp PROP_VAR(nm);                                             \
                                                          public:                                                        \
                                                            __property tp nm = { read = F##nm, write = F##nm, nodefault };
//Define property   tp nm = { read = get, nodefault };
#define DEF_PROP_P( tp,nm,gt )                        protected:                                                         \
                                                        tp   __fastcall Get##nm( void ) gt                               \
                                                      public:                                                            \
                                                        __property tp nm = { read = Get##nm, nodefault };
//------------------------------
//-------------- PROPS w DEFAULT
//------------------------------
//Define property   tp nm = { read = get, write = set,nodefault };
#define DEF_PROP_PP_DEF( tp,nm,gt,st,def )        protected:                                                         \
                                                    tp   __fastcall Get##nm( void ) gt                               \
                                                    void __fastcall Set##nm( tp val ) st                             \
                                                  public:                                                            \
                                                    __property tp nm = { read = Get##nm, write = Set##nm, default = def };

//Define property   tp nm = { read = F##nm, write = set,nodefault };
#define DEF_PROP_VP_DEF( tp,nm,st,def )               protected:                                                         \
                                                        tp PROP_VAR(nm);                                                 \
                                                        void __fastcall Set##nm( tp val ) st                             \
                                                      public:                                                            \
                                                        __property tp nm = { read = F##nm, write = Set##nm, default = def };

//Define property   tp nm = { read = get, write = F##nm,nodefault };
#define DEF_PROP_PV_DEF( tp,nm,gt,def )               protected:                                                         \
                                                        tp PROP_VAR(nm);                                                 \
                                                        tp __fastcall Get##nm( void ) gt                                 \
                                                      public:                                                            \
                                                        __property tp nm = { read = Get##nm, write = F##nm, default = def };

//Define property   tp nm = { read = F##nm, nodefault };
#define DEF_PROP_V_DEF( tp,nm,def )                       protected:                                                         \
                                                            tp PROP_VAR(nm);                                                 \
                                                          public:                                                            \
                                                            __property tp nm = { read = F##nm, default = def };

//Define property   tp nm = { read = F##nm, write = F##nm, nodefault };
#define DEF_PROP_VV_DEF( tp,nm,def )                      protected:                                                         \
                                                            tp PROP_VAR(nm);                                                 \
                                                          public:                                                            \
                                                            __property tp nm = { read = F##nm, write = F##nm, default = def };

//Define property   tp nm = { read = get, nodefault };
#define DEF_PROP_P_DEF( tp,nm,gt,def )                protected:                                                         \
                                                        tp   __fastcall Get##nm( void ) gt                               \
                                                      public:                                                            \
                                                        __property tp nm = { read = Get##nm, default = def };
//------------------------------
//-------------- ALL CLASSES properties
//------------------------------
//Define property   tp nm = { read = get, write = set,nodefault };
#define all_DEF_PROP_PP( tp,nm,get,set )                  protected:                                                         \
                                                            tp   __fastcall Get##nm( void ) get                              \
                                                            void __fastcall Set##nm( tp val ) set                            \
                                                          public:                                                            \
                                                            __property tp nm = { read = Get##nm, write = Set##nm };
//Define property   tp nm = { read = F##nm, write = set,nodefault };
#define all_DEF_PROP_VP( tp,nm,set )                      protected:                                                         \
                                                            tp PROP_VAR(nm);                                                 \
                                                            void __fastcall Set##nm( tp val ) set                            \
                                                          public:                                                            \
                                                            __property tp nm = { read = F##nm, write = Set##nm };
//Define property   tp nm = { read = get, write = F##nm,nodefault };
#define all_DEF_PROP_PV( tp,nm,get )                      protected:                                                         \
                                                            tp PROP_VAR(nm);                                                 \
                                                            tp __fastcall Get##nm( void ) get                                \
                                                          public:                                                            \
                                                            __property tp nm = { read = Get##nm, write = F##nm };
//Define property   tp nm = { read = F##nm, nodefault };
#define all_DEF_PROP_V( tp,nm )                           protected:                                                         \
                                                            tp PROP_VAR(nm);                                                 \
                                                          public:                                                            \
                                                            __property tp nm = { read = F##nm };
//Define property   tp nm = { read = F##nm, write = F##nm, nodefault };
#define all_DEF_PROP_VV( tp,nm )                          protected:                                                         \
                                                            tp PROP_VAR(nm);                                                 \
                                                          public:                                                            \
                                                            __property tp nm = { read = F##nm, write = F##nm };
//Define property   tp nm = { read = get, nodefault };
#define all_DEF_PROP_P( tp,nm,get )                       protected:                                                         \
                                                            tp   __fastcall Get##nm( void ) get                              \
                                                          public:                                                            \
                                                            __property tp nm = { read = Get##nm };

//------------------------------
//------ property GET / SET
//------------------------------
//place property `Get` procedure
#define DECL_PROP_GET( cl,tp,nm )           tp   __fastcall cl::Get##nm( void )
//place property `Set` procedure
#define DECL_PROP_SET( cl,tp,nm )           void __fastcall cl::Set##nm( tp val )

#define DECL_PROP_P( cl,tp,nm,get )         tp   __fastcall cl::Get##nm( void ) get
#define DECL_PROP_VP( cl,tp,nm,set )        void __fastcall cl::Set##nm( tp val ) set
#define DECL_PROP_PP( cl,tp,nm,get,set )    tp   __fastcall cl::Get##nm( void )   get \
                                            void __fastcall cl::Set##nm( tp val ) set

//------------------------------
//------ PROPERTY EDITORS
//------------------------------
#define REG_EDITOR( Class, To, PropertyName, Editor )           RegisterPropertyEditor( *(::GetPropInfo( __typeinfo(Class),#PropertyName ))->PropType, \
                                                                                        To, #PropertyName, __classid(Editor) )

#define REG_FIELD_EDITOR( Class,FiledProp,DataSourceProp )      REG_EDITOR( Class,__classid(Class),FiledProp,HFieldEditor_##DataSourceProp )

#define DECL_FIELD_EDITOR( DataSourceProp )                     class HFieldEditor_##DataSourceProp: public HFieldProperty {                                           \
                                                                  public:                                                                                              \
                                                                    virtual CONSTSTR __fastcall GetDataSourcePropName(void) { return #DataSourceProp ; }               \
                                                                  public:                                                                                              \
                                                                    inline __fastcall virtual HFieldEditor_##DataSourceProp (                                          \
                                                                       const Dsgnintf::_di_IFormDesigner ADesigner,                                                    \
                                                                       int APropCount)                                                                                 \
                                                                      : HFieldProperty(ADesigner, APropCount) {}                                                       \
                                                                };

//------------------------------
//------ BORDER
//------------------------------
//Determine control border size
#define VCL_WIN_BORDER(p)    (((p)->BorderStyle==bsSingle) << (p)->Ctl3D)
#define VCL_PANEL_ADDON(p)   (((p)->BevelInner != bvNone) + ((p)->BevelOuter != bvNone))
#define VCL_PANEL_BORDER(p)  (VCL_WIN_BORDER(p) + VCL_PANEL_ADDON(p))

//------------------------------
#define DECL_NOAPPWINDOW     int __APP_GetDummy( void );                   \
                             static int __APPGlobal_dummy=__APP_GetDummy();\
                             static int __APP_GetDummy( void )             \
                              { ShowWindow( Application->Handle,SW_HIDE ); \
                                return 0; }
/*******************************************************************
  TYPEDEFS
 *******************************************************************/
typedef Graphics::TBitmap   *PTBitmap;
typedef Graphics::TIcon     *PTIcon;
typedef Graphics::TMetafile *PTMetafile;
typedef TCanvas             *PTCanvas;
typedef TObject             *PTObject;
typedef TControl            *PTControl;
typedef TComponent          *PTComponent;
typedef TPersistent         *PTPersistent;
typedef TWinControl         *PTWinControl;

typedef Typinfo::TTypeKind   HTypeKind;
typedef Typinfo::PPropInfo   HPropInfo;
typedef Typinfo::TTypeKinds  HTypeKinds;
typedef Typinfo::PTypeData   HTypeData;
typedef Typinfo::PTypeInfo   HTypeInfo;
typedef Typinfo::TPropList   HPropList_t;
typedef Typinfo::PPropList   PHPropList_t;

#if defined(__BCB1__)
  #define _W_String            tkLWString
  #define PI_2_TI(nm)          nm->PropType
  #define PL_2_PLPTR(nm)       (&nm)
  #define CLPTR_2_TI(nm)       ((HTypeInfo)nm->ClassInfo())
  #define TD_2_SETINFO(nm)     (HTypeInfo)(((int)nm->CompType) >> 8)
  #define TD_2_ENUM_Max(nm)    (nm->MaxValue / 0x100)
  #define TD_2_ENUM_Min(nm)    (nm->MinValue / 0x100)
//Package
  #define _HPACKAGE
  #undef  DYNAMIC
  #define DYNAMIC              virtual
#else
#if defined(__BCB3__)
  #define _W_String            tkLWString
  #define PI_2_TI(nm)          nm->PropType
  #define PL_2_PLPTR(nm)       (&nm)
  #define CLPTR_2_TI(nm)       ((HTypeInfo)nm->ClassInfo())
  #define TD_2_SETINFO(nm)     (HTypeInfo)(((int)nm->CompType) >> 8)
  #define TD_2_ENUM_Max(nm)    (nm->MaxValue / 0x100)
  #define TD_2_ENUM_Min(nm)    (nm->MinValue / 0x100)
//Package
  #define _HPACKAGE
  #undef  DYNAMIC
  #define DYNAMIC              virtual
#else
#if defined(__BCB4__)
  #define _W_String            tkWString
  #define PI_2_TI(nm)          (*nm->PropType)
  #define PL_2_PLPTR(nm)       ((PHPropList_t)(&nm))
  #define CLPTR_2_TI(nm)       ((HTypeInfo)nm->ClassInfo())
  #define TD_2_SETINFO(nm)     nm->CompType[0]
  #define TD_2_ENUM_Max(nm)    nm->MaxValue
  #define TD_2_ENUM_Min(nm)    nm->MinValue
//Package
  #define _HPACKAGE            PACKAGE
#else
#if defined(__BCB5__)
  #define _W_String            tkWString
  #define PI_2_TI(nm)          (*nm->PropType)
  #define PL_2_PLPTR(nm)       ((PHPropList_t)(&nm))
  #define CLPTR_2_TI(nm)       ((HTypeInfo)nm->ClassInfo())
  #define TD_2_SETINFO(nm)     nm->CompType[0]
  #define TD_2_ENUM_Max(nm)    nm->MaxValue
  #define TD_2_ENUM_Min(nm)    nm->MinValue
//Package
  #define _HPACKAGE            PACKAGE
#else
#if defined(__BCB6__)
  #define _W_String            tkWString
  #define PI_2_TI(nm)          (*nm->PropType)
  #define PL_2_PLPTR(nm)       ((PHPropList_t)(&nm))
  #define CLPTR_2_TI(nm)       ((HTypeInfo)nm->ClassInfo())
  #define TD_2_SETINFO(nm)     nm->CompType[0]
  #define TD_2_ENUM_Max(nm)    nm->MaxValue
  #define TD_2_ENUM_Min(nm)    nm->MinValue
//Package
  #define _HPACKAGE            PACKAGE
#endif //6
#endif //5
#endif //4
#endif //3
#endif //1

#endif
