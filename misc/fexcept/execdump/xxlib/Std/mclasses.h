#ifndef __MY_CLASSES
#define __MY_CLASSES

/************************************
            CLASSES
 ************************************
  Strings
     (see mcString.h)
  Helpers
     - HRefObject         - referense conter objects
     - HDefValue          - store template data and it default value
     - HSafeObject        - object stored it allocated status
  Arrays
     - MyArray, MyValArray, MyRefArray (see bsArray.h)
     - CTStringArray      - [ PMyString ]
     - SortedStrings<T>   - sorted [T] where `T` is type conversable to CONSTSTR (pathString, StatString, ConstString)
     - SortedPaths        - sorted [ PathString ]
  Geometric
     - MyPoint            - Point extention
     - MySize             - Size extention
     - MyRect             - Rect extention
 ************************************/

#define ALLOC_DELTA 20

#include <Std/mcString.h>
#include <Std/bsArray.h>
#include <Std/cs.h>

/************************************
   HRefObject
 ************************************/
CLASS( HRefObject )
    CRITICAL_SECTION cs;
    int              rCount;
  protected:
    virtual ~HRefObject();
  public:
    HRefObject( void );

    virtual void AddRef( void );
    virtual void ReleaseRef( void );

    int RefCount( void );
};
/************************************
            HSafeObject
 ************************************/
CLASS( HSafeObject )
 public:
   DWORD SMID;
   BOOL  Allocated;
   DWORD Usage;
 public:
   void operator delete( void *ptr );
 protected:
   virtual ~HSafeObject();
   virtual void Destroy( void );
 public:
   HSafeObject( DWORD id );

           BOOL        Use( void );
           BOOL        Release( void );
   static  BOOL        Use( PHSafeObject p );
   static  BOOL        Release( PHSafeObject p );
   static PHSafeObject Convert( LPVOID ptr,size_t sz,DWORD id );
};

#define DEF_SAFEOBJECT( nm,magic )     STRUCTBASE( nm, public HSafeObject )                                                          \
                                         public:                                                                                     \
                                           static P##nm Cvt( LPVOID h ) { return (P##nm)HSafeObject::Convert(h,sizeof(nm),magic); }  \
                                           nm( void ) : HSafeObject(magic) {}
#define DEF_SAFEOBJECT_CR( nm,magic )  STRUCTBASE( nm, public HSafeObject )                                                          \
                                         public:                                                                                     \
                                           static P##nm Cvt( LPVOID h ) { return (P##nm)HSafeObject::Convert(h,sizeof(nm),magic); }  \
                                           nm( void ) : HSafeObject(magic)

/*******************************************************************
   HDefValue<typeT> - holver for data and it default value
 *******************************************************************/
template <class T> class HDefValue {
  public:
    T Value;
    T DefValue;
  public:
    HDefValue( T val, T def ) : Value(val), DefValue(def) {}
    HDefValue( T val )        : Value(val), DefValue(val) {}

    HDefValue&  operator =(const HDefValue& v) { Value=v.Value; DefValue=v.DefValue; return *this; }
    HDefValue&  operator =(const T& v)         { Value=v; return *this; }
    operator    T()                            const { return Value; }
    operator    T*()                           const { return (T*)(&Value); }
    BOOL        operator ==(const T& v)        const { return Value==v; }
    BOOL        operator !=(const T& v)        const { return Value!=v; }
    size_t      Sizeof( void )      const { return sizeof(T); }

    T*     Ptr()                    { return (T*)&Value; }
    BOOL   isDefault( void )        { return Value == DefValue; }
    void   setDefault( void )       { Value = DefValue; }
};

/************************************
            CTStringArray
 ************************************/
CLASSBASE( CTStringArray,public MyArray<PMyString> )
  public:
    CTStringArray( int begCount = ALLOC_DELTA,BOOL DelObj = TRUE,int ad = ALLOC_DELTA );

    PMyString Add( CONSTSTR str );
    PMyString Add( PMyString s );
    PMyString Add( const MyString& s );
};

