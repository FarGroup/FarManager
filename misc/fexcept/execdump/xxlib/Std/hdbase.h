#ifndef __MY_H_DATABASE
#define __MY_H_DATABASE

/** @defgroup HDatabase  Database manipulation
    @{
*/

/********************************************************************
  STRUCTURES
 ********************************************************************/
//- Columns ---------------------------------------------------
enum _HDB_ColumnTypes {
 HDB_TP_DATETIME        = 0x00000001UL,   //PRTime
 HDB_TP_DATE            = 0x00000002UL,   //PRDateOnly
 HDB_TP_TIME            = 0x00000003UL,   //PRTimeOnly
 HDB_TP_STRING          = 0x00000004UL,   //char[]
 HDB_TP_WORD            = 0x00000005UL,   //WORD
 HDB_TP_DWORD           = 0x00000006UL,   //DWORD
 HDB_TP_DOUBLE          = 0x00000007UL,   //double
 HDB_TP_BINARY          = 0x00000009UL,   //Any data

 HDB_TPT_INDEX          = 0x00010000UL   //Indexed fields
};

#define HDB_TP_MIN             HDB_TP_DATETIME
#define HDB_TP_MAX             HDB_TP_BINARY
#define HDB_TP_MASK            0x0000FFFFUL
#define HDB_TPT_MASK           0xFFFF0000UL

//- Tables ----------------------------------------------------------

/*
 =========== Typese set into HDBTableInfo.Type field
*/
enum _HDB_TableTypes {
 HDB_TBT_REAL           = 0x01000000UL,   // Physically stored to disk
 HDB_TBT_VIRTUAL        = 0x00000000UL   // Ceate but not load and do not write to disk
};

/*
 =========== Types passed to HDB functions
*/
// ----------- Type of table holder
enum _HDB_TableHolderTypes {
 HDB_DBTYPE_MEMORY      = 0x00000001UL   // Memory table
};

// ----------- Other creation|allocation table flags
enum _HDB_TableFlags {
 HDB_DBF_NONCONST       = 0x00000010UL,   // Names fields does not constant and must be stored localy
 HDB_DBF_NMDYNAMIC      = 0x00000020UL,   // Names fields allocated using _Alloc and should be freed on destruction
 HDB_DBF_FRONTBASE      = 0x00000040UL,   // Create front base on read (if available)
 HDB_DBF_BACKBASE       = 0x00000080UL,   // Create backup base on read (if available)
 HDB_DBF_FRONTFORCE     = 0x00000100UL,   // Create front base on read (if not available duplicate structure from backup)
 HDB_DBF_BACKFORCE      = 0x00000200UL   // Create backup base on read (if not available duplicate structure from front)
};

// ----------- Masks
#define HDB_DBTYPE_MASK        0x0000000FUL   // Table allocation type (HDB_DBTYPE_xxx)
#define HDB_TBT_MASK           0xFF000000UL   // Table flags (HDB_TBT_xxx)
#define HDB_DBF_MASK           0x00FFFFF0UL   // Database flags (HDB_DBF_xxx)

//- Table indexes --------------------------------------------------------
#define HDB_IDX_NONE           MAX_WORD
#define HDB_IDX_FIRST          ((WORD)0)

///One record columnt definition
STRUCT( HDBColumnInfo )
   CONSTSTR        Name;                    ///< Name of column in record
   DWORD           Offset;                  ///< Offset of col from start of record data
   DWORD           Size;                    ///< Size in bytes of SINGLE column data
   DWORD           Type;                    ///< Type of col data, set of HDB_TP_xxx
};

///Table definition
STRUCT( HDBTableInfo )
  WORD             Index;
  CONSTSTR         Name;                    ///< Name of table
  DWORD            Type;                    ///< Set of HDB_TBT_xxx describing table type
  DWORD            RecSize;                 ///< Full size of one record
  WORD             RecIncrement;            ///< Number of recods to grow when add records to table
  PHDBColumnInfo   CInfo;                   ///< Name=NULL-terminated array of column descriptions
};

#define HDB_TABLE( tp )                     tp##_Record<tp>::Records

#define START_HDB_TABLE( cnm )              template <class T> struct cnm##_Record : public T {       \
                                               static HDBColumnInfo Records[]; };                     \
                                             template <class T> HDBColumnInfo HDB_TABLE(cnm) [] = {
#define END_HDB_TABLE                       { NULL } };

#define ParaToStr( v )                      #v
#define HDB_COLUMN( nm, tp )                { #nm, OffsetOf(T,nm), SizeOf(T,nm), tp  },
#define HDB_COLITEM( num, nm, tp )          { ParaToStr(nm##_##num), OffsetOf(T,nm) + SizeOf(T,nm[0])*num - SizeOf(T,nm[0]), SizeOf(T,nm[0]), tp  },

#define DECL_HDB_DATABASE( nm )             HDBTableInfo nm[] = {
#define USE_HDB_TABLE( num,nm,tp,inc,snm )  { num, nm,tp,sizeof(snm),inc,HDB_TABLE(snm) },
#define END_HDB_DATABASE                    { 0 } };

/********************************************************************
  Database access interface
 ********************************************************************/
/// Structure to direct access to fields values
STRUCTBASE( HDBValue, public HDBColumnInfo )
  union {
    LPVOID       Data;
    PPRTime      vDateTime;
    PPRDateOnly  vDate;
    PPRTimeOnly  vTime;
    pchar        vString;
    LPWORD       vWord;
    LPDWORD      vDword;
    double      *vDouble;
  } Value;
};

