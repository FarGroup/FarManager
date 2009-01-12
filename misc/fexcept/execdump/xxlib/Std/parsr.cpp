#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
 #pragma package(smart_init)
#endif


#define MSG_EOL "\n"

#define ERR_EXPCOLOR      "Expected color value"
#define ERR_EXPCOMMEND    "End of file reached, but \"*/\" expected"
#define ERR_EXPINT        "Expected integer value"
#define ERR_EXPNAME       "Expected name token"
#define ERR_EXPSTR        "Expected \"%s\""
#define ERR_INTCHAR       "Wrong digit character \'%c\'"
#define ERR_INTDOT        "Can not use more then one dot"
#define ERR_INTSIGN       "Can not use more then one sign character"
#define ERR_NAMEBEGIN     "Name can not begin with \'%c\'"

/***************************************
            CTParserBase
 ***************************************/
CTParserBase::CTParserBase( void )
  {
    curX = curY = 0; lastLen = 0;
    CLineComments = CBlockComments = TRUE;
}

int CTParserBase::GetX( void ) { return curX+1; }
int CTParserBase::GetY( void ) { return curY+1; }

BOOL CTParserBase::Assign( CONSTSTR /*data*/ )
  {
    curX = curY = 0;
    lastLen = 0;
 return TRUE;
}

float CTParserBase::GetFloatToken( void )
  {
 return (float)GetDoubleToken();
}

char CTParserBase::GetChar( void )
  {  char ch = GetNextChar();

     if ( !ch )
       return 0;

     if ( ch != '\n' && ch != '\r' ) {
       curX++;
       return ch;
     }

     curX = 0;
     curY++;

     char eol = GetNextChar();
     if ( (ch == '\n' && eol == '\r') ||
          (ch == '\r' && eol == '\n') )
       return '\n';

     if (eol) UnGetChar();

 return '\n';
}

void CTParserBase::UnGet( void )
  {  char ch;

     if ( !UnGetChar() ) return;
     curX--;

     ch = CurChar();
     if ( ch != '\n' && ch != '\r' )
       return;

     curY = Max( 0, curY-1 );
     curX = 0;
     if ( !UnGetChar() ) return;

     char eol = CurChar();
     if ( (ch == '\n' && eol == '\r') ||
          (ch == '\r' && eol == '\n') )
       UnGetChar();

     while( 1 ) {
       char skip = CurChar();

       if ( skip == '\n' || skip == '\r' )
         break;

       if ( !UnGetChar() )
         break;

       curX++;
     }

     while( (ch=GetNextChar()) != 0 && ch != eol )
       /**/;
}

void CTParserBase::UnGet( CONSTSTR s )
  {  char ch;
     int  n;

    if ( !s || !s[0] ) return;
    while( (ch=CurChar()) != 0 && StrChr(CT_STDSKIPS,ch) != 0 ) UnGet();
    if (!ch) return;

    for ( n = strLen(s)-1; n >= 0; n-- )
      for( ch = CurChar(); ch && ch == s[n]; ch = CurChar() ) UnGet();
}

char CTParserBase::NextGet( void )
  {  char ch = Get();
     if ( ch != 0 ) UnGet();
 return ch;
}

char CTParserBase::Get( void )
  {  char ch;
     int  n;

   GetBegin:

    while( 1 ) {
      if ( (ch=GetChar()) == 0 )
        return 0;

      if ( StrChr( CT_STDSKIPS,ch ) != 0 )
        continue;

      break;
    }

    if ( ch == '/' ) {
      switch( GetChar() ) {
        case '*': if (CBlockComments) {
                    n = 1;
                    while( n > 0 ){
                      ch = GetChar();
                      if ( !ch ) THROW( ERR_EXPCOMMEND )
                      if ( ch == '/' ) { if ( GetChar() == '*' ) n++; else UnGet(); }
                      if ( ch == '*' ) { if ( GetChar() == '/' ) n--; else UnGet(); }
                    }
                    goto GetBegin;
                  } else
                    UnGet();
                break;

        case '/': if ( CLineComments ) {
                    while( (ch=GetChar()) != 0 && ch != '\n' )
                      /**/;
                    goto GetBegin;
                  } else
                    UnGet();
                break;

         default: UnGet();
      }
    }

 return ch;
}