/************************************
            SortedStringArray
 ************************************/
#if !defined( __GNUC__ )
CLASSBASE( SortedStringArray,public CTStringArray )
   BOOL sorted;
  public:
    SortedStringArray( BOOL Sorted = TRUE,int begCount = ALLOC_DELTA,BOOL DelObj = TRUE,int ad = ALLOC_DELTA );

    virtual PMyString Add( PMyString p );
    virtual PMyString AddAt( int pos,PMyString p );
    virtual void      DeleteNum( int num,BOOL delItem = TRUE );

    PMyString Add( CONSTSTR p );
    int       IndexOf( const MyString& val,BOOL caseSens = TRUE );
    int       IndexOf( const char *val,BOOL caseSens = TRUE );
    int       IncIndexOf( const MyString& val,BOOL caseSens = TRUE );  // ret TRUE if val begin with string from list
    int       IncIndexOf( const char *val,BOOL caseSens = TRUE );
};
#endif

/************************************
  SortedStrings
 ************************************/
template <class T,BOOL CaseSens> class SortedStrings : public MyRefArray<T> {
  public:
    typedef int (RTL_CALLBACK *SortProc)( const T *left,const T *right );

    static int RTL_CALLBACK idSort( const T *left,const T *right );

  public:
    SortedStrings( int delta = ALLOC_DELTA );

    int Insert( CONSTSTR nm );
    int Insert( const T& nm );
    int Locate( CONSTSTR nm );
    int Locate( const T& nm );

    SortProc Sorter;
};

#define SA_T template <class T,BOOL CaseSens>
#define SA   SortedStrings<T,CaseSens>
/***/SA_T SA::SortedStrings( int delta ) : MyRefArray<T>(delta,delta) { Sorter = idSort; }
/***/SA_T int RTL_CALLBACK SA::idSort( const T *left,const T *right )
/***/  {
/***/ return StrCmp( (CONSTSTR)(*left), (CONSTSTR)(*right), -1, CaseSens );
/***/}
/***/
/***/SA_T int SA::Locate( CONSTSTR nm )
/***/  {  T key;
/***/     key = nm;
/***/ return Search( key, Sorter, NULL );
/***/}
/***/
/***/SA_T int SA::Locate( const T& nm )
/***/  {
/***/ return Search( nm, Sorter, NULL );
/***/}
/***/
/***/SA_T int SA::Insert( CONSTSTR nm )
/***/  {  T key;
/***/     key = nm;
/***/
/***/     int pos,
/***/         num = Search( key, Sorter, &pos );
/***/
/***/     if ( num != -1 )
/***/       return num;
/***/
/***/     AddAt( pos, key );
/***/ return pos;
/***/}
/***/
/***/SA_T int SA::Insert( const T& nm )
/***/  {  int pos,
/***/         num = Search( nm, Sorter, &pos );
/***/
/***/     if ( num != -1 )
/***/       return num;
/***/
/***/     AddAt( pos, nm );
/***/ return pos;
/***/}
#undef SA_T
#undef SA

typedef SortedStrings<PathString,FALSE> SortedPaths;

/************************************
            MySize
 ************************************/
LOCALCLASS( MySize )
  public:
    int cx,cy;
  public:
  // Constructors
    MySize()                                {}
    MySize(int dx, int dy)                  { cx = dx; cy = dy; }
    MySize(DWORD dw)                        { cx = LO_WORD(dw); cy = HI_WORD(dw); }
  // Information functions/operators
    BOOL        operator ==(const MySize& other) const { return other.cx==cx && other.cy==cy; }
    BOOL        operator !=(const MySize& other) const { return other.cx!=cx || other.cy!=cy; }
  // Functions/binary-operators that return sizes
    MySize      operator +(const MySize& size)   const { return MySize(cx+size.cx, cy+size.cy); }
    MySize      operator -(const MySize& size)   const { return MySize(cx-size.cx, cy-size.cy); }
    MySize      operator -()                     const { return MySize(-cx, -cy); }
  // Functions/assignement-operators that modify this size
    MySize&     operator +=(const MySize& size)        { cx += size.cx; cy += size.cy; return *this; }
    MySize&     operator -=(const MySize& size)        { cx -= size.cx; cy -= size.cy; return *this; }
  //Strig operators
    operator MyString()                          const { static char str[100]; Sprintf( str,"%3d, %d",cx,cy ); return str; }
