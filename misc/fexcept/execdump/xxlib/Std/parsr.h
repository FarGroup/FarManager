#ifndef __LEXEM_SPLITTER
#define __LEXEM_SPLITTER

#define CT_STDDELIMITERSONLY           "<>()[]{}+-*/\\,.;:=%|!&^~?\"\'"
#define CT_STDFLOATDELIMITERSONLY      "<>()[]{}+-*/\\,;:=%|!&^~?\"\'"
#define CT_STDDELIMITERS               CT_STDSKIPS CT_STDDELIMITERSONLY
#define CT_STDFLOATDELIMITERS          CT_STDSKIPS CT_STDFLOATDELIMITERSONLY
#define CT_STDDELIMITERSWOSIGN         CT_STDSKIPS "<>()[]{}*/\\,.;:=%|!&^~?\"\'"
#define CT_STDFLOATDELIMITERSWOSIGN    CT_STDSKIPS "<>()[]{}*/\\,;:=%|!&^~?\"\'"
#define CT_STDSKIPS                    " \t\n\r"

/***************************************
            CTParserBase
 ***************************************/
CLASS( CTParserBase )
 protected:
  int          curX,curY;
  int          lastLen;
 public:
  BOOL         CLineComments;
  BOOL         CBlockComments;
 protected:
   virtual char GetNextChar( void )   = 0;
   virtual BOOL UnGetChar( void )     = 0;
 public:
   CTParserBase( void );

   int     GetX( void );
   int     GetY( void );

   virtual char CurChar( void )         = 0;
   virtual BOOL Assign( CONSTSTR data );

   void     UnGet( CONSTSTR s );
   void     UnGet( void );
   char     GetChar( void );
   char     Get( void );
   char     NextGet( void );
   MyString GetToken( CONSTSTR delimiter = NULL );
   MyString GetExact( CONSTSTR str );
   MyString GetBefore( const MyString& token, CONSTSTR delimiter = NULL );
   char     GetTo( char ender, char beginer = 0, MyString *place = NULL );
   char     GetTo( CONSTSTR ender, CONSTSTR beginer = NULL, MyString *place = NULL );
   void     GetToEOL( PMyString s = NULL);

   char     GetCharToken( void );
   MyString GetStringToken( void );
   DWORD    GetIntToken( void );
   double   GetDoubleToken( void );
   float    GetFloatToken( void );
   MyString GetNameToken( void );
   int      GetTypeToken( CONSTSTR *str,BOOL isCase,CONSTSTR dels = NULL,MyString *token = NULL );
   WORD     GetColorToken( void );                                                      // ret`s MK_WORD( cl,bk )
};

/***************************************
            CTParser (work w file)
 ***************************************/
LOCALCLASSBASE( CTParser, public CTParserBase )
  DWORD        pos;
  CTFileHolder file;
 protected:
   virtual char GetNextChar( void )     { return (file.Chr(pos))?file.Chr(pos++):'\x0'; }
   virtual BOOL UnGetChar( void )       { if (pos) { pos--; return TRUE; } else return FALSE; }
 public:
   CTParser( void )                     { pos = 0; }

   CONSTSTR FName( void )               { return file.GetName(); }

   virtual BOOL Assign( CONSTSTR data ) { pos = 0; if (!file.Assign( data,0 )) return FALSE; return CTParserBase::Assign(data); }
   virtual char CurChar( void )         { return file.Chr(pos); }
};

/***************************************
            CTStringParser
 ***************************************/
LOCALCLASSBASE( CTStringParser, public CTParserBase )
  char  *curLine;
  int    pos;
 protected:
   virtual char GetNextChar( void )     { if ( CurChar() ) { curX++; return curLine[pos++]; } else return 0; }
   virtual BOOL UnGetChar( void )       { if (pos) { pos--; return TRUE; } else return FALSE; }
 public:
   CTStringParser( void )               { curLine = NULL; pos = 0; }

   virtual BOOL Assign( CONSTSTR data ) { curLine = (char*)data; pos = 0; return CTParserBase::Assign(data); }
   virtual char CurChar( void )         { return (char)((curLine)?curLine[pos]:0); }
};

#endif