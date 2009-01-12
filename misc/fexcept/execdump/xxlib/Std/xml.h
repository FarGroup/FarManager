#ifndef __MY_XML_PARCER
#define __MY_XML_PARCER

/*
  Recognized syntax
   <name ...>  start
   ...         data placed in item on current level
   </name>     end
   <name .../> item
   <?          ?? (treat as normal single tag)
   <!--        comment
*/
/*******************************************************************
    Info structures
 *******************************************************************/
STRUCT( XMLTagPos )
  int Line;
  int Col;
};

STRUCT( XMLErrorInfo )
  char         Error[ MAX_PATH_SIZE ];   //last parce error
  XMLTagPos    Pos;                      //error position
};

/*
  Reader commands            TagName      Value           Data
*/
enum _XML_ReaderCommands {
 XML_READ_VALUE  = 0,       //tag name     value to add    value data
 XML_READ_TAG    = 1,       //tag name     subtag name     NULL
 XML_READ_DATA   = 2,       //tag name     NULL            data
 XML_READ_END    = 3        //tag name     NULL            NULL
};

/*******************************************************************
    XML
 *******************************************************************/
CLASS( CTXML )
 public:
  typedef BOOL   (RTL_CALLBACK *XML_WarningProc)( PCTXML xml,HANDLE Param );                  // Return TRUE for ignore warning
  typedef LPVOID (RTL_CALLBACK *XML_ReaderProc)( PCTXML reader,DWORD Cmd,CONSTSTR TagName,CONSTSTR Value,CONSTSTR Data,LPVOID Ptr );  // Return nonzero ptr if success

#if defined(__BCWIN32__)
  typedef BOOL   __fastcall   (__closure *XML_WarningProcCL)( PCTXML xml,HANDLE Param );
  typedef LPVOID __fastcall   (__closure *XML_ReaderProcCL)( PCTXML reader,DWORD Cmd,CONSTSTR TagName,CONSTSTR Value,CONSTSTR Data,LPVOID Ptr );  // Return nonzero ptr if success
#endif

 private:
  MyString          tmpName,
                    tmpString;
 protected:
  CTParser          in;

 protected:
  CONSTSTR          XML_Name( PMyString s = NULL );
  CONSTSTR          XML_String( char eos );
  CONSTSTR          XML_Value( void );
  void              XML_Comment( void );
  BOOL              XML_Header( CONSTSTR name,LPVOID Ptr,char ender );
  BOOL              XML_isFolder( CONSTSTR nm );

 protected:
  virtual LPVOID Reader( DWORD Cmd,CONSTSTR TagName,CONSTSTR Value,CONSTSTR Data,LPVOID Ptr );

 public:
//
//Valid only then error or warning occured or user progress called
//
  PathString        XML_File;                                       //Set to last loaded file
  PathString        XML_Error;                                      //Set to last parce error
  int               XML_ELine;                                      //Set to error line in input text file
  int               XML_ECol;                                       //Set to error column in parced line
//
//User can change
//
  int               SaveTabSize;                                    //2        Count of spaces used for one indent level
  BOOL              RemoveCR;                                       //FALSE    Remove CR and NL from readed data
  CONSTSTR         *FoldersList;                                    //NULL     Set to NULL terminated list of tags treated as folders
                                                                    //         Set to NULL to use std XML rulles.
  XML_WarningProc   WarningCB;                                      //NULL     user callback called for parce warnings
  XML_WarningProc   ProgressCB;                                     //NULL     user callback called for every folder tag.
                                                                    //         Return FALSE to abort parsing.
                                                                    //         (Called on Save and on Load)
  XML_ReaderProc    ReaderCB;
  HANDLE            UserParam;                                      //NULL     parameter sended to user callback

 public:
   CTXML( CONSTSTR *Folders = NULL );
   virtual ~CTXML( void );

   void         StartAction( CONSTSTR fnm );
   CONSTSTR     StartTag( CONSTSTR name,LPVOID Ptr,BOOL rdBody );
   void         GetErrorInfo( PXMLErrorInfo p );
};

