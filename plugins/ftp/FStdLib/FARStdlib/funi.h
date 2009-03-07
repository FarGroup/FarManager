#ifndef __FAR_PLUGIN_UNIPLACE_HEADER
#define __FAR_PLUGIN_UNIPLACE_HEADER

//- FAR types
typedef const char *FAR_STRING_PARAM;
typedef const char *const *FAR_MESSAGE_PARAM;

typedef PluginStartupInfo    *PPluginStartupInfo;
typedef FarStandardFunctions *PFarStandardFunctions;
typedef FarMenuItem          *PFarMenuItem;
typedef FarDialogItem        *PFarDialogItem;
typedef EditorGetString      *PEditorGetString;
typedef EditorSetString      *PEditorSetString;
typedef EditorInfo           *PEditorInfo;
typedef EditorSetPosition    *PEditorSetPosition;
typedef EditorSelect         *PEditorSelect;
typedef EditorConvertText    *PEditorConvertText;
typedef EditorConvertPos     *PEditorConvertPos;
typedef EditorColor          *PEditorColor;
typedef PluginPanelItem      *PPluginPanelItem;
typedef PanelMode            *PPanelMode;
typedef PanelRedrawInfo      *PPanelRedrawInfo;
typedef PanelInfo            *PPanelInfo;
typedef FAR_FIND_DATA        *LPFAR_FIND_DATA;

//-   FAR `plugin.hpp` header missed\addons
#define FAR_MAX_LANGID                       2000
#define FAR_MAX_CMDLINE                      1024
#define FAR_MAX_TITLE                        512
#define FAR_MAX_NAME                         256
#define FAR_MAX_PATHSIZE                     MAX_PATH_SIZE
#define FAR_MAX_REG                          1024
#define FAR_MAX_REGNAME                      80
#define FAR_MAX_MSGLINE                      13
#define FAR_MAX_CAPTION                      512
#define FAR_MAX_DLGITEM                      512

//Turn off fake NM const, use FAR_MAX_PATHNAME instead
#undef NM
//Panel
#define FAR_MAX_COLS      10                  //Maximum FAR columns modes
#define FAR_MAX_TITLES    20                  //Maximum user defined titles
//Extend panle col numbers
#define FAR_NAME_COL      (FAR_MAX_COLS+0)    //Place of string to hold dynamic allocated string for assign to NAME panel field
#define FAR_OWNER_COL     (FAR_MAX_COLS+1)    // ... for OWNER field
#define FAR_DESC_COL      (FAR_MAX_COLS+2)    // ... for DESCRIPTION field
#define FAR_MAX_USERCOLS  (FAR_DESC_COL+1)    //Maximum number of holded dynamic strings
//Key bar titles
#define  FKBT_N            0
#define  FKBT_C            1
#define  FKBT_A            2
#define  FKBT_S            3
#define  FKBT_CS           4
#define  FKBT_AS           5
#define  FKBT_CA           6
#define  FKBT_MAX          (FKBT_CA+1)
#define  FK_MAX            12

#define  FMSG_MB_MASK      0x000F0000       //Mask for all FMSG_MB_xxx

//F-key operations
#define  FKBT_N2F(n)       ((n)+1)          //Index2F
#define  FKBT_F2N(n)       ((n)-1)          //F2Index
#define  FKBT_F2M(n)       (1UL<<((n)-1))   //F2Mask
#define  FKBT_N2M(n)       (1UL<<(n))       //F2Mask

#define  FKM_F1            FKBT_F2M(1)
#define  FKM_F2            FKBT_F2M(2)
#define  FKM_F3            FKBT_F2M(3)
#define  FKM_F4            FKBT_F2M(4)
#define  FKM_F5            FKBT_F2M(5)
#define  FKM_F6            FKBT_F2M(6)
#define  FKM_F7            FKBT_F2M(7)
#define  FKM_F8            FKBT_F2M(8)
#define  FKM_F9            FKBT_F2M(9)
#define  FKM_F10           FKBT_F2M(10)
#define  FKM_F11           FKBT_F2M(11)
#define  FKM_F12           FKBT_F2M(12)

