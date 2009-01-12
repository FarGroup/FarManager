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
  HDB implementation
 ********************************************************************/
ITable::ITable( void )
  {
    memset( this,0,sizeof(*this) );
}

ITable::~ITable()
  {
}

WORD ITable::ColCount( void )
  {
 return cCount;
}

void ITable::Closeup( void )
  {  WORD n;

     if ( IS_FLAG(Flags,HDB_DBF_NONCONST) ||
          IS_FLAG(Flags,HDB_DBF_NMDYNAMIC) ) {
       StrFree( Name );
       for( n = 0; n < cCount; n++ ) {
         StrFree( Values[n].Name );
         StrFree( CInfo[n].Name );
       }
     }

     delete[] CInfo;  CInfo  = NULL;
     delete[] Values; Values = NULL;
}

inline BOOL DupName( CONSTSTR& dest, CONSTSTR src )
  {
     if ( src ) {
       dest = StrDup( src );
       if ( !dest ) return FALSE;
     } else
       dest = NULL;
 return TRUE;
}

BOOL ITable::Assign( PHDBTableInfo p,DWORD fl )
  {  WORD n;

     Flags = fl | (p->Type & HDB_TBT_MASK);

//Number of columns
     cCount = 0;
     while( p->CInfo[cCount].Name )
       cCount++;
     if ( !cCount ) {
       SetError( "Table info for [%s] does not contains columns",p->Name );
       return FALSE;
     }

//Copy info
     memcpy( this, p, sizeof(*p) );
     CInfo  = NULL;
     Values = NULL;
     if ( IS_FLAG(Flags,HDB_DBF_NONCONST) ) {
       if ( !DupName( Name, p->Name ) )
         return FALSE;
     } else
       Name = p->Name;

//Create values
     Values  = new HDBValue[ cCount+1 ];
     if ( !Values )
       return FALSE;
//Create infos
     CInfo = new HDBColumnInfo[ cCount+1 ];
     if ( !CInfo )
       return FALSE;

//Copy values infos and set RecSize
     RecSize = 0;

     for( n = 0; n <= cCount; n++ ) {
       ((HDBColumnInfo&)Values[n]) = p->CInfo[n];

       Values[n].Value.Data = NULL;
       if ( IS_FLAG(Flags,HDB_DBF_NONCONST) ) {
         if ( !DupName( Values[n].Name, p->CInfo[n].Name ) )
           return FALSE;
       }

       CInfo[n] = p->CInfo[n];
       if ( IS_FLAG(Flags,HDB_DBF_NONCONST) ) {
         if ( !DupName( CInfo[n].Name, p->CInfo[n].Name ) )
           return FALSE;
       }

       RecSize += Values[n].Size;
     }

 return TRUE;
}

PHDBValue ITable::ColValue( LPVOID data,WORD num )
  {
    if ( num >= cCount )
      return NULL;

    PHDBValue p = Values+num;

    p->Value.Data = ((LPBYTE)data) + p->Offset;

 return p;
}

PHDBValue ITable::ColValue( LPVOID data,CONSTSTR nm )
  {
    if ( !nm )
      return NULL;

    PHDBValue p;
    WORD      n;

    for( n = 0; n < cCount; n++ )
      if ( StrCmp(Values[n].Name,nm,-1,FALSE) == 0 ) {
        p = &Values[n];
        p->Value.Data = ((LPBYTE)data) + p->Offset;
        return p;
      }

 return NULL;
}

PHDBValue ITable::ColValues( LPVOID data )
  {  PHDBValue p = Values;
     WORD      n;

    if ( data )
      for( n = 0; n < cCount; p++,n++ )
        p->Value.Data = ((LPBYTE)data) + p->Offset;

 return Values;
}

// ------------------------------------------------------------------
IDatabase::IDatabase( void )
  {
     Items   = NULL;
     Count   = 0;
     Version = 0;
     Time.Zero();
}

IDatabase::~IDatabase()
  {
}

WORD      IDatabase::TableCount( void ) { return Count; }
PHDBTable IDatabase::Table( WORD num )  { return (num < Count) ? Items[num] : NULL; }
void      IDatabase::Clear( void )      { for( WORD n = 0; n < Count; n++ ) Items[n]->Clear(); }

