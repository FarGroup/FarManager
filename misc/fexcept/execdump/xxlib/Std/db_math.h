#ifndef __MY_DOUBLE_MATH
#define __MY_DOUBLE_MATH

PRESTRUCT( HRect );
PRESTRUCT( HPoint );

/*******************************************************************
  double-coordinates shapes, polygons
   HPoint                   - point
   HRect                    - rect
     HPolyPoint             - point with bezier handle
   POINT_t                  - POINT with `operator=`
   RECT_t                   - RECT with `operator=`
   HPoly                    - ref-array of HPolyPoint
 *******************************************************************/

/*******************************************************************
     MACROSES
 *******************************************************************/
#define HDB_PrepareRotate( angle )   double __rt_cs = cos( -angle*M_PI/180 ),  \
                                            __rt_sn = sin( -angle*M_PI/180 );  \
                                            double __rt_tmpx,__rt_tmpy
#define HDB_DoRotate( x,y,atx,aty ) __rt_tmpx = x - atx; __rt_tmpy = y - aty;      \
                                    x = __rt_tmpx * __rt_cs - __rt_tmpy * __rt_sn; \
                                    y = __rt_tmpx * __rt_sn + __rt_tmpy * __rt_cs; \
                                    x += atx; y += aty

/*******************************************************************
     PROCEDURES
 *******************************************************************/
//Calculate rectangle `aPart` if prev state was `bPart` in `before` bounds and bounds changed to after
HDECLSPEC void       MYRTLEXP MYRTLEXP HDB_RectPart( const HRect& before,const HRect& bPart,const HRect& after,HRect& aPart );
//Convertions
HDECLSPEC BOOL       MYRTLEXP MYRTLEXP HDB_Str2Rect( const MyString& str,HRect& v );
HDECLSPEC BOOL       MYRTLEXP MYRTLEXP HDB_Str2Point( const MyString& str,HPoint& v );
HDECLSPEC BOOL       MYRTLEXP MYRTLEXP HDB_Str2Db( const MyString& str,double& v );
HDECLSPEC MyString   MYRTLEXP MYRTLEXP HDB_Rect2Str( const HRect& v );
HDECLSPEC MyString   MYRTLEXP MYRTLEXP HDB_Point2Str( const HPoint& v );
HDECLSPEC MyString   MYRTLEXP MYRTLEXP HDB_Db2Str( double v );
//Maths
HDECLSPEC double     MYRTLEXP MYRTLEXP FMod( double x,double y );
HDECLSPEC double     MYRTLEXP MYRTLEXP BiggestRound( double x,double y );

/*******************************************************************
    TEMPLATES  { Point/Line }
 *******************************************************************/
template <class T,class T1> BOOL InPoly( T *p, int npol,T1 x, T1 y)
  {  int  i, j;
     BOOL c = 0;

   for ( i = 0, j = npol-1; i < npol; j = i++ )
     if ((((p[i].y <= y) && (y < p[j].y)) ||
          ((p[j].y <= y) && (y < p[i].y))) &&
           (x < (p[j].x - p[i].x) * (y - p[i].y) / (p[j].y - p[i].y) + p[i].x))
          c = !c;
 return c;
}

template <class T,class T1> inline double PointToLine( T *p, T *p1,T1 x,T1 y )
  {  T1 dx = p1->x-p->x,
        dy = p1->y-p->y;
 return (dx+dy != 0)?((((double)dx)*(y-p1->y)-((double)dy)*(x-p1->x))/(dx+dy)):0;
}

/*******************************************************************
     HPoint
 *******************************************************************/
STRUCT( HPoint )
    double   x,y;
    MyPoint  Screen;
  public:
    HPoint( void )                                     { x = y = 0; }
    HPoint( double val )                               { x = y = val; }
    HPoint( double _x, double _y )                     { x = _x;    y = _y; }
    HPoint( const MySize& sz )                         { x = sz.cx; y = sz.cy; }
    HPoint( const MyPoint& p )                         { x = p.x;   y = p.y; }
    HPoint( const HPoint& p )                          { x = p.x;   y = p.y; }

    virtual void RotateAt( const HPoint& at,double angle );
    virtual void ScaleAt( const HPoint& at,double dw,double dh );
    virtual void Set( double val );
    virtual void Set( double _x,double _y );
    virtual void DMove( const HPoint& p );

    void         Set( const HPoint& p );
    void         DMove( double dx,double dy );

    HPoint       Offset( double dx,double dy )         { DMove(dx,dy); return *this; }
    HPoint       operator =( const HPoint& p )                    { x = p.x; y = p.y; return *this; }
    void         SetNull( void )                       { x = y = 0; }

    BOOL         IsNull( void )                  const { return x == 0 && y == 0; }
    HPoint       Invert( void )                  const { return HPoint(-x,-y); }
    double       Hypo( const HPoint& p )         const { return (*this-p).Hypo(); }
    double       Hypo( void )                    const { return sqrt(x*x+y*y); }
    BOOL         Empty( void )                   const { return x == 0 && y == 0; }
    MyPoint      Translated( void )              const { return MyPoint((int)x,(int)y); }
    HPoint       operator +( const HPoint& p )              const { return HPoint(x+p.x,y+p.y); }
    HPoint       operator -( const HPoint& p )              const { return HPoint(x-p.x,y-p.y); }
    HPoint       operator *( double val )                   const { return HPoint(x*val,y*val); }
    HPoint       operator /( double val )                   const { return HPoint(x/val,y/val); }
    HPoint       operator -()                               const { return Invert(); }
    int          operator ==( const HPoint& p )             const { return x == p.x && y == p.y; }
    int          operator !=( const HPoint& p )             const { return x != p.x || y != p.y; }
    void         operator +=( const HPoint& p )                   { DMove(p); }
    void         operator -=( const HPoint& p )                   { DMove(-p.x,-p.y); }
