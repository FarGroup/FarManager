#ifndef __MY_HDB_FILE_STRUCTURE
#define __MY_HDB_FILE_STRUCTURE

#include <Global/pack1.h>

#define HDBF_FILE_ID      MK_ID( 'D','B','f',1 )
#define HDBF_TABLE_ID     MK_ID( 'T','B','L',1 )

#define HDB_MAX_NAME     64

STRUCT( HDBFileInfo )
  DWORD   Version;      //0 for none
  PRTime  Time;
  DWORD   Backup;       //0 for none
  PRTime  BackupTime;
};

LOCALSTRUCT( dbhFile )
  DWORD       Id;                    //HDBF_FILE_ID
  HDBFileInfo Info;
  DWORD       HeaderSize;            //sizeof dbhFile
  DWORD       InfoSize;              //Size of full info part
  DWORD       DataSize;              //Size of full data part
  DWORD       MaxRecSize;            //Maximum size of record
  WORD        TableCount;            //Number of tables in HDB
};

LOCALSTRUCT( dbhTable )
  DWORD       Id;                    //HDBF_TABLE_ID
  char        Name[ HDB_MAX_NAME ];
  DWORD       Type;                  //Type of table (HDB_TBT_MASK part of HDB_TBT_xxx flags)
  WORD        ColCount;              //Number of columns
  DWORD       RecCount;              //Number of records in table
  WORD        RecIncrement;          //
  DWORD       RecSize;               //Size of one record
};

LOCALSTRUCT( dbhColumn )
  char        Name[ HDB_MAX_NAME ];  //HDBColumnInfo fields
  DWORD       Offset;                // --`--
  DWORD       Size;                  // --`--
  DWORD       Type;                  // --`--
};
#include <Global/pop.h>

/********************************************************************
  HDB file structures.

  File format:
     1. {Header} dbhFile

     2. {Info}   { dbhTable
                   dbhColumn[ dbhTable.ColCount ]
                 }[ dbhFile.TableCount ]

     3. {Data}   { dbhTable     >>! Only `Id` and `Name` fields are valid and must be checked
                   BYTE[ dbhTable.RecSize ]
                 }[ dbhFile.TableCount ]
 ********************************************************************/
#endif
