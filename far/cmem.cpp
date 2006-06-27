/*
cmem.cpp

 CCCC   MM   MM  EEEEEE  MM   MM            v. 2.01
C    C  M M M M  E       M M M M    ��������  �������������
C       M  M  M  EEEE    M  M  M     ������ ��� ����������
C    C  M     M  E       M     M    ��������-���������������
 CCCC   M     M  EEEEEE  M     M           ����������

�������� ������� ���������� ( �.55 )
������ �.����������

�������������:
--------------
      - ���������� � ������� ���������� ���� CMEM.CPP;
      - �������� � ������ ���� CMEM.CPP;
      - �� ��������� ��������� �������� �� 100
        ������������ ��������������� �����������,
        ���� ��� ���������� �������� ��� ��������
        Options->Compiler->Code generation->Defines
        MAX_POINTERS=##;
      - ����� ��������� ������ ����� ���������
        �� ����� ����� ������ ���� CMEM.RPT,
        ������� ������� �� "������" �������������
        ������.
*/

/* Revision: 1.01 27.02.2002 $ */

/*
Modify:
  27.02.2002 SVS
    ! ��... ����� ������� ����� :-)
*/

#include "headers.hpp"
#pragma hdrstop

#ifdef ALLOC
#if defined(__BORLANDC__)

#ifdef __cplusplus
extern "C" {
#endif
typedef unsigned size_t;

void *malloc(size_t size);
void *realloc(void *block, size_t size);
void *calloc(size_t nitems, size_t size);
void free(void *block);

#ifdef __cplusplus
}
#endif

#define MEM_DELTA	4095

static HANDLE FARHeapForNew=NULL;

void *calloc(size_t nitems, size_t size)
{
  return realloc(NULL,size*nitems);
}


void *malloc(size_t size)
{
  return realloc(NULL,size);
}


void *realloc(void *block, size_t size)
{
  void *p=NULL;
  int size2;

  if(!FARHeapForNew) FARHeapForNew=GetProcessHeap();

  if(FARHeapForNew)
  {
    if(!size)
    {
      if(block)
        HeapFree(FARHeapForNew,0,block);
    }
    else if (!block) // � ������ ��� ���� �������, �������
    {
                     // ������. ���-�� ������ ������� ������� ��� ��������� ������
      p=HeapAlloc(FARHeapForNew,HEAP_ZERO_MEMORY, size);
    }
    else
    {
      p=block;
      // Required size, alligned to MEM_DELTA
      size  = (size + MEM_DELTA) & (~MEM_DELTA);
      // Current allocated size
      size2 = HeapSize(FARHeapForNew,0,block);
      if (size  > size2 ||          // ��������� ������, ��� ������� ��������
          size2 - size > MEM_DELTA) // ���� ���������� ���� �������� � MEM_DELTA ��� ������
      {
        p=HeapReAlloc(FARHeapForNew,HEAP_ZERO_MEMORY,block,size);
      }
      // else
      /*
          ������. ������ � ���� ��� �������� � ������� ��� � �� �� ������� ���.
          � ��������� �������, ��� ��������� ���������...
      */
    }
  }
  return p;
}

void free(void *block)
{
 if(block!=NULL)
 {
  if(!FARHeapForNew) FARHeapForNew=GetProcessHeap();
  if(FARHeapForNew) HeapFree(FARHeapForNew,0,block);
 }
}


#endif // defined(__BORLANDC__)
#endif // ALLOC

/* ���� ��... */
#if defined(CMEM_INCLUDE) && defined(SYSLOG)

#ifndef ALLOC
static HANDLE FARHeapForNew=NULL;
#endif

#if     !defined(MAX_POINTERS)
#define MAX_POINTERS 8096       // ������������ ����� ������� NEW
#endif

#define MEM_TO0( __nBL )  memset( __nBL,   0, sizeof __nBL )

extern unsigned _stklen = 43210U;

