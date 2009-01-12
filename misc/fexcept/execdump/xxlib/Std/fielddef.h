#ifndef __MY_FIELDDEF
#define __MY_FIELDDEF

enum _FieldDefTypes {
 FT_INT      = 0x0001,
 FT_BITS     = 0x0002,
 FT_FLOAT    = 0x0003,
 FT_DOUBLE   = 0x0004,
 FT_STR      = 0x0005,
 FT_BIN      = 0x0006,
 FT_TYPEMASK = 0x00FF,

 FT_TP_RADIO = 0x0100,
 FT_TP_PTR   = 0x1000
};

LOCALSTRUCT( HFieldDef )
  WORD     Index;         //Number of field in structure (setted automatically by enumerator)
  WORD     Type;          //Type of field (FT_xxx)
  WORD     Offset;        //Offset of field fronm start of structure
  WORD     Size;          //Size of field item
  CONSTSTR Name;          //Name of field
  DWORD    Mask;          //Mask fro bit fields
  CONSTSTR Range;         //Range for integer fields in form: <value>[-<value>] [;...]
                          //  f.e.: "1;5;10-20;22-"
                          //Or bits list for all fields from 0 to 32 bit from Mask in form: <name>=<description>[;...]
                          //  f.e.: "FIELD_1=field 1;FIELD_2=field number 2"
  CONSTSTR Description;   //Text description of structure field
};

typedef CONSTSTR (DLL_CALLBACK *HFieldCheckValueInt_T)( PHFieldDef Source,DWORD Value );
typedef CONSTSTR (DLL_CALLBACK *HFieldBitNameByNumber_T)( PHFieldDef Source,int BitNumber );

/*@@
    User enum functions called for different type of structure fields.
    Must return TRUE to continue enumeration
    User enum:
     EnumInt     - may call to `CheckProc` to check value for possibility
     EnumBits    - may call `NameProc` with number of bits (from 0 to sizeof(DWORD)) to determine name of bit
                   Must set value only with bits, setted in `Mask` or one of `Mask` valuee if `Radio` setted
     EnumString  - `Size` setted only for static data. If data is dynamic user can not modify it (Size set to 0)
     EnumBinary  - `Size` setted only for static data. If data is dynamic user can not modify it (Size set to 0)
     EnumNullPtr - called then PTR data in structure pointed to NULL value
     EnumUnkType - called then type of field is unknown
    Service callbacks:
     HFieldCheckValueInt_T   - return NULL for correct value or patter string if value not fit in range
     HFieldBitNameByNumber_T - return string name of bit field or NULL if bit not in possible range of bits
  */
LOCALSTRUCT( HFieldUserEnum )
  BOOL (DLL_CALLBACK *EnumInt)    ( PHFieldDef Source,DWORD  *Value,CONSTSTR Name,CONSTSTR Description,HFieldCheckValueInt_T CheckProc );
  BOOL (DLL_CALLBACK *EnumBits)   ( PHFieldDef Source,DWORD  *Value,DWORD Mask,BOOL Radio,CONSTSTR Name,CONSTSTR Description,HFieldBitNameByNumber_T NameProc );
  BOOL (DLL_CALLBACK *EnumFloat)  ( PHFieldDef Source,float  *Value,CONSTSTR Name,CONSTSTR Description );
  BOOL (DLL_CALLBACK *EnumDouble) ( PHFieldDef Source,double *Value,CONSTSTR Name,CONSTSTR Description );
  BOOL (DLL_CALLBACK *EnumString) ( PHFieldDef Source,char   *Value,DWORD Size,CONSTSTR Name,CONSTSTR Description );
  BOOL (DLL_CALLBACK *EnumBinary) ( PHFieldDef Source,LPBYTE Value,DWORD Size,CONSTSTR Name,CONSTSTR Description );

  BOOL (DLL_CALLBACK *EnumNullPtr)( PHFieldDef Source,CONSTSTR Name,CONSTSTR Description );
  BOOL (DLL_CALLBACK *EnumUnkType)( PHFieldDef Source,WORD Type,CONSTSTR Name,CONSTSTR Description );
};

