#ifndef __FTP_PLUGIN_DIRECTORY_LIST
#define __FTP_PLUGIN_DIRECTORY_LIST

//------------------------------------------------------------------------
//Directory listing parcer
//------------------------------------------------------------------------
#define FTP_DIRLIST_MAGIC  MK_ID( 'F','D','l',2 )   //Define version of parser used

//------------------------------------------------------------------------
// Known server types
//------------------------------------------------------------------------
#define FTP_TYPE_DETECT    (MAX_WORD-1)  //Autodetect
#define FTP_TYPE_INVALID   MAX_WORD      //Unknown or invalid

#define FTP_TYPE_UNIX      0
#define FTP_TYPE_NT        1
#define FTP_TYPE_VMS       2
#define FTP_TYPE_EPLF      3
#define FTP_TYPE_CMS       4
#define FTP_TYPE_TCPC      5
#define FTP_TYPE_OS2       6
#define FTP_TYPE_KOMUT     7
#define FTP_TYPE_NETWARE   8
#define FTP_TYPE_VXDOS     9
#define FTP_TYPE_PCTCP     10
#define FTP_TYPE_OS400     11
#define FTP_TYPE_MVS       12

//------------------------------------------------------------------------
// Serve information (set server type and SYST command reply)
//------------------------------------------------------------------------
STRUCT( FTPServerInfo )
  WORD           ServerType;                  // FTP_TYPE_xxx
  char           ServerInfo[ MAX_PATH_SIZE ]; // SYST reply, "" in not supported
};

//------------------------------------------------------------------------
// Strcture describes one file|dir entry
//------------------------------------------------------------------------
//Additional file|dir attributes
#define NET_FILE_TYPE             0   //File
#define NET_DIRECTORY             1   //Directory
#define NET_SYM_LINK              2   //Link (unknown destination)
#define NET_SYM_LINK_TO_DIR       3   //Link to directory
#define NET_SYM_LINK_TO_FILE      4   //Link to file
#define NET_SKIP                  5   //Line format correct but it must be skipped

STRUCTBASE( FTPFileInfo, public PluginPanelItem )
  DWORD  FileType;                    // NET_xxx
  char   FTPOwner[ MAX_PATH_SIZE ];      // File owner (if available)
  char   UnixMode[ 10+1 ];            // Unix mode (if available)
  char   Link[ MAX_PATH_SIZE ];       // Link (if exist)
};

//------------------------------------------------------------------------
// Listing parser control structure for single parser
//------------------------------------------------------------------------
STRUCT( FTPType )
  BOOL             Detectable;        //TRUE for parsers what can detect formats (unix, dos, OS/2)
  CONSTSTR         TypeName;          //Short name
  CONSTSTR         TypeDescription;   //Text description

  BOOL     (DECLSPEC *Parser   )( const PFTPServerInfo Server, PFTPFileInfo FileInfo, char *ListingString, int ListingLength );
  /** Parses one line of received server listing.
      One line allways contains one file entry.

      Params:
        Server        - IN  - current server information
        FileInfo      - OUT - decoded file entry
        ListingString - IN  - listing string (can be safely modified inside parser)
        ListingLength - IN  - size of list string (number of chars until ZERO)

      Ret:
        TRUE - if line was successfully parced
  */

  BOOL     (DECLSPEC *PWDParse )( const PFTPServerInfo Server, CONSTSTR Reply, char *CurDir, size_t CurDirSize );
  /** Decode server reply on PWD command.
      This procedure is optional and may be set to NULL for use default decoding methods.

      Params:
        Server     - IN  - current server information
        Reply      - IN  - server reply (without leading numbers and spaces)
        CurDir     - OUT - buffer for store current directory into
        CurDirSize - IN  - maximum size of dirrectory buffer

      Ret:
        TRUE - if string was successfully parced
  */
};

//------------------------------------------------------------------------
// Main parser-plugin interface
//------------------------------------------------------------------------
STRUCTBASE( DirListInterface, public FTPPluginInterface )

  WORD     (DECLSPEC *DetectStringType)( const PFTPServerInfo Server,char *ListingString, int ListingLength );
  /** Detects server type by server info or by given listing string.
      Note: Server type may be already detected, in such case it`s not nesessary to
            perform full detection procedure.

      params:
        Server        - server info struct filled with currently set type of server and
                        SYST reply;
        ListingString - listing stding to detect by;
                        Can be freely modified
        ListingLength - length of listing string in characters. Detect must not access symbols
                        above this offset;

      return:
        Must return FTP_TYPE_DETECT, FTP_TYPE_INVALID on invalid detection or one
        of FTP_TYPE_xxx types on successfull detect.
  */

  WORD     (DECLSPEC *DetectDirStringType)( const PFTPServerInfo Server,CONSTSTR String );
  /** Used to detect type of parser by text returned on PWD command.
      Called only if server type undefined in master plugin.

      Params:
        Server - server info struct filled with currently set type of server and
                 SYST reply;
        String - server reply on PWD command (without trailing numbers and spaces;
                 Can NOT be modified inside detector;

      return:
        Must return FTP_TYPE_DETECT, FTP_TYPE_INVALID on invalid detection or one
        of FTP_TYPE_xxx types on successfull detect.

        If detection succeed returned type will be used to select callback to parse
        PWD reply.
        If detection fail (or ignored) will be used procedure to expant quoted string
        from PWD reply. (This method of directory representation used in most server types)
  */

  WORD     (DECLSPEC *GetNumberOfSupportedTypes)( void );
  /** Returns number of defined parsers
  */

  PFTPType (DECLSPEC *GetType)( WORD Index );
  /** Returns parser descriptor by it index.
      If parser with specified index does not exist it must return NULL.
  */
};

#endif
