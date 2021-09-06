#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

#if defined(__QNX__)
    FileAttrEntry FileAttributes[] = {
       { "Fifo",                _S_IFIFO,   'F' },
       { "Character special",   _S_IFCHR,   'C' },
       { "Directory",           _S_IFDIR,   'D' },
       { "Special named",       _S_IFNAM,   'N' },
       { "Block special",       _S_IFBLK,   'B' },
       { "Regular",             _S_IFREG,   'R' },
       { "Symbolik link",       _S_IFLNK,   'L' },
       { "Socket",              _S_IFSOCK,  'S' },
        { NULL,0,0 }  };
#else
#if defined(__GNUC__)
    FileAttrEntry FileAttributes[] = {
       { "Fifo",                S_IFIFO,   'F' },
       { "Character special",   S_IFCHR,   'C' },
       { "Directory",           S_IFDIR,   'D' },
       { "Block special",       S_IFBLK,   'B' },
       { "Regular",             S_IFREG,   'R' },
       { "Symbolik link",       S_IFLNK,   'L' },
       { "Socket",              S_IFSOCK,  'S' },
        { NULL,0,0 }  };
#else
#if defined(__HDOS__) || defined(__HWIN16__)
    FileAttrEntry FileAttributes[] = {
        { "Read only",      FA_RDONLY,  'R' },
        { "Hidden",         FA_HIDDEN,  'H' },
        { "System",         FA_SYSTEM,  'S' },
        { "Label",          FA_LABEL,   'L' },
        { "Directory",      FA_DIREC,   'D' },
        { "Archive",        FA_ARCH,    'A' },
        { NULL,0,0 }  };
#else
#if defined(__HWIN32__)
    FileAttrEntry FileAttributes[] = {
        { "Read only",      FILE_ATTRIBUTE_READONLY,   'R' },
        { "Hidden",         FILE_ATTRIBUTE_HIDDEN,     'H' },
        { "System",         FILE_ATTRIBUTE_SYSTEM,     'S' },
        { "Directory",      FILE_ATTRIBUTE_DIRECTORY,  'D' },
        { "Archive",        FILE_ATTRIBUTE_ARCHIVE,    'A' },
        { "Compressed",     FILE_ATTRIBUTE_COMPRESSED, 'C' },
        { "Offline",        FILE_ATTRIBUTE_OFFLINE,    'O' },
        { "Temprorary",     FILE_ATTRIBUTE_TEMPORARY, 't' },
        { NULL,0,0 }  };
#else
  #error ERR_PLATFORM
#endif
#endif
#endif
#endif

/***************************************
            FileEntry
 ***************************************/
FileEntry::FileEntry( void )                          { Parent = NULL; Flags = 0;  }
FileEntry::FileEntry( PDirEntry owner )               { Parent = owner; Flags = 0; }
FileEntry::FileEntry( PDirEntry owner,FileEntry& p )  { Assign(owner,p); }
FileEntry::FileEntry( PDirEntry owner,CONSTSTR path ) { Assign(owner,path); }
FileEntry::FileEntry( PDirEntry owner,FILE_ENUM *fe ) { Assign(owner,fe); }

FileEntry& FileEntry::operator=( FileEntry& p )                  { Assign(p.Parent,p); return *this; }

/*
    read stat, attr & times only then need
*/
void FileEntry::FillData( char *fname,FILE_ENUM *fe )
  {
#if defined( __QNX__ )
     time_t     tmTmp;
     struct tm *tmp;
     char  tmpDir[ MAX_PATH_SIZE+1 ];

     if ( fe && fe->ent->d_stat.st_status == _FILE_USED &&
          fe->ent->d_stat.st_mode == 0x41FF ) {
       MemMove( &Info,&fe->ent->d_stat,sizeof(Info) );
       allAttr = Info.st_mode;
       SET_FLAG( Flags,FF_SETTED );
     } else {
//Stat
        if ( FIO_STAT( fname,&Info ) )
          SET_FLAG( Flags,FF_SETTED );
         else
          CLR_FLAG( Flags,FF_SETTED );
        allAttr = Info.st_mode;
//MountOn
        MemSet( tmpDir,0,sizeof(tmpDir) );
        fsys_get_mount_dev(fname,tmpDir);
        _MountOn = tmpDir;
//MountTo
        MemSet( tmpDir,0,sizeof(tmpDir) );
        fsys_get_mount_pt(fname,tmpDir);
        _MountTo = tmpDir;
      }
//Link
      if ( S_ISLNK(allAttr) ) {
        MemSet( tmpDir,0,sizeof(tmpDir) );
        readlink(fname,tmpDir,MAX_PATH_SIZE );
        _Link = tmpDir;
        if ( _Link.Length() > 0 && isDirectory(_Link.Text()) ) {
          allAttr &= ~_S_IFMT;
          allAttr |= flDirectory;
        }
      } else _Link = "";
#else
     USEARG(fe)
     if ( FIO_STAT( fname,&Info ) )
       SET_FLAG( Flags,FF_SETTED );
      else
       CLR_FLAG( Flags,FF_SETTED );
     GetFileAttr( fname,allAttr );
#endif

//correct Directory
     if ( _Name.Cmp(".") || _Name.Cmp("..") )
       allAttr |= flDirectory;
}

