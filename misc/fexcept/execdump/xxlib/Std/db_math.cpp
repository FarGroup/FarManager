#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

#define HDISTANCE       3
#define ARR_ALL         for ( int n = 0; n < Count(); n++ )
#define BEZIER_elements 20

/* FMod( x,y )         - Calculate reminder of x/y
   BiggestRound( x,y ) - Calculate nearest "X round Y" (result value not less then X)
*/
double MYRTLEXP FMod( double x,double y )
  {  unsigned long l;
     double  p = y/1000;
     double  d;

     if ( x < 0 && y > 0 || x > 0 && y < 0 ) {
       l = (unsigned long)(p - x / y);
       d = x + (double)(l * y);
     } else {
       l = (unsigned long)(p + x / y);
       d = x - (double)(l * y);
     }


     if ( Abs(d) < p )
       return 0.0;

 return d;
}

double MYRTLEXP BiggestRound( double x,double y )
  {  unsigned long l;
     double        p = Abs(y/1000);
     double        d;

     if ( x < 0 && y > 0 || x > 0 && y < 0 ) {
       l = (unsigned long)(p - x / y);
       d = - (double)(l * y);
     } else {
       l = (unsigned long)(p + x / y);
       d = l * y;
     }

     if ( d < 0 ) {
       if ( d-x > p )
         d -= y;
     } else {
       if ( x-d > p )
         d += y;
     }
 return d;
}
#if 0
void main( int argc,char *argv[] )
  {

#define T(v,v1) printf( "%lf %% %lf = FMod: %lf, fmod: %lf, BR: %lf\n", v,v1, _FMod(v,v1), fmod(v,v1), _BiggestRound(v,v1) );

      T( -5.09, .1 )
      T( -5.19, .1 )
      T( 5.09, .1 )
      T( 38344.8976560532, 1.15740740740741E-05 )
      T( 38344.897662,     1.15740740740741E-05 )
      T( 2.1,    0.7 )
      T( 5.1, .1 )
      T( 1.669, 1. )
      T( 5.19, .1 )
      T( 5.1,    7.1 )
      T( 5.001,  1. )
      T( 5.1,    0.00001 )
      getch();
}
#endif

/*  HDB_RectPart
*/
void MYRTLEXP HDB_RectPart( const HRect& before,const HRect& bPart,const HRect& after,HRect& aPart )
  {  double dw = after.Width()/before.Width(),
            dh = after.Height()/before.Height();
    aPart.Set( (bPart.x-before.x)*dw,   (bPart.y-before.y)*dh,
               (bPart.x1-before.x1)*dw, (bPart.y1-before.y1)*dh );
    aPart.Offset( after.x,after.y );
    aPart.Normalize();
}

MyString MYRTLEXP HDB_StrCorrect( const MyString& s )
  {  MyString res;
    for ( int n = 0; n <= s.Length(); n++ )
      res.Add( (s[n]==',')?'.':s[n] );
 return res;
}

BOOL MYRTLEXP HDB_Str2Db( const MyString& str,double& v )
  {
    if ( !str.Length() )
      return FALSE;

    v = atof( HDB_StrCorrect(str).Text() );
    if ( str.Chr('%') != -1 ) v /= 100;

 return TRUE;
}

BOOL MYRTLEXP HDB_Str2Point( const MyString& s,HPoint& v )
  {
  return (StrColCount(s.c_str(),",") == 2 &&
          HDB_Str2Db(StrGetCol(s.c_str(),1,","),v.x) &&
          HDB_Str2Db(StrGetCol(s.c_str(),2,","),v.y));
}

BOOL MYRTLEXP HDB_Str2Rect( const MyString& s,HRect& v )
  {  int cCount = StrColCount(s.c_str(),",");

    if ( cCount == 4 )
      return ( HDB_Str2Db(StrGetCol(s.c_str(),1,","),v.x)  &&
               HDB_Str2Db(StrGetCol(s.c_str(),2,","),v.y)  &&
               HDB_Str2Db(StrGetCol(s.c_str(),3,","),v.x1) &&
               HDB_Str2Db(StrGetCol(s.c_str(),4,","),v.y1) );
     else
    if ( cCount == 2 &&
         StrColCount(s.c_str(),";") == 2 ) {
      HPoint p;

      if ( !HDB_Str2Point(StrGetCol(s.c_str(),1,";"),p) ) return FALSE;
      v.x = p.x; v.y = p.y;

      if ( !HDB_Str2Point(StrGetCol(s.c_str(),2,";"),p) ) return FALSE;
      v.x = p.x; v.y = p.y;

      return TRUE;
    } else
     return FALSE;
}

