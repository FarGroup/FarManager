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
struct FTPServerInfo
{
	WORD           ServerType;                  // FTP_TYPE_xxx
	char           ServerInfo[MAX_PATH]; // SYST reply, "" in not supported
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

struct FTPFileInfo: public PluginPanelItem
{
	DWORD  FileType;                    // NET_xxx
	char   FTPOwner[MAX_PATH];      // File owner (if available)
	char   UnixMode[ 10+1 ];            // Unix mode (if available)
	char   Link[MAX_PATH];       // Link (if exist)
};

//------------------------------------------------------------------------
// Listing parser control structure for single parser
//------------------------------------------------------------------------
struct FTPType
{
	BOOL             Detectable;        //TRUE for parsers what can detect formats (unix, dos, OS/2)
	LPCSTR         TypeName;          //Short name
	LPCSTR         TypeDescription;   //Text description

	BOOL (WINAPI *Parser)(const FTPServerInfo* Server, FTPFileInfo* FileInfo, char *ListingString, int ListingLength);
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

	BOOL (WINAPI *PWDParse)(const FTPServerInfo* Server, LPCSTR Reply, char *CurDir, size_t CurDirSize);
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
struct DirListInterface: public FTPPluginInterface
{

	WORD(WINAPI *DetectStringType)(FTPServerInfo * const Server,char *ListingString, int ListingLength);
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

	WORD(WINAPI *DetectDirStringType)(const FTPServerInfo* Server,LPCSTR String);
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

	WORD(WINAPI *GetNumberOfSupportedTypes)(void);
	/** Returns number of defined parsers
	*/

	FTPType*(WINAPI *GetType)(WORD Index);
	/** Returns parser descriptor by it index.
	    If parser with specified index does not exist it must return NULL.
	*/
};

#endif
