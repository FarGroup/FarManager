#include <all_lib.h>
#pragma hdrstop
#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

#include <Std/hdbase.h>
#include <Std/hdb_int.h>

#if 1
  #define  PROC( v )
  #define  Log( v )
#else
  #define  PROC( v )  INProc __proc v ;
  #define  Log( v )   INProc::Say v
#endif

/********************************************************************
  MemDB
  HDB_DBTYPE_MEMORY

  Implementation of HDB for in-memory databases.
 ********************************************************************/
LOCALCLASSBASE( IMemArray, public BaseArray )
  public:
    MYRTLEXP IMemArray( void ) : BaseArray( 0,0,0 ) {}

    void   MYRTLEXP DeleteAll( void )              { DeleteAllINT(); }
    LPVOID MYRTLEXP AddNew( void )                 { return AddINT(NULL); }
    void   MYRTLEXP DeleteNum( DWORD num )         { DeleteNumINT( (int)num ); }
    LPVOID MYRTLEXP Item( DWORD num )              { return ItemINT( (int)num ); }
    void   MYRTLEXP Init( UINT sz,UINT cn )        { Initialize( sz,cn,cn ); }
    void   MYRTLEXP Sort( BaseArray::SortProc sp ) { SortINT( sp,-1,-1 ); }
    int    Search( LPVOID key,BaseArray::SortProc sp )  const { return SearchINT(key,sp,NULL); }
    int    LSearch( LPVOID key,BaseArray::SortProc sp ) const { return LSearchINT(key,0,sp); }
};

LOCALSTRUCTBASE( IMemTable, public ITable )
  public:
    PIMemArray     Records;
  public:
    IMemTable( void );

    virtual BOOL          Assign( PHDBTableInfo p,DWORD Flags );

    virtual void          Clear( void )             { Records->DeleteAll(); }
    virtual LPVOID        NewRecord( void )         { return Records->AddNew(); }
    virtual void          DeleteRecord( DWORD num ) { Records->DeleteNum( num ); }
    virtual LPVOID        Record( DWORD num )       { return Records->Item( num ); }
    virtual DWORD         RecCount( void )          { return (DWORD)Records->Count(); }

    virtual void          Sort( WORD cNum );
    virtual DWORD         QSearch( WORD cNum, LPVOID Key );
    virtual DWORD         LSearch( WORD cNum, LPVOID Key );
    virtual void          Closeup( void );
};

IMemTable::IMemTable( void )
  {
    Records = new IMemArray;
}

BOOL IMemTable::Assign( PHDBTableInfo p,DWORD Flags )
  {
    if ( !ITable::Assign(p,Flags) )
      return FALSE;

    if ( !RecIncrement ) {
      FIO_SETERRORN( ERROR_INVALID_PARAMETER );
      return FALSE;
    }

    Records->Init( RecSize,RecIncrement );

 return TRUE;
}

void IMemTable::Closeup( void )
  {
    delete Records;
    ITable::Closeup();
}

static DWORD idxOffset,
             idxSize;

#define TO_TYPE( tp,ptr ) ((tp*)(((LPBYTE)ptr) + idxOffset))
#define D_SORT( nm )      static int RTL_CALLBACK nm( const void *left, const void *right )

D_SORT( idDTSort )  { return TO_TYPE(PRTime,left)->Cmp( *TO_TYPE(PRTime,right) ); }
D_SORT( idDSort  )  { return TO_TYPE(PRDateOnly,left)->Cmp( *TO_TYPE(PRDateOnly,right) ); }
D_SORT( idTSort  )  { return TO_TYPE(PRTimeOnly,left)->Cmp( *TO_TYPE(PRTimeOnly,right) ); }
D_SORT( idSSort  )  { return StrCmp( TO_TYPE(const char,left),TO_TYPE(const char,right) ); }
D_SORT( idDWSort )  { return (int)(*TO_TYPE(DWORD,left)) - (int)(*TO_TYPE(DWORD,right)); }
D_SORT( idWSort  )  { return (int)(*TO_TYPE(WORD,left)) - (int)(*TO_TYPE(WORD,right)); }
D_SORT( idDBSort )  { return (*TO_TYPE(double,left) > *TO_TYPE(double,right)) ? (1) : (-1); }
D_SORT( idBinSort ) { return memcmp( left,right,idxSize ); }

