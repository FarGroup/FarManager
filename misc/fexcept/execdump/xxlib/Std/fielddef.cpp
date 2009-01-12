#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

#include <Std/fielddef.h>

union HFieldType {
  DWORD  IVal;
  float  FVal;
  double DVal;
  char  *SVal;
  LPBYTE BVal;
};

// Ret name or NULL on error
static CONSTSTR DLL_CALLBACK HFieldBitNameByNumber( PHFieldDef Source,int BitNumber )
  {  DWORD    val = 1UL << BitNumber;
     int      n,i;
     CONSTSTR m;

    if ( !Source->Range || !IS_FLAG(Source->Mask,val) ) return NULL;

    for ( i = 0,n = 0; n < 32; n++ ) {
      val = 1UL << n;
      if ( IS_FLAG(Source->Mask,val) ) {
        if ( BitNumber == n ) {
          m = StrGetCol( Source->Range,i+1,";|" );
          return (m && m[0])?m:NULL;
        }
        i++;
      }
    }
 return NULL;
}

//Ret NULL or Range name on error
static CONSTSTR DLL_CALLBACK HFieldCheckValueInt( PHFieldDef Source,DWORD Value )
  {  int      cn = (Source->Range)?StrColCount(Source->Range,";"):0,
              n;
     CONSTSTR m;
     char     str[50];

     for ( n = 1; n <= cn; n++ ) {
       m = StrGetCol( Source->Range,n,";" );
       if ( !m || !m[0] ) continue;
       if ( StrChr(m,'-') == NULL ) {
         if ( Value == (DWORD)AtoI(m) ) return NULL;
       } else {
         StrCpy( str,m,sizeof(str) );
         if ( Value < (DWORD)AtoI(StrGetCol(str,1,"-")) ) continue;
         if ( (m=StrGetCol(str,2,"-")) != NULL && m[0] && Value > (DWORD)AtoI(m) ) continue;
         return NULL;
       }
     }
 return (cn==0)?NULL:Source->Range;
}

static BOOL MYRTLEXP DoEnumItem( PHFieldDef def,LPBYTE Data,PHFieldUserEnum cb )
  {  HFieldType Type;
     void      *ptr;
     BOOL       res;

     MemSet( &Type,0,sizeof(Type) );
     if ( IS_FLAG(def->Type,FT_TP_PTR) ) {
       ptr = *(void**)(Data+def->Offset);
       if ( !ptr ) {
         if ( cb->EnumNullPtr && !cb->EnumNullPtr(def,def->Name,def->Description) ) return FALSE;
         return TRUE;
       }
     } else
       ptr = Data+def->Offset;

     if ( (def->Type&FT_TYPEMASK) < FT_STR )
       MemCpy( &Type,ptr,def->Size );
      else
       Type.SVal = (char*)ptr;

     switch( def->Type&FT_TYPEMASK ) {
       case    FT_INT: res = !cb->EnumInt     ||
                             cb->EnumInt( def,&Type.IVal,def->Name,def->Description,HFieldCheckValueInt );
                  break;
       case   FT_BITS: res = !cb->EnumBits    ||
                             cb->EnumBits( def,&Type.IVal,def->Mask,IS_FLAG(def->Type,FT_TP_RADIO),
                                           def->Name,def->Description,HFieldBitNameByNumber );
                  break;
       case  FT_FLOAT: res = !cb->EnumFloat   ||
                             cb->EnumFloat( def,&Type.FVal,def->Name,def->Description );
                  break;
       case FT_DOUBLE: res = !cb->EnumDouble  ||
                             cb->EnumDouble( def,&Type.DVal,def->Name,def->Description );
                  break;
       case    FT_STR: res = !cb->EnumString  ||
                             cb->EnumString( def,Type.SVal,
                                             IS_FLAG(def->Type,FT_TP_PTR)?0:def->Size,
                                             def->Name,def->Description );
                  break;
       case    FT_BIN: res = !cb->EnumBinary  ||
                             cb->EnumBinary( def,Type.BVal,
                                             IS_FLAG(def->Type,FT_TP_PTR)?0:def->Size,
                                             def->Name,def->Description );
                  break;
              default: res = !cb->EnumUnkType ||
                             cb->EnumUnkType( def,def->Type,def->Name,def->Description );
     }
     if (!res) return FALSE;
     if ( (def->Type&FT_TYPEMASK) < FT_STR ) MemCpy( ptr,&Type,def->Size );
 return TRUE;
}
int MYRTLEXP HFieldEnum( PHFieldDef def,LPBYTE Data,PHFieldUserEnum cb )
  {  int n;

     if ( !def || !Data || !cb ) return 0;

     for ( n = 0; def[n].Type; n++ ) {
       def[n].Index = (WORD)n;
       if ( !DoEnumItem( &def[n],Data,cb ) ) return n;
     }
 return -1;
}
BOOL MYRTLEXP HFieldItemEnum( PHFieldDef def,LPBYTE Data,PHFieldUserEnum cb,int Index )
  {  int n;

     if ( !def || !Data || !cb ) return FALSE;

     for ( n = 0; def[n].Type; n++ ) {
       def[n].Index = (WORD)n;
       if ( n == Index ) return DoEnumItem( &def[n],Data,cb );
     }
 return FALSE;
}