// ��������� ////////////////////////////////////////////////////////
enum flag      { Off, On, End = -1 };
enum line_char { U='\xDA', D='\xC0', H='\xC4',
                 V='\xB3', C='\xC5', S=' ' };

typedef void ( *pvf)();

//static HANDLE FARHeapForNew=NULL;

void GetNameEXE( char name[] )
{
    GetModuleFileName(NULL,name,512);
    char *ptr=strrchr(name,'.');
    if(ptr)
    {
      *ptr='\0';
      strcat(name,".rpt");
    }
}

// �������� ����� ������ ////////////////////////////////////////////
struct HeapBlock {
    void      *Addr;         // �����
    DWORD      SizeHOST,     // ������ - �����������
               SizeALLOC;    //        - ��������������
    flag       Status;       // ������ ���������
    int        NCall;        // ����� � time_call
    // ���������� ������ --------------------------------------------
    DWORD      MemBefore,    // �� �������������
               MemLater;     // ����� ������������
};

// �����, �������������� � �������������� ������ ������������� ������
class ControlMem {
 public:
  ControlMem( void );                // �����������
  ~ControlMem( void );               // ����������

  int  search( void  * );        // ����� ������
  int  search_free( void );          // ����� ���������� �����
  // ������������� ���������� ��������� ������������� ������ ------
  friend void  *operator new( size_t );
  friend void   operator delete( void  *);
  friend void  *operator new[]( size_t );
  friend void   operator delete[]( void  *);

 private:
  struct HeapBlock HBL[ MAX_POINTERS ]; // ������ ���������� ������
  static flag      StatusClass; // ������ ����������� ������
                               // On-��, Off-���
  static flag      CloseBlock;  // ������ "������ �� ����������
                               // ����" On-��, Off-���
  FILE            *rpt;    // ��������� ������ ����� ������
  static int       Meter;  // ������� ������� new & delete
  line_char        ch;     // ������ �����

  flag    time_call[ MAX_POINTERS*2 ];  // ������� �������
                                       // �������� � �������
  int     find_state( void );           // ���������� �����
                                       // � time_call[]
  void    state_end( void );           // ���������� ������� �����
  // ��������� ������ ������������� ������ ------------------------
  pvf  old_handler;   // ��������� �� ������ �������
                     // ��������� ������ �������������
  // ������� ������������ ������� --------------------------------
  void open_r( void );     // ��������
  void close_r( void );    // ��������
  void out_mem  ( int );   // ������ ������������� �����
  void out_size ( int );   // ������ ���������� �����
  void out_addr ( int );   // ������ �����
  void out_tab  ( int = -1 );   // ������
  void out_space( void );  // ������
  void out_open ( int, int = 0 );   // �������� �����
  void out_close( int, int = 0 );   // �������� �����
  void out_unknown( void * );    // ����������� ����
  void out_heap( void );   // ������� ��������� ����������� ��������������
                           // ������� ������
};

// ������������� static /////////////////////////////////////////////
flag ControlMem::StatusClass = Off;    // ����� ��������� �����������
flag ControlMem::CloseBlock  = Off;    // ��� �������� ������
int  ControlMem::Meter       = 0;      // ������� �������

// ���������� ������ ��������� //////////////////////////////////////
ControlMem MemAlloc;

// ����������� ������ ��������� /////////////////////////////////////
ControlMem::ControlMem( void ) {
    ControlMem::StatusClass = On;
    MEM_TO0( HBL );
    MEM_TO0( time_call );
    time_call[0] = End;   // End - ������� �����
    open_r();
}

// �������� ����� ������ ///////////////////////////////////////////
// ������� ������:
//     � ������ ������������� �������� ����� - stderr
//-------------------------------------------------------------------
void ControlMem::open_r( void ) {
    char n[512] = "";
    HANDLE hFile;
    GetNameEXE( n );

    if( (rpt = fopen( n, "wt" )) == NULL )
         rpt = stderr;
    fprintf( rpt,
  "+-----------------------------------------------------------+\n"
  "|Control Memory Allocation                                  |\n"
  "+-----------------------------------------------------------+\n\n"
    );
}