MyString CTParserBase::GetToken( CONSTSTR delimiter )
  {  MyString str;
     char     ch;

     if (!delimiter)
       delimiter = CT_STDDELIMITERS;

     if ( (ch=Get()) == 0 )
       return str;

     for( str.Add(ch); (ch=GetChar()) != 0 && StrChr( delimiter,ch ) == NULL; )
       str.Add( ch );

     if ( ch )
       UnGet();

 return str;
}

MyString CTParserBase::GetBefore( const MyString& token,CONSTSTR delimiter )
  {  MyString s,tmp;

    while( (tmp=GetToken(delimiter)) != token )
      s.Add( tmp );

    UnGet( tmp.Text() );

 return s;
}

MyString CTParserBase::GetExact( CONSTSTR token )
  {  MyString s;
     char   ch;

     for( int n = 0; token[n]; n++ )
       if ( token[n] == (ch=Get()) )
         s.Add( ch );
        else
         THROW( Message(ERR_EXPSTR,token) )
 return s;
}


int GetPDigit( CTParserBase& p,CONSTSTR chars,int base,char /*end*/ )
  {  char ch;
     int  val = 0;
     while( (ch=p.NextGet()) != 0 && strchr(chars,ch) ) {
       p.GetChar();
       val = val*base + (ch>'a'?(ch-'a'+10):((ch>'A')?(ch-'A'+10):(ch-'0')));
     }
 return val;
}

char CTParserBase::GetCharToken( void )
  {  char ch;

    if ( NextGet() != '\'' ) return (char)GetIntToken();
    GetExact( "\'" );
    ch = GetChar();
    if ( ch == '\\' )
      switch( GetChar() )
        {
       case  'n': ch = '\n'; break;
       case  'r': ch = '\r'; break;
       case  'b': ch = '\b'; break;
       case  't': ch = '\t'; break;
       case '\\': ch = '\\'; break;
       case '\'': ch = '\''; break;
       case '\"': ch = '\"'; break;
        case 'x': ch = (char)GetPDigit(*this,"0123456789ABCDEFabcdef",16,'\''); break;
        case '0': if ( NextGet() == 'x' ) {
                    GetChar();
                    ch = (char)GetPDigit(*this,"0123456789ABCDEFabcdef",16,'\''); break;
                  } else {
                    ch = (char)GetPDigit(*this,"01234567",8,'\''); break;
                  }
        }
    GetExact( "\'");
 return ch;
}

MyString CTParserBase::GetStringToken( void )
  {  MyString str;
     char   ch;

QuotedString:
    GetExact( "\"");
    while( (ch=GetChar()) != '\"' && ch )
      if ( ch == '\\' )
        switch( GetChar() )
          {
         case  'n': str.Add( '\n' ); break;
         case  'r': str.Add( '\r' ); break;
         case  'b': str.Add( '\b' ); break;
         case  't': str.Add( '\t' ); break;
         case '\\': str.Add( '\\' ); break;
         case '\'': str.Add( '\'' ); break;
         case '\"': str.Add( '\"' ); break;
          case 'x': str.Add( (char)GetPDigit(*this,"0123456789ABCDEFabcdef",16,'\'') ); break;
          case '0': if ( NextGet() == 'x' ) {
                      GetChar();
                      str.Add( (char)GetPDigit(*this,"0123456789ABCDEFabcdef",16,'\'') ); break;
                    } else {
                      str.Add( (char)GetPDigit(*this,"01234567",8,'\'') ); break;
                    }
           default: str.Add( CurChar() );
          }
       else
        str.Add( ch );
    ch = NextGet();
    if ( ch == '\"' ) goto QuotedString;
    if ( ch == '\\' ) {
      GetChar();
      if ( NextGet() != '\"' ) THROW( "Bad string expend" )
      goto QuotedString;
    }
 return str;
}