#if defined(__VCL__)
    operator AnsiString()                        const { char str[100]; Sprintf( str,"%3d, %d",cx,cy ); return AnsiString(str); }
#endif
};

/************************************
            MyPoint
 ************************************/
#if defined(__HWIN__)
  LOCALCLASSBASE( MyPoint,public tagPOINT )
  public:
    MyPoint( const POINT& p ) { x = p.x; y = p.y; }
#else
  LOCALCLASS( MyPoint )
  public:
    int x,y;
#endif
  public:
  // Constructors
    MyPoint()                                           {}
    MyPoint(int _x, int _y)                             {x = _x; y = _y;}
    MyPoint(DWORD dw)                                   {x = LO_WORD(dw); y = HI_WORD(dw); }

  //Setters
    void Set(int _x, int _y)                            {x = _x; y = _y;}
    void Set(int v)                                     {x = y = v; }
    void Set(DWORD dw)                                  {x = LO_WORD(dw); y = HI_WORD(dw); }
    void Set(const MyPoint& p)                          {x = p.x; y = p.y; }
#if defined(__HWIN__)
    void Set( const POINT& p)                           {x = p.x; y = p.y; }
#endif

#if defined(__HWIN__) && defined(__VCL__)
    operator TPoint()                             const { TPoint p; p.x = x; p.y = y; return p; }
#endif
#if defined(__HWIN__)
    operator    POINT*()                          const { return (POINT*)this; }
#endif
  //Offset
    void Offset(const MyPoint& p)                       {x += p.x; y += p.y; }
    void Offset(int dx,int dy)                          {x += dx; y += dy; }
    void Offset(DWORD dw)                               {x += LO_WORD(dw); y += HI_WORD(dw); }

  // Information functions/operators
    BOOL        operator ==(const MyPoint& other) const { return other.x==x && other.y==y; }
    BOOL        operator !=(const MyPoint& other) const { return other.x!=x || other.y!=y; }

  // Functions/binary-operators that return points or sizes
    MyPoint      OffsetBy(int dx, int dy)         const { return MyPoint(x+dx, y+dy); }
    MyPoint      operator +(const MySize& size)   const { return MyPoint(x+size.cx, y+size.cy); }
    MySize       operator -(const MyPoint& point) const { return MySize(x-point.x, y-point.y); }
    MyPoint      operator -(const MySize& size)   const { return MyPoint(x-size.cx, y-size.cy); }
    MyPoint      operator -()                     const { return MyPoint(-x, -y); }

  // Functions/assignement-operators that modify this point
    MyPoint&     operator +=(const MySize& size)        { x += size.cx; y += size.cy; return *this; }
    MyPoint&     operator -=(const MySize& size)        { x -= size.cx; y -= size.cy; return *this; }

    BOOL         operator >(const MyPoint& p)     const { return y > p.y || y == p.y && x > p.x; }
    BOOL         operator <(const MyPoint& p)     const { return y < p.y || y == p.y && x < p.x; }

  // Testing functions
    BOOL         IsNull()                         const { return x == 0 && y == 0; }

  //Strig operators
    operator MyString()   const { char str[30]; SNprintf( str,sizeof(str),"%d,%d",x,y ); return MyString(str); }
#if defined(__VCL__)
    operator AnsiString() const { char str[30]; SNprintf( str,sizeof(str),"%d,%d",x,y ); return AnsiString(str); }
#endif
};

/************************************
            MyRect
 ************************************/