#if defined(__VCL__)
    operator AnsiString() const { return HDB_Point2Str(*this); }
#endif
    operator MyString()   const { return HDB_Point2Str(*this); }
};

/*******************************************************************
     HRect
 *******************************************************************/
STRUCT( HRect )
   double x,y,x1,y1;
  public:
    HRect( void )                                     { x = y = x1 = y1 = 0; }
    HRect( double val )                               { x = y = x1 = y1 = val; }
    HRect( const HRect& p )                           { x = p.x; y = p.y; x1 = p.x1; y1 = p.y1; }
    HRect( const HPoint& p )                          { x = x1 = p.x; y = y1 = p.y; }
    HRect( const HPoint& p, double w, double h )      { x = p.x; y = p.y; x1 = x+w; y1 = y+h; }
    HRect( const HPoint& p, const MySize& sz )        { x = p.x; y = p.y; x1 = p.x+sz.cx; y1 = p.y+sz.cy; }
    HRect( const HPoint& p, const HPoint& p1 )        { x = p.x; y = p.y; x1 = p1.x; y1 = p1.y; }
    HRect( double X, double Y, double X1, double Y1 ) { x = X; y = Y; x1 = X1; y1 = Y1; }
    HRect( const MyRect& r )                          { x = r.x; y = r.y; x1 = r.x1; y1 = r.y1; }

    void    Set( double X, double Y );
    void    Set( double X, double Y, double X1, double Y1 );
    void    Set( double X, double Y, const MySize& sz );
    void    Set( const HPoint& p, const MySize& sz );
    void    Set( const HRect& r );
    void    Set( const HPoint& p, double w, double h = 0 );
    void    Set( const HPoint& p, const HPoint& p1 );

    BOOL    Contains(const HPoint& p )       const;
    BOOL    Contains(const HRect& other)     const;

    BOOL    Touches(const HRect& other)      const;

    HRect   Normalized( void )               const;
    void    Normalize( void );
    HRect   Rotated( const HPoint& at,double angle ) const;
    void    RotateAt( const HPoint& at,double angle );

    void    And(const HRect& other);
    HRect   operator &( const HRect& other )            const;

    void    Or( const HRect& other );
    void    Or( double px,double py );
    HRect   operator |( const HRect& other )            const;
    HRect   operator |( const HPoint& p )               const;

    BOOL    IsNull( void )                   const { return x == 0 && y == 0 && x1 == 0 && y1 == 0; }
    BOOL    IsEmpty( void )                  const { return x1 == x && y == y1; }

    HPoint  Center( void )                   const { return HPoint((x+x1)/2,(y+y1)/2); }
    double  CenterX( void )                  const { return (x+x1)/2; }
    double  CenterY( void )                  const { return (y+y1)/2; }

    double  Width( void )                    const { return x1-x; }
    double  Height( void )                   const { return y1-y; }
    double  W( void )                        const { return x1-x; }
    double  H( void )                        const { return y1-y; }

    HPoint  TopLeft( void )                  const { return HPoint(x,y);   }
    HPoint  TopRight( void )                 const { return HPoint(x1,y);  }
    HPoint  BottomLeft( void )               const { return HPoint(x,y1);  }
    HPoint  BottomRight( void )              const { return HPoint(x1,y1); }

    HPoint  XY( void )                       const { return HPoint(x,y);   }
    HPoint  X1Y( void )                      const { return HPoint(x1,y);  }
    HPoint  XY1( void )                      const { return HPoint(x,y1);  }
    HPoint  X1Y1( void )                     const { return HPoint(x1,y1); }

    HPoint  Size( void )                     const { return HPoint(W(),H());}
    double  Area( void )                     const { return W()*H(); }

    BOOL    operator ==(const HRect& p )                const { return x==p.x && y==p.y && x1==p.x1 && y1==p.y1; }
    BOOL    operator !=(const HRect& p )                const { return x!=p.x || y!=p.y || x1!=p.x1 || y1!=p.y1; }

    HRect   Inflated( double dx, double dy)  const { return HRect( x-dx,y-dy,x1+dx,y1+dy); }
    HRect   Inflated( double d )             const { return HRect( x-d,y-d,x1+d,y1+d); }
    HRect   Inflated( const HPoint& p )      const { return HRect( x-p.x,y-p.y,x1+p.x,y1+p.y); }

    MyRect  Translated( void )               const { return MyRect((int)x,(int)y,(int)x1,(int)y1); }

    void    Or( const HPoint& p )                           { Or(p.x,p.y); }
    HRect   operator |=(const HPoint& p )                              { Or(p); return *this; }

    HRect   Offset( double dx, double dy )                  { x+=dx; y+=dy; x1+=dx; y1+=dy; return *this; }
    HRect   Offset( const HPoint& p )                       { x+=p.x; y+=p.y; x1+=p.x; y1+=p.y; return *this; }

    HRect   MoveTo( double X,double Y )                     { x1 = X+Width(); y1=Y+Height(); x=X; y=Y; return *this; }
    HRect   MoveTo( const HPoint& p )                       { return MoveTo(p.x,p.y); }

    HRect   Inflate( double dx, double dy)                  { x-=dx; y-=dy; x1+=dx; y1+=dy; return *this; }
    HRect   Inflate( double d )                             { x-=d; y-=d; x1+=d; y1+=d; return *this; }
    HRect   Inflate( const HPoint& p )                      { x-=p.x; y-=p.y; x1+=p.x; y1+=p.y; return *this; }

    void    Scale( double dw, double dh)                    { x1 = x+(x1-x)*dw; y1 = y+(y1-y)*dh; }

    HRect   operator =( const HRect& p )                               { x = p.x; y = p.y; x1 = p.x1; y1 = p.y1; return *this; }

    void    SetNull( void )                                 { x = y = x1 = y1 = 0; }
