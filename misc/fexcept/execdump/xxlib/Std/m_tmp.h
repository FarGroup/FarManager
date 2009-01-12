#ifndef __MY_TEMP_FILE_HANDLERS
#define __MY_TEMP_FILE_HANDLERS

#if defined(__cplusplus)
struct t_FILE {
  FILE *File;

  t_FILE( CONSTSTR nm )               { File = nm ? fopen( nm,"w" ) : NULL ; }
  t_FILE( CONSTSTR nm,CONSTSTR mode ) { File = nm ? fopen( nm,mode ) : NULL ; }
  ~t_FILE()                           { if (File) fclose(File); }

  operator FILE*() { return File; }
};

struct t_File {
  int File;

  t_File( CONSTSTR nm )               { File = nm ? FIO_OPEN( nm,O_RDWR ) : (-1); }
  t_File( CONSTSTR nm,int mode )      { File = nm ? FIO_OPEN( nm,mode ) : (-1); }
  ~t_File()                           { if (File != -1) FIO_CLOSE(File); }

  operator int() { return File; }
};
#endif

#endif