typedef char *FKeyBarTitle[12];
typedef FKeyBarTitle *PFKeyBarTitle;

//Hot key for operate with codes getted from plugin `ProcessKey` and passed to FPanel `DoProcessKey`
#define FHK_MAKE(sh,key)   MK_DWORD(((WORD)sh),((WORD)key))
#define FHK_SHIFT(hkey)    ((unsigned int)LO_WORD((DWORD)hkey))
#define FHK_KEY(hkey)      ((int)HI_WORD((DWORD)hkey))

//Usefull console chars
#define FAR_SHADOW_CHAR                      '\xB0' //°
#define FAR_FULL_CHAR                        '\xDB' //Û
#define FAR_VERT_CHAR                        '\xB3' //³
#define FAR_DVERT_CHAR                       '\xBA' //º
#define FAR_HORZ_CHAR                        '\xC4' //Ä
#define FAR_DHORZ_CHAR                       '\xCD' //Í
#define FAR_CHECK_CHAR                       '\xFb' //û
#define FAR_SBMENU_CHAR                      '\x10' //
#define FAR_LEFT_CHAR                        '\x11' //
#define FAR_RIGHT_CHAR                       FAR_SBMENU_CHAR
#define FAR_SPACE_CHAR                       '\xFA' //ú
#define FAR_TAB_CHAR                         '\xFE' //þ
#define FAR_DOWN_CHAR                        '\x19' //
#define FAR_SKIP_CHAR                        '\x20' //' '

#define FAR_SHADOW_STR                       "\xB0" //°
#define FAR_FULL_STR                         "\xDB" //Û
#define FAR_VERT_STR                         "\xB3" //³
#define FAR_DVERT_STR                        "\xBA" //º
#define FAR_HORZ_STR                         "\xC4" //Ä
#define FAR_DHORZ_STR                        "\xCD" //Í
#define FAR_CHECK_STR                        "\xFb" //û
#define FAR_SBMENU_STR                       "\x10" //
#define FAR_LEFT_STR                         "\x11" //
#define FAR_RIGHT_STR                        FAR_SBMENU_STR
#define FAR_SPACE_STR                        "\xFA" //ú
#define FAR_TAB_STR                          "\xFE" //þ
#define FAR_DOWN_STR                         "\x19" //
#define FAR_SKIP_STR                         "\x20" //" "
//Menu spec characters
#define FMENU_CHECKED                        FAR_CHECK_STR
#define FMENU_DELIMITER                      FAR_VERT_STR
#define FMENU_SUBMENU                        FAR_SBMENU_STR
#define FMENU_DIALOG                         FAR_SKIP_STR
#define FMENU_NORMAL                         FAR_SKIP_STR

//Diffs
#define FE_CURSTRING    -1      //number of current string in editor

//Console colors
enum FarConsoleColors {
    fccBLACK,          /* dark colors */
    fccBLUE,
    fccGREEN,
    fccCYAN,
    fccRED,
    fccMAGENTA,
    fccBROWN,
    fccLIGHTGRAY,
    fccDARKGRAY,       /* light colors */
    fccLIGHTBLUE,
    fccLIGHTGREEN,
    fccLIGHTCYAN,
    fccLIGHTRED,
    fccLIGHTMAGENTA,
    fccYELLOW,
    fccWHITE
};

#define FAR_COLOR( foreground,background )   ((((background)&0x0F) << 4) | ((foreground)&0x0F))
#define FAR_COLOR_BK( color )                (((color)>>4)&0x0F)
#define FAR_COLOR_FORE( color )              ((color)&0x0F)

extern int DECLSPEC FP_Color( int far_color_num );