DWORD CTParserBase::GetIntToken( void )
  {  MyString str;

     if ( NextGet() == '\'' ) return GetCharToken();

     str = GetToken( CT_STDDELIMITERSWOSIGN );

     if ( str.Length() == 0 )
       THROW( ERR_EXPINT )

     if ( str == "TRUE"  || str == "TRUE" )  return TRUE;
     if ( str == "FALSE" || str == "FALSE" ) return FALSE;
     if ( str == "NULL" )                    return 0;

 return Str2DigitDetect( str.Text(), 10, 0 );
}

double CTParserBase::GetDoubleToken( void )
  {  MyString str;
     BOOL   sign = FALSE,
            dot = FALSE;
     int    n;

     str = GetToken( CT_STDFLOATDELIMITERSWOSIGN );
     if ( str.Length() == 0 ) THROW( ERR_EXPINT )

     for ( n = 0; n < str.Length(); n++ )
       switch( str[n] )
         {
        case '.': if ( dot ) THROW( ERR_INTDOT )
                  dot = TRUE;
               break;
        case '-': if ( sign ) THROW( ERR_INTSIGN )
                  sign = TRUE;
               break;
         default: if ( !isdigit(str[n]) )
                    THROW( Message( ERR_INTCHAR,str[n] ) )
         }
 return atof( str.Text() );
}

MyString CTParserBase::GetNameToken( void )
  {  MyString name;
    name = GetToken(CT_STDDELIMITERS);
    if ( name.Length() == 0 ) THROW( ERR_EXPNAME )
    if ( name[0] != '_' && !isalpha(name[0]) ) THROW( Message( ERR_NAMEBEGIN,name[0] ) )
 return name;
}

int CTParserBase::GetTypeToken( CONSTSTR *str,BOOL isCase,CONSTSTR dels,MyString *token )
  {  MyString s( GetToken( dels ) );

     if ( token ) *token = s;

     for ( int n = 0; str[n]; n++ )
       if ( StrCmp(s.Text(),str[n],-1,isCase) == 0 )
         return n;

 return -1;
}

CONSTSTR stdColorTokens[] = {
  "clBlack", "clBlue",    "clGreen",  "clCyan",
  "clRed",   "clMagenta", "clYellow", "clWhite"
};

WORD CTParserBase::GetColorToken( void )
  {  int fg,bk;

    fg = GetTypeToken( stdColorTokens,TRUE,CT_STDDELIMITERS );
    if ( fg < 0 || fg > 7 ) THROW( ERR_EXPCOLOR )
    GetExact( "/" );
    bk = GetTypeToken( stdColorTokens,TRUE,CT_STDDELIMITERS );
    if ( bk < 0 || bk > 7 ) THROW( ERR_EXPCOLOR )
    if ( NextGet() == '+' ) {
      GetChar();
      return (WORD)CLRH( fg,bk );
    } else
      return (WORD)CLR( fg,bk );
}

char CTParserBase::GetTo( char ender,char beginer,MyString *place )
  {  char ch;
     int  level = 1;

   if ( place ) *place = "";

   while( (ch=GetChar()) != 0 && level > 0  ) {
     if ( beginer && ch == beginer ) level++;
     if ( ch == ender )   level--;
     if (place && level) place->Add(ch);
   }
 return ch;
}

char CTParserBase::GetTo( CONSTSTR ender, CONSTSTR beginer, MyString *place )
  {  char ch;
     int level = 1;

   if (place) *place = "";

   while( (ch=GetChar()) != 0 && level > 0 ) {
    if ( beginer && StrChr(beginer,ch) ) level++;
    if ( StrChr(ender,ch) )              level--;
    if ( place && level )                place->Add(ch);
   }
 return ch;
}

void CTParserBase::GetToEOL( PMyString s )
  {  char ch;
     int  n = curY;
    if ( s ) *s = "";
    while( (ch=GetChar()) != 0 && n == curY ) {
      if ( s ) s->Add( ch );
    }
    if (ch) UnGet();
}