MyString MYRTLEXP HDB_Rect2Str( const HRect& v )
  {  char str[100];
     Sprintf( str,"%3.2f, %3.2f; %3.2f, %3.2f",v.x,v.y,v.x1,v.y1 );
  return MyString(str);
}

MyString MYRTLEXP HDB_Point2Str( const HPoint& v )
  {  char str[100];
     Sprintf( str,"%3.2f, %3.2f",v.x,v.y );
  return MyString(str);
}

MyString MYRTLEXP HDB_Db2Str( double v )
  {  char str[20];
     Sprintf( str,"%3.4f",v );
  return MyString(str);
}

/*******************************************************************
   HPoint
 *******************************************************************/
void HPoint::Set( double val )            { x = y = val; }
void HPoint::Set( double _x,double _y )   { x = _x; y = _y; }
void HPoint::Set( const HPoint& p )       { Set(p.x,p.y); }

void HPoint::DMove( double dx,double dy ) { DMove(HPoint(dx,dy)); }
void HPoint::DMove( const HPoint& p ) { x += p.x; y += p.y; }

/*
x` = x * cos(angle) + y * (-sin(angle)) + eDx,
y` = x * sin(angle) + y * cos(angle) + eDy,
*/
void HPoint::RotateAt( const HPoint& at,double angle )
  {
    HDB_PrepareRotate( angle );
    HDB_DoRotate( x,y,at.x,at.y );
}

void HPoint::ScaleAt( const HPoint& at,double dw,double dh )
  {
    x = at.x + (x-at.x)*dw;
    y = at.y + (y-at.y)*dh;
}
/*******************************************************************
     HRect
 *******************************************************************/
void HRect::Set( double X, double Y )                       { x = X; y = Y; x1 = X; y1 = Y; }
void HRect::Set( double X, double Y, double X1, double Y1 ) { x = X; y = Y; x1 = X1; y1 = Y1; }
void HRect::Set( double X, double Y, const MySize& sz )     { x = X; y = Y; x1 = X+sz.cx; y1 = Y+sz.cy; }
void HRect::Set( const HPoint& p, const MySize& sz )        { x = p.x; y = p.y; x1 = p.x+sz.cx; y1 = p.y+sz.cy; }
void HRect::Set( const HRect& r )                       { x = r.x; y = r.y; x1 = r.x1; y1 = r.y1; }
void HRect::Set( const HPoint& p, double w, double h )      { x = p.x; y = p.y; x1 = x+w; y1 = y+h; }
void HRect::Set( const HPoint& p, const HPoint& p1 )        { x = p.x; y = p.y; x1 = p1.x; y1 = p1.y; }

HRect HRect::Rotated( const HPoint& at,double angle ) const
  {  HRect  r;
     double tx,ty;
     HDB_PrepareRotate( angle );

     r = *this;
     tx = x; ty = y;   HDB_DoRotate( tx,ty,at.x,at.y ); r.Set(tx,ty);
     tx = x1; ty = y;  HDB_DoRotate( tx,ty,at.x,at.y ); r.Or(tx,ty);
     tx = x1; ty = y1; HDB_DoRotate( tx,ty,at.x,at.y ); r.Or(tx,ty);
     tx = x; ty = y1;  HDB_DoRotate( tx,ty,at.x,at.y ); r.Or(tx,ty);
 return r;
}

void HRect::RotateAt( const HPoint& at,double angle )
  {  HRect r;
     double tx,ty;
     HDB_PrepareRotate( angle );

     r = *this;
     tx = r.x;  ty = r.y;  HDB_DoRotate( tx,ty,at.x,at.y ); Set(tx,ty);
     tx = r.x1; ty = r.y;  HDB_DoRotate( tx,ty,at.x,at.y ); Or(tx,ty);
     tx = r.x1; ty = r.y1; HDB_DoRotate( tx,ty,at.x,at.y ); Or(tx,ty);
     tx = r.x;  ty = r.y1; HDB_DoRotate( tx,ty,at.x,at.y ); Or(tx,ty);
}

HRect HRect::Normalized( void ) const {
  HRect r;
  r.x  = Min( x, x1 );
  r.y  = Min( y, y1 );
  r.x1 = Max( x, x1 );
  r.y1 = Max( y, y1 );
 return r;
}

void HRect::Normalize( void ) {
  HRect r;
  r.x  = Min(x, x1),
  r.y  = Min(y, y1),
  r.x1 = Max(x, x1),
  r.y1 = Max(y, y1);

  *this = r;
}