// --------------------------------------------------------------
#if !defined(__FP_NOT_FUNCTIONS__)
  extern PPluginStartupInfo          FP_Info;     ///< Pointer to FAR plugin comunication info structure.
  extern PFarStandardFunctions       FP_FSF;
  extern HMODULE                     FP_HModule;
  extern char                       *FP_PluginRootKey;
  extern BOOL                        FP_IsOldFar;
  extern OSVERSIONINFO              *FP_WinVer;
  extern DWORD                       FP_WinVerDW;
  extern int                         FP_LastOpMode;
  extern char                       *FP_PluginStartPath;
#endif
/** @brief Return name of plugin DLL

    Plugin must define this function.
*/
#if !defined(__FP_NOT_FUNCTIONS__)
  extern CONSTSTR DECLSPEC FP_GetPluginName( void );
#endif

/** @defgroup DefDialog Dialog description
    @{
    Usefull tools for define and set arrays of FAR dialog items.
*/
  #define FFDI_FOCUSED   0x0100UL
  #define FFDI_SELECTED  0x0200UL
  #define FFDI_DEFAULT   0x0400UL
  #define FFDI_GRAYED    0x0800UL
  #define FFDI_MASKED    0x1000UL
  #define FFDI_HISTORY   0x2000UL

  #define FFDI_MASK      0xFFUL
  #define FFDI_NONE      0xFFFFUL

  #define        FDI_CONTROL( tp,x,y,x1,y1,fl,txt )    { tp,x,y,x1,y1,fl,txt },
  #define          FDI_LABEL( x,y,txt )                FDI_CONTROL( DI_TEXT,x,y,0,0,0,txt )
  #define        FDI_LABELFL( x,y,fl,txt )             FDI_CONTROL( DI_TEXT,x,y,0,0,fl,txt )
  #define          FDI_HLINE( x,y )                    FDI_CONTROL( DI_TEXT,x,y,x,y,DIF_BOXCOLOR|DIF_SEPARATOR,NULL )
  #define         FDI_HSLINE( x,y,txt )                FDI_CONTROL( DI_TEXT,x,y,x,y,DIF_BOXCOLOR|DIF_SEPARATOR,txt )
  #define           FDI_SBOX( x,y,x1,y1,txt )          FDI_CONTROL( DI_SINGLEBOX,x,y,x1,y1,DIF_BOXCOLOR|DIF_LEFTTEXT,txt )
  #define           FDI_DBOX( x,y,x1,y1,txt )          FDI_CONTROL( DI_DOUBLEBOX,x,y,x1,y1,DIF_BOXCOLOR|DIF_LEFTTEXT,txt )
  #define        FDI_SBORDER( x,y,x1,y1,txt )          FDI_CONTROL( DI_SINGLEBOX,x,y,x1,y1,DIF_BOXCOLOR,txt )
  #define        FDI_DBORDER( x,y,x1,y1,txt )          FDI_CONTROL( DI_DOUBLEBOX,x,y,x1,y1,DIF_BOXCOLOR,txt )
  #define     FDI_COLORLABEL( x,y,clr,txt )            FDI_CONTROL( DI_TEXT,x,y,0,0,DIF_SETCOLOR|(clr),txt )
  #define           FDI_EDIT( x,y,x1 )                 FDI_CONTROL( DI_EDIT,x,y,x1,y,0,NULL )
  #define         FDI_EDITOR( x,y,x1 )                 FDI_CONTROL( DI_EDIT,x,y,x1,y,DIF_EDITOR,NULL )
  #define       FDI_HISTEDIT( x,y,x1,hist )            FDI_CONTROL( DI_EDIT|FFDI_HISTORY,x,y,x1,y,0,hist )
  #define        FDI_PSWEDIT( x,y,x1 )                 FDI_CONTROL( DI_PSWEDIT,x,y,x1,y,0,NULL )
  #define        FDI_FIXEDIT( x,y,x1 )                 FDI_CONTROL( DI_FIXEDIT,x,y,x1,y,0,NULL )
  #define       FDI_MASKEDIT( x,y,x1,msk )             FDI_CONTROL( DI_FIXEDIT|FFDI_MASKED,x,y,x1,y,DIF_MASKEDIT,msk )
  #define   FDI_DISABLEDEDIT( x,y,x1 )                 FDI_CONTROL( DI_EDIT,x,y,x1,y,DIF_DISABLE,NULL )
  #define          FDI_CHECK( x,y,txt )                FDI_CONTROL( DI_CHECKBOX,x,y,0,0,0,txt )
  #define  FDI_DISABLEDCHECK( x,y,txt )                FDI_CONTROL( DI_CHECKBOX|FFDI_GRAYED,x,y,0,0,0,txt )
  #define          FDI_RADIO( x,y,txt )                FDI_CONTROL( DI_RADIOBUTTON,x,y,0,0,0,txt )
  #define  FDI_DISABLEDRADIO( x,y,txt )                FDI_CONTROL( DI_RADIOBUTTON|FFDI_GRAYED,x,y,0,0,0,txt )
  #define     FDI_STARTRADIO( x,y,txt )                FDI_CONTROL( DI_RADIOBUTTON,x,y,0,0,DIF_GROUP,txt )
  #define         FDI_BUTTON( x,y,txt )                FDI_CONTROL( DI_BUTTON,x,y,0,0,0,txt )
  #define FDI_DISABLEDBUTTON( x,y,txt )                FDI_CONTROL( DI_BUTTON|FFDI_GRAYED,x,y,0,0,0,txt )
  #define      FDI_DEFBUTTON( x,y,txt )                FDI_CONTROL( DI_BUTTON|FFDI_DEFAULT,x,y,0,0,0,txt )
  #define         FDI_GBUTTON( x,y,txt )               FDI_CONTROL( DI_BUTTON,x,y,0,0,DIF_CENTERGROUP,txt )
  #define FDI_GDISABLEDBUTTON( x,y,txt )               FDI_CONTROL( DI_BUTTON|FFDI_GRAYED,x,y,0,0,DIF_CENTERGROUP,txt )
  #define      FDI_GDEFBUTTON( x,y,txt )               FDI_CONTROL( DI_BUTTON|FFDI_DEFAULT,x,y,0,0,DIF_CENTERGROUP,txt )

  #define FP_DEF_DIALOG(nm,sz)  FP_DialogItem nm[sz+1];
  #define FP_DECL_DIALOG(nm)    FP_DialogItem nm[]={
  #define FP_DIALOG_SIZE(nm)    (sizeof(nm)/sizeof(nm[0])-1)
  #define FP_END_DIALOG         {FFDI_NONE,0,0,0,0,0,NULL} };