void FileEntry::AssignPath( const MyString& p )
  {
    if ( IS_FLAG(Flags,FF_FULLPATH) ) return;
    _Path = p;
    SET_FLAG(Flags,FF_FULLPATH);
}

void FileEntry::Assign( PDirEntry owner,FileEntry& p )
  {
     Parent  = owner;
     Flags   = p.Flags;
     allAttr = p.allAttr;
     if ( !p.Parent || !p.Parent->isRealPath() ) {
       _Name = p.NameExt();
       _Path = p.Path();
       SET_FLAG( Flags,FF_FULLPATH );
     } else {
       _Name  = p._Name;
       _Path  = p._Path;
     }
     MemCpy( &Info,&p.Info,sizeof(Info) );
     SET_FLAG( Flags,FF_SETTED );
#if defined(__QNX__)
     _Link    = p._Link;
     _MountOn = p._MountOn;
     _MountTo = p._MountTo;
#endif
}

void FileEntry::Assign( PDirEntry owner,CONSTSTR _fname )
  {  char      *fname = (char*)_fname;

     Parent = owner;
     Flags  = 0;
     MemSet( &Info,0,sizeof(Info) );
     if ( fname && *fname && fname[strLen(fname)-1] == SLASH_CHAR ) fname[strLen(fname)-1] = 0;
     if ( StrChr(fname,SLASH_CHAR) ) {
       SET_FLAG( Flags,FF_FULLPATH );
       _Name = GetFName( fname );
       _Path = GetFPath( fname );
     } else {
       _Name = fname;
       _Path = "";
     }
     CLR_FLAG( Flags,FF_SETTED );
     FillData( fname,NULL );
}

void FileEntry::Assign( PDirEntry owner,FILE_ENUM *fe )
  {
     Parent = owner;
     _Name  = fe->FName();
     if ( !owner || !owner->isRealPath() ) {
       _Path = fe->Path;
       Flags = FF_FULLPATH;
     } else {
       _Path = "";
       Flags  = 0;
     }
     MemSet( &Info,0,sizeof(Info) );
     CLR_FLAG( Flags,FF_SETTED );
     FillData( fe->PathName(),fe );
}

MyString FileEntry::sSize( char del,int div, CONSTSTR fmt )
  {
  return MakeIntString( (DWORD)Size(),del,div,fmt );
}


MyString FileEntry::PathName( void )
  {  MyString s;

     if ( IS_FLAG(Flags,FF_FULLPATH) ) {
       s.Set( _Path );
       s.Add( NameExt() );
       return s;
     }

     if ( Parent && !IS_FLAG(Flags,FF_VIRTUALFILE) && Parent->isRealPath() ) {
       s.Set( Parent->Path );
       s.Add( NameExt() );
       return s;
     } else
       return NameExt();
}

MyString FileEntry::ShortPathName( void )
  {  MyString s;

    if ( IS_FLAG(Flags,FF_FULLPATH) ) {
       s.Set( _Path );
       s.Add( ShortNameExt() );
       return s;
     }

    if ( Parent && !IS_FLAG(Flags,FF_VIRTUALFILE) && Parent->isRealPath() ) {
       s.Set( Parent->Path );
       s.Add( ShortNameExt() );
       return s;
     } else
      return ShortNameExt();
}


MyString FileEntry::ShortNameExt( void )
  {
#if defined(__QNX__) || defined(__GNUC__) || defined(__HDOS__)
  return NameExt();
#else
#if defined(__HWIN__)
  return NameExt();    //?
#else
  #error ERR_PLATFORM
#endif
#endif
}

