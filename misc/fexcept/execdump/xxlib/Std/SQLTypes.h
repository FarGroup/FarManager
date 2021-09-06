#ifndef __MY_SQLTYPES
#define __MY_SQLTYPES

/*
 ┌───────────────────┬─────────────────┬──────────────────────┬────────────┬─────────────────────┬─────────────────┐
 │TYPE               │DIMENTIONS       │ CAPACITY             │ LENGTH     │  DEFAULT VALUE      │ DEFAULT LENGTH  │
 ├───────────────────┼─────────────────┼──────────────────────┼────────────┼─────────────────────┼─────────────────┤
 │char:              │                 │                      │            │                     │                 │
 ├───────────────────┼─────────────────┼──────────────────────┼────────────┼─────────────────────┼─────────────────┤
 │  CHAR             │[ (maxlen) ]     │maxlen                │maxlen      │ ""                  │ 1               │
 │  CHARACTER        │--               │--                    │--          │ ""                  │ 1               │
 │  CHARACTER VARYING│--               │--                    │--          │ ""                  │ 1               │
 │  VARCHAR          │--               │--                    │--          │ ""                  │ 1               │
 │  LONG VARCHAR     │                 │2Gb                   │2Gb         │ <NULL>              │                 │
 ├───────────────────┼─────────────────┼──────────────────────┼────────────┼─────────────────────┼─────────────────┤
 │int:               │                 │                      │            │                     │                 │
 ├───────────────────┼─────────────────┼──────────────────────┼────────────┼─────────────────────┼─────────────────┤
 │  DECIMAL          │[ (total[,var]) ]│value                 │total+1     │ 0.0                 │ 30.6            │
 │  NUMERIC          │--               │--                    │--          │ 0.0                 │ --              │
 │  DOUBLE           │                 │2.2e-308 - 1.7e+308   │8 byte      │ 0.0                 │ 8               │
 │  FLOAT            │[ (precs) ]      │!precs not support    │4 byte      │ 0.0                 │ 4               │
 │  INT              │                 │DWORD                 │4 byte      │ 0                   │ 4               │
 │  INTEGER          │--               │--                    │--          │ 0                   │ --              │
 │  REAL             │                 │1.17e-38 - 3.40e+38   │6 byte      │ 0                   │ 6               │
 │  SMALLINT         │                 │SHORT                 │2 byte      │ 0                   │ 2               │
 │  TINYINT          │                 │BYTE                  │1 byte      │ 0                   │ 1               │
 ├───────────────────┼─────────────────┼──────────────────────┼────────────┼─────────────────────┼─────────────────┤
 │datetime:          │                 │                      │            │                     │                 │
 ├───────────────────┼─────────────────┼──────────────────────┼────────────┼─────────────────────┼─────────────────┤
 │  DATE             │                 │r: 1600-02-28 23:59:59│4 byte      │ dd-mm-yy            │ 4               │
 │  TIME             │                 │o: 7911-01-01 00:00:00│8 byte      │ hh:mm:ss:hh         │ 8               │
 │  TIMESTAMP        │                 │                      │8 byte      │ dd-mm-yy hh:mm:ss:hh│ 8               │
 ├───────────────────┼─────────────────┼──────────────────────┼────────────┼─────────────────────┼─────────────────┤
 │binary:            │                 │                      │            │                     │                 │
 ├───────────────────┼─────────────────┼──────────────────────┼────────────┼─────────────────────┼─────────────────┤
 │  BINARY           │[ (maxlen) ]     │1..32767              │maxlen      │ ""                  │ 1               │
 │  LONG BINARY      │                 │2Gb                   │2Gb         │ 2Gb                 │                 │
 └───────────────────┴─────────────────┴──────────────────────┴────────────┴─────────────────────┴─────────────────┘
*/

#if !defined(_SQLDEF_H_INCLUDED)
  #define SQL_MAX_NFIELDS             50   //максимальное количество полей в SQL базах
  #define SQL_MAX_FIELDLEN            320  //максимальное поле в SQL базах
  #define SQL_MAX_NAME_LEN            30
  #define DB_MAX_IDENTIFIER_LEN       128
//Options
enum _SQL_Options {
  SQL_DT_NULLS_ALLOWED        = 0x0001,
  SQL_DT_PROCEDURE_OUT        = 0x8000,
  SQL_DT_PROCEDURE_IN         = 0x4000,
  SQL_DT_UPDATABLE            = 0x2000,
  SQL_DT_DESCRIBE_INPUT       = 0x1000,
  SQL_DT_AUTO_INCREMENT       = 0x0800,
  SQL_DT_KEY_COLUMN           = 0x0400,
  SQL_DT_HIDDEN_COLUMN        = 0x0200,
  SQL_DT_IS_NULL              = 0x0002
};

