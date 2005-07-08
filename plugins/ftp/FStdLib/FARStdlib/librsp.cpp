#include <all_far.h>
#pragma hdrstop

#include "fstdlib.h"

int main( int argc, char *argv[] )
  {  char *m;
     char  ch;
     int   n,i,sw;
     char  word[ MAX_PATH_SIZE+1 ];

//Exename
    m = strrchr(argv[0],'\\');
    if ( !m ) m = argv[0]; else m++;

//USAGE
    if ( argc < 3 ) {
      printf( "USAGE: %s <start word> <listfile>\n"
              "  Print each word starting from `start word`  from input file in form to use with library manager\n"
              "  as:\n"
              "     -+<word>\n"
              "  in each line\n"
              "",
              m );
      return -1;
    }

//Start word
    sw = atoi( argv[1] );

//Input file
    int f = FIO_OPEN( argv[2],O_RDONLY );
    if (!f) { printf( "Error open input file [%s]\n",argv[2] ); return -1; }

//Format out each word
    i = 0,n = 0;
    while( FIO_READ(f,&ch,sizeof(ch)) == sizeof(ch) ) {
      if ( ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n') ch = ' ';
      if ( ch != ' ' && i < MAX_PATH_SIZE ) {
        word[i++] = ch;
        continue;
      }
      if ( i ) {
        word[i] = 0;
        if (n>=sw) printf( "+-%s &\n",word );
        n++;
      }
      i = 0;
    }
    if ( i ) {
      word[i] = 0;
      if (n>=sw) printf( "+-%s &\n",word );
    }
    FIO_CLOSE(f);

//Close
  return 0;
}