MyString FileEntry::ShortName( void )
  {
#if defined(__GNUC__) || defined(__QNX__) || defined(__HDOS__)
  return Name();
#else
#if defined(__HWIN__)
  return Name();    //?
#else
  #error ERR_PLATFORM
#endif
#endif
}

MyString FileEntry::ShortExt( void )
  {
#if defined(__GNUC__) || defined(__QNX__) || defined(__HDOS__)
  return Ext();
#else
#if defined(__HWIN__)
  return Ext();    //?
#else
  #error ERR_PLATFORM
#endif
#endif
}

MyString FileEntry::Link( void )
  {
#if defined(__GNUC__) || defined(__QNX__)
  return _Link;
#else
#if defined(__HDOS__) || defined(__HWIN__)
  return "<link>";
#else
#error ERR_PLATFORM
#endif
#endif
}

MyString FileEntry::MountOn( void )
  {
#if defined(__GNUC__) || defined(__QNX__)
  return _MountOn;
#else
#if defined(__HWIN__) || defined(__HDOS__)
     MyString s;
     s.printf( "%c:\\", (char)('A' + Info.st_dev) );
  return s;
#else
#error ERR_PLATFORM
#endif
#endif
}

MyString FileEntry::MountTo( void )
  {
#if defined(__GNUC__) || defined(__QNX__)
  return _MountTo;
#else
#if defined(__HWIN__) || defined(__HDOS__)
     return "<mntto>";
#else
#error ERR_PLATFORM
#endif
#endif
}

char *FileEntry::Path( void )
  {
 return (IS_FLAG(Flags,FF_FULLPATH) || !Parent)
          ? _Path.Text()
          : Parent->Path.Text();
}

MyString FileEntry::Name( void )
  {  char *m;
     char str[MAX_PATH_SIZE+1];
    if ( StrChr(NameExt(),SLASH_CHAR) )
      strcpy( str,StrRChr(NameExt(),SLASH_CHAR)+1 );
     else
      strcpy( str,NameExt() );
    if ( (m=StrRChr(str,'.')) != 0 ) *m = 0;
 return MyString(str);
}

MyString FileEntry::Ext( void )
  {  char *m;
     char str[MAX_PATH_SIZE+1];
    if ( StrChr(NameExt(),SLASH_CHAR) )
      strcpy( str,StrRChr(NameExt(),SLASH_CHAR)+1 );
     else
      strcpy( str,NameExt() );
    if ( (m=StrRChr(str,'.')) != 0 )
      return m+1;
     else
      return "";
}

long FileEntry::Size( void )
  {
   return Info.st_size;
}

void FileEntry::AssignSize( long sz )
  {
   Info.st_size = sz;
}

void FileEntry::SetTime( time_t cr, time_t ac, time_t mod )
  {
    SetFileTimes( PathName().Text(), cr, ac, mod );
}

void FileEntry::SetAttr( ATTR_TYPE v )
  {  ATTR_TYPE clr;

#if defined(__GNUC__) || defined(__QNX__)
    clr = flDirectory;
#else
#if defined(__HDOS__) || defined(__HWIN16__)
    clr = FA_DIREC | FA_LABEL;
#else
#if defined(__HWIN32__)
    clr = FILE_ATTRIBUTE_DIRECTORY;
#else
#error ERR_PLATFORM
#endif
#endif
#endif
    CLR_FLAG( v,clr );
    if ( !SetFileAttr( PathName().Text(),v ) ) return;
    allAttr = (allAttr&clr) | v;
}

void FileEntry::SetGroupUser( int user,int group )
  {
#if defined(__QNX__)
    if ( FIO_CHOWN( PathName().Text(),user,group ) )
#endif
      {  Info.st_gid = (short)group;
         Info.st_uid = (short)user;
      }
}