/*******************************************************************
    CTXMLTree
 *******************************************************************/
/*  Can be readed from Root tree key

    Sample:
      XML         xml(...);
      xml.ReadFile( ... )
      printf( "Readed file [%s] has %d bs in length",
              xml.Root->Read(XML_TAG_FILE,""),xml.Root->Read(XML_TAG_FILELEN,0) );
     or
      PHTreeConfig Tree = LoadRTRegTree(...);
      if ( !Tree->GetRoot()->Read(XML_TAG_XML,FALSE) )
        printf( "RTReg does not contain XML structure" );
*/
//                                                     Type                Description
#define XML_TAG_XML       ">Xml"                       //BOOL              has a nonzero if key as a base of successfully parced XML file
#define XML_TAG_FILE      ">File"                      //STRING            file name as given to parces
#define XML_TAG_FILELEN   ">FileLength"                //DWORD             parsed file size
#define XML_TAG_FILETIME  ">FileTime"                  //(DWORD)time_t     parsed file write time

/* Can be readed from every tag

    Sample:
      XML         xml(...);
      PXMLTagInfo info;
      xml.ReadFile( ... )
      xml.Locate( "key/key",&info,NULL );
      printf( "Key at %d,%d",info.Handle->Read(XML_TAG_NLINE) );
*/
//                                                     Type                Description
#define XML_TAG_NLINE   XML_TAG_POS "/Line"          //int                 tag line number in source file (-1 if unknown)
#define XML_TAG_NCOL    XML_TAG_POS "/Col"           //int                 tag column in line of source file (-1 if unknown)

/*
  Used internally by parcer
*/
#define XML_TAG_NAME    ">n"
#define XML_TAG_POS     ">p"
#define XML_TAG_ITEMS   ">i"
#define XML_TAG_ITEMN   ">%d"
#define XML_TAG_COUNT   ">c"
#define XML_TAG_VALUE   ">v"
#define XML_TAG_DATA    ">d"

STRUCT( XMLTagInfo )
  PHConfigItem Handle;         //Handle of current queried item
  PHConfigItem Items;          //Handle to items array (not a valid XML item)
  int          ItemsCount;     //Count of subreys
  CONSTSTR     Name;           //Name of tag
  CONSTSTR     Value;          //Value assigned to tag ( <name=value>)
  CONSTSTR     Data;           //Data placed between start and end of tag (exist in group tags only)
  XMLTagPos    Pos;
};

STRUCT( XMLFileInfo )
  CONSTSTR FileName;           //Parsed file name
  DWORD    FileSize;           //Size of parced file
  time_t   FileTime;           //Time of last write of parced file
};

CLASSBASE( CTXMLTree, public CTXML )
    FILE *File;
  protected:
    BOOL SaveItem( PHConfigItem start,int lvl );
  protected:
    virtual LPVOID Reader( DWORD Cmd,CONSTSTR TagName,CONSTSTR Value,CONSTSTR Data,LPVOID Ptr );
  public:
//
//Accessible data (!!do not change)
//
    PHTreeConfig Tree;                                          //use it to direct access to RTTree
    PHConfigItem Root;
  public:
   CTXMLTree( CONSTSTR *Folders = NULL );
   ~CTXMLTree();

   BOOL ReadFile( CONSTSTR fnm );
   BOOL SaveFile( CONSTSTR fnm,PHConfigItem start = NULL );

   BOOL Locate( CONSTSTR KeyName = NULL,PXMLTagInfo ti = NULL,PHConfigItem start = NULL );
   BOOL Item( PHConfigItem p,int number = -1,PXMLTagInfo ti = NULL );
   int  ValuesCount( PHConfigItem p );
   BOOL ItemValue( PHConfigItem p,int number,PXMLTagInfo ti = NULL );
   int  GroupIndex( PHConfigItem Item );           //Return array index of `Item` subkey
};

#endif