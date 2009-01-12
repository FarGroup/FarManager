#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

#if !defined(__USE_OLD_ARRAYS__)

#if 1
  #define  PROC( v )
  #define Log( v )
#else
  #define  PROC( v )  INProc __proc v ;
  #define  Log( v )   INProc::Say v
#endif

#if defined(__GUARD_MEMORY__)
  #define GTYPE         DWORD
  #define G_2_P(v)      ((v) ? ( ((LPBYTE)v)+sizeof(GTYPE) ) : NULL)
  #define P_2_G(v)      ( (GTYPE*)(((LPBYTE)(v))-sizeof(GTYPE)) )
  #define P_2_G2(v)     ( (GTYPE*)( ((LPBYTE)(v))+G_SIZE ) )
  #define G_SET(v,num)  do{ *P_2_G(v) = num; *P_2_G2(v) = 0-(num); }while(0)
  #define G_FILL(num)   GuardSet(this,num)
  #define G_CHECK       GuardCheck(this);

#define G_SIZE        (ar->ItemSize-sizeof(GTYPE)*2)

static void GuardSet( const BaseArray *ar,UINT RenumberStartItem )
  {  LPBYTE p = G_2_P( ar->ItemsPtr ) + RenumberStartItem*ar->ItemSize;

    for ( ; RenumberStartItem < ar->CurrentCount; p += ar->ItemSize,RenumberStartItem++ )
      G_SET( p,RenumberStartItem+1 );
}

static void PGuard( const BaseArray *ar )
  {  LPBYTE p = G_2_P( ar->ItemsPtr );

    printf( "GP %p[%d]=",p,ar->CurrentCount );
    for ( UINT n = 0; n < ar->CurrentCount; p += ar->ItemSize,n++ ) {
      printf( " %d[%d,%d]",n,*P_2_G(p),*P_2_G2(p) );
    }
    printf( "\n" );
}

static void GuardCheck( const BaseArray *ar )
  {  LPBYTE p = G_2_P( ar->ItemsPtr );

    TraceAssert( _HeapCheck() );

    if ( p )
      for ( UINT n = 0; n < ar->CurrentCount; p += ar->ItemSize,n++ ) {
        GTYPE GuardValue;

        GuardValue = n+1;
        if ( *P_2_G(p) != GuardValue ) {
          FILELog( "bsArray: LO check fail to %08X on %d(%d) item(s) in %p",*P_2_G(p),n,ar->Count(),G_2_P(ar->ItemsPtr) );
          TraceAssert( !"!LO checkmark" );
        }

        GuardValue = 0-(n+1);
        if ( *P_2_G2(p) != GuardValue ) {
          FILELog( "bsArray: HI check fail to %08X on %d(%d) item(s) in %p",*P_2_G2(p),n,ar->Count(),G_2_P(ar->ItemsPtr) );
          TraceAssert( !"HI checkmark" );
        }
      }
}

  #undef G_SIZE
  #define G_SIZE        (ItemSize-sizeof(GTYPE)*2)

#else
  #define G_2_P(v)      ((LPBYTE)v)
  #define P_2_G(v)      ((LPBYTE)v)
  #define P_2_G2(v)     ((LPBYTE)v)
  #define G_SET(v,num)
  #define G_FILL(num)
  #define G_SIZE        ItemSize
  #define G_CHECK
#endif

static BaseArray::SortProc UserSortProc;
static int                       CmpItemSize;

static int RTL_CALLBACK idMemCmp( const void *Left,const void *Right )
  {
 return memcmp( G_2_P(Left), G_2_P(Right), CmpItemSize );
}
static int RTL_CALLBACK idUserCmp( const void *Left,const void *Right )
  {
 return UserSortProc( G_2_P(Left), G_2_P(Right) );
}
//---------------------------------------------------------------------------
BaseArray::BaseArray( UINT ItemSz, UINT BeginAllocationCount, UINT AllocationDelta )
      : ItemsPtr(NULL),
        CurrentCount(0),
        MaxCount(0)
  { PROC(( "BaseArray","%p isz: %d ba: %d, ad: %d",this,ItemSz,BeginAllocationCount,AllocationDelta ));

    if ( ItemSz )
      Initialize( ItemSz,BeginAllocationCount,AllocationDelta );
}

