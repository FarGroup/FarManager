#ifndef __MY_DATAAUTOPTR
#define __MY_DATAAUTOPTR

/*******************************************************************
    HDataPtr        - holder for dynamicaly allocated data
      HDataArray    - template to work with simple dynamic datas
        HCharArray  - array of chars
    HAutoPtr        - holds onto a pointer obtained via new and deletes that
                      object when it itself is destroyed (such as when leaving block scope).
    HAutoStd        - holds onto a value and callback to destroy this value
 *******************************************************************/

/*******************************************************************
    HDataPtr - holder for dynamicaly allocated data
 *******************************************************************/
CLASS( HDataPtr )
 protected:
  LPVOID Data;
  DWORD  Size,MaxSize;

 public:
  HDataPtr( void );
  HDataPtr( DWORD sz );
  HDataPtr( DWORD sz, LPVOID p,DWORD psz );
  HDataPtr( const HDataPtr& p );
  virtual ~HDataPtr();

  BOOL   Assign( LPVOID p,DWORD sz );
  BOOL   Resize( DWORD sz );
  void   Free( void );

  LPVOID Ptr( void )       const;
  BOOL   Allocated( void ) const;
  DWORD  Sizeof( void )    const;
};

/*******************************************************************
    HDataPtr - holder for dynamicaly allocated data
 *******************************************************************/
template <class T> class HDataArray : public HDataPtr {
  protected:
    virtual void Refresh( void ) {}
  public:
    HDataArray( DWORD StartSize = ALLOC_DELTA,const T *StartData = NULL,int StartDataLen = 0 )
     : HDataPtr( StartSize*sizeof(T), (LPVOID)StartData, StartDataLen*sizeof(T) )
     {}

    DWORD Count( void )           const { return Sizeof() / sizeof(T); }
    T    *Start( void )           const { return (T*)Ptr(); }

    void  Clear( void )                 { Free(); }
    T    *Alloc( T *Data,DWORD sz )     { Assign(Data,sz*sizeof(T)); Refresh(); return Start(); }
    T    *Alloc( DWORD sz )             { return Resize(sz*sizeof(T)) ? Start() : NULL; }

#if defined(HAS_LONGARRAY)
    T    *End( void )             const { return Start() ? (&Start()[ Count() ]) : NULL; }
    T&    operator[]( DWORD num )       { Assert( num < Count() ); return Start()[ num ]; }
#else
    T    *End( void )             const { return Start() ? (&Start()[ (size_t)Count() ]) : NULL; }
    T&    operator[]( DWORD num )       { Assert( num < Count() ); return Start()[ (size_t)num ]; }
#endif
};

CLASSBASE( HCharArray, public HDataArray<char> )
    int Len;
  protected:
    virtual void Refresh( void );
  public:
    HCharArray( int ssz = ALLOC_DELTA,CONSTSTR str = NULL );

    char *Text( void )                     const;
    char *c_str( void )                    const;
    int   Length( void )                   const;
    char *Set( char *s );
    void  SetChar( int num,char v );
    BOOL  Cmp( CONSTSTR s,int count = -1 ) const;
    void  Add( char ch );
};

/*******************************************************************
    HAutoPtr - holds onto a pointer obtained via new and deletes that
               object when it itself is destroyed (such as when leaving block scope).
 *******************************************************************/
template <class T> class HAutoPtr {
    T* Object;
  public:
    HAutoPtr( T* p = NULL )  : Object(p)           {}
    HAutoPtr(HAutoPtr<T>& a) : Object(a.Release()) {}
    ~HAutoPtr ()                                   { delete Object; }

    HAutoPtr<T>& operator=( HAutoPtr<T>& rhs )     { Reset(rhs.Release()); return *this; }
    HAutoPtr<T>& operator=( T *rhs )               { Reset(rhs); return *this; }

    T& operator*  ( void )                   const { return *Object; }
    T* operator-> ( void )                   const { return  Object; }
    T* Ptr        ( void )                   const { return  Object; }

    T*   Release( void )                           { T* tmp = Object; Object = 0; return tmp; }
    void Reset( T* p = NULL )                      { if (Object != p) { delete Object; Object = p; } }
};

#endif
