#ifndef __MY_BASE_ARRAY
#define __MY_BASE_ARRAY

/************************************
   BaseArray
 ************************************/
CLASS( BaseArray )
  public:
    typedef int (RTL_CALLBACK *SortProc)( const void *left, const void *right );
  public:
    UINT   ItemSize;      //Includes double size of guard region
    LPVOID ItemsPtr;
    UINT   CurrentCount,
           MaxCount,
           AllocDelta;
  protected:
    LPVOID  ItemNum( int pos )  const;
    int     ItemPos( LPVOID p ) const;
    int     BSearchINT( int left, int right, LPVOID key, BaseArray::SortProc Cmp,int *pos ) const;
  protected:
    BaseArray( UINT ItemSz/*=0*/,  //If ==0 `Initialize` must be used
                                UINT BeginAllocationCount = ALLOC_DELTA,
                                UINT AllocationDelta = ALLOC_DELTA );

    virtual ~BaseArray();

    void   Initialize( UINT ItemSz/*!=0*/,
                                  UINT BeginAllocationCount = ALLOC_DELTA,
                                  UINT AllocationDelta = ALLOC_DELTA );

    LPVOID AddINT( LPVOID p );
    LPVOID AddAtINT( int pos,LPVOID p );
    void   DeleteAllINT( void );
    void   DeleteINT( LPVOID p );
    void   DeleteNumINT( int num );

//QSearch
    void   SortINT( BaseArray::SortProc sp/*=NULL*/, int start /*=-1*/, int end /*=-1*/ );

//Search in sorted array
    int    SearchINT( LPVOID key,BaseArray::SortProc sp/*=NULL*/,int *NearestPosition/*=NULL*/ ) const;

//Linear search
    int    LSearchINT( LPVOID key,int StartFrom/*=0*/,BaseArray::SortProc sp/*=NULL*/ ) const;

    LPVOID ItemINT( int num )       const;
    int    IndexOfINT( LPVOID p )   const;

    LPVOID LastObjectINT( void )    const;
    LPVOID ItemsINT( void )         const;
  public:
    int    Count( void )            const;
    void   Swap( int num,int num1 );
    void   Alloc( int AtLeastThisSize );
    void   DeleteRange( int start, int end );
};

/************************************
   ValArray
 ************************************/
template <class ValT> class MyValArray : public BaseArray {
  public:
    typedef int (RTL_CALLBACK *SortProc)( const ValT *left,const ValT *right );
  public:
    MyValArray( int begCount = ALLOC_DELTA,int ad = ALLOC_DELTA )
      : BaseArray( sizeof(ValT),begCount,ad ) {}

    ValT   AddEmpty( void )                                      { return *(ValT*)AddINT( NULL ); }
    ValT   Add( const ValT& p )                                  { return *(ValT*)AddINT( (LPVOID)&p ); }
    ValT   AddAt( int pos,const ValT& p )                        { return *(ValT*)AddAtINT( pos,(LPVOID)&p ); }
    void   Delete( const ValT& v )                               { DeleteNum( IndexOf(v) ); }
    void   DeleteAll( void )                                     { DeleteAllINT(); }
    void   DeleteNum( int num )                                  { DeleteNumINT(num); }
    ValT   Item( int num )                                 const { ValT *p = (ValT*)ItemINT(num); return p ? (*p) : ((ValT)0); }
    ValT  *Items( void )                                   const { return (ValT*)ItemINT(0); }
    ValT&  operator[]( int num )                           const { /*TraceAssert( ((UINT)num) < ((UINT)Count()) );*/ return *(ValT*)ItemINT(num); }
    int    IndexOf( const ValT& v )                        const { return LSearch(v,NULL); }
    ValT   LastObject( void )                              const { return *(ValT*)LastObjectINT(); }

#undef _N_CPP
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
#define _SP_  SortProc
#else
#define _SP_  MyValArray<ValT>::SortProc
#endif
    void   Sort( _SP_ sp, int start = -1, int end = -1 )         { SortINT( (BaseArray::SortProc)sp,start,end ); }
    int    Search( const ValT& key, _SP_ sp = NULL,
                   int *NearestPosition = NULL ) const           { return SearchINT( (LPVOID)&key,(BaseArray::SortProc)sp,NearestPosition ); }
    int    LSearch( const ValT& key,int StartFrom = 0,
                    _SP_ sp = NULL )                       const { return LSearchINT( (LPVOID)&key,StartFrom,(BaseArray::SortProc)sp ); }
#undef _SP_

#if defined(HAS_INLINE_ENUM)
    template <class ItteratorT,class Data> ValT Enum( ItteratorT itterator, Data d )
      {  ValT it;
        for ( int n = 0; n < Count(); n++ )
          if ( itterator.Compare(it=Item(n),n,d) )
            return it;
     return NULL;
    }
    template <class ItteratorT,class Data> int EnumNum( ItteratorT itterator, Data d )
      {
        for ( int n = 0; n < Count(); n++ )
          if ( itterator.Compare(Item(n),n,d) )
            return n;
     return -1;
    }
#endif
};