BOOL HRect::Contains(const HPoint& p ) const {
  return (p.x >= x && p.x <= x1 && p.y >= y && p.y <= y1);
}

BOOL HRect::Contains(const HRect& p) const {
  return p.x >= x && p.x1 <= x1 && p.y >= y && p.y1 <= y1;
}

BOOL HRect::Touches(const HRect& p) const {
  return ( p.x1 < x || p.y1 < y || p.y > y1 || p.x > x1
         )?FALSE:TRUE;
}

HRect HRect::operator &(const HRect& p) const {
  HRect r;
    r.x  = Max((double)x, p.x);
    r.y  = Max((double)y, p.y);
    r.x1 = Min((double)x1, p.x1);
    r.y1 = Min((double)y1, p.y1);
  return r;
}

HRect HRect::operator |(const HRect& p) const {
  HRect r;
    r.x  = Min((double)x, p.x);
    r.y  = Min((double)y, p.y);
    r.x1 = Max((double)x1, p.x1);
    r.y1 = Max((double)y1, p.y1);
  return r;
}

HRect HRect::operator |( const HPoint& p ) const {
  HRect r;
  r.Set( Min((double)x,p.x),Min((double)y,p.y),Max((double)x1,p.x),Max((double)y1,p.y) );
 return r;
}

void HRect::And( const HRect& p )
  {
    Set( Max((double)x,  p.x),  Max((double)y,  p.y),
         Min((double)x1, p.x1), Min((double)y1, p.y1) );
}

void HRect::Or( const HRect& p )
  {
   Set( Min((double)x, p.x),   Min((double)y, p.y),
        Max((double)x1, p.x1), Max((double)y1, p.y1) );
}

void HRect::Or( double px,double py )
  {
   Set( Min((double)x,  px), Min((double)y,  py),
        Max((double)x1, px), Max((double)y1, py) );
}
/*******************************************************************
   HPolyPoint
 *******************************************************************/
HPolyPoint::HPolyPoint( double v )
   : Handle(v)
  {
    x = y = 0;
    HandleUsed = HandleCan = FALSE;
}
HPolyPoint::HPolyPoint( const HPolyPoint& p )
   : HPoint(p),
     Handle(p.Handle)
  {
    HandleUsed = p.HandleUsed;
    HandleCan  = p.HandleCan;
}
HPolyPoint::HPolyPoint( const HPoint& p,const HPoint& h,BOOL used )
   : HPoint(p),
     Handle(h)
  {
    HandleUsed = used;
    HandleCan = TRUE;
}
HPolyPoint::HPolyPoint( const HPoint& p,BOOL can,BOOL used )
   : HPoint(p),
     Handle(p)
  {
    HandleUsed = used;
    HandleCan = can;
}

void  HPolyPoint::DMove( const HPoint& p )   { HPoint::DMove(p); if (HandleUsed) Handle.DMove(p); }
void  HPolyPoint::Set( double val )          { if (HandleUsed) Handle.DMove(val-x,val-y); HPoint::Set(val); }
void  HPolyPoint::Set( double _x,double _y ) { if (HandleUsed) Handle.DMove(_x-x,_y-y); HPoint::Set(_x,_y); }

void HPolyPoint::RotateAt( const HPoint& at,double angle )
  {
    HPoint::RotateAt( at,angle );
    if ( HandleUsed ) Handle.RotateAt( at,angle );
}

void HPolyPoint::ScaleAt( const HPoint& at,double dw,double dh )
  {
    HPoint::ScaleAt( at,dw,dh );
    if ( HandleUsed ) Handle.ScaleAt(at,dw,dh);
}
/*******************************************************************
     HPoly
 *******************************************************************/
HPoly::HPoly( BOOL c )                 { Closed = c; }
HPoly::HPoly( const HPoint& p,BOOL c ) { Add(p); Closed = c; }
HPoly::HPoly( const HRect& p )         { Assign(p); }
HPoly::HPoly( const HPoly& p )         { Assign(p); }

HPoly::~HPoly()
  {
    ScrPoints.DeleteAll();
    DeleteAll();
}

void     HPoly::DMove( double dx, double dy ) { DMove(HPoint(dx,dy)); }
POINT_t *HPoly::ScreenPoints( void )          { return (POINT_t*)ScrPoints.Items(); }
int      HPoly::ScreenPointsCount( void )     { return ScrPoints.Count(); }

