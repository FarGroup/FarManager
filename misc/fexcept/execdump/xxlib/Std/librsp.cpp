#include <all_lib.h>
#pragma hdrstop

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
              "  Print each word from input file starting it with `start word` as:\n"
              "     -+<word>\n"
              "  at each line.\n"
              "",
              m );
      return -1;
    }

//Start word
    sw = atoi( argv[1] );

//Input file
    int f = open( argv[2],O_RDONLY );
    if (!f) { printf( "Error open input file [%s]\n",argv[2] ); return -1; }

//Format out each word
    i = 0,n = 0;
    while( read(f,&ch,sizeof(ch)) == sizeof(ch) ) {
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
    close(f);

//Close
  return 0;
}