// �������� ����� ������� ///////////////////////////////////////////
void ControlMem::close_r( void )
{
  if (rpt != stderr)
        fclose( rpt );
}

// ������ ������������� ���������� ����� ////////////////////////////
void ControlMem::out_mem( int n )
{
  switch( HBL[ n ].Status )
  {
    case  On:
      fprintf( rpt, "[%lu]\n", ( DWORD )HBL[ n ].MemBefore );
      break;
    case Off:
      fprintf( rpt, "[%lu]\n", ( DWORD)HBL[ n ].MemLater );
      break;
  }
}

// ������������� � ������� �������������� ������� ����� "n" //////////
void ControlMem::out_size( int n )
{
    fprintf( rpt, "          Size: %lu(%lu)\n",
         ( DWORD )HBL[ n ].SizeHOST,
         ( DWORD )HBL[ n ].SizeALLOC
    );
}

// ����� ����� //////////////////////////////////////////////////////
void ControlMem::out_addr( int n )
{
  fprintf( rpt, "%c%Fp{\n", ch=U, HBL[ n ].Addr );
}

// ������ ������� �� ������������ ����� � ������������ � time_call //
void ControlMem::out_tab( int d )
{
    int i=0;
    if(d != -1) d = HBL[ d ].NCall;
    while( time_call[ i ]!=End )
    {
         if (d==-1 || i<d)
           if( time_call[ i ] ) ch = V;
           else ch = S;
         else if( i==d ) ch = D;
         else if( i>d )
              if( time_call[ i ] ) ch = C;
              else ch = H;
         putc( ch, rpt ); i++;
    }
}

// �������� ������ //////////////////////////////////////////////////
void ControlMem::out_space( void )
{
    putc( ' ', rpt );
}

// �������� ���������� �� ������������� ����� //////////////////////
void ControlMem::out_open( int n,int typs )
{
    if( !ControlMem::CloseBlock )
    {
         out_tab(); out_space();
         if(typs) fprintf(rpt, " new[] ");
         else     fprintf(rpt, " new " );
         out_mem( n );
    }
    out_tab(); out_addr( n );
    HBL[ n ].NCall = MemAlloc.find_state();
    time_call[ HBL[ n ].NCall ] = On;
    out_tab(); out_size( n );
  fflush(rpt);
}

// �������� ����� �������������� �����, ������� �� ���������� ///////
void ControlMem::out_unknown( void  *p )
{
    out_tab(); fprintf( rpt, "<%Fp>{?}\n", p );
  fflush(rpt);
}

// ��������� ���� "n" ///////////////////////////////////////////////
void ControlMem::out_close( int n, int typs)
{
    out_tab( n ); putc( '}', rpt );
    if(typs) fprintf(rpt, " delete[] ");
    else     fprintf(rpt, " delete " );
    out_mem( n );
    time_call[ HBL[ n ].NCall ] = Off;
    state_end();
  fflush(rpt);
}

// ���������� � ����������� �������������� ������� ������ ///////////
void ControlMem::out_heap( void )
{
    struct heapinfo hi;
    int      i = 0;

    fprintf( rpt, "\nHeap:\n" );
    fprintf( rpt, "+---+-------+------+\n");
    fprintf( rpt, "| # | Size  |Status|\n" );
    fprintf( rpt, "+---+-------+------+\n" );
    hi.ptr = NULL;
    while( heapwalk(&hi ) == _HEAPOK )
         fprintf( rpt, "|%3d|%7lu| %4s |\n",
              ++i,
              (DWORD)hi.size,
              hi.in_use ? "used" : "free"
         );
    fprintf( rpt, "+---+-------+------+\n" );

    if(heapcheck() == _HEAPCORRUPT)
         fprintf( rpt, "\nHeap is corrupted.\n" );
    else
         fprintf( rpt, "\nHeap is OK.\n" );
  fflush(rpt);
}