/************************************
   RefArray
 ************************************/
template <class ValT> class MyRefArray : public BaseArray {
  public:
    typedef int (RTL_CALLBACK *SortProc)( const ValT *left,const ValT *right );
  public:
    MyRefArray( int begCount = ALLOC_DELTA,int ad = ALLOC_DELTA )
      : BaseArray( sizeof(ValT),begCount,ad ) {}

    virtual ~MyRefArray() { DeleteAll(); }

    virtual void DeleteAll( void )                               { DeleteAllINT(); }
    virtual void DeleteNum( int num )                            { DeleteNumINT(num); }

    ValT  *AddEmpty( void )                                      { return (ValT*)AddINT( NULL ); }
    ValT  *Add( const ValT& p )                                  { return (ValT*)AddINT( (LPVOID)&p ); }
    ValT  *AddAt( int pos,const ValT& p )                        { return (ValT*)AddAtINT( pos,(LPVOID)&p ); }
    void   Delete( ValT *p )                                     { DeleteNum( IndexOf(p) ); }
    ValT  *Item( int num )                                 const { return (ValT*)ItemINT(num); }
    ValT  *Items( void )                                   const { return (ValT*)ItemINT(0); }
    ValT  *operator[]( int num )                           const { /*TraceAssert( ((UINT)num) < ((UINT)Count()) );*/ return (ValT*)ItemINT(num); }
    int    IndexOf( ValT *p )                              const { return IndexOfINT( p ); }
    ValT  *LastObject( void )                              const { return (ValT*)LastObjectINT(); }

#undef _N_CPP
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
#define _SP_  SortProc
#else
#define _SP_  MyRefArray<ValT>::SortProc
#endif
    void   Sort( _SP_ sp, int start = -1, int end = -1 )         { SortINT( (BaseArray::SortProc)sp,start,end ); }
    int    Search( const ValT& key,
                   _SP_ sp = NULL,
                   int *NearestPosition = NULL )           const { return SearchINT( (LPVOID)&key,(BaseArray::SortProc)sp,NearestPosition ); }
    int    LSearch( const ValT& key,int StartFrom = 0,
                    _SP_ sp = NULL )                       const { return LSearchINT( (LPVOID)&key,StartFrom,(BaseArray::SortProc)sp ); }
#undef _SP_

#if defined(HAS_INLINE_ENUM)
    template <class ItteratorT,class Data> ValT *Enum( ItteratorT itterator, Data d )
      {  ValT *it;
        for ( int n = 0; n < Count(); n++ )
          if ( itterator.Compare(it=Item(n),n,d) )
            return it;
     return NULL;
    }
    template <class ItteratorT,class Data> int EnumNum( ItteratorT itterator, Data d )
      {
        for ( int n = 0; n < Count(); n++ )
          if ( itterator.Compare(Item(n),n,d) )
            return n;
     return -1;
    }
#endif
};
/************************************
   Array
 ************************************/
