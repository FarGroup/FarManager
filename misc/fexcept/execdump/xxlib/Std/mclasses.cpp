#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

/************************************
   DATA
 ************************************/
jmp_buf  jbuff[ MAX_CATCHS ];
int      curblock = -1;
MSG_TYPE jpara;

static BOOL caseSens = TRUE;

/************************************
   HRefObject
 ************************************/
HRefObject::HRefObject( void )
    : rCount(0)
  {
    InitializeCriticalSection( &cs );
}

HRefObject::~HRefObject()
  {
     DeleteCriticalSection( &cs );
     Assert( rCount == 0 );
}

int HRefObject::RefCount( void )
  {
 return rCount;
}

void HRefObject::AddRef( void )
  {
    EnterCriticalSection( &cs );
    rCount++;
    LeaveCriticalSection( &cs );
}

void HRefObject::ReleaseRef( void )
  {
    EnterCriticalSection( &cs );
    rCount--;
    LeaveCriticalSection( &cs );
    if ( rCount <= 0 )
      delete this;
}

/************************************
 ************************************/
int RTL_CALLBACK MyStringSortProc( PMyString const *l, PMyString const *r )          { return strcmp( (*l)->Text(),(*r)->Text() ); }
int RTL_CALLBACK MyStringSearchProc( PMyString const *l, PMyString const *r )        { return StrCmp( (*l)->Text(),(*r)->Text(),-1,caseSens ); }
int RTL_CALLBACK MyStringIncSearchProc( PMyString const *l, PMyString const *r )     { return StrCmp( (*l)->Text(),(*r)->Text(),(*r)->Length()-1,caseSens ); }
int RTL_CALLBACK MyStringSearchCharProc( PMyString const *l, PMyString const *r )    { return StrCmp( (*l)->Text(),(*r)->Text(),-1,caseSens ); }
int RTL_CALLBACK MyStringIncSearchCharProc( PMyString const *l, PMyString const *r ) { return StrCmp( (*l)->Text(),(*r)->Text(),(*r)->Length()-1,caseSens ); }

/************************************
  CTStringArray
 ************************************/
CTStringArray::CTStringArray( int begCount,BOOL DelObj,int ad )
   : MyArray<PMyString>( begCount,DelObj,ad )
  {
}
PMyString CTStringArray::Add( CONSTSTR str )      { return MyArray<PMyString>::Add( new MyString(str) ); }
PMyString CTStringArray::Add( PMyString s )       { return MyArray<PMyString>::Add( s ); }
PMyString CTStringArray::Add( const MyString& s ) { return MyArray<PMyString>::Add( new MyString(s) ); }

#if !defined( __GNUC__ )
/************************************
  SortedStringArray
 ************************************/
SortedStringArray::SortedStringArray( BOOL Sorted,int begCount,BOOL DelObj,int ad )
   : CTStringArray( begCount,DelObj,ad )
  {
    sorted = Sorted;
}

PMyString SortedStringArray::Add( CONSTSTR p )
  {
 return Add( new MyString(p) );
}

PMyString SortedStringArray::Add( PMyString ptr )
  {  PMyString p = MyArray<PMyString>::Add(ptr);
     if (p && Count() > 1)
       Sort( MyStringSortProc );
 return p;
}

PMyString SortedStringArray::AddAt( int pos,PMyString ptr )
  {  PMyString p = MyArray<PMyString>::AddAt(pos,ptr);
     if (p && Count() > 1)
       Sort(MyStringSortProc);
 return p;
}

void SortedStringArray::DeleteNum( int num,BOOL delItem )
  {
    MyArray<PMyString>::DeleteNum( num,delItem );
}

int SortedStringArray::IndexOf( const MyString& val,BOOL caseSens )
  {
   ::caseSens = caseSens;
   return Search( (PMyString)&val,MyStringSearchProc );
}

int SortedStringArray::IncIndexOf( const MyString& val,BOOL caseSens )
  {
   ::caseSens = caseSens;
   return Search( (PMyString)&val,MyStringIncSearchProc );
}

int SortedStringArray::IndexOf( const char *val,BOOL caseSens )
  {  MyString s( val );
     ::caseSens = caseSens;
   return Search( &s,MyStringSearchCharProc );
}

int SortedStringArray::IncIndexOf( const char *val,BOOL caseSens )
  {  MyString s( val );
     ::caseSens = caseSens;
   return Search( &s,MyStringIncSearchCharProc );
}
#endif