// ------------------------------------------------------------------------
#include <FARStdlib/fstd_String.h>

// ------------------------------------------------------------------------
/** @brief
    [fstd_Dlg.cpp]
*/
  STRUCT( FP_DialogItem )
    WORD     Type;
    short    X1,Y1,X2,Y2;
    DWORD    Flags;
    CONSTSTR Text;
  };

#if !defined(__FP_NOT_FUNCTIONS__)
  extern void DECLSPEC FP_InitDialogItems( const FP_DialogItem *Init,FarDialogItem *Items );
  extern void DECLSPEC FP_InitDialogItem( const FP_DialogItem *Init,FarDialogItem *Items );
#endif
/**@}*/

  extern int DECLSPEC FP_ShowMsg( CONSTSTR Text, UINT Flags = FMSG_MB_OK, CONSTSTR Help = NULL );
  extern int DECLSPEC FP_ShowDialog( int w, int h,PFarDialogItem itms,int cn, CONSTSTR Help = NULL );
  extern int DECLSPEC FP_Message( unsigned int Flags,CONSTSTR HelpTopic,CONSTSTR *Items,int ItemsNumber,int ButtonsNumber, LPBOOL Delayed = NULL );

// --------------------------------------------------------------
/** @defgroup GetMsg Language messages
    @{

    [fstd_Msg.cpp]
    Wrapers for FAR language API.
*/
#define FMSG( v )       ((CONSTSTR)(INT_PTR)(v))
#define FISMSG(v)      ((v) != NULL && (DWORD_PTR)(v) > FAR_MAX_LANGID)
#define FGETID( v )    Abs((int)LO_WORD((DWORD)(DWORD_PTR)(v)))