BaseArray::~BaseArray()
  {
    Log(( "BaseArray::~BaseArray %p",this ));
    G_CHECK
    _Del(ItemsPtr);
    ItemsPtr = NULL;
}

void BaseArray::Initialize( UINT ItemSz,UINT BeginAllocationCount,UINT AllocationDelta )
  {
    DeleteAllINT();

    ItemSize   = ItemSz;
    AllocDelta = AllocationDelta;

    Assert( ItemSize > 0 );
    Assert( AllocationDelta > 0 );

#if defined(__GUARD_MEMORY__)
    ItemSize += sizeof(GTYPE)*2;
#endif

    if (BeginAllocationCount) Alloc( BeginAllocationCount );
}

LPVOID  BaseArray::ItemNum( int pos )      const { return G_2_P(ItemsPtr) + ItemSize*pos; }
int     BaseArray::ItemPos( LPVOID p )     const { return (int)( ((LPBYTE)p) - G_2_P(ItemsPtr) ) / ItemSize; }
LPVOID  BaseArray::ItemINT( int num )      const { return ( ((UINT)num) >= CurrentCount ) ? NULL : ItemNum(num); }
int     BaseArray::IndexOfINT( LPVOID p )  const { int num = p ? ItemPos(p) : (-1); return ( ((UINT)num) >= CurrentCount ) ? (-1) : num; }
LPVOID  BaseArray::LastObjectINT( void )   const { return ItemINT(CurrentCount-1); }
LPVOID  BaseArray::ItemsINT( void )        const { return ItemsPtr; }
int     BaseArray::Count( void )           const { return (int)CurrentCount; }

void    BaseArray::DeleteAllINT( void )          { CurrentCount = 0; }

//---------------------------------------------------------------------------
void BaseArray::Alloc( int size )
  {
    if ( ((UINT)size) < MaxCount ) return;
    G_CHECK

    MaxCount = size + AllocDelta;
    LPVOID ptr = _Realloc( ItemsPtr,MaxCount*ItemSize );

    Assert( ptr && "!Resize array" );

    ItemsPtr = ptr;
}
//---------------------------------------------------------------------------
LPVOID BaseArray::AddINT( LPVOID p )
  {  LPVOID newP;

    G_CHECK
    Alloc( CurrentCount+1 );

    newP = ItemNum(CurrentCount);

    if ( p )
      memcpy( newP,p,G_SIZE );
     else
      memset( newP,0,G_SIZE );

    CurrentCount++;

    G_SET( newP, CurrentCount );

 return newP;
}
LPVOID BaseArray::AddAtINT( int atPos,LPVOID p )
  {
    G_CHECK
    Alloc( CurrentCount+1 );

    Assert( ((UINT)atPos) <= CurrentCount );

    LPVOID newP = ItemNum(atPos);

  //Add
    CurrentCount++;
    atPos++;

  //Move tail
    if ( ((UINT)atPos) < CurrentCount ) {
      memmove( ((LPBYTE)P_2_G(newP)) + ItemSize, ((LPBYTE)P_2_G(newP)), ItemSize * (CurrentCount-atPos) );
      G_FILL( atPos );
    } else
      G_SET( newP, atPos );

  //Copy new item data
    if ( p )
      memcpy( newP,p,G_SIZE );
     else
      memset( p,0,G_SIZE );

 return newP;
}
//---------------------------------------------------------------------------
void BaseArray::DeleteINT( LPVOID p )
  {
     DeleteNumINT( IndexOfINT(p) );
}

void BaseArray::DeleteRange( int start, int end )
  {
     if ( !CurrentCount ) return;

     G_CHECK
     start = Max( start,0 );
     end   = (int)Min( (UINT)end,CurrentCount-1 );

     if ( start >= end ) return;

     LPVOID p = ItemNum(start);
     memmove( ((LPBYTE)P_2_G(p)),
              ((LPBYTE)P_2_G(p)) + (end-start)*ItemSize,
              ItemSize*(CurrentCount-end-1) );

     CurrentCount -= end-start;
     G_FILL( start );
}

