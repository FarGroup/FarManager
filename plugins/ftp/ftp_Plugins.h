#ifndef __FTP_PLUGINS_HOLDERS
#define __FTP_PLUGINS_HOLDERS

//------------------------------------------------------------------------
/* Number in array of interfaces [ftp_Plugin.cpp]
*/
#define PLUGIN_PROGRESS  0
#define PLUGIN_DIRLIST   1
#define PLUGIN_NOTIFY    2

//------------------------------------------------------------------------
STRUCT( FTPPluginHolder )
   HMODULE             Module;
   PFTPPluginInterface Interface;

   virtual BOOL Assign( HMODULE m,PFTPPluginInterface Interface );
   virtual void Destroy( void );
};

extern BOOL             InitPlugins( void );
extern void             FreePlugins( void );
extern PFTPPluginHolder GetPluginHolder( WORD Number );
extern BOOL             PluginAvailable( WORD Number );

//------------------------------------------------------------------------
template <class Cl, WORD Index> struct FTPPlugin {
   PFTPPluginHolder Holder;

   FTPPlugin( void )    { Holder = GetPluginHolder(Index); }
   virtual ~FTPPlugin() { Holder = NULL; }

   Cl Interface( void ) { Assert( Holder ); Assert( Holder->Interface ); return (Cl)Holder->Interface; }
};
//------------------------------------------------------------------------
typedef struct FTPProgress *PFTPProgress;
struct FTPProgress : public FTPPlugin<PProgressInterface,PLUGIN_PROGRESS> {
   HANDLE Object;

   FTPProgress( void ) { Object = NULL; }
   ~FTPProgress()      { if (Object) { Interface()->DestroyObject(Object); Object = NULL; } }

   void Resume( CONSTSTR LocalFileName );
   void Resume( __int64 size );
   BOOL Callback( int Size );
   void Init( HANDLE Connection,int tMsg,int OpMode,PFP_SizeItemList il );
   void InitFile( __int64 sz, CONSTSTR SrcName, CONSTSTR DestName );
   void InitFile( PluginPanelItem *pi, CONSTSTR SrcName, CONSTSTR DestName );
   void InitFile( LPFAR_FIND_DATA pi, CONSTSTR SrcName, CONSTSTR DestName );
   void Skip( void );
   void Waiting( time_t paused );
   void SetConnection( HANDLE Connection );
};

//------------------------------------------------------------------------
typedef struct FTPDirList *PFTPDirList;
struct FTPDirList : public FTPPlugin<PDirListInterface,PLUGIN_DIRLIST> {

   WORD     DetectStringType( const PFTPServerInfo Server,char *ListingString, int ListingLength );
   WORD     DetectDirStringType( const PFTPServerInfo Server,CONSTSTR ListingString );
   WORD     GetNumberOfSupportedTypes( void );
   PFTPType GetType( WORD Index );
};

//------------------------------------------------------------------------
typedef struct FTPNotify *PFTPNotify;
struct FTPNotify : public FTPPlugin<PNotifyInterface,PLUGIN_NOTIFY> {

   void     Notify( const PFTNNotify p );
};

#endif