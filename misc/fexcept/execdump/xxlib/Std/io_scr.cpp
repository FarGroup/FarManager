#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

MyString MYRTLEXP MakePathString( CONSTSTR path, int maxWidth )
  {  MyString s;
     int      len = strLen(path);

     s.Alloc( maxWidth );

     if ( len <= maxWidth ) {
       s = path;
       return s;
     }

#define NPART ((maxWidth-4)/2)

     s.Set( path, 0, NPART );
     s.Add( SLASH_STR  ".."  SLASH_STR );
     s.Add( path, len-NPART, NPART );

 return s;
}

MyString MYRTLEXP MakePatternStr( const MyString& fname,const MyString& astr )
  {  MyString res;

     for ( int n = 0; n < astr.Length(); n++ )
       if ( astr[n] == '!' ) {
         n++;
         switch( astr[n] ) {
           case '!': res.Add( astr[n] );
                     break;
           case ':': res.Add( MakeRootDir(fname.Text()) );
                     break;
           case '\\': res.Add( GetFPath(fname) );
                     break;
           case '.': n++;
                     if ( astr[n] == '!' ) {
                       res.Add( GetFName(fname) );
                       break;
                     } else {
                       res.Add( GetFNameOnly(fname) );
                       res.Add( '.' );
                       res.Add( astr[n] );
                     }
                   break;
            default: res.Add( GetFNameOnly(fname) );
                     res.Add( astr[n] );
         }
       } else
       if ( astr[n] == '.' && astr[n+1] == '!' ) {
         res.Add( GetFExtOnly(fname) );
         n+=2;
       } else
         res.Add( astr[n] );
 return res;
}