#if !defined(__TEC32__)
LOCALSTRUCT( MyRect )
#if defined(HAS_ANONSTRUCT)
    union {
      #if defined(__HWIN__)
        tagRECT  wRect;
        tagPOINT wPoint;
      #endif
      struct { int left,top,right,bottom; };
      struct { int Left,Top,Right,Bottom; };
      struct { int x,y,x1,y1; };
    };
#else
    int x,y,x1,y1;
#endif
  public:
    // Constructors
    MyRect( void )                                         { x = y = x1 = y1 = 0; }
    MyRect( const MyPoint& p, int w,int h )                { Set(p,w,h); }
    MyRect( int _x, int _y, int _x1, int _y1)              { Set(_x, _y, _x1, _y1); }
    MyRect( const MyPoint& p, const MyPoint& p1 )          { Set(p,p1); }
    MyRect( const MyPoint& p, const MySize& sz)            { Set(p,sz); }
    MyRect( int _x, int _y, const MySize& sz)              { Set(_x,_y,sz); }
#if defined(__HWIN__)
  #if defined(HAS_ANONSTRUCT)
    MyRect( const RECT& r )                                { wRect = r; }
  #else
    MyRect( const RECT& r )                                { x = r.left; y = r.top; x1 = r.right; y1 = r.bottom; }
  #endif
#endif
#if defined(__VCL__)
    MyRect( const TRect& r )                               { x = r.Left; y = r.Top; x1 = r.Right; y1 = r.Bottom; }
#endif

    // Type Conversion operators
    operator    const MyPoint*()                     const { return (const MyPoint*)this; }
    operator    MyPoint*()                                 { return (MyPoint*)this; }
#if defined(__HWIN__) && defined(HAS_ANONSTRUCT)
    operator    RECT()                               const { return wRect; }
    operator    RECT*()                              const { return (RECT*)&wRect; }
    operator    POINT*()                             const { return (POINT*)&wPoint; }
#endif
#if defined(__HWIN__) && !defined(HAS_ANONSTRUCT)
    operator    RECT()                               const { return *((RECT*)this); }
    operator    RECT*()                              const { return (RECT*)this; }
    operator    POINT*()                             const { return (POINT*)this; }
#endif
#if defined(__VCL__)
    operator    TRect()                              const { return *((TRect*)this); }
    operator    TRect*()                             const { return (TRect*)this; }
#endif

    //Assign operators
#if defined(__HWIN__) && defined(HAS_ANONSTRUCT)
    MyRect      operator=( const tagRECT& r )              { wRect = r; return *this; }
#endif
#if defined(__HWIN__) && !defined(HAS_ANONSTRUCT)
    MyRect      operator=( const tagRECT& r )              { *((tagRECT*)this) = r; return *this; }
#endif
#if defined(__VCL__)
    MyRect      operator=( const TRect& r )                { x = r.Left; y = r.Top; x1 = r.Right; y1 = r.Bottom; return *this; }
