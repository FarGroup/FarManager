#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

#define DECL_ST( tp )        if ( isFixed() )        return (tp)(Value._Dword);  else                                      \
                             if ( isReal() )         return (tp)(Value._Double); else                                      \
                             if ( Type == HVHandle ) return (tp)( (CheckReadable( (LPVOID)GetHandle(),sizeof(tp)))?(*((tp*)GetHandle())):0 ); else
#define DECL_END(tp)         return (tp)0;
#define DECL_SIMPLE( tp )    DECL_ST( tp ) \
                             if ( Type == HVString ) return (tp)Str2DigitDetect(GetString(),10,(tp)0); else  \
                             DECL_END(tp)
#define DECL_REAL( tp )      DECL_ST( tp ) \
                             if ( Type == HVString ) return (tp)atof(GetString()); else  \
                             DECL_END(tp)

/*******************************************************************
   HValue      - descedant to meke type conversion
 *******************************************************************/
BYTE   HValue::ConvertAsByte( void )   { DECL_SIMPLE( BYTE ) }
char   HValue::ConvertAsChar( void )   { DECL_SIMPLE( char ) }
WORD   HValue::ConvertAsWord( void )   { DECL_SIMPLE( WORD ) }
short  HValue::ConvertAsShort( void )  { DECL_SIMPLE( short ) }
DWORD  HValue::ConvertAsDword( void )  { DECL_SIMPLE( DWORD ) }
int    HValue::ConvertAsInt( void )    { DECL_SIMPLE( int ) }
float  HValue::ConvertAsFloat( void )  { DECL_REAL( float ) }
double HValue::ConvertAsDouble( void ) { DECL_REAL( double ) }

HANDLE HValue::ConvertAsHandle( void ) { return (HANDLE)Ptr(); }

CONSTSTR HValue::ConvertAsString( void )
  {  static char str[ 50 ];
 return ConvertAsString( str,sizeof(str) );
}

CONSTSTR HValue::ConvertAsString( char *Buff,int BuffSize )
  {
     if ( !Buff || !BuffSize ) return "";

     switch( Type ) {
       case   HVByte: if (BuffSize < 4)  return ""; SNprintf( Buff, BuffSize, "0x%02X",GetByte() ); break;
       case   HVChar: if (BuffSize < 3)  return ""; SNprintf( Buff, BuffSize, "\'%c\'",GetChar() ); break;
       case   HVWord: if (BuffSize < 6)  return ""; SNprintf( Buff, BuffSize, "0x%04X",GetWord() ); break;
       case  HVShort: if (BuffSize < 6)  return ""; SNprintf( Buff, BuffSize, "%d",GetShort() ); break;
       case  HVDword: if (BuffSize < 10) return ""; SNprintf( Buff, BuffSize, "0x%08X",GetDword() ); break;
       case    HVInt: if (BuffSize < 10) return ""; SNprintf( Buff, BuffSize, "%d",GetInt() ); break;
       case  HVFloat: if (BuffSize < 10) return ""; SNprintf( Buff, BuffSize, "%f",GetFloat() ); break;
       case HVDouble: if (BuffSize < 10) return ""; SNprintf( Buff, BuffSize, "%lf",GetDouble() ); break;
       case HVString: StrCpy( Buff, GetString(), BuffSize ); break;
             default: Buff[0] = 0;
     }
 return Buff;
}

BOOL HValue::isDirectConvertible( HVType to )
  {
 return isDirectConvertible( Type, to );
}

BOOL HValue::isDirectConvertible( HVType from, HVType to )
  {
     switch( from ) {
       case  HVVoid: return FALSE;

       case  HVByte:
       case  HVChar:
       case  HVWord:
       case HVShort:
       case HVDword:
       case   HVInt: switch( to ) {
                      case   HVByte:
                      case   HVChar:
                      case   HVWord:
                      case  HVShort:
                      case  HVDword:
                      case    HVInt: return TRUE;

                      case  HVFloat:
                      case HVDouble:
                      case   HVVoid:
                      case HVHandle:
                      case HVString:
                            default: return FALSE;
                    }

       case  HVFloat:
       case HVDouble: switch( to ) {
                      case  HVFloat:
                      case HVDouble: return TRUE;

                      case   HVByte:
                      case   HVChar:
                      case   HVWord:
                      case  HVShort:
                      case  HVDword:
                      case    HVInt:
                      case   HVVoid:
                      case HVHandle:
                      case HVString:
                            default: return FALSE;
                    }

       case HVHandle:
       case HVString: switch( to ) {
                      case HVHandle:
                      case HVString: return TRUE;

                      case   HVByte:
                      case   HVChar:
                      case   HVWord:
                      case  HVShort:
                      case  HVDword:
                      case    HVInt:
                      case   HVVoid:
                      case  HVFloat:
                      case HVDouble:
                            default: return FALSE;
                    }
     }

 return FALSE;
}
