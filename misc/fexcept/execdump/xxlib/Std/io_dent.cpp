#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

BOOL MYRTLEXP isDirectory( CONSTSTR fname )
  {  ATTR_TYPE a;
     CONSTSTR m = FName(fname);

    if ( strcmp( m,".") == 0 ||
         strcmp( m,"..") == 0 )
      return TRUE;

    if ( !GetFileAttr(fname,a) )
      return FALSE;

    if ( IS_FLAG(a,flDirectory) )
      return TRUE;

#if defined(__QNX__)
      if ( S_ISLNK(a) ) {
        char tmpDir[ MAX_PATH_SIZE ];
        int  rc = readlink( fname,tmpDir,MAX_PATH_SIZE );
        if ( rc <= 0 )
          return FALSE;
        tmpDir[rc] = 0;
        return isDirectory( tmpDir );
      }
#endif

 return FALSE;
}

#if defined(__WIN32__)
BOOL MYRTLEXP CreateFullDirectory( char *nm )
  {  char *m;
     char  ch;
     UINT  tp;

//Skip UNC abs path
    if ( StrCmp( nm, "\\\\?\\",4 ) == 0 )
      nm += 4;

//Skip UNC share name
    if ( nm[0] == SLASH_CHAR && nm[1] == SLASH_CHAR ) {
      m = StrChr( nm+2,SLASH_CHAR );
      if ( !m ) {
        SetError( "UNC does not contains resource name" );
        return FALSE;
      }
      m = StrChr( m+1,SLASH_CHAR );
      if ( !m )
        return TRUE;
      m = m+1;
    } else
      m = nm;

//Normal
    do{
      m = StrChr( m+1,SLASH_CHAR );
      if ( m ) {
        ch = *m;
        *m = 0;
      }

      tp = GetDriveType(nm);
      if ( tp == 0 ) return FALSE;

      if ( tp < DRIVE_REMOVABLE &&
           !CreateDirectory(nm,NULL) ) {

        if ( GetLastError() == ERROR_ALREADY_EXISTS ||
             GetLastError() == ERROR_INVALID_NAME ) {
          ;
        } else {
          SetError( "Unk LastError: %d",GetLastError() );
          return FALSE;
        }
      }

      if ( m )
        *m = ch;
    }while( m );

 return TRUE;
}
#endif

/***************************************
            DirEntry
 ***************************************/
DirEntry::DirEntry( BOOL allocBig )
    : MyArray<PFileEntry>( (allocBig)?MAX_FILE_COUNT:20,100 )
  {
}

BOOL DirEntry::Assign( DirEntry& from, BOOL selected )
  {  PFileEntry en;
    Path = from.Path;
    DeleteAll();
    Alloc( from.Count()+1 );
    for ( int n = 0; n < from.Count(); n++ ) {
      if ( selected ) {
        if ( IS_FLAG(from[n]->Flags,FF_SELECTED) )
           en = Add( new FileEntry(this,*from[n]) );
          else
           en = NULL;
      } else
        en = Add( new FileEntry(this,*from[n]) );
      if ( en ) en->AssignPath( from.Path );
    }
 return TRUE;
}

BOOL DirEntry::Assign( DirEntry& from, PFileEntry p )
  {  PFileEntry en;
    if ( !p ) return FALSE;
    Path = from.Path;
    DeleteAll();
    en = Add( new FileEntry(this,*p) );
    en->AssignPath(from.Path);
 return TRUE;
}

int DirEntry::SelCount( void )
  {  int cn = 0;
     for ( int n = 0; n < Count(); n++ )
       if ( IS_FLAG(Item(n)->Flags,FF_SELECTED) )
         cn++;
 return cn;
}

BOOL DirEntry::OpenDir( CONSTSTR path, OpenDirProc callBack, CONSTSTR filter,CONSTSTR exfilter )
   {  PFileEntry p;
      FileEntry  fl;
      long       counter = 0;
      FILE_ENUM  fe;
      int        n;

    if ( !path ) return FALSE;
    if ( path[0] == ':' ) return TRUE;
    if ( !fe.OpenDir( path ) ) return FALSE;
    Path = fe.Path;

    do{
        if ( strLen(fe.FName()) == 0 ) break;
        if ( CMP_FILE( fe.FName(),".") ) continue;
        fl.Assign( this,&fe );
//if not in filter
        if ( filter && filter[0] && !fl.Directory() )
          if ( !InPattern( filter,fe.FName() ) )
            continue;
//if in exclude
        if ( exfilter && exfilter[0] && !fl.Directory() )
          if ( InPattern( exfilter,fe.FName() ) )
            continue;
//file in list
        if ( (n=IndexOf(fe.PathName())) != -1 ||
             (n=IndexOf(fe.FName())) != -1 )
          {  p = Item(n);
             *p = fl;
          }
//or add new
         else {
          p = Add( new FileEntry );
          *p = fl;
         }
//Mark as used
        SET_FLAG( p->Flags,FF_USED );
//Counter & other
        counter++;
  #if !defined(FCOUNT_UNLIM)
        if ( counter >= MAX_FILE_COUNT )
          { ErrorBeep(); break; }
  #endif
        if ( callBack ) if ( !callBack( counter ) ) break;
    }while( fe.NextFile() );
//Close dir
    fe.CloseDir();

//Remove `unused` files
    for ( n = 0; n < Count(); )
      if ( !IS_FLAG( (p=Item(n))->Flags,FF_USED ) )
        Delete( p );
       else
        {  CLR_FLAG( p->Flags,FF_USED );
           n++;
        }
//Check for ".." {prev}
    n = IndexOf("..");
    if ( RootDir(Path) ) {
//Remove ".." from root
      if ( n != -1 ) Delete( Item(n) );
    } else {
//Add ".." dir if no
      if ( n == -1 ) Add( new FileEntry( this,".." ) );
    }

return TRUE;
}

int DirEntry::IndexOf(const char* fname)
  {
    for ( int n = 0; n < Count(); n++ )
      if ( CMP_FILE( Item(n)->NameExt(),fname ) )
        return n;
 return -1;
}