#endif
    MyRect      operator=( const MyRect& r )               { x = r.x; y = r.y; x1 = r.x1; y1 = r.y1; return *this; }

    // Testing functions
    BOOL        IsEmpty()                            const { return x==x1 || y==y1; }
    BOOL        IsNull()                             const { return x == 0 && x1 == 0 && y == 0 && y1 == 0; }

    BOOL        operator ==(const MyRect& p )        const { return x==p.x && y==p.y && x1==p.x1 && y1==p.y1; }
    BOOL        operator !=(const MyRect& p )        const { return x!=p.x || y!=p.y || x1!=p.x1 || y1!=p.y1; }

    // (re)Initializers
    void        SetNull()                                  { x = y = x1 = y1 = 0; }
    void        SetEmpty()                                 { x1 = x; y1 = y; }

    void        Set( const MyPoint& p, int w,int h )       { x = p.x; y = p.y; x1 = x+w; y1 = y+h; }
    void        Set( int X, int Y, int X1, int Y1 )        { x = X; y = Y; x1 = X1; y1 = Y1; }
    void        Set( int v )                               { x = y = x1 = y1 = v; }
    void        Set( const MyPoint& p, const MyPoint& p1 ) { Set(p.x,p.y,p1.x,p1.y); }
    void        Set( const MyRect& r )                     { x=r.x; y=r.y; x1=r.x1; y1=r.y1; }
    void        Set( const MyPoint& p, const MySize& sz)   { Set(p.x,p.y,p.x+sz.cx,p.y+sz.cy); }
    void        Set( int X,int Y, const MySize& sz)        { Set(X,Y,X+sz.cx,Y+sz.cy); }

    // Information/access functions(const and non-const)
    MyPoint        TopLeft()                         const { return MyPoint(x, y ); }
    MyPoint        TopRight()                        const { return MyPoint(x1,y );}
    MyPoint        BottomLeft()                      const { return MyPoint(x, y1);}
    MyPoint        BottomRight()                     const { return MyPoint(x1,y1);}
    int            Width()                           const { return x1-x;}
    int            Height()                          const { return y1-y;}
    int            CenterX()                         const { return (x+x1)/2;}
    int            CenterY()                         const { return (y+y1)/2;}
    MyPoint        Center()                          const { return MyPoint((x+x1)/2,(y+y1)/2);}
    MySize         Size()                            const { return MySize(Width(), Height());}
    long           Area()                            const { return long(Width())*long(Height());}

    BOOL           Contains(const MyPoint& point)    const { return point.x >= x && point.x < x1 && point.y >= y && point.y < y1; }
    BOOL           Contains( int X,int Y )           const { return X >= x && X < x1 && Y >= y && Y < y1; }
    BOOL           Contains(const MyRect& other)     const { return other.x >= x && other.x1 <= x1 && other.y >= y && other.y1 <= y1; }

    BOOL           Touches(const MyRect& other)      const { return other.x1 > x && other.x < x1 && other.y1 > y && other.y < y1; }

    MyRect         Visible( void )                   const { return MyRect( Max(0,(int)x),Max(0,(int)y),Max(0,(int)x1),Max(0,(int)y1) ); }

    MyRect         OffsetBy(int dx, int dy)          const { return MyRect(x+dx, y+dy, x1+dx, y1+dy); }
    MyRect         OffsetBy(int d)                   const { return MyRect(x+d, y+d, x1+d, y1+d); }
    MyRect         OffsetBy(const MySize& sz )       const { return MyRect(x+sz.cx, y+sz.cy, x1+sz.cx, y1+sz.cy); }
    MyRect         OffsetBy(const MyPoint& p )       const { return MyRect(x+p.x, y+p.y, x1+p.x, y1+p.y); }

    MyRect&        Offset(int dx, int dy)                  { x+=dx; y+=dy; x1+=dx; y1+=dy;     return *this; }
    MyRect&        Offset(int d)                           { x+=d; y+=d; x1+=d; y1+=d;         return *this; }
    MyRect&        Offset(const MyPoint& p)                { x+=p.x; y+=p.y; x1+=p.x; y1+=p.y; return *this; }

    MyRect         Inflated( int dx, int dy)         const { return MyRect(x-dx, y-dy, x1+dx, y1+dy); }
    MyRect         Inflated( int d )                 const { return MyRect(x-d,y-d,x1+d,y1+d); }
    MyRect         Inflated( const MySize& size )    const { return MyRect(x-size.cx, y-size.cy, x1+size.cx, y1+size.cy); }

    MyRect&        Inflate(int dx, int dy)                 { x-=dx; y-=dy; x1+=dx; y1+=dy; return *this; }
    MyRect&        Inflate(int d)                          { x-=d;  y-=d;  x1+=d;  y1+=d;  return *this; }
    MyRect&        Inflate(const MySize& delta)            { return Inflate(delta.cx, delta.cy); }

    MyRect         Normalized()                      const { return MyRect( Min((int)x,(int)x1), Min((int)y,(int)y1), Max((int)x, (int)x1), Max((int)y, (int)y1) ); }
    MyRect&        Normalize( void )                       { Set(Normalized()); return *this; }

    MyRect         Anded(const MyRect& other)        const { return MyRect( Max((int)x,(int)other.x), Max((int)y,(int)other.y), Min((int)x1,(int)other.x1), Min((int)y1,(int)other.y1) ); }
    MyRect&        And(const MyRect& other)                { Set(Anded(other)); return *this; }
    MyRect         Ored(const MyRect& other)         const { return MyRect( Min((int)x,(int)other.x), Min((int)y,(int)other.y), Max((int)x1,(int)other.x1), Max((int)y1,(int)other.y1) ); }
    MyRect&        Or(const MyRect& other)                 { Set(Ored(other)); return *this; }

    //Strig operators
    operator MyString()   const { MyString rc; rc.printf( "{ %d,%d:%d,%d }", x, y, x1, y1 ); return rc; }
