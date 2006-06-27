#include <all_far.h>
#pragma hdrstop

#include "fstdlib.h"

/************************************
            String
 ************************************/
String::~String()                  { if (str) { _Del(str); str = NULL; } }

String::String( void )             { BeginSet(); }
String::String( const String& s )  { BeginSet( s.Length() ); Alloc( s.c_str() ); }
String::String( size_t def_size )  { BeginSet( def_size ); }
String::String( CONSTSTR fmt )
  {
     if ( !fmt || !fmt[0] )
       BeginSet();
      else {
       int sz = strLen(fmt);
       BeginSet( Max( DEF_STR_ALLOC, sz+1 ) );
       StrCpy( str, fmt, maxchar );
       len = sz;
     }
}

int String::printf( CONSTSTR fmt,... )
  {  va_list  a;
     int      res;

     va_start( a,fmt );
       res = vprintf( fmt,a );
     va_end( a );
 return res;
}

int String::vprintf( CONSTSTR fmt,va_list list )
  {  int sz = VSNprintf( NULL,0,fmt,list );
     Alloc( sz+1 );
 return len = VSNprintf( str,maxchar,fmt,list );
}

void String::BeginSet( size_t sz )
  {
   str = (char*)_Alloc( maxchar = Max( (size_t)DEF_STR_ALLOC, sz ) );
   str[len = 0] = 0;
}

CONSTSTR String::Alloc( CONSTSTR s,int maxLen )
  {
     if ( maxLen == -1 )
       maxLen = strLen(s);

     Alloc( maxLen+1 );

     if ( !s || !maxLen ) {
       str[len = 0] = 0;
       return c_str();
     }

     strncpy( str, s, maxLen );
     str[ maxLen ] = 0;
     len = strLen(str);

 return c_str();
}

CONSTSTR String::Alloc( int t )
  {
     if ( t < maxchar )
       return str;

     str = (char*)_Realloc( str,maxchar = Max(DEF_STR_ALLOC,t) );
 return str;
}

String& String::Add( const String& s )
  {
    Alloc( Length() + s.Length() + 1 );
    strcpy( str+Length(), s.c_str() );
    len += s.Length();
 return *this;
}

String& String::Add( CONSTSTR s )
  {  int slen = strLen(s);

     if ( slen ) {
       Alloc( Length() + slen + 1 );
       strcpy( str+Length(),s );
       len += slen;
     }
 return *this;
}

String& String::Add( CONSTSTR s, int from, int to /*-1*/ )
  {
    if ( to <= from )
      return *this;

    if ( to == -1 )
      return Add( s+from );

    Alloc( len+(to-from+1) );
    StrCpy( str+len, s+from, to-from );

    len = strLen( str );

 return *this;
}

String& String::Add( char ch )
  {
    if ( ch == 0 )
      return *this;

    if ( len + 1 >= maxchar )
      Alloc( len + 10 );

    str[len++] = ch;
    str[len] = 0;
 return *this;
}

void String::cat( CONSTSTR s,... )
  {  va_list a;

     va_start( a, s );
       vcat( s,a );
     va_end( a );
}

void String::vcat( CONSTSTR s,va_list a )
  {  int slen, clen;

     if ( !s || !s[0] ) return;

     slen = VSNprintf( NULL,0,s,a );
     if ( !slen ) return;

     clen = Length();

     if ( !Alloc( clen + slen + 1 ) ) return;

     vsprintf( str+clen,s,a );
     len += slen;
}


void String::InsCharPos( int pos,char ch )
  {
    if ( !str ) pos = 0;
    pos = Max(Min(pos,len),0);
    Alloc( Length()+1 );
    if (pos < len) MemMove( str+pos+1,str+pos,len-pos );
    str[pos]   = ch;
    str[++len] = 0;
}

void String::Del( int pos, int cn )
  {
    if ( !cn || pos >= len ) return;

    cn = Min( len-pos, cn );
    MemMove( str+pos, str+pos+cn, len-pos-cn );
    len -= cn;
    str[len] = 0;
}

void String::DelChars( char ch )
  {
    if ( str ) {
      for ( int n = 0; str[n]; n++ )
        if ( str[n] == ch ) {
          MemMove( str+n,str+n+1,len-n );
          len--;
        }
      str[len] = 0;
    }
}

int String::RChr( char ch,int pos ) const
  {
     if ( !str || !str[0] )
       return -1;

     if ( pos == -1 )
       pos = Length()-1;

     for( pos = Max(0,Min(Length(),pos));
          pos && str[pos] != ch;
          pos-- );
 return pos;
}

char String::SetChar( int num,char ch )
  {
    if ( !str || num < 0 || num > maxchar )
      return 0;

    str[num] = ch;
    if ( ch && num == len )
      str[++len] = 0;
     else
    if ( !ch && num < len )
      len = strLen(str);

 return ch;
}

int String::Chr( CONSTSTR ch,int pos ) const
  {
     if ( pos >= len )
       return -1;

     for( int n = pos; n < len; n++ )
       if ( strchr( ch,str[n] ) != NULL )
         return n;
 return -1;
}

int       String::Chr( char ch,int pos )         const { return StrPosChr( c_str(),ch,pos ); }
int       String::Str( CONSTSTR s,int pos )      const { return StrPosStr( c_str(),s,pos ); }

String&   String::operator=( const String& s )         { Alloc( s.c_str() ); return *this; }
String&   String::operator=( CONSTSTR s )              { Alloc( s ); return *this; }
BOOL      String::operator!=( const String& s )  const { return len != s.len || !Cmp(s.c_str()); }
BOOL      String::operator!=( CONSTSTR s )       const { return !Cmp(s); }
BOOL      String::operator==( const String& s )  const { return len == s.len && Cmp(s.c_str()); }
BOOL      String::operator==( CONSTSTR s )       const { return Cmp( s ); }
char      String::operator[]( int num )          const { return (num >= 0 && num <= len && str) ? str[num] : '\0'; }

void String::SetLength( int sz )
  {
     if (sz >= 0 && sz < maxchar) {
       len     = sz;
       str[sz] = 0;
     }
}

CONSTSTR String::Set( CONSTSTR s, int from, int to )
  {
    if ( to == -1 ) {
      Alloc( s+from );
      return c_str();
    } else
    if ( to <= from )
      return c_str();

    Alloc( to-from+1 );
    StrCpy( str, s+from, to-from+1 );
 return c_str();
}

BOOL String::Cmp( CONSTSTR s,int count, BOOL isCase ) const
  {
    if ( !s )
      return FALSE;

    if ( !s[0] )
      return str[0] == 0;

    if ( count < 0 )
      return strcmp( c_str(),s ) == 0;
     else
      return StrCmp( c_str(),s,count,isCase ) == 0;
}

void String::LTrim( char ch )
  {  int      n;

     if ( len == 0 ) return;

     for ( n = 0; str[n] && str[n] == ch; n++ );
     if ( n ) MemMove( str,str+n,len-n );
     len-=n; str[len] = 0;
}

void String::RTrim( char ch )
  {  int      n;

     if ( len == 0 ) return;

     for ( n = len-1; n >= 0 && str[n] == ch; len--,n-- )
       str[n] = 0;
}

void String::Trim( char ch )
  {  int      n;

     if ( len == 0 ) return;

     for ( n = 0; str[n] && str[n] == ch; n++ );
     if ( n ) MemMove( str,str+n,len-n );
     len-=n; str[len] = 0;
     for ( n = len-1; n >= 0 && str[n] == ch; len--,n-- )
       str[n] = 0;
}
