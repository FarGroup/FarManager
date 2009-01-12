#include <all_lib.h>
#pragma hdrstop
#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

#include <Std/hdbase.h>

#if 1
  #define  PROC( v )
  #define  Log( v )
#else
  #define  PROC( v )  INProc __proc v ;
  #define  Log( v )   INProc::Say v
#endif

LOCALSTRUCTBASE( RDTable, public dbhTable )
  PdbhColumn Columns;
  LPWORD     Index;

  RDTable( void ) { Id = 0; Columns = NULL; Index = NULL; }
  ~RDTable()      { delete[] Columns; delete[] Index; }

  BOOL      ReadInfo( PHIStream s );
  PHDBTable FindBase( PHDBDatabase db );
  BOOL      IsTheSame( PHDBTable t );
  BOOL      NewRecord( PHDBTable t,LPVOID ptr );
  BOOL      NewRecordCvt( PHDBTable t,LPVOID ptr );
  WORD      FindColumn( CONSTSTR nm,PHDBValue val,WORD cn );
};

LOCALSTRUCT( RDBase )
  PRDTable     *Table;
  WORD          Count;
  PHDBTableInfo Infos;

  RDBase( WORD cn );
  ~RDBase();

  BOOL          ReadInfo( PHIStream s );
  BOOL          Read( PHIStream s,PHDBDatabase db );
  BOOL          Skip( PHIStream s );
  PHDBTableInfo CreateInfos( void );
};

//--------------------------------------------------------------------
static LPVOID     RecBuff     = NULL;
static DWORD      RecBuffSize = 0;
static AbortProc  aProc       = NULL;

static void RTL_CALLBACK idDelRecBuff( void )
  {
    _Del( RecBuff );
    RecBuff = NULL;
    if ( aProc )
      aProc();
}
//--------------------------------------------------------------------
BOOL RDTable::ReadInfo( PHIStream s )
  {  dbhTable th;

     if ( s->Read(&th,sizeof(th)) != sizeof(th) ) {
       FIO_SETERROR( "Error reading table header" );
       Log(( FIO_ERROR ));
       return FALSE;
     }

     if ( th.Id != HDBF_TABLE_ID ) {
       FIO_SETERROR( "Table has wrong ID %08X != %08X", th.Id, HDBF_TABLE_ID );
       Log(( FIO_ERROR ));
       return FALSE;
     }

     if ( !th.ColCount || !th.RecSize ) {
       FIO_SETERROR( "Table contains wrong data info: Cols: %d RecSz: %d",th.ColCount,th.RecSize );
       Log(( FIO_ERROR ));
       return FALSE;
     }

     StrCpy( Name, th.Name, sizeof(Name) );
     Id           = th.Id;
     ColCount     = th.ColCount;
     RecCount     = th.RecCount;
     RecIncrement = th.RecIncrement;
     RecSize      = th.RecSize;

     Columns = new dbhColumn[ ColCount ];
     for( WORD n = 0; n < ColCount; n++ ) {
       if ( s->Read( &Columns[n],sizeof(Columns[n]) ) != sizeof(Columns[n]) ) {
         FIO_SETERROR( "!read column info[%d]",n );
         Log(( FIO_ERROR ));
         return FALSE;
       }
       if ( !Columns[n].Size ) {
         FIO_SETERROR( "Column [%d] has wrong size %d",n,Columns[n].Size );
         Log(( FIO_ERROR ));
         return FALSE;
       }
       if ( Columns[n].Type < HDB_TP_MIN || Columns[n].Type > HDB_TP_MAX ) {
         FIO_SETERROR( "Column [%d] has wrong type %d",n,Columns[n].Type );
         Log(( FIO_ERROR ));
         return FALSE;
       }
     }

     Index = new WORD[ ColCount ];

 return TRUE;
}

PHDBTable RDTable::FindBase( PHDBDatabase db )
  {
    for( WORD n = 0; n < db->TableCount(); n++ ) {
      PHDBTable  p = db->Table(n);
      if ( StrCmp(p->Name,Name,-1,FALSE) == 0 )
        return p;
    }
 return NULL;
}