#if defined(__VCL__)
    operator AnsiString() const { AnsiString rc; rc.printf( "{ %d,%d:%d,%d }", x, y, x1, y1 ); return rc;  }
#endif
};
#endif //__TEC32__

/************************************
         TYPES & FUNCTIONS
 ************************************/
typedef MyString        MSG_TYPE;
#define Msg2Str( v )    (v).Text()
#define MAX_CATCHS      10

/************************************
          TYRY / CATCHeS
 ************************************/
#if defined(__REALDOS__) || defined(USEJUMPEXCEPTIONS) //For DOS Release
  #define DEFTRYVAR( var )      MSG_TYPE var;
  #define TRY( nn )             assert( curblock < MAX_CATCHS );        \
                                curblock++;                             \
                                if ( setjmp( jbuff[curblock] ) != 0 )   \
                                goto _JCatch##nn; {
  #define CATCH( name,nn )      curblock--; goto _JQuit##nn; }          \
                                _JCatch##nn: { name = jpara;
  #define CATCH_END( nn )       } _JQuit##nn:
  #define THROW( val )          { jpara = MSG_TYPE(val);                \
                                  assert(curblock>=0);                  \
                                  longjmp( jbuff[curblock--],1 ); }
  #define Ex2Str( v )           Msg2Str( v )

  extern jmp_buf   jbuff[];  //except
  extern int       curblock;
  extern MSG_TYPE  jpara;
#else  //DOS 16
  #define DEFTRYVAR( var )
  #define TRY( nn )               try{
  #define CATCH( name,nn )        }catch( const MSG_TYPE& name ) {
  #define CATCH_END( nn )         }
  #define THROW( val )            throw( MSG_TYPE(val) );
  #define Ex2Str( v )             Msg2Str(v)
#endif

HDECLSPEC CONSTSTR  MYRTLEXP_PT Message( CONSTSTR patt,... );
HDECLSPEC CONSTSTR  MYRTLEXP    MessageV( CONSTSTR patt,va_list a );
#if defined(__WINUNICODE__)
HDECLSPEC WCONSTSTR MYRTLEXP_PT Message( WCONSTSTR patt,... );
HDECLSPEC WCONSTSTR MYRTLEXP    MessageV( WCONSTSTR patt,va_list a );
#endif
/************************************
   SortProcedures
 ************************************/
HDECLSPEC int RTL_CALLBACK MyStringSortProc( PMyString const *l, PMyString const *r );
HDECLSPEC int RTL_CALLBACK MyStringSearchProc( PMyString const *l, PMyString const *r );
HDECLSPEC int RTL_CALLBACK MyStringIncSearchProc( PMyString const *l, PMyString const *r );
HDECLSPEC int RTL_CALLBACK MyStringSearchCharProc( PMyString const *l, PMyString const *r );
HDECLSPEC int RTL_CALLBACK MyStringIncSearchCharProc( PMyString const *l, PMyString const *r );

#endif
