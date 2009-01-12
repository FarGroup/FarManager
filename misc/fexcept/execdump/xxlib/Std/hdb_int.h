#ifndef __MY_HDB_INTERNAL
#define __MY_HDB_INTERNAL

/********************************************************************
  Base for all HDB databases.
    ITable
    IDatabase

  Implemented in [hdbase.cpp]
 ********************************************************************/
STRUCTBASE( ITable, public HDBTable )
  public:
    DWORD          Flags;
    WORD           cCount;
    PHDBValue      Values;
  public:
    virtual BOOL Assign( PHDBTableInfo p,DWORD Flags );
  public:
    ITable( void );
    virtual ~ITable();

//HDBTable implementation
    virtual WORD          ColCount( void );
    virtual PHDBValue     ColValue( LPVOID RecordData,WORD num );
    virtual PHDBValue     ColValue( LPVOID RecordData,CONSTSTR nm );
    virtual PHDBValue     ColValues( LPVOID RecordData );

//Destructor for descendants
    virtual void          Closeup( void );
};

STRUCTBASE( IDatabase, public HDBDatabase )
  public:
    DWORD          Flags;
    PITable       *Items;
    WORD           Count;
  protected:
    virtual PITable   NewTable( PHDBTableInfo p ) = 0;
  public:
    virtual BOOL      Assign( PHDBTableInfo p,DWORD Flags );
  public:
    IDatabase( void );
    virtual ~IDatabase();

//HDBTable implementation
    virtual WORD      TableCount( void );
    virtual PHDBTable Table( WORD num );
    virtual PHDBTable Table( CONSTSTR nm );
    virtual void      Clear( void );

//Destructor for descendants
    virtual void      Closeup( void );
};

//[hdb_mem.cpp]
extern PIDatabase MYRTLEXP hdbCreate_MemDB( PHDBTableInfo dbInfo,DWORD Flags );

#endif