BOOL RDTable::IsTheSame( PHDBTable t )
  {
    if ( ColCount != t->ColCount() )
      return FALSE;

    PHDBValue  val  = t->ColValues(NULL);
    PdbhColumn lval = Columns;

    for( WORD n = 0; n < ColCount; val++,lval++,n++ )
      if ( lval->Type != val->Type ||
           lval->Size != val->Size ||
           StrCmp( lval->Name,val->Name,-1,FALSE ) != 0 )
       return FALSE;

 return TRUE;
}

BOOL RDTable::NewRecord( PHDBTable t,LPVOID ptr )
  {  LPVOID p = t->NewRecord();
     if ( !p ) {
       Log(( "!NewRecord" ));
       return FALSE;
     }
     memcpy( p,ptr,RecSize );
 return TRUE;
}

WORD RDTable::FindColumn( CONSTSTR nm,PHDBValue val,WORD cn )
  {
    for( WORD n = 0; n < cn; n++ )
      if ( StrCmp(val[n].Name,nm,-1,FALSE) == 0 )
        return n;
 return MAX_WORD;
}

BOOL RDTable::NewRecordCvt( PHDBTable t,LPVOID ptr )
  {  LPVOID     p = t->NewRecord();
     WORD       n;
     PHDBValue  val,v;
     HDBValue   cv;
     BOOL       rc;

     if ( !p ) {
       Log(( "!NewRecCvt !add" ));
       return FALSE;
     }

     //Fill data vals
     val = t->ColValues(p);

     //Get indexes for existing cols
     for( n = 0; n < ColCount; n++ )
       Index[n] = FindColumn( Columns[n].Name,val,t->ColCount() );

     //Copy readed cols
     for( n = 0; n < ColCount; n++ ) {
       //Not exist
       if ( Index[n] == MAX_WORD ) {
         Log(( "Column [%s] dropped",Columns[n].Name ));
         continue;
       }
       //Setup
       v             = val + Index[n];
       cv.Value.Data = ((LPBYTE)ptr) + Columns[n].Offset;
       rc            = TRUE;
       //Convert
       switch( Columns[n].Type ) {
         case HDB_TP_DATETIME: switch( v->Type ) {
                                 case HDB_TP_DATETIME: *v->Value.vDateTime = *cv.Value.vDateTime;
                                                    break;
                                 case     HDB_TP_DATE: cv.Value.vDateTime->GetDO( v->Value.vDate );
                                                    break;
                                 case     HDB_TP_TIME: cv.Value.vDateTime->GetTO( v->Value.vTime );
                                                    break;
                                 case   HDB_TP_STRING: cv.Value.vDateTime->GetStr( v->Value.vString,v->Size );
                                                    break;
                                 case     HDB_TP_WORD: rc = FALSE;
                                                    break;
                                 case    HDB_TP_DWORD: rc = FALSE;
                                                    break;
                                 case   HDB_TP_DOUBLE: rc = FALSE;
                                                    break;
                                 case   HDB_TP_BINARY: rc = FALSE;
                                                    break;
                               }
                            break;
         case     HDB_TP_DATE: switch( v->Type ) {
                                 case HDB_TP_DATETIME: v->Value.vDateTime->year = cv.Value.vDate->year;
                                                       v->Value.vDateTime->mon  = cv.Value.vDate->mon;
                                                       v->Value.vDateTime->mday = cv.Value.vDate->mday;
                                                    break;
                                 case     HDB_TP_DATE: *v->Value.vDate = *cv.Value.vDate;
                                                    break;
                                 case     HDB_TP_TIME: rc = FALSE;
                                                    break;
                                 case   HDB_TP_STRING: SNprintf( v->Value.vString, v->Size, "%04d-%02d-%02d",
                                                                 cv.Value.vDate->year, cv.Value.vDate->mon, cv.Value.vDate->mday );
                                                    break;
                                 case     HDB_TP_WORD: rc = FALSE;
                                                    break;
                                 case    HDB_TP_DWORD: rc = FALSE;
                                                    break;
                                 case   HDB_TP_DOUBLE: rc = FALSE;
                                                    break;
                                 case   HDB_TP_BINARY: rc = FALSE;
                                                    break;
                               }
                            break;
         case     HDB_TP_TIME: switch( v->Type ) {
                                 case HDB_TP_DATETIME: v->Value.vDateTime->hour = cv.Value.vTime->hour;
                                                       v->Value.vDateTime->min  = cv.Value.vTime->min;
                                                       v->Value.vDateTime->sec  = cv.Value.vTime->sec;
                                                    break;
                                 case     HDB_TP_DATE: rc = FALSE;
                                                    break;
                                 case     HDB_TP_TIME: *v->Value.vTime = *cv.Value.vTime;
                                                    break;
                                 case   HDB_TP_STRING: SNprintf( v->Value.vString, v->Size, "%02d:%02d:%02d",
                                                                 cv.Value.vTime->hour, cv.Value.vTime->min, cv.Value.vTime->sec );
                                                    break;
                                 case     HDB_TP_WORD: rc = FALSE;
                                                    break;
                                 case    HDB_TP_DWORD: rc = FALSE;
                                                    break;
                                 case   HDB_TP_DOUBLE: rc = FALSE;
                                                    break;
                                 case   HDB_TP_BINARY: rc = FALSE;
                                                    break;
                               }
                            break;
         case   HDB_TP_STRING: switch( v->Type ) {
                                 case HDB_TP_DATETIME: v->Value.vDateTime->Set( cv.Value.vString );
                                                    break;
                                 case     HDB_TP_DATE: v->Value.vDate->Set( cv.Value.vString );
                                                    break;
                                 case     HDB_TP_TIME: v->Value.vTime->Set( cv.Value.vString );
                                                    break;
                                 case   HDB_TP_STRING: StrCpy( v->Value.vString, cv.Value.vString, v->Size );
                                                    break;
                                 case     HDB_TP_WORD: *v->Value.vWord  = (WORD)Str2DigitDetect( cv.Value.vString, 10, (WORD)0 );
                                                    break;
                                 case    HDB_TP_DWORD: *v->Value.vDword = (DWORD)Str2DigitDetect( cv.Value.vString, 10, (DWORD)0 );
                                                    break;
                                 case   HDB_TP_DOUBLE: *v->Value.vDouble = atof( cv.Value.vString );
                                                    break;
                                 case   HDB_TP_BINARY: memcpy( v->Value.Data,cv.Value.vString,Min( Columns[n].Size,v->Size ) );
                                                    break;
                               }
                            break;
         case     HDB_TP_WORD: switch( v->Type ) {
                                 case HDB_TP_DATETIME: rc = FALSE;
                                                    break;
                                 case     HDB_TP_DATE: rc = FALSE;
                                                    break;
                                 case     HDB_TP_TIME: rc = FALSE;
                                                    break;
                                 case   HDB_TP_STRING: Digit2Str( *cv.Value.vWord, v->Value.vString, 10, v->Size );
                                                    break;
                                 case     HDB_TP_WORD: *v->Value.vWord = *cv.Value.vWord;
                                                    break;
                                 case    HDB_TP_DWORD: *v->Value.vDword = (DWORD)*cv.Value.vWord;
                                                    break;
                                 case   HDB_TP_DOUBLE: *v->Value.vDouble = (double)*cv.Value.vWord;
                                                    break;
                                 case   HDB_TP_BINARY: rc = NULL;
                                                    break;
                               }
                            break;
         case    HDB_TP_DWORD: switch( v->Type ) {
                                 case HDB_TP_DATETIME: rc = FALSE;
                                                    break;
                                 case     HDB_TP_DATE: rc = FALSE;
                                                    break;
                                 case     HDB_TP_TIME: rc = FALSE;
                                                    break;
                                 case   HDB_TP_STRING: Digit2Str( *cv.Value.vDword, v->Value.vString, 10, v->Size );
                                                    break;
                                 case     HDB_TP_WORD: *v->Value.vWord = (WORD)*cv.Value.vDword;
                                                    break;
                                 case    HDB_TP_DWORD: *v->Value.vDword = *cv.Value.vDword;
                                                    break;
                                 case   HDB_TP_DOUBLE: *v->Value.vDouble = (double)*cv.Value.vDword;
                                                    break;
                                 case   HDB_TP_BINARY: rc = NULL;
                                                    break;
                               }
                            break;
         case   HDB_TP_DOUBLE: switch( v->Type ) {
                                 case HDB_TP_DATETIME: rc = FALSE;
                                                    break;
                                 case     HDB_TP_DATE: rc = FALSE;
                                                    break;
                                 case     HDB_TP_TIME: rc = FALSE;
                                                    break;
                                 case   HDB_TP_STRING: SNprintf( v->Value.vString, v->Size, "%3.3lf", *cv.Value.vDouble );
                                                    break;
                                 case     HDB_TP_WORD: *v->Value.vWord = (WORD)*cv.Value.vDouble;
                                                    break;
                                 case    HDB_TP_DWORD: *v->Value.vDword = (DWORD)*cv.Value.vDouble;
                                                    break;
                                 case   HDB_TP_DOUBLE: *v->Value.vDouble = *cv.Value.vDouble;
                                                    break;
                                 case   HDB_TP_BINARY: rc = NULL;
                                                    break;
                               }
                            break;
         case   HDB_TP_BINARY: switch( v->Type ) {
                                 case HDB_TP_DATETIME: rc = FALSE;
                                                    break;
                                 case     HDB_TP_DATE: rc = FALSE;
                                                    break;
                                 case     HDB_TP_TIME: rc = FALSE;
                                                    break;
                                 case   HDB_TP_STRING: { DWORD i,cn;
                                                         char  *src,*dest;

                                                         cn   = Min(Columns[n].Size,v->Size) - 1;
                                                         src  = (char*)cv.Value.Data;
                                                         dest = v->Value.vString;

                                                         for( i = 0; i < cn && isprint(*src); i++ )
                                                           *dest++ = *src++;
                                                         *dest = 0;
                                                       }
                                                    break;
                                 case     HDB_TP_WORD: rc = FALSE;
                                                    break;
                                 case    HDB_TP_DWORD: rc = FALSE;
                                                    break;
                                 case   HDB_TP_DOUBLE: rc = FALSE;
                                                    break;
                                 case   HDB_TP_BINARY: memcpy( v->Value.Data,cv.Value.Data,Min( Columns[n].Size,v->Size ) );
                                                    break;
                               }
                            break;
                      default: rc = FALSE;
       }
       //RC
       if ( !rc ) {
         Log(( "Can not convert %s to %s on %s column",
               HDBType2Str(Columns[n].Type), HDBType2Str(v->Type), v->Name ));
       }
     }

 return TRUE;
}

