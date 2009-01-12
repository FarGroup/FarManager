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
  HDB file WRITE
 ********************************************************************/
static BOOL dbWrite( PHOStream s, PHDBDatabase db )
  {
     for( WORD i = 0; i < db->TableCount(); i++ ) {
       PITable t = (PITable)db->Table( i );
       if ( !t ) {
         Log(( "!get table [%d]",i ));
         return FALSE;
       }
       //Skip virtuals
       if ( !IS_FLAG(t->Flags,HDB_TBT_REAL) )
         continue;

       dbhTable  th;
       memset( &th,0,sizeof(th) );
       th.Id = HDBF_TABLE_ID;
       StrCpy( th.Name,t->Name,sizeof(th.Name) );

       if ( !s->Write(&th,sizeof(th)) )
         return FALSE;

       for( DWORD n = 0; n < t->RecCount(); n++ ) {
         LPVOID ptr = t->Record( n );
         if ( !ptr ) {
           Log(( "!get rec data[%d]",n ));
           return FALSE;
         }

         if ( !s->Write(ptr,t->RecSize) )
           return FALSE;
       }
     }

 return TRUE;
}

static BOOL dbWriteInfo( PHOStream s, PHDBDatabase db )
  {
     for( WORD n = 0; n < db->TableCount(); n++ ) {
       PITable t = (PITable)db->Table( n );
       if ( !t ) {
         Log(( "!get table [%d]",n ));
         return FALSE;
       }
       //Skip virtuals
       if ( !IS_FLAG(t->Flags,HDB_TBT_REAL) )
         continue;

       dbhTable  th;
       memset( &th,0,sizeof(th) );

       th.Id = HDBF_TABLE_ID;
       StrCpy( th.Name,t->Name,sizeof(th.Name) );
       th.Type         = t->Type & HDB_TBT_MASK;
       th.ColCount     = t->ColCount();
       th.RecCount     = t->RecCount();
       th.RecIncrement = t->RecIncrement;
       th.RecSize      = t->RecSize;

       if ( !s->Write(&th,sizeof(th)) )
         return FALSE;

       WORD      i;
       PHDBValue val = t->ColValues(NULL);

       if ( !val ) {
         Log(( "!get ColValues" ));
         return FALSE;
       }

       for( i = 0; i < t->ColCount(); i++,val++ ) {
         dbhColumn ch;
         memset( &ch,0,sizeof(ch) );

         StrCpy( ch.Name,val->Name,sizeof(ch.Name) );
         ch.Offset = val->Offset;
         ch.Size   = val->Size;
         ch.Type   = val->Type;

         if ( !s->Write(&ch,sizeof(ch)) )
           return FALSE;
       }
     }

 return TRUE;
}
//--------------------------------------------------------------------
BOOL MYRTLEXP HDBWrite( PHOStream s, PHDBDatabase db/*=NULL*/, PHDBDatabase bdb/*=NULL*/ )
  {
     if ( !s || (!db && !bdb) ) {
       FIO_SETERRORN( ERROR_INVALID_PARAMETER );
       return FALSE;
     }

//File header
     dbhFile fh;
     memset( &fh,0,sizeof(fh) );

     fh.Id           = HDBF_FILE_ID;
     fh.Info.Version = db ? db->Version : 0;
     fh.Info.Backup  = bdb ? bdb->Version : 0;
     fh.HeaderSize   = sizeof( fh );

     if ( fh.Info.Version )
       fh.Info.Time.Set( (time_t)fh.Info.Version );

     if ( fh.Info.Backup )
       fh.Info.BackupTime.Set( (time_t)fh.Info.Backup );

     PHDBDatabase d = db ? db : bdb;
     WORD         n;

     for( n = 0; n < d->TableCount(); n++ ) {
       PITable t = (PITable)d->Table( n );
       Assert( t );

       //Only real tables
       if ( IS_FLAG(t->Flags,HDB_TBT_REAL) ) {
         fh.TableCount ++;
         fh.InfoSize   += sizeof(dbhTable) + sizeof(dbhColumn) * t->ColCount();
         fh.DataSize   += sizeof(dbhTable) + t->RecSize * t->RecCount();
         fh.MaxRecSize  = Max( fh.MaxRecSize, t->RecSize );
       }
     }
     if ( db && bdb ) {
       fh.InfoSize *= 2;
       fh.DataSize *= 2;
     }

     if ( !s->Write(&fh,sizeof(fh)) )
       return FALSE;

//Write tables info
     if ( db && !dbWriteInfo(s,db) )
       return FALSE;

     if ( bdb && !dbWriteInfo(s,bdb) )
       return FALSE;

//Write tables data
     if ( db && !dbWrite(s,db) )
       return FALSE;

     if ( bdb && !dbWrite(s,bdb) )
       return FALSE;

 return TRUE;
}