template <class ValT> class MyArray : public BaseArray {
    BOOL NeedToDelete;
  public:
    typedef int (RTL_CALLBACK *SortProc)( const ValT *left,const ValT *right );
  protected:
    virtual void ItemAdded( ValT /*ptr*/ );
    virtual void ItemDeleted( ValT ptr,BOOL FreePtr );
  public:
    MyArray( int begCount = ALLOC_DELTA,
             BOOL DelItems = TRUE,
             int ad = ALLOC_DELTA )
      : BaseArray( sizeof(void*),begCount,ad )
        { NeedToDelete = DelItems; }

    virtual ~MyArray()    { DeleteAll(); }

    ValT  Add( ValT p );
    ValT  AddAt( int pos,ValT p );
    void  DeleteNum( int num,BOOL del = TRUE );
    void  DeleteAll( void );
    void  Delete( ValT p,BOOL del = TRUE )                   { DeleteNum(IndexOf(p),del); }
    ValT  Item( int num )                              const { ValT *p = (ValT*)ItemINT(num); return p ? (*p) : NULL; }
    ValT *Items( void )                                const { return (ValT*)ItemINT(0); }
    ValT  operator[]( int num )                        const { /*TraceAssert( ((UINT)num) < ((UINT)Count()) );*/ return *(ValT*)ItemINT(num); }
    int   IndexOf( ValT p )                            const { return LSearch(p,0,NULL); }
    ValT  LastObject( void )                           const { return Item(Count()-1); }

#undef _N_CPP
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
#define _SP_  SortProc
#else
#define _SP_  MyArray<ValT>::SortProc
#endif
    void  Sort( _SP_ sp,  int start = -1, int end = -1 )           { SortINT( (BaseArray::SortProc)sp,start,end ); }
    int   Search( ValT key,
                  _SP_ sp = NULL,
                  int *NearestPosition = NULL )              const { return SearchINT( &key,(BaseArray::SortProc)sp,NearestPosition ); }
    int   LSearch( ValT key,int StartFrom = 0,
                   _SP_ sp = NULL )                          const { return LSearchINT( &key,StartFrom,(BaseArray::SortProc)sp ); }
#undef _SP_

#if defined(HAS_INLINE_ENUM)
    template <class ItteratorT,class Data> ValT Enum( ItteratorT itterator, Data d )
      {  ValT it;
        for ( int n = 0; n < Count(); n++ )
          if ( itterator.Compare(it=Item(n),n,d) )
            return it;
     return NULL;
    }
    template <class ItteratorT,class Data> int EnumNum( ItteratorT itterator, Data d )
      {
        for ( int n = 0; n < Count(); n++ )
          if ( itterator.Compare(Item(n),n,d) )
            return n;
     return -1;
    }
#endif
};

template <class ValT> void MyArray<ValT>::ItemAdded( ValT /*ptr*/ )            {}
template <class ValT> void MyArray<ValT>::ItemDeleted( ValT ptr,BOOL FreePtr ) { if (FreePtr) delete ptr; }

template <class ValT> ValT  MyArray<ValT>::Add( ValT p )
  {  ValT *pp = (ValT*)AddINT( &p );
     if (pp) {
       ItemAdded(*pp);
       return *pp;
     } else
       return NULL;
}
template <class ValT> ValT MyArray<ValT>::AddAt( int pos,ValT p )
  {  ValT *pp = (ValT*)AddAtINT( pos,&p );
     if (pp) {
       ItemAdded(*pp);
       return *pp;
     } else
       return NULL;
}
template <class ValT> void MyArray<ValT>::DeleteNum( int num,BOOL del )
  {  ValT *it = (ValT*)ItemINT(num),
           p;

     if (!it) return;

     p = *it;
     BaseArray::DeleteNumINT( num );
     ItemDeleted( p,NeedToDelete && del );
}
template <class ValT> void MyArray<ValT>::DeleteAll( void )
  {
    for ( int n = 0; n < Count(); n++ )
      ItemDeleted( *(ValT*)ItemNum(n),NeedToDelete );
    BaseArray::DeleteAllINT();
}

#endif