// ���������� ������ ��������� //////////////////////////////////////
ControlMem::~ControlMem( void )
{
    ControlMem::StatusClass = Off;
    out_heap();
    close_r();
  //  set_new_handler( old_handler );
}

// ���������� ����� � time_call[] ����� i ///////////////////////////
int ControlMem::find_state( void )
{
    int k=0, i=0;
    while( time_call[i]!=End )
         if( time_call[i++]==On ) k = i;
    time_call[ k ] = Off; time_call[ k+1 ] = End;
    return k;
}

// ���������� ������� ����� //
void ControlMem::state_end( void )
{
    int i=0, k;
    while( time_call[i++]!=End );
    k=(--i);
    while( i>=0 )
    {
         switch( time_call[ i ] )
         {
              case  On: return;
              case Off: time_call[ k ] = Off;
                        time_call[ i ] = End;
                        k=i;
         }
         i--;
    }
}

// ����� ���������� � ����� /////////////////////////////////////////
// ����������:
//     p-����� �����
// ����������:
//     ����� ����� (i) ��� -1, ���� �� �� ������
//-------------------------------------------------------------------
int ControlMem::search( void  *p )
{
    for( int i=0; i<MAX_POINTERS; i++ )
        if (HBL[ i ].Addr == p)
            return i;
    return -1;
}

// ����� ���������� ��������� ///////////////////////////////////////
int ControlMem::search_free( void )
{
    int i = 0;
    do{
        if (HBL[ i ].Status == Off)
            return i;
    }while(++i < MAX_POINTERS);
    return -1;
}


// �������� �������� new ////////////////////////////////////////////
void  *o_NEW( size_t s )
{
    extern new_handler _new_handler;
    void  *p;
    s = s ? abs( s ) : 1;
    while ( (p = malloc( s )) == NULL && _new_handler != NULL )
        _new_handler();
    return p;
}

// ������������� �������� new ///////////////////////////////////////
// ������� ������:
//   ���� ����� ��������� �����������, �������� ��� ������������ new
//-------------------------------------------------------------------
#define coreleft()	0x7FFF8
void  *operator new( size_t size_block )
{
    int        i;
    void  *ptr;
    DWORD      mb = coreleft();

    ptr = o_NEW( size_block );
    if( ControlMem::StatusClass )
    {
         if( (i=MemAlloc.search_free()) == -1 )
         {
                  ControlMem::StatusClass = Off;
                  MemAlloc.out_heap();
                  MemAlloc.close_r();
              fprintf(stderr, "\nStack of pointers is full.\n");
              abort(); //exit(0);
         }

         if(!FARHeapForNew)
           FARHeapForNew=GetProcessHeap();

         MemAlloc.HBL[ i ].SizeHOST  = ( DWORD )size_block;
         MemAlloc.HBL[ i ].SizeALLOC = HeapSize(FARHeapForNew,0,ptr);
         MemAlloc.HBL[ i ].Status    = On;
         MemAlloc.HBL[ i ].MemBefore = mb;
         MemAlloc.HBL[ i ].Addr      = ptr;
         MemAlloc.out_open( i );
    }
    ControlMem::CloseBlock = Off;
    return ptr;
}

void * operator new[]( size_t size_block )
{
    int        i;
    void  *ptr;
    DWORD      mb = coreleft();

    ptr = o_NEW( size_block );
    if( ControlMem::StatusClass )
    {
         if( (i=MemAlloc.search_free()) == -1 )
         {
                  ControlMem::StatusClass = Off;
                  MemAlloc.out_heap();
                  MemAlloc.close_r();
              fprintf(stderr, "\nStack of pointers is full.\n");
              abort(); //exit(0);
         }

         if(!FARHeapForNew)
           FARHeapForNew=GetProcessHeap();

         MemAlloc.HBL[ i ].SizeHOST  = ( DWORD )size_block;
         MemAlloc.HBL[ i ].SizeALLOC = HeapSize(FARHeapForNew,0,ptr);
         MemAlloc.HBL[ i ].Status    = On;
         MemAlloc.HBL[ i ].MemBefore = mb;
         MemAlloc.HBL[ i ].Addr      = ptr;
         MemAlloc.out_open( i ,1 );
    }
    ControlMem::CloseBlock = Off;
    return ptr;
}