BOOL HPoly::Inside( const HPoint& p,BOOL borderOnly ) const
  {  PHPolyPoint pt,pt1;
     int         n;

    if ( Count() == 0 ) return FALSE;
    pt = Items();
    if ( Count() == 1 ) return Abs(pt->x-p.y) <= HDISTANCE && Abs(pt->y-p.y) < HDISTANCE;

    if ( borderOnly || !Closed ) {
      if ( Closed && Count() > 2 ) {
        pt = LastObject();
        n = 0;
      } else
        n = 1;
      for ( ; n < Count(); pt=pt1,n++ ) {
        pt1 = Item(n);
        double x = Min(pt->x,pt1->x), y = Min(pt->y,pt1->y),
               x1= Max(pt->x,pt1->x), y1= Max(pt->y,pt1->y);
        if ( p.x >= x && p.x <= x1 && p.y >= y && p.y <= y1 &&
             Abs(PointToLine(pt,pt1,p.x,p.y)) <= HDISTANCE )
          return TRUE;
      }
      return FALSE;
    } else
      return InPoly( Items(),Count(),p.x,p.y );
}

BOOL HPoly::Inside( const MyPoint& p,BOOL borderOnly ) const
  {  POINT_t *pt,*pt1;
     int      n;

    if ( ScrPoints.Count() == 0 ) return FALSE;
    pt = ScrPoints.Items();
    if ( ScrPoints.Count() == 1 ) return Abs(pt->x-p.y) <= HDISTANCE && Abs(pt->y-p.y) <= HDISTANCE;

    if ( borderOnly || !Closed ) {
      if ( Closed && ScrPoints.Count() > 2 ) {
        pt = ScrPoints.LastObject();
        n = 0;
      } else
        n = 1;
      for ( ; n < ScrPoints.Count(); pt=pt1,n++ ) {
        pt1 = ScrPoints.Item(n);
        int x = Min(pt->x,pt1->x), y = Min(pt->y,pt1->y),
            x1= Max(pt->x,pt1->x), y1= Max(pt->y,pt1->y);
        if ( p.x >= x && p.x <= x1 && p.y >= y && p.y <= y1 &&
             Abs(PointToLine(pt,pt1,p.x,p.y)) <= HDISTANCE )
          return TRUE;
      }
      return FALSE;
    } else
      return InPoly( ScrPoints.Items(),ScrPoints.Count(),p.x,p.y );
}

int HPoly::InsideBorder( const MyPoint& p ) const
  {  POINT_t *pt,*pt1;
     int      n,i;

    if ( ScrPoints.Count() == 0 ) return -1;
    pt = ScrPoints.Items();
    if ( ScrPoints.Count() == 1 ) return (Abs(pt->x-p.y) <= HDISTANCE && Abs(pt->y-p.y) <= HDISTANCE)?1:(-1);

    if ( Closed && ScrPoints.Count() > 2 ) {
      pt = ScrPoints.LastObject();
      n = 0;
    } else
      n = 1;
    for ( ; n < ScrPoints.Count(); pt=pt1,n++ ) {
      pt1 = ScrPoints.Item(n);
      int x = Min(pt->x,pt1->x), y = Min(pt->y,pt1->y),
          x1= Max(pt->x,pt1->x), y1= Max(pt->y,pt1->y);
      if ( p.x >= x && p.x <= x1 && p.y >= y && p.y <= y1 &&
           Abs(PointToLine(pt,pt1,p.x,p.y)) <= HDISTANCE ) {
        if (n)
          for ( i = 0; i < Count(); i++ )
            if ( Item(i)->ScreenPoint >= n ) return i;
        return Count();
      }
    }
 return -1;
}

void HPoly::Assign( const HPoly& p )
  {
    DeleteAll();
    Closed = p.Closed;
    for ( int n = 0; n < 0; n++ ) Add( *p[n] );
}

void HPoly::Assign( const HRect& p )
  {
    DeleteAll();
    Add( p.TopLeft() );     Add( p.TopRight() );
    Add( p.BottomRight() ); Add( p.BottomLeft() );
    Closed = TRUE;
}

HPoint HPoly::Center( void ) const
  {  HPoint center( *Item(0) );
     ARR_ALL center.DMove( *Item(n) );
 return (Count()>0)?(center/Count()):center;
}