#if !defined(__FP_NOT_FUNCTIONS__)
typedef CONSTSTR (DECLSPEC *FP_GetMsgINT_t)( int MsgId );
typedef CONSTSTR (DECLSPEC *FP_GetMsgSTR_t)( CONSTSTR Msg );

extern FP_GetMsgINT_t FP_GetMsgINT;
extern FP_GetMsgSTR_t FP_GetMsgSTR;

inline CONSTSTR FP_GetMsg( int MsgId )   { return FP_GetMsgINT(MsgId); }
inline CONSTSTR FP_GetMsg( CONSTSTR Msg) { return FP_GetMsgSTR(Msg); }
#endif
/**@}*/

// --------------------------------------------------------------
/** @defgroup FarRegXX Regestry manipulations.
    @{

    [fstd_RegXX.cpp]
    Wrapers for regestry Win API.
*/
#if !defined(__FP_NOT_FUNCTIONS__)
  extern int   DECLSPEC FP_GetRegKey(CONSTSTR Key,CONSTSTR ValueName,DWORD Default);
  extern BYTE *DECLSPEC FP_GetRegKey(CONSTSTR Key,CONSTSTR ValueName,BYTE *ValueData,LPCBYTE Default,DWORD DataMaxSize);
  extern char *DECLSPEC FP_GetRegKey(CONSTSTR Key,CONSTSTR ValueName,char *ValueData,CONSTSTR Default,DWORD DataMaxSize);
  inline int   DECLSPEC FP_GetRegKey(CONSTSTR ValueName,DWORD Default)                                                { return FP_GetRegKey(NULL,ValueName,Default); }
  inline char *DECLSPEC FP_GetRegKey(CONSTSTR ValueName,char *ValueData,CONSTSTR Default,DWORD DataSize)              { return FP_GetRegKey(NULL,ValueName,ValueData,Default,DataSize); }
  inline BYTE *DECLSPEC FP_GetRegKey(CONSTSTR ValueName,BYTE *ValueData,LPCBYTE Default,DWORD DataSize)               { return FP_GetRegKey(NULL,ValueName,ValueData,Default,DataSize); }

  extern BOOL  DECLSPEC FP_SetRegKey(CONSTSTR Key,CONSTSTR ValueName,DWORD ValueData);
  extern BOOL  DECLSPEC FP_SetRegKey(CONSTSTR Key,CONSTSTR ValueName,LPCBYTE ValueData,DWORD ValueSize);
  extern BOOL  DECLSPEC FP_SetRegKey(CONSTSTR Key,CONSTSTR ValueName,CONSTSTR ValueData);
  inline BOOL  DECLSPEC FP_SetRegKey(CONSTSTR ValueName,DWORD ValueData)                               { return FP_SetRegKey(NULL,ValueName,ValueData); }
  inline BOOL  DECLSPEC FP_SetRegKey(CONSTSTR ValueName,CONSTSTR ValueData)                            { return FP_SetRegKey(NULL,ValueName,ValueData); }
  inline BOOL  DECLSPEC FP_SetRegKey(CONSTSTR ValueName,LPCBYTE ValueData,DWORD ValueSize)             { return FP_SetRegKey(NULL,ValueName,ValueData,ValueSize); }

  extern HKEY  DECLSPEC FP_CreateRegKey( CONSTSTR Key );
  extern HKEY  DECLSPEC FP_OpenRegKey(   CONSTSTR Key );
  extern BOOL  DECLSPEC FP_DeleteRegKey( CONSTSTR Key );
  extern BOOL  DECLSPEC FP_CheckRegKey(  CONSTSTR Key );
  extern BOOL  DECLSPEC FP_DeleteRegKeyFull( CONSTSTR Key );   //!!Do not uses FP_PluginRootKey - absolute path from HKCU
  extern BOOL  DECLSPEC FP_CheckRegKeyFull(  CONSTSTR Key );   //!!Do not uses FP_PluginRootKey - absolute path from HKCU

  extern BOOL  DECLSPEC FP_CopyRegKeyAll( HKEY TargetBase, CONSTSTR TargetSubkeyName,
                                             HKEY SrcBase,    CONSTSTR SrcSubkeyName );
  extern BOOL  DECLSPEC FP_RenameRegKeyAll( HKEY hParentKey, CONSTSTR Dest, CONSTSTR Src );
  extern BOOL  DECLSPEC FP_DeleteRegKeyAll( HKEY BaseKey, CONSTSTR SubKeyName );
  extern BOOL  DECLSPEC FP_DeleteRegKeyAll( CONSTSTR hParentKey,CONSTSTR Key );               // HKCU + hParentKey + Key
  extern BOOL  DECLSPEC FP_DeleteRegKeyAll( CONSTSTR Key );                                   // HKCU + PluginKey + Key

  extern HANDLE DECLSPEC FP_PushKey( CONSTSTR Subkey );
  extern void   DECLSPEC FP_PopKey( HANDLE PushedKey );