#if defined(__VCL__)
    operator AnsiString()  const { return HDB_Rect2Str(*this); }
#endif
    operator MyString()    const { return HDB_Rect2Str(*this); }
};

/*******************************************************************
     HPolyPoint
 *******************************************************************/
STRUCTBASE( HPolyPoint, public HPoint )
  HPoint Handle;
  BOOL   HandleUsed;
  BOOL   HandleCan;
  int    ScreenPoint;

  HPolyPoint( double v = 0 );
  HPolyPoint( const HPolyPoint& p );
  HPolyPoint( const HPoint& p,const HPoint& h,BOOL used );
  HPolyPoint( const HPoint& p,BOOL can = FALSE,BOOL used = FALSE );

  virtual void  DMove( const HPoint& p );
  virtual void  Set( double val );
  virtual void  Set( double _x,double _y );
  virtual void  RotateAt( const HPoint& at,double angle );
  virtual void  ScaleAt( const HPoint& at,double dw,double dh );
};
/*******************************************************************
     WIN types
 *******************************************************************/
#if defined(__HWIN__)
LOCALSTRUCTBASE( POINT_t,public tagPOINT )
  POINT_t( int _x = 0,int _y = 0 )    { x = _x; y = _y; }

  BOOL operator==( const POINT_t& p ) { return x == p.x && y == p.y; }
};

LOCALSTRUCTBASE( RECT_t,public tagRECT )
  RECT_t( void )                         { left = top = right = bottom = 0; }
  RECT_t( int x,int y,int x1,int y1 )    { left = x; top = y; right = x1; bottom = y1; }

  BOOL operator==( const RECT_t& p )     { return left == p.left && top == p.top && right == p.right && bottom == p.bottom; }
};
#else
  typedef MyPoint POINT_t;
  typedef MyRect  RECT_t;
#endif


/*******************************************************************
     HPoly
 *******************************************************************/
CLASSBASE( HPoly,public MyRefArray<HPolyPoint> )
    MyRefArray<POINT_t> ScrPoints;
  public:
    BOOL Closed;
  private:
    //Fill ScrPoints
    void            AddLine( const MyPoint& p,const MyPoint& p1 );
    void            AddBezierLine( PHPolyPoint p,PHPolyPoint p1 );
    //Update rectangle
    void            UpdateBezierLine( HRect& r,PHPolyPoint p,PHPolyPoint p1 ) const;
  public:
    HPoly( BOOL c = FALSE );
    HPoly( const HPoint& p,BOOL c = FALSE );
    HPoly( const HRect& p );
    HPoly( const HPoly& p );
    ~HPoly();

    void     Assign( const HRect& p );
    void     Assign( const HPoly& p );
    HPoint   Center( void ) const;
    HRect    Bounds( void ) const;
    BOOL     Inside( const HPoint& p,BOOL borderOnly )  const;
    BOOL     Inside( const MyPoint& p,BOOL borderOnly ) const;
    int      InsideBorder( const MyPoint& p )           const; //Ret number of screen line
    void     RotateAt( const HPoint& at, double angle );
    void     Scale( double dw,double dh );
    void     DMove( const HPoint& p );
    void     DMove( double dx, double dy );
    POINT_t *ScreenPoints( void );
    int      ScreenPointsCount( void );
    void     TranslatePoints( DWORD data );

    virtual void MkScreenPoints( DWORD data );
};

#endif