// �������� �������� delete /////////////////////////////////////////
void o_DELETE( void  *ptr )
{
    if( ptr ) free( ptr );
}

// ������������� �������� delete ////////////////////////////////////
// ������� ������:
// ���� ����� ��������� �����������, �������� ��� ������������ delete
//-------------------------------------------------------------------
void operator delete( void  *ptr )
{
    int        i;
    o_DELETE( ptr );
    if( ControlMem::StatusClass )
    {
         if( (i=MemAlloc.search( ptr )) == -1 )
         {
              MemAlloc.out_unknown( ptr ); return;
         }
         else
         {
              MemAlloc.HBL[ i ].MemLater = coreleft();
              MemAlloc.HBL[ i ].Status = Off;
              MemAlloc.out_close( i );
         }
    }
    ControlMem::CloseBlock = On;
}
void operator delete[]( void *ptr )
{
    int        i;
    o_DELETE( ptr );
    if( ControlMem::StatusClass )
    {
         if( (i=MemAlloc.search( ptr )) == -1 )
         {
              MemAlloc.out_unknown( ptr ); return;
         }
         else
         {
              MemAlloc.HBL[ i ].MemLater = coreleft();
              MemAlloc.HBL[ i ].Status = Off;
              MemAlloc.out_close( i ,1 );
         }
    }
    ControlMem::CloseBlock = On;
}


/*
VOID GlobalMemoryStatus(

    LPMEMORYSTATUS lpBuffer 	// pointer to the memory status structure
   );

typedef struct _MEMORYSTATUS { // mst
    DWORD dwLength;        // sizeof(MEMORYSTATUS)
    DWORD dwMemoryLoad;    // percent of memory in use
    DWORD dwTotalPhys;     // bytes of physical memory
    DWORD dwAvailPhys;     // free physical memory bytes
    DWORD dwTotalPageFile; // bytes of paging file
    DWORD dwAvailPageFile; // free bytes of paging file
    DWORD dwTotalVirtual;  // user bytes of address space
    DWORD dwAvailVirtual;  // free user bytes

} MEMORYSTATUS, *LPMEMORYSTATUS;
*/
/*
204800
1413120
*/

#else //� _����_ - ��������, �� ����������� ��� ����...

#ifdef ALLOC
/*
    ! ����������!!! �����, rtfm - ������ forever :-)))
      ������ � ���� �������� �� C++:
         ftp://ftp.ldz.lv/pub/doc/ansi_iso_iec_14882_1998.pdf (������ 2860601)
      � ��� ��� ������ �� ������... ������, ������������� � new/delete ��� ����
      (���� ���� ������, �� ���� �������� �� ����������, �.�. �� ��� ����������
      ��������), � ������� ������, ��� � _����_ ������ �� ���������! ����
      ��������� �����������, �������� � ��������� ��������� �������� 18.4
*/
void *operator new(size_t size)
 {
  extern new_handler _new_handler;
  void *p=NULL;
  size=size?size:1;
  while((p=malloc(size))==NULL)
   {
    if(_new_handler!=NULL)_new_handler();
    else break;
   }
  return p;
 }
void *operator new[](size_t size) {return ::operator new(size);}
void *operator new(size_t size, void *p) {return p;}
void operator delete(void *p) {free(p);}
void operator delete[](void *ptr) {::operator delete(ptr);}
#endif

#endif // CMEM_INCLUDE