void IDatabase::Closeup( void )
  {
    for( WORD n = 0; n < Count; n++ ) {
      Items[n]->Closeup();
      delete Items[n];
    }
    delete[] Items;
    Items = NULL;
}

BOOL IDatabase::Assign( PHDBTableInfo p,DWORD fl )
  {  WORD n;

     Flags = fl;

     //Tables number
     for( n = 0; p[n].Name; n++ );
     Count = n;

     //Create tables
     Items = new PITable[ n ];
     memset( Items, 0, sizeof(PITable)*Count );

     for( n = 0; n < Count; p++,n++ ) {
       Items[n] = NewTable(p);
       if ( !Items[n] )
         return FALSE;

       if ( !Items[n]->Assign(p,Flags) ) {
         delete Items[n];
         Items[n] = NULL;
         return FALSE;
       }
     }

 return TRUE;
}

PHDBTable IDatabase::Table( CONSTSTR nm )
  {
    for( WORD n = 0; n < Count; n++ )
      if ( StrCmp(nm,Items[n]->Name,-1,FALSE) == 0 )
        return Items[n];

 return NULL;
}

/********************************************************************
  HDB info
 ********************************************************************/
CONSTSTR MYRTLEXP HDBType2Str( DWORD Type )
  {
    switch( Type ) {
      case HDB_TP_DATETIME:  //PRTime_t
                           return "DATETIME";
      case     HDB_TP_DATE:  //PRDateOnly_t
                           return "DATE";
      case     HDB_TP_TIME:  //PRTimeOnly_t
                           return "TIME";
      case   HDB_TP_STRING:  //char[]
                           return "STRING";
      case     HDB_TP_WORD:  //WORD
                           return "WORD";
      case    HDB_TP_DWORD:  //DWORD
                           return "DWORD";
      case   HDB_TP_DOUBLE:  //double
                           return "DOUBLE";
      case   HDB_TP_BINARY:  //Any data
                           return "BINARY";
    }
 return "";
}

DWORD MYRTLEXP HDBColWidth( PHDBColumnInfo Info )
  {
    if ( Info )
      switch( Info->Type ) {
        case HDB_TP_DATETIME:  //PRTime_t
                             return 2+2+4 + 2 + 1 + 2+2+2 + 2;
        case     HDB_TP_DATE:  //PRDateOnly_t
                             return 2+2+4 + 2;
        case     HDB_TP_TIME:  //PRTimeOnly_t
                             return 2+2+2 + 2;
        case   HDB_TP_STRING:  //char[]
                             return Info ? Info->Size : 0;
        case     HDB_TP_WORD:  //WORD
                             return 5;
        case    HDB_TP_DWORD:  //DWORD
                             return 11;
        case   HDB_TP_DOUBLE:  //double
                             return 7;
        case   HDB_TP_BINARY:  //Any data
                             return Info ? Info->Size : 0;
      }

 return 0;
}

/********************************************************************
  HDB memory bases
 ********************************************************************/
PIDatabase __ILastDatabase;

PHDBDatabase MYRTLEXP HDBCreate( PHDBTableInfo dbInfo,DWORD Type )
  {  PIDatabase p;

    if ( !dbInfo || !dbInfo->Name )
      return NULL;

    switch( Type&HDB_DBTYPE_MASK ) {
      case HDB_DBTYPE_MEMORY: p = hdbCreate_MemDB( dbInfo,Type );
                              __ILastDatabase = p;
                           break;
                     default: Log(( "Unknown DB type %d",Type ));
                           return NULL;
    }

    if ( p ) {
      if ( !p->Assign(dbInfo,Type) ) {
        delete p;
        return NULL;
      }
      p->Version = (DWORD)time(NULL);
      p->Time.Set( (time_t)p->Version );
    }

 return p;
}

void MYRTLEXP HDBDestroy( PHDBDatabase& db )
  {
    if ( db ) {
      ((PIDatabase)db)->Closeup();
      delete db;
      db = NULL;
    }
}