//--------------------------------------------------------------------
RDBase::RDBase( WORD cn )
  {
    Infos = NULL;
    Table = new PRDTable[cn];
    Count = cn;
    for( WORD n = 0; n < Count; n++ )
      Table[n] = new RDTable;
}

RDBase::~RDBase()
  {  WORD n;

    if ( Infos ) {
      for( n = 0; n < Count; n++ )
        delete[] Infos[n].CInfo;
      delete[] Infos;
    }

    for( n = 0; n < Count; n++ )
      delete Table[n];
    delete[] Table;
}

PHDBTableInfo RDBase::CreateInfos( void )
  {  WORD n,i;

    if ( Infos )
      return Infos;

    Infos = new HDBTableInfo[ Count+1 ];

    for( n = 0; n < Count; n++ ) {
      PRDTable      t = Table[n];
      PHDBTableInfo inf = &Infos[n];

      inf->Name          = StrDup( t->Name );
      inf->Type          = t->Type | HDB_TBT_REAL;  //Force real table (only real tables can be saved and loaded)
      inf->RecSize       = t->RecSize;
      inf->RecIncrement  = t->RecIncrement;
      inf->CInfo         = new HDBColumnInfo[ t->ColCount+1 ];

      PHDBColumnInfo ci = inf->CInfo;
      PdbhColumn     ri = t->Columns;
      for( i = 0; i < t->ColCount; ci++,ri++,i++ ) {
        ci->Name   = StrDup( ri->Name );
        ci->Offset = ri->Offset;
        ci->Size   = ri->Size;
        ci->Type   = ri->Type;
      }
      ci->Name = NULL; //NULL terminate columns
    }
    Infos[n].Name = NULL; //NULL terminate tables

 return Infos;
}

