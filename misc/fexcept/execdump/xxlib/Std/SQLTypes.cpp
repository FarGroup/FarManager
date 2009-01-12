#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

static char LocalBuff[ 100 ];

CONSTSTR DBColumn::TypeName( void )          const { return Type2Name(Type); }
CONSTSTR DBColumn::OptionName( void )        const { return Option2Name(Options); }
DWORD    DBColumn::FixLen( void )            const { return (DWORD)(SQL_IS_FLOAT(Type)?(Length-Decimal):Length); }
DWORD    DBColumn::DecLen( void )            const { return (DWORD)(SQL_IS_FLOAT(Type)?Decimal:0); }
CONSTSTR DBColumn::Type2DBType( void )       const { return Type2DBType((PDBColumn)this); }
CONSTSTR DBColumn::Type2Size( char *buff )   const { return Type2Size((PDBColumn)this,buff); }
CONSTSTR DBColumn::Type2Def( char *buff )    const { return Type2Def((PDBColumn)this,buff); }

#define RET_TP( tp ) case tp: return #tp + 4;
CONSTSTR DBColumn::Type2Name( WORD v )
  {
    switch( v ) {
      RET_TP(SQL_DT_NOTYPE)
      RET_TP(SQL_DT_DATE)
      RET_TP(SQL_DT_TIME)
      RET_TP(SQL_DT_TIMESTAMP_STRUCT)
      RET_TP(SQL_DT_TIMESTAMP)
      RET_TP(SQL_DT_VARCHAR)
      RET_TP(SQL_DT_FIXCHAR)
      RET_TP(SQL_DT_LONGVARCHAR)
      RET_TP(SQL_DT_STRING)
      RET_TP(SQL_DT_DOUBLE)
      RET_TP(SQL_DT_FLOAT)
      RET_TP(SQL_DT_DECIMAL)
      RET_TP(SQL_DT_INT)
      RET_TP(SQL_DT_SMALLINT)
      RET_TP(SQL_DT_BINARY)
      RET_TP(SQL_DT_LONGBINARY)
      RET_TP(SQL_DT_VARIABLE)
      RET_TP(SQL_DT_TINYINT)
      RET_TP(SQL_DT_BIGINT)
      RET_TP(SQL_DT_UNSINT)
      RET_TP(SQL_DT_UNSSMALLINT)
      RET_TP(SQL_DT_UNSBIGINT)
      RET_TP(SQL_DT_BIT)
    }
 return "<unk>";
}
#define ADD_OPT( tp ) if ( IS_FLAG(v,tp) ) { if (*str) strcat(str," | "); strcat(str,#tp+4); }
CONSTSTR DBColumn::Option2Name( WORD v )
  {  static char str[60];
    str[0] = 0;
    ADD_OPT( SQL_DT_NULLS_ALLOWED );
    ADD_OPT( SQL_DT_PROCEDURE_OUT );
    ADD_OPT( SQL_DT_PROCEDURE_IN );
    ADD_OPT( SQL_DT_UPDATABLE );
    ADD_OPT( SQL_DT_DESCRIBE_INPUT );
    ADD_OPT( SQL_DT_AUTO_INCREMENT );
    ADD_OPT( SQL_DT_KEY_COLUMN );
    ADD_OPT( SQL_DT_HIDDEN_COLUMN );
 return str;
}

CONSTSTR DBColumn::Type2DBType( PDBColumn col )
  {
    switch( col->Type ) {
      case             SQL_DT_DATE: return "DATE";
      case             SQL_DT_TIME: return "TIME";
      case SQL_DT_TIMESTAMP_STRUCT:
      case        SQL_DT_TIMESTAMP: return "TIMESTAMP";
      case           SQL_DT_STRING:
      case          SQL_DT_VARCHAR:
      case          SQL_DT_FIXCHAR: return "CHARACTER";
      case      SQL_DT_LONGVARCHAR: return "LONG VARCHAR";
      case           SQL_DT_DOUBLE: return "DOUBLE";
      case            SQL_DT_FLOAT: return "FLOAT";
      case          SQL_DT_DECIMAL: return "DECIMAL";
      case           SQL_DT_UNSINT:
      case           SQL_DT_BIGINT:
      case        SQL_DT_UNSBIGINT:
      case              SQL_DT_INT: return "INT";
      case      SQL_DT_UNSSMALLINT:
      case         SQL_DT_SMALLINT: return "SMALLINT";
      case           SQL_DT_BINARY: return "BINARY";
      case       SQL_DT_LONGBINARY: return "LONG BINARY";
      case          SQL_DT_TINYINT: return "TINYINT";
      case         SQL_DT_VARIABLE:
      case              SQL_DT_BIT:
                           default: return NULL;
    }
}
CONSTSTR DBColumn::Type2Size( PDBColumn col,char *buff )
  {
    if (!buff) buff = LocalBuff;
    buff[0] = 0;
    switch( col->Type ) {
      case           SQL_DT_UNSINT:
      case           SQL_DT_BIGINT:
      case        SQL_DT_UNSBIGINT:
      case              SQL_DT_INT:
      case      SQL_DT_UNSSMALLINT:
      case         SQL_DT_SMALLINT:
      case       SQL_DT_LONGBINARY:
      case          SQL_DT_TINYINT:
      case           SQL_DT_DOUBLE:
      case            SQL_DT_FLOAT:
      case      SQL_DT_LONGVARCHAR:
      case             SQL_DT_DATE:
      case             SQL_DT_TIME:
      case SQL_DT_TIMESTAMP_STRUCT:
      case        SQL_DT_TIMESTAMP: buff[0] = 0; break;
      case           SQL_DT_BINARY:
      case           SQL_DT_STRING:
      case          SQL_DT_VARCHAR:
      case          SQL_DT_FIXCHAR: sprintf( buff,"( %d )",col->Length );
                                break;
      case          SQL_DT_DECIMAL: sprintf( buff,"( %d,%d )",col->Length,col->DecLen() );
                                break;
      case         SQL_DT_VARIABLE:
      case              SQL_DT_BIT:
                           default: return NULL;
    }
 return buff;
}
CONSTSTR DBColumn::Type2Def( PDBColumn col,char *buff )
  {  size_t len;
    if (!buff) buff = LocalBuff;
    buff[0] = 0;
    strcpy( buff,col->Name );
    strcat( buff," " );
    strcat( buff,Type2DBType(col) );
    len = strlen(buff);
    Type2Size(col,buff+len+1);
    if ( buff[len+1] ) buff[len] = ' ';
 return buff;
}
