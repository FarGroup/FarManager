#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

#include "xml.h"

#define XML_NAMEEND " \t\n\r/=<>"

/****************************************************
   CTXML
 ****************************************************/
CTXML::CTXML( CONSTSTR *Folders )
    : FoldersList( Folders )
  {
   in.CLineComments  = FALSE;
   in.CBlockComments = FALSE;
   XML_ELine    = 0;
   XML_ECol     = 0;
   SaveTabSize  = 2;
   RemoveCR     = FALSE;
   WarningCB    = NULL;
   ProgressCB   = NULL;
   ReaderCB     = NULL;
   UserParam    = NULL;
}
CTXML::~CTXML( void )
  {
}

void CTXML::GetErrorInfo( PXMLErrorInfo p )
  {
    if (p) {
      StrCpy( p->Error,XML_Error.Text(),sizeof(p->Error) );
      p->Pos.Line = XML_ELine;
      p->Pos.Col  = XML_ECol;
    }
}

BOOL CTXML::XML_isFolder( CONSTSTR nm )
  {
    if (!FoldersList || !nm || !nm[0]) return TRUE;

    for ( int n = 0; FoldersList[n]; n++ )
      if ( StrCmp(nm,FoldersList[n],-1,FALSE) == 0 ) return TRUE;

 return FALSE;
}

CONSTSTR CTXML::XML_Name( PMyString s )
  {  char ch;

     if ( !s ) s = &tmpName;

    s->Set("");
    in.NextGet();
    while( (ch=in.GetChar()) != 0 && strchr(XML_NAMEEND,ch) == NULL )
      s->Add( ch );

    if (!ch) THROW( "No name readed" )

    in.UnGet();

 return s->Text();
}

CONSTSTR CTXML::XML_String( char eos )
  {  char ch;
    tmpString.Set("");
    in.Get();
    while( (ch=in.GetChar()) != 0 && ch != eos )
      tmpString.Add( ch );
    if (!ch) THROW( "No string end" )
 return tmpString.Text();
}

CONSTSTR CTXML::XML_Value( void )
  {  char ch;
    if ( (ch=in.NextGet()) == '\"' || ch == '\'' )
      return XML_String(ch);
     else
      return XML_Name( &tmpString );
}

void CTXML::XML_Comment( void )
  {  char ch;
     int  cm = 0;
// catch -->
    while( (ch=in.Get()) != 0 ) {
      if ( ch == '>' && cm >= 2 )
        break;
      if ( ch == '-' )
        cm++;
       else
        cm = 0;
    }
    if (!ch) THROW( "No comment end" )
}

BOOL CTXML::XML_Header( CONSTSTR name,LPVOID p,char ender )
  {  CONSTSTR     m;
     char         ch;
     PathString   nm;

    while( 1 ) {
      m  = XML_Name();
      nm = m;
      ch = in.NextGet();
      if ( !m[0] && ender == ch ) {
        in.Get();
        if ( in.NextGet() == '>' ) {
          in.Get();
          return FALSE;
        }
      } else
      if ( !m[0] && ch == '>' ) {
        in.Get();
        return TRUE;
      } else
      if ( ch == '=' ) {
        in.Get();
        if ( !Reader( XML_READ_VALUE,name,nm.Text(),XML_Value(),p ) )
          THROW( "Can not add value" );
      } else
      if ( !m[0] && strchr(XML_NAMEEND,ch) ) {
        THROW( Message("Symbol \'%c\' not possible in tag header",ch) );
      } else
      if ( !ch )
        THROW( "Error in TAG header data (EOT not found)" );
    }
}

LPVOID CTXML::Reader( DWORD Cmd,CONSTSTR TagName,CONSTSTR Value,CONSTSTR Data,LPVOID Ptr )
  {
    if ( !ReaderCB )
      return NULL;

 return ReaderCB( this,Cmd,TagName,Value,Data,Ptr );
}