char  *FileEntry::sDate( void )         { time_t tmTmp = Time(); struct tm *tmp = localtime( &tmTmp ); static char str[20]; strftime( str,20,"%d.%m.%Y",tmp ); return str; }
char  *FileEntry::sModifyDate( void )   { time_t tmTmp = ModifyTime(); struct tm *tmp = localtime( &tmTmp ); static char str[20]; strftime( str,20,"%d.%m.%Y",tmp ); return str; }
char  *FileEntry::sAccessDate( void )   { time_t tmTmp = AccessTime(); struct tm *tmp = localtime( &tmTmp ); static char str[20]; strftime( str,20,"%d.%m.%Y",tmp ); return str; }
char  *FileEntry::sTime( void )         { time_t tmTmp = Time(); struct tm *tmp = localtime( &tmTmp ); static char str[20]; strftime( str,20,"%H:%M:%S",tmp ); return str; }
char  *FileEntry::sModifyTime( void )   { time_t tmTmp = ModifyTime(); struct tm *tmp = localtime( &tmTmp ); static char str[20]; strftime( str,20,"%H:%M:%S",tmp ); return str; }
char  *FileEntry::sAccessTime( void )   { time_t tmTmp = AccessTime(); struct tm *tmp = localtime( &tmTmp ); static char str[20]; strftime( str,20,"%H:%M:%S",tmp ); return str; }
int    FileEntry::Attr( void )       { return allAttr; }
int    FileEntry::Owner( void )      { return Info.st_dev; }
int    FileEntry::Group( void )      { return Info.st_gid; }
int    FileEntry::User( void )       { return Info.st_uid; }
time_t FileEntry::ModifyTime( void ) { return Info.st_mtime; }
time_t FileEntry::AccessTime( void ) { return Info.st_atime; }
time_t FileEntry::Time( void )       { return Info.st_ctime; }

BOOL   FileEntry::Directory( void )
  {
#if defined(__GNUC__) || defined(__QNX__)
 return S_ISDIR( Attr() );
#else
#if defined(__HWIN__) || defined(__HDOS__)
 return IS_FLAG( Attr(),flDirectory );
#else
#error ERR_PLATFORM
#endif
#endif
}

char FileEntry::GetTypeChar( void )
  {
#if defined(__GNUC__)
    if ( Directory()  ) return '/';
    if ( Executable()  ) return '*';
    if ( IS_FLAG(Attr(),S_IFIFO) ) return '&';
    if ( IS_FLAG(Attr(),S_IFCHR) ) return '>';
    if ( IS_FLAG(Attr(),S_IFBLK) ) return '\\';
    if ( IS_FLAG(Attr(),S_IFREG) ) return ' ';
    if ( IS_FLAG(Attr(),S_IFLNK) ) return '@';
    if ( IS_FLAG(Attr(),S_IFSOCK)) return '$';
   return ' ';
#else
#if defined(__QNX__)
    if ( Directory()  ) return '/';
    if ( Executable()  ) return '*';
    if ( IS_FLAG(Attr(),_S_IFIFO) ) return '&';
    if ( IS_FLAG(Attr(),_S_IFCHR) ) return '>';
    if ( IS_FLAG(Attr(),_S_IFNAM) ) return ']';
    if ( IS_FLAG(Attr(),_S_IFBLK) ) return '\\';
    if ( IS_FLAG(Attr(),_S_IFREG) ) return ' ';
    if ( IS_FLAG(Attr(),_S_IFLNK) ) return '@';
    if ( IS_FLAG(Attr(),_S_IFSOCK)) return '$';
   return ' ';
#else
#if defined(__HDOS__) || defined(__HWIN16__)
    if ( Executable()  ) return '*';
    if ( IS_FLAG(Attr(),FA_LABEL)  ) return '>';
    if ( IS_FLAG(Attr(),FA_HIDDEN) ) return '░';
    if ( IS_FLAG(Attr(),FA_RDONLY) ) return '▓';
    if ( IS_FLAG(Attr(),FA_DIREC)  ) return '/';
    if ( IS_FLAG(Attr(),FA_SYSTEM) ) return '%';
    if ( IS_FLAG(Attr(),FA_ARCH)   ) return ' ';
   return ' ';
#else
#if defined(__HWIN32__)
    if ( Executable()  ) return '*';
    if ( IS_FLAG(Attr(),FILE_ATTRIBUTE_HIDDEN) )    return '░';
    if ( IS_FLAG(Attr(),FILE_ATTRIBUTE_READONLY) )  return '▓';
    if ( IS_FLAG(Attr(),FILE_ATTRIBUTE_DIRECTORY) ) return '/';
    if ( IS_FLAG(Attr(),FILE_ATTRIBUTE_SYSTEM) )    return '%';
    if ( IS_FLAG(Attr(),FILE_ATTRIBUTE_ARCHIVE) )   return ' ';
   return ' ';
#else
#error ERR_PLATFORM
#endif
#endif
#endif
#endif
}

char *FileEntry::sOwner( void )
  {  static char str[10];
    Sprintf( str,"%d",Owner() );
 return str;
}