void BaseArray::DeleteNumINT( int num )
  {
     G_CHECK
     if ( ((UINT)num) >= ((UINT)CurrentCount) ) return;
    //Sub
     CurrentCount--;
    //Move tail
     LPVOID p = ItemNum(num);
     memmove( ((LPBYTE)P_2_G(p)), ((LPBYTE)P_2_G(p)) + ItemSize, ItemSize*(CurrentCount-num) );

     G_FILL( num );
}
//---------------------------------------------------------------------------
void BaseArray::SortINT( BaseArray::SortProc sp, int start, int end )
   {
     G_CHECK
     if ( !sp ) {
       sp           = idMemCmp;
       CmpItemSize  = G_SIZE;
     } else {
       UserSortProc = sp;
       sp           = idUserCmp;
     }

     if ( start < 0 ) start = 0;              else start = Max( 0,start );
     if ( end   < 0 ) end   = CurrentCount-1; else end   = Min( (int)CurrentCount-1, end );
     if ( end-start > 1 ) {
       qsort( ((LPBYTE)ItemsPtr) + start*ItemSize, end-start+1, ItemSize, sp );
       G_FILL(0);
     }
}

int BaseArray::BSearchINT( int left, int right, LPVOID key, BaseArray::SortProc Cmp,int *pos ) const
  {  int lres,rres;

#define ITEM(n) (((LPBYTE)ItemsPtr) + ItemSize*n)

     lres = Cmp( key,ITEM(left) );
     if ( lres == 0 ) return left;
     if ( lres < 0 )  {
       if (pos) *pos = left;
       return -1;
     }

     if ( left == right ) {
       if (pos) *pos = left+1;
       return -1;
     }

     rres = Cmp( key,ITEM(right) );
     if ( rres == 0 ) return right;
     if ( rres > 0 ) {
       if (pos) *pos = right+1;
       return -1;
     }

     if ( right - left == 1 ) {
       if (pos) *pos = right;
       return -1;
     }

     lres = (left + right) / 2;
     rres = Cmp( key,ITEM(lres) );

     if ( rres == 0 )
       return lres;

     if ( rres < 0 )
       return BSearchINT( left+1,lres-1,key,Cmp,pos );
      else
       return BSearchINT( lres+1,right-1,key,Cmp,pos );
}

int BaseArray::SearchINT( LPVOID key,BaseArray::SortProc sp,int *NearestPosition ) const
   {
     G_CHECK

     if ( NearestPosition )
       *NearestPosition = 0;

     if ( CurrentCount == 0 )
       return -1;

     if ( !sp ) {
       sp          = idMemCmp;
       CmpItemSize = ItemSize;
     } else {
       UserSortProc = sp;
       sp           = idUserCmp;
     }

 return BSearchINT( 0,CurrentCount-1,P_2_G(key),sp,NearestPosition );
}

int BaseArray::LSearchINT( LPVOID key,int StartFrom,BaseArray::SortProc sp ) const
  {  LPBYTE p;

    G_CHECK

    if ( ((UINT)StartFrom) >= CurrentCount )
      return -1;

    p = G_2_P(ItemsPtr) + StartFrom*ItemSize;

    for ( ; ((UINT)StartFrom ) < CurrentCount; p += ItemSize, StartFrom++ )
      if (sp) {
        if ( sp(key,p) == 0 )
          return StartFrom;
      } else {
        if ( memcmp(key,p,G_SIZE) == 0 )
          return StartFrom;
      }
 return -1;
}
//---------------------------------------------------------------------------
void BaseArray::Swap( int num,int num1 )
  {
     G_CHECK

    if ( num == num1 ||
         ((UINT)num)  >= ((UINT)CurrentCount) ||
         ((UINT)num1) >= ((UINT)CurrentCount) )
      return;

    LPBYTE p  = G_2_P(ItemsPtr) + num*ItemSize;
    LPBYTE p1 = G_2_P(ItemsPtr) + num1*ItemSize;

    UINT   dVal;
    LPVOID buff = &dVal;

    if ( G_SIZE > sizeof(dVal) )
      buff = _Alloc( G_SIZE );

    memcpy( buff, p,    G_SIZE );
    memcpy( p,    p1,   G_SIZE );
    memcpy( p1,   buff, G_SIZE );

    if ( G_SIZE > sizeof(dVal) )
      _Del( buff );

    G_SET( p,  num+1 );
    G_SET( p1, num1+1 );
}

#endif
