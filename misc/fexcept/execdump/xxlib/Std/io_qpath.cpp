#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

#if defined(__BC31__)
  static HotPathArray __dummy;
#endif

void MYRTLEXP QueryHotPaths( PHotPathArray arr )
  {
     arr->DeleteAll();
#if defined(__QNX__)
    int     num_nids;
    char    node_name[22];
    char    buffer[ 500 ];
    char    str[100];
    struct _osinfo osi;

    if ( (num_nids=qnx_net_alive(buffer,sizeof(buffer))) == -1)
      return;

    for( int n = 1; n < num_nids+1; n++) {
      if ( !buffer[n] ||
           qnx_osinfo( n,&osi ) == -1 )
        continue;

      qnx_nidtostr( n, node_name, sizeof(node_name));
      SNprintf( str, sizeof(str), "//%ld/ ~%-6s~ CPU:~%3u~-~%3d~/~%3d~ Ver:~%2d.%02d%c~ Mem:~%d~/~%d~",
                osi.nodename,
                osi.machine, osi.cpu, osi.fpu,
                osi.cpu_speed, osi.version/100, osi.version%100, osi.release,
                osi.freememk,  osi.totmemk );

      PHotPathEntry pe = arr->Add( new HotPathEntry );
      pe->Path.printf( "//%s/", node_name );
      pe->Label.printf( " ~%c~ │",'A'+n-1 );
      pe->Description = str;
    }
#else
#if defined(__HDOS__)
   int  count,c,old;
   char astr[] = "X:\\",
        str[]  = "X";

   count = setdisk( old = getdisk() );
   for ( c = 0; c < count; c++ ) {
     if ( _chdrive(c+1) != 0 )
       continue;

     astr[0] = (char)('A'+c);
     astr[0] = (char)('A'+c);

     CONSTSTR d;
     switch( GetDiskType( c+1 ) ) {
      case     DRIVE_CDROM: d = "CD-ROM drive";     break;
      case   DRIVE_RAMDISK: d = "RAM drive";        break;
      case    DRIVE_REMOTE: d = "Remote drive";     break;
      case     DRIVE_FIXED: d = "Hard drive";       break;
      case DRIVE_REMOVABLE: d = "Removable drive";  break;
      case     DRIVE_SUBST: d = "Subst drive";      break;
      case  DRIVE_DBLSPACE: d = "DblSpace drive";   break;
                   default: d = NULL;
     }
     if ( d ) {
       MyString s;
       s.printf( " ~%s~ │", str );
       arr->Add( new HotPathEntry( astr, s, d ) );
     }
   }
   setdisk( old );

#else
#if defined(__HWIN32__)
   DWORD dw = GetLogicalDrives();
   UINT  type;
   char  astr[] = "X:\\",
         str[]  = "X";
   for ( int n = 0; n < 32; n++ ) {
     astr[0] = (char)('A'+n);
     str[0]  = (char)('A'+n);

     if ( (dw & (1UL<<n)) == 0 || (type=GetDriveType(astr)) <= 1 )
       continue;

     CONSTSTR d;
     switch( type ) {
      case     DRIVE_CDROM: d = "CD-ROM drive";    break;
      case   DRIVE_RAMDISK: d = "RAM drive";        break;
      case    DRIVE_REMOTE: d = "Remote drive";     break;
      case     DRIVE_FIXED: d = "Hard drive";       break;
      case DRIVE_REMOVABLE: d = "Removable drive";  break;
                   default: d = NULL;
     }
     if ( d ) {
       MyString s;
       s.printf( " ~%s~ │", str );
       arr->Add( new HotPathEntry( astr, s, d ) );
     }
   }
#else
#if defined(__HWIN16__)
   UINT  type;
   char  astr[] = "X:\\",
         str[]  = "X";

   for ( int n = 0; n < 26; n++ ) {
     if ( (type=GetDriveType(n)) == 1 )
       continue;

     astr[0] = (char)('A'+n);
     str[0]  = (char)('A'+n);

     CONSTSTR d;
     switch( type ) {
      case    DRIVE_REMOTE: d = "Remote (network) drive"; break;
      case     DRIVE_FIXED: d = "Hard drive";             break;
      case DRIVE_REMOVABLE: d = "Removable drive";        break;
      case               0: d = "Undefined drive";        break;
                   default: d = NULL;
     }
     if ( d ) {
       MyString s;
       s.printf( " ~%s~ │", str );
       arr->Add( new HotPathEntry( astr, s, d ) );
     }
   }
#else
#error ERR_PLATFORM
#endif
#endif
#endif
#endif
}