char *FileEntry::sGroup( void )
  {  static char str[10];
#if defined(__GNUC__) || defined(__QNX__)
    struct group *g = getgrgid( Group() );
    if ( g != NULL ) return g->gr_name;
#else
#if defined(__HWIN__) || defined(__HDOS__)
   ;
#else
#error ERR_PLATFORM
#endif
#endif
    Sprintf( str,"%d",Group() );
 return str;
}

char *FileEntry::sUser( void )
  {  static char str[10];
#if defined(__GNUC__) || defined(__QNX__)
    struct passwd *pw = getpwuid( User() );
    if ( pw != NULL )
      return pw->pw_name;
#else
#if defined(__HWIN__) || defined(__HDOS__)
  ;
#else
#error ERR_PLATFORM
#endif
#endif
    Sprintf( str,"%d",User() );
 return str;
}

char *FileEntry::sAttr( void )
  {  static char str[ 20 ];
     int         n;

#if defined(__GNUC__)
    int ac = Attr(),val = 0400;
    static char temp[]  = "rwxrwxrwx";
    static char types[0xF] = "-fc-dnb-r-l-s";
    str[0] = types[ (Attr()&S_IFMT)/0x1000 ];
    for ( n = 0; n < sizeof(temp)/sizeof(char); val>>=1,n++ )
      str[n+1] = (IS_FLAG(ac,val))?temp[n]:'-';
#else
#if defined(__QNX__)
    int ac = Attr(),val = 0400;
    static char temp[]  = "rwxrwxrwx";
    static char types[0xF] = "-fc-dnb-r-l-s";
    str[0] = types[ (Attr()&_S_IFMT)/0x1000 ];
    for ( n = 0; n < sizeof(temp)/sizeof(char); val>>=1,n++ )
      str[n+1] = (IS_FLAG(ac,val))?temp[n]:'-';
#else
#if defined(__HWIN__) || defined(__HDOS__)
     for ( n = 0; FileAttributes[n].Name; n++ )
       str[n] = ( IS_FLAG( Attr(),FileAttributes[n].Value ) ) ? FileAttributes[n].CharDescr : '.';
#else
#error ERR_PLATFORM
#endif
#endif
#endif
    str[n] = 0;
 return str;
}

BOOL FileEntry::Executable( void )
  {
#if defined(__GNUC__) || defined(__QNX__)
  return (IS_FLAG(Attr(),S_IXOTH)) ||
         (IS_FLAG(Attr(),S_IXGRP) && getgid() == Info.st_gid) ||
         (IS_FLAG(Attr(),S_IXUSR) && getuid() == Info.st_uid);
#else
#if defined(__HWIN__) || defined(__HDOS__)
     MyString str( Ext() );
     str.Case( mscUpper );
     return str.Cmp("EXE") || str.Cmp("COM") || str.Cmp("BAT");
#else
#error ERR_PLATFORM
#endif
#endif
}

/***************************************
   PROCS
 ***************************************/
void MYRTLEXP SetAttrAs( const MyString& dest,PFileEntry source )
  {  FileEntry tmp( NULL,dest.Text() );
        tmp.SetTime( source->Time(),source->AccessTime(),source->ModifyTime() );
        tmp.SetAttr( source->Attr() );
        tmp.SetGroupUser( source->User(),source->Group() );
}

MyString MYRTLEXP MakeInName( const MyString& source,PFileEntry file )
  {
    if ( IS_FLAG(file->Flags,FF_FULLPATH) )
      return file->PathName();
     else {
      MyString s( source );
      s.Add( file->NameExt() );
      return s;
     }
}

MyString MYRTLEXP MakeOutName( const MyString& Dest,PFileEntry file )
  {  MyString str,
              p = GetFName( Dest ),
              f;
     int      n,i;

     if ( p.Length() == 0 ) {
       str.Set( Dest );
       str.Add( file->NameExt() );
       return str;
     }

     str = GetFPath( Dest );
     f   = file->NameExt();

     if ( f.Chr('.') == -1 )
       f.Add( '.' );

     for( n = 0; p[n]; n++ )
       switch( p[n] ) {
         case '*': do
                     n++;
                   while( p[n] && p[n] == '*' );

                   for( i = 0; f[i] && f[i-1] != p[n]; i++ )
                     str.Add( f[i] );
                break;

         case '?': str.Add( f[n] );
                break;

         case '.': if ( p[n+1] == '*' ) {
                     n+=2;
                     for( i = 0; f[i] && f[i] != '.'; i++ );
                     for(; f[i]; i++ )
                       str.Add( f[i] );
                   }

          default: str.Add( p[n] );
                break;
       }
 return str;
}