CONSTSTR CTXML::StartTag( CONSTSTR name,LPVOID p,BOOL rdBody )
  {  char         ch;
     CONSTSTR     m;
     MyString     data;
     int          sp,spold = TRUE;

     if (ProgressCB) {
       XML_Error = name;
       XML_ELine = in.GetY();
       XML_ECol  = in.GetX();
       if ( !ProgressCB(this,UserParam) )
         return ">!<";
     }

     while( (ch=in.GetChar()) != 0 ) {

       if ( ch != '<' ) {
         if ( rdBody ) {
           if ( RemoveCR && (ch == '\n' || ch == '\r') ) ch = ' ';
           sp = strchr( " \n\r\t\b",ch) != 0;
           if ( !sp || sp != spold )
             data.Add( ch );
           spold = sp;
         }
         continue;
       }

       ch = in.NextGet();
       if ( !ch ) THROW( "Bad open tag" );

       switch( ch ) {
         case '!': in.Get();
                   if ( in.GetChar() == '-' ) {
                     while( (ch=in.NextGet()) != 0 && ch == '-' ) in.Get();
                     XML_Comment();
                   } else
                     while( ch != '>' && (ch=in.NextGet()) != 0 )
                       in.Get();
                break;

         case '/': in.Get();
                   m = XML_Name();
                   if ( !m[0] )
                     THROW( "No name for closing tag" );

                   if ( in.Get() != '>' )
                     THROW( "No tail '>' for closing tag" );

                   if ( data.Length() &&
                        !Reader( XML_READ_DATA,name,NULL,data.Text(),p) )
                     THROW( "Can not assign tag data" );

                   Reader( XML_READ_END,name,NULL,NULL,p);

                   if ( StrCmp(m,name,-1,FALSE) != 0 ) {
                       if ( WarningCB ) {
                         XML_Error.printf( "Reached unexpected closing tag [%s] in [%s]",m,name );
                         XML_ELine = in.GetY();
                         XML_ECol  = in.GetX();
                         if ( WarningCB(this,UserParam) )
                           return NULL;
                       }
                     return m;
                   }

                   return NULL;

         case '?':
          default: {  PathString nm;
                      LPVOID     it;

                      if (ch == '?') in.Get();
                      m  = XML_Name();
                      if ( !m[0] )
                        THROW( "No tag name found" );

                      nm = m;
                      it = Reader( XML_READ_TAG,name,nm.Text(),NULL,p );
                      if ( !it )
                        THROW( Message("Error create subtag [%s] in [%s]",nm.Text(),name) )

                      if ( XML_Header( nm.Text(),it,(ch == '?')?'?':'/' ) &&
                           XML_isFolder( nm.Text() ) ) {
                        m = StartTag(nm.Text(),it,rdBody);
                        if ( m && StrCmp(nm.Text(),m,-1,FALSE) != 0 )
                          return m;
                      } else
                        Reader( XML_READ_END,nm.Text(),NULL,NULL,p);
                   }
                 break;
       }
     }
 return NULL;
}

void CTXML::StartAction( CONSTSTR fnm )
  {
     XML_File     = fnm;
     XML_Error    = "";
     XML_ELine    = 0;
     XML_ECol     = 0;
}
/****************************************************
   CTXMLTree
 ****************************************************/
CTXMLTree::CTXMLTree( CONSTSTR *Folders )
    : CTXML( Folders )
  {
     Tree = HConfigCreate();
     Root = Tree->GetRoot();
}

CTXMLTree::~CTXMLTree()
  {
    HConfigDestroy(Tree);
}

LPVOID CTXMLTree::Reader( DWORD Cmd,CONSTSTR TagName,CONSTSTR Value,CONSTSTR Data,LPVOID Ptr )
  {  PHConfigItem p = ((PHConfigItem)Ptr),
                  pItems,it;
     int          cn;

     USEARG( TagName )

     switch( Cmd ) {
       case XML_READ_VALUE: if ( !Value[0] ) Value = XML_TAG_VALUE;
                            p->Write( Value,Data );
                          return (LPVOID)1;

       case   XML_READ_TAG: pItems = p->ConfigGetCreate( XML_TAG_ITEMS );
                            cn     = pItems->Read( XML_TAG_COUNT,(int)0 );

                            if ( !pItems ||
                                 (it = pItems->ConfigGetCreate( Message(XML_TAG_ITEMN,cn) )) == NULL )
                              return NULL;

                            pItems->Write( XML_TAG_COUNT,cn+1 );

                            it->Write( XML_TAG_NAME,  Value );
                            it->Write( XML_TAG_NLINE, in.GetY() );
                            it->Write( XML_TAG_NCOL,  in.GetX() - strLen(Value) );
                          return it;

       case  XML_READ_DATA: p->Write( XML_TAG_DATA,Data );
                          return (LPVOID)1;

       case   XML_READ_END: return (LPVOID)1;

                   default: return NULL;
     }
}

