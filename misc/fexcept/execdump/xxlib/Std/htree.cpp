#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

/*******************************************************************
   LOCAL PROCEDURES
 *******************************************************************/
static HDataPtr _tmpName;

static CONSTSTR HTNextName( CONSTSTR nm )
  {
    while( *nm && (*nm == '\\' || *nm == '/') )
      nm++;
 return nm;
}

/*******************************************************************
   HTreeValue
 *******************************************************************/
HTreeBase::HTreeBase( PHDataPool p )
  {
    Pool = p;
    Pool->AddRef();
}
HTreeBase::~HTreeBase()
  {
    Pool->ReleaseRef();
    _tmpName.Free();
}
/*******************************************************************
   HTreeValue
 *******************************************************************/
HTreeValue::HTreeValue( void )
  {
    Tree = NULL;
    Parent = NULL;
}

HTreeValue::~HTreeValue()
  {
    Clear();
}

CONSTSTR HTreeValue::Name( void )            { return GetName(); }
CONSTSTR HTreeValue::GetClassName( void )    { return ClassName(); }
void     HTreeValue::Clear( void )           { Items.DeleteAll(); }
BOOL     HTreeValue::isRoot( void )          { return Parent == NULL; }
BOOL     HTreeValue::isLeath( void )         { return Items.Count() == 0; }
int      HTreeValue::BackCount( void ) const { return BackCount(this); }
BOOL     HTreeValue::DoLoadItem( int /*File*/ ){ return TRUE; }
BOOL     HTreeValue::DoSaveItem( int /*File*/ ){ return TRUE; }

BOOL HTreeValue::Enum( HTreeEnum cb,int Level,HANDLE ptr ) const
  {
    for ( int n = 0; n < Items.Count(); n++ ) {
      if ( !cb(Items[n],Level,ptr) ) return FALSE;
      Items[n]->Enum(cb,Level+1,ptr);
    }
 return TRUE;
}
BOOL HTreeValue::EnumBack( HTreeEnum cb,int Level,HANDLE ptr ) const
  {
    if ( !Parent ) return TRUE;
 return cb(Parent,Level,ptr) && Parent->EnumBack(cb,Level+1,ptr);
}
#if defined(__BCWIN32__)
BOOL HTreeValue::EnumCL( HTreeEnumCL cb,int Level ) const
  {
    for ( int n = 0; n < Items.Count(); n++ ) {
      if ( !cb(Items[n],Level) ) return FALSE;
      Items[n]->EnumCL(cb,Level+1);
    }
 return TRUE;
}
BOOL HTreeValue::EnumBackCL( HTreeEnumCL cb,int Level ) const
  {
    for ( int n = 0; n < Items.Count(); n++ ) {
      if ( !cb(Items[n],Level) ) return FALSE;
      Items[n]->EnumCL(cb,Level+1);
    }
 return TRUE;
}
#endif

int HTreeValue::BackCount( HTreeValue const *p )
  { int cn;
    for( cn = 0; (p=p->Parent) != NULL; cn++);
 return cn;
}

PHTreeValue HTreeValue::intFindValue( CONSTSTR nm )
  {
     for ( int n = 0; n < Items.Count(); n++ ) {
       PHTreeValue p = Items[n];
       if ( StrCmp(p->Name(),nm) == 0 )
         return p;
     }
 return NULL;
}

PHTreeValue HTreeValue::intLocate( CONSTSTR PathName )
  {
     if ( !PathName || !PathName[0] ) return this;

     MyString s;
     StrGetCol( s, PathName, 1, HT_KEYDELIMITER );
     if ( !s.Length() ) return this;

     for ( int n = 0; n < Items.Count(); n++ ) {
       PHTreeValue p = Items[n];
       if ( StrCmp(p->Name(),s.c_str()) == 0 )
         return p->intLocate( HTNextName(PathName+s.Length()) );
     }
 return NULL;
}

PHTreeValue HTreeValue::DoAdd( CONSTSTR PathName )
  {  PHTreeValue p;

     if ( !PathName || !PathName[0] ) return this;

     MyString str;
     StrGetCol( str, PathName,1,HT_KEYDELIMITER);
     if ( !str.Length() ) return this;

     for ( int n = 0; n < Items.Count(); n++ ) {
       p = Items[n];
       if ( StrCmp(p->Name(),str.c_str()) == 0 )
         return p->DoAdd( HTNextName(PathName+str.Length()) );
     }

     p = AddItem( str.c_str(),this);
     if ( !p )
       return NULL;

     PathName = HTNextName(PathName+str.Length());
     if ( !PathName[0] )
       return p;
      else
       return p->DoAdd(PathName);
}