//Types
enum _SQL_FiledTypes {
  SQL_DT_NOTYPE               = 0,
  SQL_DT_DATE                 = 384,
  SQL_DT_TIME                 = 388,
  SQL_DT_TIMESTAMP_STRUCT     = 390,
  SQL_DT_TIMESTAMP            = 392,
  SQL_DT_VARCHAR              = 448,
  SQL_DT_FIXCHAR              = 452,
  SQL_DT_LONGVARCHAR          = 456,
  SQL_DT_STRING               = 460,
  SQL_DT_DOUBLE               = 480,
  SQL_DT_FLOAT                = 482,
  SQL_DT_DECIMAL              = 484,
  SQL_DT_INT                  = 496,
  SQL_DT_SMALLINT             = 500,
  SQL_DT_BINARY               = 524,
  SQL_DT_LONGBINARY           = 528,
  SQL_DT_VARIABLE             = 600,
  SQL_DT_TINYINT              = 604,
  SQL_DT_BIGINT               = 608,
  SQL_DT_UNSINT               = 612,
  SQL_DT_UNSSMALLINT          = 616,
  SQL_DT_UNSBIGINT            = 620,
  SQL_DT_BIT                  = 624
};
#endif

PRESTRUCT( HConfigItem );

enum DBSQLRes {
  DB_RES_OK       = 1,
  DB_RES_ERROR    = 0,
  DB_RES_NOTFOUND = -1
};

enum _SQL_SelectOptions {
  DSQL_NULLSTEXT    = 0x0001,    // Place NULL_TEXT to (null) fields
  DSQL_NULLSEMPTY   = 0x0000,    // Emptyes (null) fields ("" for DSQL_STRRESULT, NULL for binary)
  DSQL_STRRESULT    = 0x0002     // Result in string form
};

//---------------------------------------------------------------------------
#define NULL_TEXT            "<NULL>"

inline BOOL SQL_IS_FLOAT( WORD tp)    { return (tp)==SQL_DT_DECIMAL; }
inline BOOL SQL_IS_QUOTABLE( WORD tp) { return (tp)==SQL_DT_DATE     || (tp)==SQL_DT_TIME ||
                                               (tp)==SQL_DT_TIMESTAMP || (tp)==SQL_DT_VARCHAR ||
                                               (tp)==SQL_DT_FIXCHAR   || (tp)==SQL_DT_LONGVARCHAR ||
                                               (tp)==SQL_DT_STRING; }

//---------------------------------------------------------------------------
typedef char   *DBData;
typedef DBData *PDBData;

STRUCT( DBColumn )
    DWORD Length,
          Decimal;
    WORD  Type,
          Options;
    char  Name[ SQL_MAX_NAME_LEN ];

    CONSTSTR    TypeName( void )          const;
    CONSTSTR    OptionName( void )        const;
    DWORD       FixLen( void )            const;
    DWORD       DecLen( void )            const;
    CONSTSTR    Type2DBType( void )       const;
    CONSTSTR    Type2Size( char *buff )   const;
    CONSTSTR    Type2Def( char *buff )    const;

    static CONSTSTR Type2Name( WORD v );
    static CONSTSTR Option2Name( WORD v );
    static CONSTSTR Type2DBType( PDBColumn col );
    static CONSTSTR Type2Size( PDBColumn col,char *buff );
    static CONSTSTR Type2Def( PDBColumn col,char *buff );
};

//---------------------------------------------------------------------------
#define DBSEL_MAGIC MK_ID('D','B','s','l')
DEF_SAFEOBJECT_CR( DBSelect,DBSEL_MAGIC )
     { RowCount=ColCount=0; Header = NULL; Data = NULL; }
   int       ColCount;
   int       RowCount;
   PDBColumn Header;
   PDBData   Data;

   virtual DBSQLRes FetchNext( void )                            = 0;
   virtual DBSQLRes Update( const PDBData p,int from,int count ) = 0;
   virtual DBSQLRes Insert( const PDBData p,int from,int count ) = 0;
   virtual int      InsertMulti( const PDBData p,int from,
                                 int count,int row_count )       = 0;
   virtual DBSQLRes Delete( void )                               = 0;
   virtual DBSQLRes Top( void )                                  = 0;
   virtual DBSQLRes Bottom( void )                               = 0;
};
/*
 1) On open, database !!NOT read record values
    Use Fetch,... procedures to position and read data
 2) Data in DBSelect valid ONLY if DBSQLRes == DB_RES_OK
 3) Functions:
      FetchNext                     - move to next and read it data
      Update( const PDBData p,
              int from,int count )  - update all (or part) of columns data
                                      Cursor must be set on valid record
      Move( int pos )               - move to absolute position
      Top( void )                   - move to top (0) record
      Bottom( void )                - move to last record
*/

//---------------------------------------------------------------------------
#if defined(__QNX__)
/* Can be executed without connection.
*/
HDECLSPEC BOOL        MYRTLEXP SQLMakeInitString( PHConfigItem p, char *istr, size_t isz );
HDECLSPEC void        MYRTLEXP SQLSaveInitString( PHConfigItem p, CONSTSTR istr );

/* Init and close connection.
*/
HDECLSPEC BOOL        MYRTLEXP SQLInit( CONSTSTR parmstr );
HDECLSPEC void        MYRTLEXP SQLFinish( void );
HDECLSPEC void        MYRTLEXP SQLAbort( void );

/* Can be executed only with initialized connection.
*/
HDECLSPEC CONSTSTR    MYRTLEXP SQLError( char *buff/*=NULL*/,int sz );
HDECLSPEC PDBSelect   MYRTLEXP SQLSelect( CONSTSTR exec,int RowCount,DWORD Flags );
HDECLSPEC void        MYRTLEXP SQLCloseSelect( PDBSelect _p );
HDECLSPEC BOOL        MYRTLEXP SQLExec( CONSTSTR exec );
#endif

#endif