/// HDB table interface
STRUCTBASE( HDBTable, public HDBTableInfo )
//Service
  virtual void          Sort( WORD cNum ) = 0;
  virtual DWORD         QSearch( WORD cNum, LPVOID Key ) = 0;  //Returns MAX_DWORD if record not found
  virtual DWORD         LSearch( WORD cNum, LPVOID Key ) = 0;   //Returns MAX_DWORD if record not found
//Data
  virtual void          Clear( void ) = 0;
  virtual LPVOID        NewRecord( void ) = 0;
  virtual void          DeleteRecord( DWORD num ) = 0;
  virtual LPVOID        Record( DWORD num ) = 0;
  virtual DWORD         RecCount( void ) = 0;
//Columns
  virtual WORD          ColCount( void ) = 0;
  virtual PHDBValue     ColValues( LPVOID RecordData )            = 0;
  virtual PHDBValue     ColValue( LPVOID RecordData,WORD num )    = 0;
  virtual PHDBValue     ColValue( LPVOID RecordData,CONSTSTR nm ) = 0;
};

/// HDB database inteface
STRUCT( HDBDatabase )
  DWORD   Version;
  PRTime  Time;

  virtual WORD      TableCount( void ) = 0;
  virtual PHDBTable Table( WORD num ) = 0;
  virtual PHDBTable Table( CONSTSTR nm ) = 0;
  virtual void      Clear( void ) = 0;
};

#include <Std/hdb_file.h>

/********************************************************************
  HDB
 ********************************************************************/
HDECLSPEC PHDBDatabase MYRTLEXP HDBCreate( PHDBTableInfo dbInfo,DWORD Type /*HDB_DBTYPE_xxx + HDB_DBF_*/ );
HDECLSPEC void         MYRTLEXP HDBDestroy( PHDBDatabase& db );

HDECLSPEC CONSTSTR     MYRTLEXP HDBType2Str( DWORD Type );                       // Gets string name of HDB type.
HDECLSPEC DWORD        MYRTLEXP HDBColWidth( PHDBColumnInfo Info );              // Gets textual width of field.
HDECLSPEC char*        MYRTLEXP HDBValue2Str( PHDBValue p,char *Buff, int bSz ); // Get string representation of value
HDECLSPEC void         MYRTLEXP HDBCharConvert( PHDBDatabase db, BOOL ToOEM );

HDECLSPEC BOOL         MYRTLEXP HDBWrite( PHOStream OutStream,                   // Writes one or two database to stream.
                                          PHDBDatabase db/*=NULL*/,
                                          PHDBDatabase bdb/*=NULL*/ );

HDECLSPEC BOOL         MYRTLEXP HDBReadInfo( PHIStream s,                        // Reads starting file info.
                                             PHDBFileInfo p/*=NULL*/ );

HDECLSPEC BOOL         MYRTLEXP HDBRead( PHIStream s,                            // Reads one or two databases from stream.
                                         PHDBDatabase db/*=NULL*/,
                                         PHDBDatabase bdb/*=NULL*/ );

HDECLSPEC BOOL         MYRTLEXP HDBReadCreate( PHIStream InputS,                 // Reads and create database.
                                               DWORD Flags/*=HDB_DBF_xxx + HDB_DBTYPE_xxx*/,
                                               PHDBDatabase& db,
                                               PHDBDatabase& bdb );

/********************************************************************
  Helper classes
 ********************************************************************/
/// Template helper to easy access HDB tables with MyArray functionality
template <class T> class HDBTableArray {
  public:
    PHDBTable Handle;
  public:
    HDBTableArray<T>( PHDBTable tbl ) : Handle(tbl)  {}
    HDBTableArray<T>( void )          : Handle(NULL) {}

    void          Sort( WORD cNum )         const { if (Handle) Handle->Sort(cNum); }
    void          DeleteAll( void )         const { if (Handle) Handle->Clear(); }
    T*            AddNew( void )            const { return Handle ? ((T*)Handle->NewRecord()) : NULL; }
    void          DeleteNum( DWORD num )    const { if (Handle) Handle->DeleteRecord(num); }
    T*            Item( DWORD num )         const { return Handle ? ((T*)Handle->Record(num)) : NULL; }
    DWORD         Count( void )             const { return Handle ? Handle->RecCount() : 0; }
    PHDBTable     operator->( void )        const { return Handle; }
};

LOCALCLASS( HDBHolder )
  public:
    PHDBDatabase Handle;
  public:
    HDBHolder( void )           : Handle(NULL) {}
    HDBHolder( PHDBDatabase p ) : Handle(p)    {}
    ~HDBHolder( void )          { HDBDestroy(Handle); }

    PHDBDatabase operator->( void ) { return Handle; }
};

/********************************************************************
  Columns helpers
 ********************************************************************/
LOCALSTRUCT( HDBColumn )
  PHDBValue      Values;
};

HDECLSPEC PHDBColumn MYRTLEXP HDBCreateColumns( PHDBColumn Cols/*=NULL*/,PHDBColumnInfo ci );
HDECLSPEC void       MYRTLEXP HDBDestroyColumns( PHDBColumn& Cols );
HDECLSPEC BOOL       MYRTLEXP HDBFillColumns( PHDBColumn Cols,LPVOID RawRecordData );

/**@}*/
#endif