BOOL RDBase::ReadInfo( PHIStream s )
  {
    for( WORD n = 0; n < Count; n++ )
      if ( !Table[n]->ReadInfo(s) ) {
         Log(( "!DBReadInfo !table[%d]",n ));
        return FALSE;
      }

 return TRUE;
}

BOOL RDBase::Skip( PHIStream s )
  {
    for( WORD n = 0; n < Count; n++ ) {
      PRDTable p = Table[n];

      dbhTable ht;
      if ( s->Read(&ht,sizeof(ht)) != sizeof(ht) ) {
        Log(( "!Skip read header[%d]",n ));
        return FALSE;
      }
      if ( ht.Id != HDBF_TABLE_ID ||
           StrCmp(ht.Name,p->Name,-1,FALSE) != 0) {
        FIO_SETERRORN( ERROR_INVALID_PARAMETER );
        Log(( "!Skip !ID[%d]",n ));
        return FALSE;
      }

      for( DWORD i = 0; i < p->RecCount; i++ )
        if ( s->Read(RecBuff,p->RecSize) != p->RecSize ) {
          Log(( "!Skip !read data[%d,%d]",n,i ));
          return FALSE;
        }
    }

 return TRUE;
}

BOOL RDBase::Read( PHIStream s,PHDBDatabase db )
  {

    for( WORD n = 0; n < Count; n++ ) {
      PRDTable p = Table[n];

      //Table
      dbhTable ht;
      if ( s->Read(&ht,sizeof(ht)) != sizeof(ht) ) {
        Log(( "!Read !header[%d]",n ));
        return FALSE;
      }
      if ( ht.Id != HDBF_TABLE_ID ||
           StrCmp(ht.Name,p->Name,-1,FALSE) != 0 ) {
        FIO_SETERRORN( ERROR_HEADER );
        Log(( "!Read !ID" ));
        return FALSE;
      }

      //Find Dbase
      PHDBTable pt = p->FindBase( db );
      if ( !pt ) {
        Log(( "!DB [%s] can not be imported",p->Name ));
        continue;
      }

      //Clear
      pt->Clear();

      //Records
      BOOL isSame = p->IsTheSame( pt );

      for( DWORD i = 0; i < p->RecCount; i++ ) {
        if ( s->Read(RecBuff,p->RecSize) != p->RecSize ) {
          Log(( "!Read read data[%d,%d]",n,i ));
          return FALSE;
        }
        if ( isSame ) {
          if ( !p->NewRecord(pt,RecBuff) )
            return FALSE;
        } else {
          if ( !p->NewRecordCvt(pt,RecBuff) )
            return FALSE;
        }
      }
    }

 return TRUE;
}

