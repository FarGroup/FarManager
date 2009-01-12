#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

/*******************************************************************
   DATA
 *******************************************************************/
static HBTreeBase::HBTreeEnum HBTEnumCB;
static HANDLE                 HBTUPtr;
static int                    CBLevel;

/*******************************************************************
   HBTreeBase
 *******************************************************************/
HBTreeBase::HBTreeBase( void )
  {
    Parent = Left = Right = NULL;
}

HBTreeBase::~HBTreeBase()
  {
    Clear();
}

BOOL HBTreeBase::DoEnum( void ) const
  {
    CBLevel++;
    if (Left)    if ( !HBTEnumCB(Left,CBLevel+1,HBTUPtr) )  return FALSE;
    if (Right)   if ( !HBTEnumCB(Right,CBLevel+1,HBTUPtr) ) return FALSE;
    if (Left)    if ( !Left->DoEnum() )                     return FALSE;
    if (Right)   if ( !Right->DoEnum() )                    return FALSE;
    CBLevel--;
 return TRUE;
}

BOOL HBTreeBase::DoEnumSort( void ) const
  {
    CBLevel++;
    if (Right)   if ( !Right->DoEnumSort() )                return FALSE;
    if (Right)   if ( !HBTEnumCB(Right,CBLevel+1,HBTUPtr) ) return FALSE;
    if (Left)    if ( !HBTEnumCB(Left,CBLevel+1,HBTUPtr) )  return FALSE;
    if (Left)    if ( !Left->DoEnumSort() )                 return FALSE;
    CBLevel--;
 return TRUE;
}

BOOL HBTreeBase::Enum( HBTreeEnum CB, HANDLE Ptr ) const
  {
    HBTEnumCB = CB;
    HBTUPtr   = Ptr;
    CBLevel   = -2;
 return DoEnum();
}

BOOL HBTreeBase::EnumSort( HBTreeEnum CB, HANDLE Ptr ) const
  {
    HBTEnumCB = CB;
    HBTUPtr   = Ptr;
    CBLevel   = -2;
 return DoEnumSort();
}
#if defined(__BCWIN32__)
static HBTreeBase::HBTreeEnumCL HBTEnumCB_CL;

BOOL HBTreeBase::DoEnumCL( void ) const
  {
    CBLevel++;
    if (Left)    if ( !HBTEnumCB_CL(Left,CBLevel+1) )  return FALSE;
    if (Right)   if ( !HBTEnumCB_CL(Right,CBLevel+1) ) return FALSE;
    if (Left)    if ( !Left->DoEnumCL() )    return FALSE;
    if (Right)   if ( !Right->DoEnumCL() )   return FALSE;
    CBLevel--;
 return TRUE;
}

BOOL HBTreeBase::DoEnumSortCL( void ) const
  {
    CBLevel++;
    if (Right)   if ( !Right->DoEnumSortCL() )         return FALSE;
    if (Right)   if ( !HBTEnumCB_CL(Right,CBLevel+1) ) return FALSE;
    if (Left)    if ( !HBTEnumCB_CL(Left,CBLevel+1) )  return FALSE;
    if (Left)    if ( !Left->DoEnumSortCL() )          return FALSE;
    CBLevel--;
 return TRUE;
}

BOOL HBTreeBase::EnumCL( HBTreeEnumCL CB ) const
  {
    HBTEnumCB_CL = CB;
    CBLevel      = -2;
 return DoEnumCL();
}

BOOL HBTreeBase::EnumSortCL( HBTreeEnumCL CB ) const
  {
    HBTEnumCB_CL = CB;
    CBLevel      = -2;
 return DoEnumSortCL();
}
#endif

void HBTreeBase::UnlinkSubtree( void )
  {
    if ( !Parent ) return;
    if (Parent->Left == this) Parent->Left   = NULL; else
    if (Parent->Right == this) Parent->Right = NULL;
    Parent = NULL;
}

void HBTreeBase::UnlinkSelf( void )
  {  PHBTreeBase p = Parent;

    if ( !p ) return;

    UnlinkSubtree();

    if (Left) {
      p->Insert( Left );
      Left = NULL;
    }
    if (Right) {
      p->Insert( Right );
      Right = NULL;
    }
}

BOOL HBTreeBase::Insert( PHBTreeBase p )
  {  maCmp cmp = Cmp(p);
     if ( cmp == masEqual ) return FALSE;
     if ( cmp == masMore ) {
       if ( Left ) return Left->Insert(p);
       Left = p;
       Left->Parent = this;
     } else {
       if ( Right ) return Right->Insert(p);
       Right = p;
       Right->Parent = this;
     }
 return TRUE;
}

PHBTreeBase HBTreeBase::Add( HANDLE nm )
  {  maCmp cmp = Cmp(nm);

     if ( cmp == masEqual ) return NULL;

     if ( cmp == masMore ) {
       if ( Left ) return Left->Add(nm);
       Left = DoCreate( nm );
       if ( !Left ) return NULL;
       Left->Parent = this;
       return Left;
     } else {
       if ( Right ) return Right->Add(nm);
       Right = DoCreate( nm );
       if ( !Right) return NULL;
       Right->Parent = this;
       return Right;
     }
}

PHBTreeBase HBTreeBase::Locate( HANDLE nm )
  {  maCmp cmp = Cmp(nm);
     if ( cmp == masEqual ) return this;
     if ( cmp == masMore )
       return (Left)?Left->Locate(nm):NULL;
      else
       return (Right)?Right->Locate(nm):NULL;
}

void HBTreeBase::DoDeleteLeath( PHBTreeBase& p )
  {
    if (p) {
      delete p;
      p = NULL;
    }
}
/*******************************************************************
   HBNumTree
 *******************************************************************/
maCmp HBNumTree::Cmp( HANDLE val )
  {  int   n;
     int   cmp  = masEqual;
     DWORD self = (DWORD)(ID),
           v    = (DWORD)(val);

     if ( v == self ) return masEqual;

     for ( n = 0; n < (sizeof(DWORD)/sizeof(BYTE))*2; self >>= 4,v >>= 4,n++ ) {
       cmp = ((int)(self&0xF)) - ((int)(v&0xF));
       if (cmp) break;
     }
 return Int2Cmp(cmp);
}

maCmp HBNumTree::Cmp( PHBTreeBase p )
  {
 return Cmp(((PHBNumTree)p)->ID);
}

PHBTreeBase HBNumTree::DoCreate( HANDLE Param )
  {
 return new HBNumTree((HANDLE)Param);
}