void HPoly::UpdateBezierLine( HRect& r,PHPolyPoint p,PHPolyPoint p1 ) const
  {
     double   t,dt = 1. / BEZIER_elements;
     int      i;
     double   x,y,
              bx = p->x,  hbx = p->Handle.x,
              by = p->y,  hby = p->Handle.y,
              ex = p1->x, hex = p1->x*2 - p1->Handle.x,
              ey = p1->y, hey = p1->y*2 - p1->Handle.y;

    for ( t = 0,i = 0; i <= BEZIER_elements; t+=dt,i++ ) {
      x = (1-t)*(1-t)*(1-t)*bx + 3*t*(1-t)*(1-t)*hbx + 3*t*t*(1-t)*hex + t*t*t*ex;
      y = (1-t)*(1-t)*(1-t)*by + 3*t*(1-t)*(1-t)*hby + 3*t*t*(1-t)*hey + t*t*t*ey;
      r.Or( HPoint(x,y) );
    }
}

HRect HPoly::Bounds( void ) const
  {  HRect       r;
     PHPolyPoint p,p1 = NULL;

    if ( Count() == 0 ) return r;

    p = Item(0);
    r.Set( p->x,p->y );

    for ( int n = 1; n < Count(); p=p1,n++ ) {
      p1 = Item(n);
      if ( p->HandleUsed || p1->HandleUsed )
        UpdateBezierLine( r,p,p1 );
       else
        r.Or( *p1 );
    }
    if ( Closed && Count() > 2 ) {
      p = Item(0);
      if ( p->HandleUsed || p1->HandleUsed )
        UpdateBezierLine( r,p1,p );
    }
 return r;
}

void HPoly::AddLine( const MyPoint& p,const MyPoint& p1 )
  {
    if ( !ScrPoints.Count() ) ScrPoints.Add( POINT_t(p.x,p.y) );
    ScrPoints.Add( POINT_t(p1.x,p1.y) );
}

void HPoly::AddBezierLine( PHPolyPoint p,PHPolyPoint p1 )
  {
     double   t,dt = 1. / BEZIER_elements;
     int      oldx,oldy,i,x,y,
              bx = p->Screen.x,  hbx = p->Handle.Screen.x,
              by = p->Screen.y,  hby = p->Handle.Screen.y,
              ex = p1->Screen.x, hex = p1->Screen.x*2 - p1->Handle.Screen.x,
              ey = p1->Screen.y, hey = p1->Screen.y*2 - p1->Handle.Screen.y;

    oldx = bx; oldy = by;
    for ( t = 0,i = 0; i <= BEZIER_elements; t+=dt,i++ ) {
      x = (int)((1-t)*(1-t)*(1-t)*bx + 3*t*(1-t)*(1-t)*hbx + 3*t*t*(1-t)*hex + t*t*t*ex);
      y = (int)((1-t)*(1-t)*(1-t)*by + 3*t*(1-t)*(1-t)*hby + 3*t*t*(1-t)*hey + t*t*t*ey);
      if ( oldx != x || oldy != y ) {
        AddLine( MyPoint(oldx,oldy),MyPoint(x,y) );
        oldx = x; oldy = y;
      }
    }
}

void HPoly::MkScreenPoints( DWORD /*data*/ )
  {  PHPolyPoint p;

    ARR_ALL {
      p = Item(n);
      p->Screen.x = (int)p->x;
      p->Screen.y = (int)p->y;
      p->Handle.Screen.x = (int)p->Handle.x;
      p->Handle.Screen.y = (int)p->Handle.y;
    }
}

void HPoly::TranslatePoints( DWORD data )
  {  int         n;
     PHPolyPoint p,p1 = NULL;

    if ( Count() == 0 ) return;

    MkScreenPoints( data );

//Fill array
    ScrPoints.DeleteAll();
    Item(0)->ScreenPoint = 0;
    for ( n = 1; n < Count(); n++ ) {
      p = Item(n-1); p1 = Item(n);
      if ( p->HandleUsed || p1->HandleUsed )
        AddBezierLine( p,p1 );
       else
        AddLine( p->Screen,p1->Screen );
      p1->ScreenPoint = ScrPoints.Count()-1;
    }
    if ( Closed && Count() > 2 ) {
      p = Item(0);
      if ( p->HandleUsed || p1->HandleUsed )
        AddBezierLine( p1,p );
       else
        AddLine( p1->Screen,p->Screen );
    }
}

void HPoly::RotateAt( const HPoint& at, double angle )
  {
    ARR_ALL Item(n)->RotateAt( at,angle );
}

void HPoly::Scale( double dw,double dh )
  {  HPoint p( Bounds().TopLeft() );
    ARR_ALL Item(n)->ScaleAt( p,dw,dh );
}

void HPoly::DMove( const HPoint& p )
  {
    ARR_ALL Item( n )->DMove( p );
}