/*
   HFieldEnum( PHFieldDef FieldDefs,LPBYTE Data,PHFieldUserEnum EnumStruct );
    Call user enum procedures from `EnumStruct` for all fields from structure data `Data` described by `FieldDefs`
    Any of `EnumStruct` procedure may be not set (set to NULL). Such fields will be skipped from enumeration.
   Returns - index of error field or -1 if all enumeration completed
   HFieldItemEnum( PHFieldDef FieldDefs,LPBYTE Data,PHFieldUserEnum EnumStruct,int Index );
    Call user enum procedures from `EnumStruct` for structure item indexed by `Index`
    Any of `EnumStruct` procedure may be not set (set to NULL). Such fields will be skipped from enumeration.
   Returns - TRUE on no error, FALSE on error or user cancel
*/
HDECLSPEC int  MYRTLEXP HFieldEnum( PHFieldDef FieldDefs,LPBYTE Data,PHFieldUserEnum EnumStruct );
HDECLSPEC BOOL MYRTLEXP HFieldItemEnum( PHFieldDef FieldDefs,LPBYTE Data,PHFieldUserEnum EnumStruct,int Index );

/*
Macroses for declare structure fields
Example C sructure:
   STRUCT( Cfg )
     int        Style;
     BOOL       Bool;
     DWORD      Flags;
     DWORD      Enum;
     double     PowLevel;
     char       Back[ MAX_PATH_SIZE ];
     char      *Str;
   };
Declaration for this structure:
   FDECL_START ( defs )
     FDECL_INTRANGE ( Cfg,Style,   "0-10;12-",          "Style value" )
         FDECL_BOOL ( Cfg,Bool,                         "Bool value" )
         FDECL_BITS ( Cfg,Flags,   FL_1|FL_2|FL_3|FL_5, "FL_xxx flags" )
        FDECL_RADIO ( Cfg,Enum,    FL_1|FL_2|FL_3|FL_5, "One of FL_xxx flags" )
       FDECL_DOUBLE ( Cfg,PowLevel,                     "Double value" )
          FDECL_STR ( Cfg,Back,                         "Local string value" )
       FDECL_STRPTR ( Cfg,Str,                          "Ext string value" )
   FDECL_END
*/

#define FDECL_START( Name )                               HFieldDef Name[] = {
#define FDECL_END                                         { 0,0,0,0,NULL,0,NULL,NULL } };

#define FDECL_INT( Struct,Field,Descr )                   { 0,FT_INT, offsetof(Struct,Field), SizeOf(Struct,Field), #Field, 0,    NULL,  Descr  },
#define FDECL_INTRANGE( Struct,Field,Range,Descr )        { 0,FT_INT, offsetof(Struct,Field), SizeOf(Struct,Field), #Field, 0,    Range, Descr  },
#define FDECL_BOOL( Struct,Field,Descr )                  { 0,FT_INT, offsetof(Struct,Field), SizeOf(Struct,Field), #Field, 0,    "1;0", Descr  },
#define FDECL_BITS( Struct,Field,Bits,Descr )             { 0,FT_BITS,offsetof(Struct,Field), SizeOf(Struct,Field), #Field, Bits, #Bits, Descr  },
#define FDECL_RADIO( Struct,Field,Bits,Descr )            { 0,FT_BITS|FT_TP_RADIO,offsetof(Struct,Field), SizeOf(Struct,Field), #Field, Bits, #Bits, Descr  },
#define FDECL_FLOAT( Struct,Field,Descr )                 { 0,FT_FLOAT,offsetof(Struct,Field), SizeOf(Struct,Field), #Field, 0,   NULL,  Descr  },
#define FDECL_DOUBLE( Struct,Field,Descr )                { 0,FT_DOUBLE,offsetof(Struct,Field), SizeOf(Struct,Field), #Field, 0,   NULL,  Descr  },
#define FDECL_STR( Struct,Field,Descr )                   { 0,FT_STR,  offsetof(Struct,Field), SizeOf(Struct,Field), #Field, 0,   NULL,  Descr  },
#define FDECL_STRPTR( Struct,Field,Descr )                { 0,FT_STR|FT_TP_PTR,  offsetof(Struct,Field), SizeOf(Struct,Field), #Field, 0,   NULL,  Descr  },
#define FDECL_BIN( Struct,Field,Descr )                   { 0,FT_BIN,  offsetof(Struct,Field), SizeOf(Struct,Field), #Field, 0,   NULL,  Descr  },
#define FDECL_BINPTR( Struct,Field,Descr )                { 0,FT_BIN|FT_TP_PTR,  offsetof(Struct,Field), SizeOf(Struct,Field), #Field, 0,   NULL,  Descr  },

#endif