/********************************************************************
  HDB file READ
 ********************************************************************/
static BOOL dbReadInfo( PHIStream s, dbhFile *fh )
  {
     if ( s->Read(fh,sizeof(*fh)) != sizeof(*fh) ) {
       Log(( "!dbReadInfo !header" ));
       return FALSE;
     }

     if ( fh->Id != HDBF_FILE_ID || fh->HeaderSize != sizeof(*fh) ) {
       FIO_SETERRORN( ERROR_HEADER );
       Log(( "!dbReadInfo !ID" ));
       return FALSE;
     }
     if ( !fh->Info.Version && !fh->Info.Backup ) {
       FIO_SETERRORN( ERROR_HEADER );
       Log(( "!dbReadInfo !version" ));
       return FALSE;
     }

 return TRUE;
}

static void InitRBuff( DWORD sz )
  {  static BOOL Added = FALSE;

     if ( !Added ) {
       aProc = AtExit(idDelRecBuff);
       Added = TRUE;
     }

     if ( !RecBuff || RecBuffSize < sz ) {
       _Del( RecBuff );
       RecBuff = _Alloc( sz+1 );
       Assert( RecBuff );
       RecBuffSize = sz;
     }
}
//--------------------------------------------------------------------
BOOL MYRTLEXP HDBReadInfo( PHIStream s, PHDBFileInfo p/*=NULL*/ )
  {
     if ( !s ) {
       FIO_SETERRORN( ERROR_INVALID_PARAMETER );
       Log(( "!ReadInfo !s" ));
       return FALSE;
     }

     dbhFile fh;
     if ( !dbReadInfo(s,&fh) )
       return FALSE;

     if ( p )
       memcpy( p,&fh.Info,sizeof(*p) );

 return TRUE;
}