#endif
/**@}*/


// --------------------------------------------------------------
/** @defgroup Clipboard Clipboard operations.
    @{

    [fstd_ClpS.cpp]
    Wrapers for clipboard Win API.
*/
#if !defined(__FP_NOT_FUNCTIONS__)
  extern BOOL DECLSPEC FP_CopyToClipboard( LPVOID Data, SIZE_T DataSize );
  extern BOOL DECLSPEC FP_GetFromClipboard( LPVOID& Data, SIZE_T& DataSize );  //The calles should call `free()` to recvd data
#endif
/**@}*/

// --------------------------------------------------------------
/** @defgroup Other Other utilities functions.
    @{

*/
#if !defined(__FP_NOT_FUNCTIONS__)
  extern int DECLSPEC FP_ConWidth( void );
  extern int DECLSPEC FP_ConHeight( void );
#endif
/**@}*/

// --------------------------------------------------------------
/*  - []
    isFARWin9x, isFARWinNT
      Returns TRUE if plugin executed under expected
      operation system.

    - [std_Patt.cpp]
    FP_InPattern

    - [f_ChEsc.cpp]
    Returns `key index+1` if one of keys in zero-terminated array, or ESC if array not set,
    pressed. Return 0 if no key pressed.

    - []
    Debug functions and helpers

    -# GetDMStr Returns DM_x and DN_x text name

    @def CHK_INITED
      Assertly checks if FSTD initialized.

    @def IS_SILENT( <OpMode> )
       Checks if OpMode is `silent` or interactive mode.

    @def SET_SILENT( <OpMode> )
       Sets "silent" bits to OpMode.
*/
#if !defined(__FP_NOT_FUNCTIONS__)
  extern void     DECLSPEC FP_SetStartupInfo( const PluginStartupInfo *Info,const char *KeyName );
  extern BOOL     DECLSPEC FP_PluginStartup( DWORD Reason );
  extern CONSTSTR DECLSPEC FP_GetPluginLogName( void );

  inline BOOL     isFARWin9x( void ) { return FP_WinVer->dwPlatformId != VER_PLATFORM_WIN32_NT; }
  inline BOOL     isFARWinNT( void ) { return FP_WinVerDW < 0x80000000UL; }

  extern BOOL     DECLSPEC FP_InPattern( CONSTSTR ListOfPatterns,CONSTSTR NameToFitInPattern );
  extern int      DECLSPEC FP_CheckKeyPressed( int *keys = NULL );
  extern CONSTSTR DECLSPEC GetDMStr( int Msg );

  #define OPM_NODIALOG 0x1000
  #define OPM_USER     0x2000

  #define CHK_INITED                 Assert( FP_Info && FP_Info->StructSize );
  #define IS_SILENT(v)               ( ((v) & (OPM_FIND|OPM_VIEW|OPM_EDIT)) != 0 )
  #define SET_SILENT(v)              (v) |= OPM_FIND|OPM_VIEW|OPM_EDIT
