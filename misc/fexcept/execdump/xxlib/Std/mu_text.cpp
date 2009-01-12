#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

static MyString resStrText;

int MYRTLEXP AttrStrLen( char *str )
  {
     int  n,count = 0;
     for ( n = 0; str[n]; n++ )
      if ( str[n] != '|' ) count++;
 return count;
}

CONSTSTR MYRTLEXP MakeOuttedStr( char *dest,CONSTSTR source,int maxlen )
  {  int i,n;

    for ( i = 0,n = 0; source[n] && i < maxlen; i++,n++ ) {
      if ( source[n] == '~' ) dest[i++] = '~';
      dest[i] = source[n];
    }
    dest[i] = 0;
 return dest;
}

CONSTSTR MYRTLEXP Str2Text( CONSTSTR name )
  {  int    n;

    resStrText = "";
    for ( n = 0; name[n]; n++ )
      switch( name[n] ) {
        case '\\': resStrText.Add('\\'); resStrText.Add('\\'); break;
        case '\t': resStrText.Add('\\'); resStrText.Add('t'); break;
        case '\n': resStrText.Add('\\'); resStrText.Add('n'); break;
        case '\r': resStrText.Add('\\'); resStrText.Add('r'); break;
        case '\b': resStrText.Add('\\'); resStrText.Add('b'); break;
        case '\'': resStrText.Add('\\'); resStrText.Add('\''); break;
        case '\"': resStrText.Add('\\'); resStrText.Add('\"'); break;
          default: resStrText.Add( name[n] );
      }
 return resStrText.Text();
}

CONSTSTR MYRTLEXP Text2Str( CONSTSTR val1 )
  {  int n;

    resStrText = "";
    for ( n = 0; val1[n]; n++ )
      if ( val1[n] == '\\' ) {
        n++;
        switch( val1[n] ) {
          case '\\': resStrText.Add('\\'); break;
          case '\'': resStrText.Add('\''); break;
          case '\"': resStrText.Add('\"'); break;
          case  't': resStrText.Add('\t'); break;
          case  'n': resStrText.Add('\n'); break;
          case  'r': resStrText.Add('\r'); break;
          case  'b': resStrText.Add('\b'); break;
            default: resStrText.Add('\\'); resStrText.Add(val1[n]);
        }
      } else
        resStrText.Add( val1[n] );
 return resStrText.Text();
}

DWORD MYRTLEXP Text2Bytes( CONSTSTR text, LPBYTE buff, DWORD bsz )
  {
     if ( !text || !text[0] || !buff || !bsz )
       return 0;

     DWORD cn;
     char  ch;
     char  str[ 3 ] = "  ";
     for( cn = 0; cn < bsz; cn++,text++ ) {
       if ( (ch=text[0]) == 0 )
         break;

       if ( ch != '\\' ) {
         buff[ cn ] = ch;
         continue;
       }

       if ( (ch = text[1]) == 0 )
         break;
       text++;

       switch( ch ) {
         case 'n': buff[cn] = '\n'; break;
         case 'r': buff[cn] = '\r'; break;
         case 'b': buff[cn] = '\b'; break;
         case 't': buff[cn] = '\r'; break;
         case '\"': buff[cn] = '\"'; break;
         case '\'': buff[cn] = '\''; break;

         case 'x':  text++; str[0] = *text;
                    text++; str[1] = *text;
                    buff[cn] = Str2Digit( str, 16, 0xFF );
                break;

         default:  buff[cn++] = '\\';
                   buff[cn]   = ch;
                break;
       }
     }

 return cn;
}
