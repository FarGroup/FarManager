#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

/*******************************************************************
   HValue - holder for all possible data types in one place
 *******************************************************************/
HValue::HValue( void ) {  Empty(); }
HValue::~HValue()      { }

void   HValue::DoEmpty( void )            {}
DWORD  HValue::DoSizeofData( void ) const { return Sizeof(); }
LPVOID HValue::DoPtrData( void )    const { return Ptr(); }
void   HValue::DoTypeChange( HVType /*oldT*/,HVType /*newT*/,LPVOID /*val*/ ) {}

BOOL   HValue::isSimple( HVType Type )    { return (Type >= HVByte   && Type <= HVDouble); }
BOOL   HValue::isReal( HVType Type )      { return (Type >= HVFloat  && Type <= HVDouble); }
BOOL   HValue::isFixed( HVType Type )     { return (Type >= HVByte   && Type <= HVInt); }
BOOL   HValue::isVariable( HVType Type )  { return Type == HVHandle || Type == HVString; }
BOOL   HValue::isSetted( HVType Type )    { return Type != 0; }
BOOL   HValue::isEmpty( HVType Type )     { return Type == 0; }

BOOL     HValue::WriteToBuff( LPVOID Buff,DWORD sz )        const { return WriteToBuff(this,Buff,sz); }
BOOL     HValue::ReadFromBuff( const LPVOID Buff,DWORD sz )       { return ReadFromBuff(this,Buff,sz); }

DWORD    HValue::Sizeof( void )     const { return Sizeof(Type); }
LPVOID   HValue::Ptr( void )        const { return (LPVOID)(&Value); }
void     HValue::Empty( void )            { DoEmpty(); Value._Double = 0; Type = 0; }
LPVOID   HValue::Data( void )       const { return DoPtrData(); }
DWORD    HValue::DataSize( void )   const { return DoSizeofData(); }

BOOL     HValue::isSimple( void )   const { return isSimple(Type); }
BOOL     HValue::isReal( void )     const { return isReal(Type); }
BOOL     HValue::isFixed( void )    const { return isFixed(Type); }
BOOL     HValue::isVariable( void ) const { return isVariable(Type); }
BOOL     HValue::isSetted( void )   const { return isSetted(Type); }
BOOL     HValue::isEmpty( void )    const { return isEmpty(Type); }

BYTE     HValue::GetByte( void )    const { return (BYTE)Value._Dword; }
char     HValue::GetChar( void )    const { return (char)Value._Dword; }
WORD     HValue::GetWord( void )    const { return (WORD)Value._Dword; }
short    HValue::GetShort( void )   const { return (short)Value._Dword; }
DWORD    HValue::GetDword( void )   const { return (DWORD)Value._Dword; }
int      HValue::GetInt( void )     const { return (int)Value._Dword; }
float    HValue::GetFloat( void )   const { return (float)Value._Double; }
double   HValue::GetDouble( void )  const { return (double)Value._Double; }
HANDLE   HValue::GetHandle( void )  const { return (HANDLE)Value._Dword; }
CONSTSTR HValue::GetString( void )  const { return (CONSTSTR)Value._Dword; }

#define HVSETTYPE( tp )  if (Type != tp) DoTypeChange(Type,tp,&p); Type = tp;
BYTE     HValue::SetByte( BYTE p )       { HVSETTYPE( HVByte )   Value._Dword  = (DWORD)p;  return p; }
char     HValue::SetChar( char p )       { HVSETTYPE( HVChar )   Value._Dword  = (DWORD)p;  return p; }
WORD     HValue::SetWord( WORD p )       { HVSETTYPE( HVWord )   Value._Dword  = (DWORD)p;  return p; }
short    HValue::SetShort( short p )     { HVSETTYPE( HVShort )  Value._Dword  = (DWORD)p;  return p; }
DWORD    HValue::SetDword( DWORD p )     { HVSETTYPE( HVDword )  Value._Dword  = (DWORD)p;  return p; }
int      HValue::SetInt( int p )         { HVSETTYPE( HVInt )    Value._Dword  = (DWORD)p;  return p; }
float    HValue::SetFloat( float p )     { HVSETTYPE( HVFloat )  Value._Double = (double)p; return p; }
double   HValue::SetDouble( double p )   { HVSETTYPE( HVDouble ) Value._Double = p;         return p; }
HANDLE   HValue::SetHandle( HANDLE p )   { HVSETTYPE( HVHandle ) Value._Dword  = (DWORD)p;  return p; }
CONSTSTR HValue::SetString( CONSTSTR p ) { HVSETTYPE( HVString ) Value._Dword  = (DWORD)p;  return p; }