BOOL CTXMLTree::ReadFile( CONSTSTR fnm )
  {  DEFTRYVAR( ex )
     time_t tm;
     BOOL   res = FALSE;

     StartAction( fnm );
     if ( !GetFileTimes(fnm,NULL,&tm,NULL) || !in.Assign(fnm) )
       XML_Error = "Error open input file";
      else
       TRY(1)
         Root->Clear();
         Root->Write( XML_TAG_FILE,     XML_File.Text() );
         Root->Write( XML_TAG_FILELEN,  FileLength(XML_File.Text()) );
         Root->Write( XML_TAG_FILETIME, (DWORD)tm );
         Root->Write( XML_TAG_XML,      TRUE );
         res = StartTag("",Root,TRUE) == NULL;
       CATCH(ex,1)
         XML_Error = Ex2Str(ex);
       CATCH_END(1)

     if ( !res ) {
       Root->Clear();
       XML_ELine = in.GetY();
       XML_ECol  = in.GetX();
     }
 return res;
}

BOOL CTXMLTree::SaveItem( PHConfigItem start,int lvl )
  {  XMLTagInfo ti,val;
     int        n;
     BOOL       needEnd = FALSE;

     if ( !Item(start,-1,&ti) )
       return TRUE;

     if (ProgressCB) {
       XML_Error = ti.Name;
       XML_ELine = lvl;
       XML_ECol  = ti.Handle->Index();
       if ( !ProgressCB(this,UserParam) ) return FALSE;
     }
     if ( ti.Name[0] ) {
//Header
       if (lvl && SaveTabSize) fprintf(File,"%*c",lvl*SaveTabSize,' ');
       fprintf(File, "<%s",ti.Name );
       if ( ti.Value[0] )
         fprintf(File, "=\"%s\"",ti.Value );
//header values
       for ( n = 0; ItemValue(start,n,&val); n++ )
         fprintf(File, " %s=\"%s\"",val.Name,val.Value );
//EOH
       if ( ti.ItemsCount || ti.Data[0] || (FoldersList && XML_isFolder(ti.Name)) ) {
         fprintf(File, ">" );
         needEnd = TRUE;
       } else
         fprintf(File, "/>" );
//Data
       if ( ti.Data[0] ) {
         if ( ti.ItemsCount || strchr(ti.Data,'\n') || strchr(ti.Data,'\r') ) {
           fprintf(File, "%s\n",ti.Data );
           needEnd = TRUE;
         } else {
           fprintf(File, "%s</%s>\n",ti.Data,ti.Name );
           needEnd = FALSE;
          }
       } else {
         fprintf(File, "\n" );
         needEnd = needEnd || ti.ItemsCount > 0;
       }
     }
//Items
     for ( n = 0; Item(start,n,&val); n++ )
       if ( !SaveItem(val.Handle,lvl+(ti.Name[0] != 0)) )
         return FALSE;
//EOT
     if ( ti.Name[0] && needEnd ) {
       if (lvl && SaveTabSize) fprintf(File,"%*c",lvl*SaveTabSize,' ');
       fprintf(File, "</%s>\n",ti.Name );
     }
 return TRUE;
}

BOOL CTXMLTree::SaveFile( CONSTSTR fnm,PHConfigItem start )
  {  BOOL res;

    if ( !fnm )
      return FALSE;

    if ( fnm[0] == '-' && fnm[1] == 0 )
      File = stdout;
     else
      File = fopen( fnm,"w" );
    if (!File)
      return FALSE;

    StartAction( fnm );

    if (!start) start = Root;
    res = SaveItem(start,0);

    fclose(File); File = NULL;
    if (!res) unlink(fnm);

  return res;
}

