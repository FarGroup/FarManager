#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

static BOOL CharInStr( const MyString& str, char ch )
  {
    for ( int n = 0; str[n]; n++ )
      if ( CMP_CHAR(str[n],ch) )
        return TRUE;
 return FALSE;
}

BOOL MYRTLEXP InPattern( CONSTSTR pattern,CONSTSTR name )
  {  int cn = StrColCount( pattern,",;");
     for ( int n = 1; n <= cn; n++ )
       if ( CmpFile(StrGetCol(pattern,n,",;"),name) )
         return TRUE;
 return FALSE;
}

BOOL MYRTLEXP CmpFile( const MyString& pattern, const MyString& _name )
  {  MyString str,name(_name);
     int      n,i;
     char     ch;

    if ( pattern.Length() == 0 ) return TRUE;
    if ( name.Length() == 0 ) return FALSE;
    if ( name.Chr('.') == -1 && pattern.Chr('.') != -1 )
      name.Add( '.' );

    for ( i = n = 0; pattern[n]; ) {
      switch( pattern[n] ) {
        case '?': i++; n++;
                break;

        case '[': n++; str = "";
                  for ( ; pattern[n] && pattern[n] != ']';n++ )
                    str.Add( pattern[n] );
                  if ( !CharInStr(str,name[i]) ) return FALSE;
                  i++; n++;
                break;

        case '*':  while(pattern[n] == '*') n++; ch = pattern[n];
                   if ( !ch ) return TRUE;
                   switch( ch ) {
                     case   0: return TRUE;
                     case '?': n++; i++; break;
                     case '[': n++; str = "";
                               for ( ; pattern[n] && pattern[n] != ']';n++ )
                                 str.Add( pattern[n] );
                               if ( !CharInStr(str,name[i]) ) return FALSE;
                               i++; n++;
                            break;
                      default:  for (; !CMP_CHAR(pattern[n],name[i]) && name[i]; i++ );
                                if ( !CMP_CHAR(pattern[n],name[i]) ) return FALSE;
                                i++; n++;
                   }
               break;

        case '\\': n++;

          default: if ( !CMP_CHAR(pattern[n],name[i]) )
                     return FALSE;
                   i++; n++;
      }
      if ( name[i] == 0 ) {
        if ( pattern[n] != 0 )
          return pattern[n] == '*' &&
                 pattern.Length()-n == 1;
         else
          return TRUE;
      }
      if ( pattern[n] == 0 )
        return name[i] == 0;
    }
 return i >= name.Length();
}