DWORD HValue::Sizeof( HVType tp )
  {
    switch( tp ) {
      case   HVChar: return sizeof(char);
      case   HVByte: return sizeof(BYTE);
      case  HVShort: return sizeof(short);
      case   HVWord: return sizeof(WORD);
      case    HVInt: return sizeof(int);
      case  HVDword: return sizeof(DWORD);
      case  HVFloat: return sizeof(float);
      case HVDouble: return sizeof(double);
      case HVHandle: return sizeof(HANDLE);
      case HVString: return sizeof(CONSTSTR);
    }
  return 0;
}

BOOL HValue::ReadFromBuff( PHValue p,const LPVOID Buff,DWORD sz )
  {
    if ( !Buff || !sz ) return FALSE;
    switch( p->Type ) {
      case   HVChar:
      case   HVByte: if (sz < sizeof(BYTE))   return FALSE; p->Value._Dword  = (DWORD)(*((BYTE*)Buff));    break;
      case  HVShort: if (sz < sizeof(short))  return FALSE; p->Value._Dword  = (DWORD)(*((short*)Buff));   break;
      case   HVWord: if (sz < sizeof(WORD))   return FALSE; p->Value._Dword  = (DWORD)(*((WORD*)Buff));    break;
      case    HVInt: if (sz < sizeof(int))    return FALSE; p->Value._Dword  = (DWORD)(*((int*)Buff));     break;
      case HVString: if (sz < sizeof(char*))  return FALSE; p->Value._Dword  = (DWORD)(*((char**)Buff));   break;
      case HVHandle: if (sz < sizeof(HANDLE)) return FALSE; p->Value._Dword  = (DWORD)(*((HANDLE*)Buff));  break;
      case  HVDword: if (sz < sizeof(DWORD))  return FALSE; p->Value._Dword  = (DWORD)(*((DWORD*)Buff));   break;
      case  HVFloat: if (sz < sizeof(float))  return FALSE; p->Value._Double = (double)(*((float*)Buff));  break;
      case HVDouble: if (sz < sizeof(double)) return FALSE; p->Value._Double = (double)(*((double*)Buff)); break;
            default: return FALSE;
    }
 return TRUE;
}

BOOL HValue::WriteToBuff( const HValue *p,LPVOID Buff,DWORD sz )
  {
    if ( !Buff || !sz ) return FALSE;
    switch( p->Type ) {
      case   HVChar:
      case   HVByte: if (sz < sizeof(BYTE))   return FALSE; *((BYTE*)Buff)   = (BYTE)p->Value._Dword;   break;
      case  HVShort: if (sz < sizeof(short))  return FALSE; *((short*)Buff)  = (short)p->Value._Dword;  break;
      case   HVWord: if (sz < sizeof(WORD))   return FALSE; *((WORD*)Buff)   = (WORD)p->Value._Dword;   break;
      case HVHandle: if (sz < sizeof(HANDLE)) return FALSE; *((HANDLE*)Buff) = (HANDLE)p->Value._Dword; break;
      case HVString: if (sz < sizeof(char*))  return FALSE; *((char**)Buff)  = (char*)p->Value._Dword;  break;
      case    HVInt: if (sz < sizeof(int))    return FALSE; *((int*)Buff)    = (int)p->Value._Dword;    break;
      case  HVDword: if (sz < sizeof(DWORD))  return FALSE; *((DWORD*)Buff)  = p->Value._Dword;         break;
      case  HVFloat: if (sz < sizeof(float))  return FALSE; *((float*)Buff)  = (float)p->Value._Double; break;
      case HVDouble: if (sz < sizeof(double)) return FALSE; *((double*)Buff) = p->Value._Double;        break;
            default: return FALSE;
    }
 return TRUE;
}