BOOL CTXMLTree::Locate( CONSTSTR KeyName,PXMLTagInfo ti,PHConfigItem p )
  {  PHConfigItem it;
     CONSTSTR     nm = KeyName,m;
     int          n,cn,nmc,
                  num = 0;
     char         ch;
     char         str[10];

     if (!p) p = Root;
     it = p->ConfigGetCreate( XML_TAG_ITEMS );
     if (!it) return FALSE;

     if (!KeyName || !KeyName[0] ) {
       if ( ti ) {
         ti->Handle     = p;
         ti->Items      = it;
         ti->ItemsCount = it->Read( XML_TAG_COUNT,0 );
         ti->Name       = p->Read( XML_TAG_NAME,"" );
         ti->Value      = p->Read( XML_TAG_VALUE,"" );
         ti->Data       = p->Read( XML_TAG_DATA,"" );
         ti->Pos.Line   = p->Read( XML_TAG_NLINE, -1 );
         ti->Pos.Col    = p->Read( XML_TAG_NCOL,  -1 );
       }
       return TRUE;
     }

     for( nmc = 0; *KeyName && strchr("\\/[",*KeyName) == NULL; nmc++,KeyName++ );
     if ( !nmc ) return FALSE;

     if ( *KeyName && *KeyName == '[' ) {
       for( KeyName++,num = 0; (ch=*KeyName) != 0 && isdigit(ch); KeyName++ )
         num = ((int)ch-'0') + num*10;
       if ( !ch || *KeyName != ']' ) return FALSE;
       if (*KeyName) KeyName++;
     }
     if (*KeyName) KeyName++;


     cn = it->Read( XML_TAG_COUNT,0 );
     for ( n = 0; n < cn; n++ ) {
       Sprintf(str,XML_TAG_ITEMN,n );
       p = it->Locate(str);
       if (!p) break;
       m = p->Read(XML_TAG_NAME,"");
       if ( StrCmp( m,nm,nmc,FALSE ) != 0 || m[nmc] ) continue;
       if ( num == 0 ) return Locate( KeyName,ti,p );
       num--;
     }
 return FALSE;
}

BOOL CTXMLTree::Item( PHConfigItem p,int number,PXMLTagInfo ti )
  {  PHConfigItem it;

     if ( !p ) return FALSE;

     it = p->Locate( XML_TAG_ITEMS );
     if ( number != -1 ) {
       if (!it) return FALSE;
       return Item( it->Locate(Message(XML_TAG_ITEMN,number)),-1,ti );
     }

     if (ti) {
       ti->Handle     = p;
       ti->Items      = it;
       ti->ItemsCount = it?it->Read( XML_TAG_COUNT,0 ):0;
       ti->Name       = p->Read( XML_TAG_NAME,"" );
       ti->Value      = p->Read( XML_TAG_VALUE,"" );
       ti->Data       = p->Read( XML_TAG_DATA,"" );
       ti->Pos.Line   = p->Read( XML_TAG_NLINE, -1 );
       ti->Pos.Col    = p->Read( XML_TAG_NCOL,  -1 );
     }
 return TRUE;
}
BOOL CTXMLTree::ItemValue( PHConfigItem p,int number,PXMLTagInfo ti )
  {  PHConfigItem it;
     CONSTSTR     m;

     for ( int n = 0; (it=p->Leath(n)) != NULL; n++ ) {
       m = it->GetName();
       if ( !m[0] || m[0] == '>' ) continue;
       if (number == 0) {
         if (ti) {
           ti->Handle     = it;
           ti->Items      = NULL;
           ti->ItemsCount = 0;
           ti->Name       = m;
           ti->Value      = it->Read("","");
           ti->Data       = "";
         }
         return TRUE;
       }
       number--;
     }
 return FALSE;
}
int CTXMLTree::ValuesCount( PHConfigItem p )
  {  PHConfigItem it;
     CONSTSTR     m;
     int          cn = 0;

     for ( int n = 0; (it=p->Leath(n)) != NULL; n++ ) {
       m = it->GetName();
       if ( *m && *m != '>' ) cn++;
     }
 return cn;
}
int CTXMLTree::GroupIndex( PHConfigItem item )
  {  PHConfigItem gr,p;
     int          cn = 0,n;
     CONSTSTR     nm;

     if ( !item ) return -1;
     nm = item->Read(XML_TAG_NAME,"");
     if ( !nm[0] ) return -1;

     gr = item->Parent();
     if ( !gr ||
          StrCmp(gr->GetName(),XML_TAG_ITEMS) != 0 )
       return -1;

     for ( n = 0; (p=gr->Locate(Message(XML_TAG_ITEMN,n))) != NULL; n++ ) {
       if ( p == item ) break;
       if ( StrCmp(p->Read(XML_TAG_NAME,""),nm) == 0 ) cn++;
     }
 return p?cn:(-1);
}