int HTreeValue::FullCount( void )
  {  int dw = Items.Count();
     for ( int n = 0; n < Items.Count(); n++ )
       dw += 1 + Items[n]->FullCount();
 return dw;
}

PHTreeValue HTreeValue::AddItem( CONSTSTR Name,PHTreeValue Into )
  {  PHTreeValue p = DoCreate(Name);
     if ( !p ) return NULL;
     if (Into) Into->Items.Add( p );
     p->Tree   = Tree;
     p->Parent = Into;
 return p;
}

CONSTSTR HTreeValue::ReadFileClassName( CONSTSTR FileName )
  {  int      f = FIO_OPEN( FileName,O_RDONLY );
     CONSTSTR m;

     if ( f == -1 ) return NULL;
     m = ReadFileClassName(f);
     FIO_CLOSE(f);
 return m;
}

CONSTSTR HTreeValue::ReadFileClassName( int f )
  {  DWORD dw;

   if ( FIO_READ(f,&dw,sizeof(dw)) != sizeof(dw) ||
        dw != HT_MAGIC ||
        FIO_READ(f,&dw,sizeof(dw)) != sizeof(dw) )
     return NULL;

   if ( !_tmpName.Resize( dw+1 ) ||
        FIO_READ(f,_tmpName.Ptr(),dw) != (int)dw )
     return NULL;

   ((pchar)_tmpName.Ptr())[dw] = 0;

 return (CONSTSTR)_tmpName.Ptr();
}

BOOL HTreeValue::intLoad( CONSTSTR FileName )
  {  int      f   = FIO_OPEN( FileName,O_RDONLY );
     BOOL     res = FALSE;
     CONSTSTR m;

    do{
     if ( f == -1 ||
          (m=ReadFileClassName(f)) == NULL ||
          StrCmp(m,ClassName()) != 0 )
       break;

     Clear();
     res = DoLoadTree(f);
     if (!res)
       Clear();
   }while(0);

   if ( f != -1 )
     FIO_CLOSE(f);

 return res;
}

BOOL HTreeValue::intSave( CONSTSTR FileName )
  {  int   f   = FIO_OPEN( FileName,O_RDWR | O_TRUNC );
     DWORD res = FALSE;
     DWORD dw;

     if ( f == -1 )
       f = FIO_CREAT( FileName,0 );

     if ( f == -1 )
       return FALSE;

     do{
       dw = HT_MAGIC;
       if ( FIO_WRITE(f,&dw,sizeof(dw)) != sizeof(dw) ) break;
       dw = strLen(ClassName());

       if ( FIO_WRITE(f,&dw,sizeof(dw)) != sizeof(dw) ) break;
       if ( FIO_WRITE(f,ClassName(),dw) != (int)dw )    break;

       res = DoSaveTree(f);

     }while(0);

     FIO_CLOSE(f);
     if (!res)
       DeleteFile(FileName);

 return (BOOL)res;
}

BOOL HTreeValue::DoSaveTree( int File )
  {  DWORD       dw;
     WORD        w;
     PHTreeValue p;

    dw = HT_MAGIC;
    if ( FIO_WRITE(File,&dw,sizeof(dw)) != sizeof(dw) ) return FALSE;
    dw = (DWORD)Items.Count();
    if ( FIO_WRITE(File,&dw,sizeof(dw)) != sizeof(dw) ) return FALSE;

    for ( int n = 0; n < Items.Count(); n++ ) {
      p = Items[n];
      w = (WORD)strLen( p->Name() );
      if ( FIO_WRITE(File,&w,sizeof(w)) != sizeof(w) ) return FALSE;
      if ( FIO_WRITE(File,p->Name(),w)  != w         ) return FALSE;
      if ( !p->DoSaveItem(File)                      ) return FALSE;
      if ( !p->DoSaveTree(File)                      ) return FALSE;
    }
 return TRUE;
}

BOOL HTreeValue::DoLoadTree( int File )
  {  WORD        w;
     DWORD       dw,cn;
     PHTreeValue p;

    if ( FIO_READ(File,&dw,sizeof(dw)) != sizeof(dw) ||
         dw                            != HT_MAGIC   ||
         FIO_READ(File,&cn,sizeof(cn)) != sizeof(cn)
       ) return FALSE;

    for ( int n = 0; n < (int)cn; n++ ) {
      if ( FIO_READ(File,&w,sizeof(w)) != sizeof(w)  ||
           !_tmpName.Resize(w+1) ||
           FIO_READ(File,_tmpName.Ptr(),w) != w
         ) break;

      ((pchar)_tmpName.Ptr())[w] = 0;

      if ( (p=AddItem((CONSTSTR)_tmpName.Ptr(),this)) == NULL ||
           !p->DoLoadItem(File) ||
           !p->DoLoadTree(File) )
        return FALSE;
    }
 return TRUE;
}