#endif

// ------------------------------------------------------------------------
/** @brief
    []
*/
#if !defined(__FP_NOT_FUNCTIONS__)
CLASS( FP_Screen )
  public:
    FP_Screen( void ) { Save(); }
    ~FP_Screen()      { Restore(); }

    static void DECLSPEC Save( void );                 //Save console screen, inc save counter
    static void DECLSPEC Restore( void );              //Dec save counter, Restore console screen on zero
    static void DECLSPEC RestoreWithoutNotes( void );  //Restore screen without counter changes
    static void DECLSPEC FullRestore( void );          //Dec counter to zero and restore screen
    static int  DECLSPEC isSaved( void );              //Save counter value
};
#endif
// ------------------------------------------------------------------------
/** @brief
    []
*/
#if !defined(__FP_NOT_FUNCTIONS__)
CLASS( FPOpMode )
    int OpMode;
  public:
    FPOpMode( int mode ) { OpMode = FP_LastOpMode; FP_LastOpMode = mode; }
    ~FPOpMode()          { FP_LastOpMode = OpMode; }
};
#endif
// ------------------------------------------------------------------------
/** @brief
    []
*/
STRUCTBASE( SRect, public SMALL_RECT )
  int Width( void )                     const { return Right-Left; }
  int Height( void )                    const { return Bottom-Top; }
  int X( void )                         const { return Left; }
  int Y( void )                         const { return Top; }
  int X1( void )                        const { return Right; }
  int Y1( void )                        const { return Bottom; }

  void Set( int x,int y,int x1,int y1 )       { Left = x; Top = y; Right = x1; Bottom = y1; }
  void Set( int x,int y )                     { Left = x; Top = y; Right = x; Bottom = y; }

  bool isEmpty( void )                  const { return Right-Left == 0 && Bottom-Top == 0; }
  bool isNull( void )                   const { return Right == 0 && Left == 0 && Bottom == 0 && Top == 0; }

  void Empty( void )                          { Left = Top = Right = Bottom = 0; }
  void Normalize( void )                      { if (Top>Bottom)                { Swap(Bottom,Top); Swap(Left,Right); }
                                                if (Top==Bottom && Left>Right) Swap(Left,Right); }

  operator SMALL_RECT*()                      { return (SMALL_RECT*)this; }
};

// ------------------------------------------------------------------------
/** @brief
    []
*/
STRUCT( SaveConsoleTitle )
    char      SaveTitle[FAR_MAX_TITLE];
    BOOL      NeedToRestore;
    int       Usage;

  public:
    TIME_TYPE LastChange;

  public:
    SaveConsoleTitle( BOOL NeedToRestore = TRUE );
    ~SaveConsoleTitle();

    static void Text( CONSTSTR buff );

    void          Set( CONSTSTR Buff );
    void          Using( void );
    void          Restore( void );
    CMP_TIME_TYPE Changed( void );
};

// ------------------------------------------------------------------------
/** @brief
    []
*/
STRUCT( SaveLastError )
    DWORD Error;

  public:
    SaveLastError( void ) { Error = GetLastError(); }
    ~SaveLastError()      { SetLastError( Error ); }
};

extern pchar    DECLSPEC StrFromOEMDup( CONSTSTR str,int num = 0 /*0|1*/ );
extern pchar    DECLSPEC StrToOEMDup( CONSTSTR str,int num = 0 /*0|1*/  );
extern pchar    DECLSPEC StrFromOEM( pchar str,int sz /*=-1*/ );
extern pchar    DECLSPEC StrToOEM( pchar str,int num /*0|1*/ );

//FUtils
  extern int DECLSPEC CheckForKeyPressed( WORD *Codes,int NumCodes );

#endif
