   This folder contains character tables for viewer,  editor  and  "Find
file" command and character frequency distribution tables.

Read file descriptions for contents of the tables.

1. Dist
~~~~~~~
   Character frequency distribution information is required in order  to
autodetect the table in viewer and editor (you do not need it if you use
only one character table). It describes  characters  frequency  for  OEM
codepage of concrete language. 0 corresponds to lowest frequency, 254 to
highest, value 255 means that this  character  should  be  ignored  when
analysing file.

   The simple C++ program  below  can  be  used  to  generate  frequency
distribution information for your language. It has only one parameter  -
the name of a large enough text file in OEM codepage which is typical of
the language used.


#include <stdio.h>

void main(int argc, char *argv[])
{
  FILE *SrcFile;
  unsigned long Count[256], MaxCount=0;
  int I, Ch, PrevCh=0, Divider, Value;

  if (argc != 2)
  {
    printf("\nSyntax: DISTR <file>");
    return;
  }

  if ((SrcFile=fopen(argv[1],"rb")) == NULL)
  {
    printf("\nCannot open %s",argv[1]);
    return;
  }

  for (I=0; I < sizeof(Count)/sizeof(Count[0]); I++)
    Count[I]=0;

  while ((Ch=fgetc(SrcFile)) != EOF)
    if (Ch != PrevCh)
      Count[Ch]++;

  fclose(SrcFile);

  for (I=128; I < sizeof(Count)/sizeof(Count[0]); I++)
    if (MaxCount<Count[I])
      MaxCount=Count[I];

  Divider=MaxCount/254;

  if (Divider<10)
  {
    printf("\nSource file too small");
    return;
  }

  printf("REGEDIT4\n\n");
  printf("[HKEY_CURRENT_USER\\Software\\Far\\CodeTables]\n");
  printf("\"Distribution\"=hex:\\\n    ");

  for (I=0; I < 256; I++)
  {
    if (I < 128 && (I >= 32 || Count[I] != 0))
      Value=0xFF;
    else
      if ((Value=Count[I]/Divider) > 254)
        Value=254;
    printf("%02X%s",Value,(I != 255?",":""));
    if ((I%16) == 15 && I != 255)
      printf("\\\n    ");
  }
  printf("\n");
}


2. Win
~~~~~~
   If you want to generate a WINDOWS character table, to use it  in  the
FAR (it is displayed as "Win" in the translation table  list),  you  can
use the following C/C++ program. It will display the  current  table  on
the stdout. In order to write a generated table  in  the  file  win.reg,
type in the command line prompt: WinCTGen.exe>win.reg.

#include <windows.h>
#include <stdio.h>

void main(void)
{
  int Chr[256], I, J, P;
  char b[2];

  b[1]=0;

  for(I=0;I < 256; I++)
  {
    b[0]=I;
    CharToOem(b,b);
    Chr[I]=(unsigned char)b[0];
  }

  printf("REGEDIT4\n\n");
  printf("[HKEY_CURRENT_USER\\Software\\Far\\CodeTables\\Win]\n");
  printf("\"Mapping\"=hex:\\\n    ");

  for(I=0; I < 21;I++)
    printf("%02X,",Chr[I]);
  printf("\\\n  ");

  for(J=0; J < 9;J++)
  {
    for(P=0; P < 25; P++, I++)
      printf("%02X,",Chr[I]);
    printf("\\\n  ");
  }

  for(; I < 255; I++)
    printf("%02X,",Chr[I]);
  printf("%02X\n",Chr[I]);
}
