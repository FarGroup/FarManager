#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

static MyString resStrSCol;

int MYRTLEXP StrColCount( CONSTSTR str,CONSTSTR seps )
  {  int res = 1;

     if ( !str || !seps || !str[0] || !seps[0] ) return 0;

     for ( int n = 0; str[n]; n++ )
       if ( StrChr(seps,str[n]) != NULL )
         res++;
  return res;
}

CONSTSTR MYRTLEXP StrGetCol( MyString& buff, CONSTSTR str,int number,CONSTSTR seps )
  {  int res;

     for ( res = 1; *str && res < number; str++ )
       if ( StrChr(seps,*str) != NULL ) res++;
     buff = "";
     if ( res == number )
       for( ; *str && StrChr(seps,*str) == NULL; str++ )
         buff.Add(*str);
 return buff.Text();
}

CONSTSTR MYRTLEXP StrGetCol( CONSTSTR str,int number,CONSTSTR seps )
  {
 return StrGetCol( resStrSCol, str, number, seps );
}

CONSTSTR MYRTLEXP StrDelCol( MyString& buff, CONSTSTR str,int number,CONSTSTR seps )
  {  int res;

     buff = "";
     for ( res = 1; *str && res < number; buff.Add(*str),str++ )
       if ( StrChr(seps,*str) != NULL )
         res++;

     if ( res == number )
       for( ; *str && StrChr(seps,*str) == NULL; str++ )
         /**/;

     for( ; *str; str++ )
       buff.Add(*str);
 return buff.Text();
}

CONSTSTR MYRTLEXP StrDelCol( CONSTSTR str,int number,CONSTSTR seps )
  {
 return StrDelCol( resStrSCol, str, number, seps );
}

int MYRTLEXP StrFindCol( CONSTSTR str,CONSTSTR seps,CONSTSTR col,int fromCol )
  {  int cn = StrColCount(str,seps);
     fromCol = (fromCol<1)?1:(fromCol);
     if (col )
       for ( int n = fromCol; n <= cn; n++ )
         if ( strcmp(StrGetCol(str,n,seps),col) == 0  ) return n;
 return -1;
}

BOOL MYRTLEXP StrContainCol( CONSTSTR str,CONSTSTR seps,CONSTSTR col )
  {
  return StrFindCol(str,seps,col) != -1;
}

CONSTSTR MYRTLEXP StrDelStr( CONSTSTR str,CONSTSTR subStr,int pos )
  {  CONSTSTR m = (str && subStr)?strstr(str+Min(pos,(int)strLen(str)),subStr):NULL;
    resStrSCol = "";
    if ( !m ) {
      if (str) resStrSCol = str;
      return resStrSCol.Text();
    }
    for ( ; *str && str != m; str++ ) resStrSCol.Add(*str);
    for ( ; *str && *subStr; subStr++,str++ );
    for ( ; *str; str++ ) resStrSCol.Add(*str);
 return resStrSCol.Text();
}