void IMemTable::Sort( WORD cNum )
  {
    if ( cNum >= cCount )
      return;

    PHDBValue p = &Values[ cNum ];
    idxOffset = p->Offset;

    switch( p->Type ) {
      case HDB_TP_DATETIME: Records->Sort( idDTSort ); break;
      case     HDB_TP_DATE: Records->Sort( idDSort ); break;
      case     HDB_TP_TIME: Records->Sort( idTSort ); break;
      case   HDB_TP_STRING: Records->Sort( idSSort ); break;
      case     HDB_TP_WORD: Records->Sort( idWSort ); break;
      case    HDB_TP_DWORD: Records->Sort( idDWSort ); break;
      case   HDB_TP_DOUBLE: Records->Sort( idDBSort ); break;
      case   HDB_TP_BINARY: Records->Sort( idBinSort ); break;
                   default: break;
    }
}

DWORD IMemTable::QSearch( WORD num, LPVOID Key )
  {
    if ( !Key || num >= cCount )
      return MAX_DWORD;

    PHDBValue p = Values+num;
    int       rc;
    idxOffset = p->Offset;
    idxSize   = p->Size;

    switch( p->Type ) {
      case HDB_TP_DATETIME: rc = Records->Search( Key, idDTSort );   break;
      case     HDB_TP_DATE: rc = Records->Search( Key, idDSort );    break;
      case     HDB_TP_TIME: rc = Records->Search( Key, idTSort );    break;
      case   HDB_TP_STRING: rc = Records->Search( Key, idSSort );    break;
      case     HDB_TP_WORD: rc = Records->Search( Key, idWSort );    break;
      case    HDB_TP_DWORD: rc = Records->Search( Key, idDWSort );   break;
      case   HDB_TP_DOUBLE: rc = Records->Search( Key, idDBSort );   break;
      case   HDB_TP_BINARY: rc = Records->Search( Key, idBinSort );  break;
                   default: return MAX_DWORD;
    }

 return rc == -1 ? MAX_DWORD : ((DWORD)rc);
}

DWORD IMemTable::LSearch( WORD num, LPVOID Key )
  {
    if ( !Key || num >= cCount )
      return MAX_DWORD;

    PHDBValue p = Values+num;
    int       rc;
    idxOffset = p->Offset;
    idxSize   = p->Size;

    switch( p->Type ) {
      case HDB_TP_DATETIME: rc = Records->LSearch( Key, idDTSort  );  break;
      case     HDB_TP_DATE: rc = Records->LSearch( Key, idDSort   );  break;
      case     HDB_TP_TIME: rc = Records->LSearch( Key, idTSort   );  break;
      case   HDB_TP_STRING: rc = Records->LSearch( Key, idSSort   );  break;
      case     HDB_TP_WORD: rc = Records->LSearch( Key, idWSort   );  break;
      case    HDB_TP_DWORD: rc = Records->LSearch( Key, idDWSort  );  break;
      case   HDB_TP_DOUBLE: rc = Records->LSearch( Key, idDBSort  );  break;
      case   HDB_TP_BINARY: rc = Records->LSearch( Key, idBinSort );  break;
                   default: return MAX_DWORD;
    }

 return rc == -1 ? MAX_DWORD : ((DWORD)rc);
}

LOCALSTRUCTBASE( IMemDatabase, public IDatabase )
  protected:
    virtual PITable   NewTable( PHDBTableInfo p ) { return new IMemTable; }

  public:
    IMemDatabase( void ) {}
};

/********************************************************************
  Interface
 ********************************************************************/
PIDatabase MYRTLEXP hdbCreate_MemDB( PHDBTableInfo dbInfo,DWORD Flags )
  {
    USEARG( Flags )
    USEARG( dbInfo )
 return new IMemDatabase;
}