char* MYRTLEXP HDBValue2Str( PHDBValue p,char *Buff, int bSz )
  {  DWORD n;
     int   len;

     Buff[0] = 0;
     switch( p->Type ) {
       case HDB_TP_DATETIME: SNprintf( Buff, bSz,
                                       "%02d-%02d-%04d %02d:%02d:%02d",
                                       p->Value.vDateTime->mon, p->Value.vDateTime->mday, p->Value.vDateTime->year,
                                       p->Value.vDateTime->hour, p->Value.vDateTime->min, p->Value.vDateTime->sec );
                          break;
       case     HDB_TP_DATE: SNprintf( Buff, bSz,
                                       "%02d-%02d-%04d",
                                       p->Value.vDate->mon, p->Value.vDate->mday, p->Value.vDate->year );
                          break;
       case     HDB_TP_TIME: SNprintf( Buff, bSz,
                                       "%02d:%02d:%02d",
                                       p->Value.vTime->hour, p->Value.vTime->min, p->Value.vTime->sec );
                          break;
       case   HDB_TP_STRING: SNprintf( Buff, bSz, "\"%s\"", p->Value.vString );
                          break;
       case     HDB_TP_WORD: SNprintf( Buff, bSz, "%5d", *p->Value.vWord );
                          break;
       case    HDB_TP_DWORD: SNprintf( Buff, bSz, "%08X", *p->Value.vDword );
                          break;
       case   HDB_TP_DOUBLE: SNprintf( Buff, bSz, "%3.3lf", *p->Value.vDouble );
                          break;
       case   HDB_TP_BINARY: len = SNprintf( Buff, bSz, "%d:", p->Size );
                             for( bSz -= 2,n = 0; len < bSz && n < p->Size; len += 2, n++ )
                               Sprintf( Buff+len, "%02X", ((LPBYTE)p->Value.Data)[n] );
                          break;
     }

 return Buff;
}
/********************************************************************
  Columns helpers
 ********************************************************************/
LOCALSTRUCTBASE( HDBColumnINT, public HDBColumn )
  PHDBColumnInfo Base;
  int            Count;
};

void MYRTLEXP HDBDestroyColumns( PHDBColumn& Cols )
  {
     if ( !Cols )
       return;

     delete[] Cols->Values;
     delete Cols;
     Cols = NULL;
}

PHDBColumn MYRTLEXP HDBCreateColumns( PHDBColumn _Cols,PHDBColumnInfo ci )
  {  PHDBColumnINT Cols = (PHDBColumnINT)_Cols;

     if ( !ci ) {
       FIO_SETERRORN( ERROR_INVALID_PARAMETER );
       return NULL;
     }

     if ( Cols && Cols->Base == ci )
       return Cols;

     HDBDestroyColumns( _Cols );

     WORD n;

     Cols = new HDBColumnINT;
     Cols->Base  = ci;
     Cols->Count = 0;

     for( n = 0; ci[n].Name; n++ )
       Cols->Count++;

     Cols->Values = new HDBValue[ Cols->Count+1 ];

     for( n = 0; n <= Cols->Count; n++ ) {
       memcpy( &Cols->Values[n], &ci[n], sizeof(Cols->Values[0]) );
       Cols->Values[n].Value.Data = NULL;
     }

 return Cols;
}

BOOL MYRTLEXP HDBFillColumns( PHDBColumn _Cols,LPVOID RawRecordData )
  {  PHDBColumnINT Cols = (PHDBColumnINT)_Cols;

     if ( !Cols || !RawRecordData ) {
       FIO_SETERRORN( ERROR_INVALID_PARAMETER );
       return FALSE;
     }

     for( WORD n = 0; n < Cols->Count; n++ )
       Cols->Values[n].Value.Data = ((LPBYTE)RawRecordData) + Cols->Values[n].Offset;

 return TRUE;
}

void MYRTLEXP HDBCharConvert( PHDBDatabase db, BOOL ToOEM )
  {
    if ( !db) return;

#if defined(__HWIN32__)
    for( WORD n = 0; n < db->TableCount(); n++ ) {
      PHDBTable tb = db->Table(n);
      for( DWORD i = 0; i < tb->RecCount(); i++ ) {
        PHDBValue col = tb->ColValues( tb->Record(i) );
        for( WORD j = 0; j < tb->ColCount(); j++,col++ ) {
          if ( col->Type != HDB_TP_STRING ) continue;
          if ( ToOEM )
            CharToOemBuff( col->Value.vString, col->Value.vString, col->Size );
           else
            OemToCharBuff( col->Value.vString, col->Value.vString, col->Size );
        }
      }
    }
#else
    USEARG( ToOEM )
#endif
}