//--------------------------------------------------------------------
BOOL MYRTLEXP HDBRead( PHIStream s, PHDBDatabase db/*=NULL*/, PHDBDatabase bdb/*=NULL*/ )
  {  dbhFile fh;

     if ( !s ) {
       FIO_SETERRORN( ERROR_INVALID_PARAMETER );
       return FALSE;
     }

//File header
     if ( !dbReadInfo(s,&fh) )
       return FALSE;

     //Init rec buffer
     InitRBuff( fh.MaxRecSize );

     //Init rec columns
     RDBase rdb( fh.TableCount ),
            rbdb( fh.TableCount );
     BOOL   rc;

//Read info
     if ( fh.Info.Version )
       if ( !rdb.ReadInfo( s ) )
         return FALSE;

     if ( fh.Info.Backup )
       if ( !rbdb.ReadInfo( s ) )
         return FALSE;

//Read data
     if ( db )  db->Clear();
     if ( bdb ) bdb->Clear();

     //Front
     if ( fh.Info.Version ) {
       if ( db )
         rc = rdb.Read( s,db );
        else
         rc = rdb.Skip( s );

       if ( !rc )
         return FALSE;
     }

     //Backup
     if ( fh.Info.Backup ) {
       if ( bdb )
         rc = rbdb.Read( s,bdb );
        else
         rc = rbdb.Skip( s );

       if (!rc)
         return FALSE;
     }

     if ( db && fh.Info.Version ) {
       db->Version = fh.Info.Version;
       db->Time    = fh.Info.Time;
     }
     if ( bdb && fh.Info.Backup ) {
       bdb->Version = fh.Info.Backup;
       bdb->Time    = fh.Info.BackupTime;
     }
     Log(( "HDBRead rc=%d",rc ));

 return TRUE;
}

//--------------------------------------------------------------------
BOOL MYRTLEXP HDBReadCreate( PHIStream s, DWORD Flags,PHDBDatabase& db, PHDBDatabase& bdb )
  {  dbhFile fh;

     db  = NULL;
     bdb = NULL;

     if ( !s ||
          //Noone base requested
          (Flags&(HDB_DBF_FRONTBASE|HDB_DBF_BACKBASE|HDB_DBF_FRONTFORCE|HDB_DBF_BACKFORCE)) == 0 ) {
       FIO_SETERRORN( ERROR_INVALID_PARAMETER );
       Log(( "!HDBReadCreate !params" ));
       return FALSE;
     }

   //Force NONCONST names
     SET_FLAG( Flags, HDB_DBF_NONCONST );

//File header
     if ( !dbReadInfo(s,&fh) )
       return FALSE;

     //Init rec buffer
     InitRBuff( fh.MaxRecSize );

     //Init rec columns
     RDBase rdb( fh.TableCount ),
            rbdb( fh.TableCount );

//Read info
     if ( fh.Info.Version )
       if ( !rdb.ReadInfo( s ) )
         return FALSE;

     if ( fh.Info.Backup )
       if ( !rbdb.ReadInfo( s ) )
         return FALSE;

//Read data
     BOOL rc = TRUE;

     do{
       if ( db )  db->Clear();
       if ( bdb ) bdb->Clear();
       //Clear
       //Create temp info structure
       if ( IS_FLAG(Flags,HDB_DBF_FRONTBASE) && fh.Info.Version ||
            IS_FLAG(Flags,HDB_DBF_FRONTFORCE) ) {
         db = HDBCreate( fh.Info.Version ? rdb.CreateInfos() : rbdb.CreateInfos(),HDB_DBF_NONCONST|Flags );
         if ( !db )
           break;
         db->Version = fh.Info.Version;
         db->Time    = fh.Info.Time;
       }
       if ( IS_FLAG(Flags,HDB_DBF_BACKBASE) && fh.Info.Backup ||
            IS_FLAG(Flags,HDB_DBF_BACKFORCE) ) {
         bdb = HDBCreate( fh.Info.Backup ? rbdb.CreateInfos() : rdb.CreateInfos(),HDB_DBF_NONCONST|Flags );
         if ( !bdb )
           break;
         bdb->Version = fh.Info.Backup;
         bdb->Time    = fh.Info.BackupTime;
       }

       //Front
       if ( fh.Info.Version ) {
         if ( db )
           rc = rdb.Read( s,db );
          else
           rc = rdb.Skip( s );

         if ( !rc )
           break;
       }


       //Backup
       if ( fh.Info.Backup ) {
         if ( bdb )
           rc = rbdb.Read( s,bdb );
          else
           rc = rbdb.Skip( s );

         if (!rc)
           break;
       }
     }while(0);

     if ( !rc ) {
       HDBDestroy( db );
       HDBDestroy( bdb );
     }

 return rc;
}
