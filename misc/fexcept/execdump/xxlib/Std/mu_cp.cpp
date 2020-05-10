#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif


static const char* stdEngChars = "QWERTYUIOP[]ASDFGHJKL;\'ZXCVBNM,.qwertyuiop[]asdfghjkl:\"zxcvbnm<>";
static const char* stdRusChars[CT_CP_MAXTABLES] = {
  "����������������������������������㪥�������뢠�஫�����ᬨ���",
  "������������������������������������������������������� ��������",
  "��ú����ɷ���˲��������������̱���������������������������������",
  "����������� ����������������������������������������������������",
  "����������������������������������������������������������������",
  "������������������������������������������� �������������������",
  "��嫥���������⯬�����㭩�聠��������������������������������"
  };

char MYRTLEXP Eng2Rus( char ch,int tablenum )
  {  int n;
    if ( tablenum < 0 || tablenum >= CT_CP_MAXTABLES ||
         (n=StrNChr(stdEngChars,ch)) == -1 ) return ch;
 return stdRusChars[tablenum][n];
}

char MYRTLEXP Rus2Eng( char ch,int tablenum )
  {  int n;
    if ( tablenum < 0 || tablenum >= CT_CP_MAXTABLES ||
         (n=StrNChr(stdRusChars[tablenum],ch)) == -1 ) return ch;
 return stdEngChars[n];
}

char MYRTLEXP Rus2CP( char ch,int tablefrom, int tableto )
  {  int n;

    if ( tablefrom < 0 || tablefrom >= CT_CP_MAXTABLES ||
         tableto   < 0 || tableto   >= CT_CP_MAXTABLES ||
         (n=StrNChr(stdRusChars[tablefrom],ch)) == -1 ) return ch;

  return stdRusChars[tableto][n];
}
