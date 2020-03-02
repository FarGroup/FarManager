#ifndef __FAR_PLUGIN_UNIPLACE_HEADER
#define __FAR_PLUGIN_UNIPLACE_HEADER

//-   FAR `plugin.hpp` header missed\addons
#define FAR_MAX_LANGID                       2000
#define FAR_MAX_CMDLINE                      1024
#define FAR_MAX_TITLE                        512
#define FAR_MAX_NAME                         256
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
#define FAR_SHADOW_CHAR                      '\xB0' //░
#define FAR_FULL_CHAR                        '\xDB' //█
#define FAR_VERT_CHAR                        '\xB3' //│
#define FAR_DVERT_CHAR                       '\xBA' //║
#define FAR_HORZ_CHAR                        '\xC4' //─
#define FAR_DHORZ_CHAR                       '\xCD' //═
#define FAR_CHECK_CHAR                       '\xFb' //√
#define FAR_SBMENU_CHAR                      '\x10' //
#define FAR_LEFT_CHAR                        '\x11' //
#define FAR_RIGHT_CHAR                       FAR_SBMENU_CHAR
#define FAR_SPACE_CHAR                       '\xFA' //·
#define FAR_TAB_CHAR                         '\xFE' //■
#define FAR_DOWN_CHAR                        '\x19' //
#define FAR_SKIP_CHAR                        '\x20' //' '

#define FAR_SHADOW_STR                       "\xB0" //░
#define FAR_FULL_STR                         "\xDB" //█
#define FAR_VERT_STR                         "\xB3" //│
#define FAR_DVERT_STR                        "\xBA" //║
#define FAR_HORZ_STR                         "\xC4" //─
#define FAR_DHORZ_STR                        "\xCD" //═
#define FAR_CHECK_STR                        "\xFb" //√
#define FAR_SBMENU_STR                       "\x10" //
#define FAR_LEFT_STR                         "\x11" //
#define FAR_RIGHT_STR                        FAR_SBMENU_STR
#define FAR_SPACE_STR                        "\xFA" //·
#define FAR_TAB_STR                          "\xFE" //■
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
enum FarConsoleColors
{
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

extern int WINAPI FP_Color(int far_color_num);

// --------------------------------------------------------------
#if !defined(__FP_NOT_FUNCTIONS__)
extern PluginStartupInfo*          FP_Info;     ///< Pointer to FAR plugin comunication info structure.
extern FarStandardFunctions*       FP_FSF;
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

/** @defgroup DefDialog Dialog description
    @{
    Usefull tools for define and set arrays of FAR dialog items.
*/

#define FFDI_MASK      0xFFUL

// ------------------------------------------------------------------------
#include <FARStdlib/fstd_String.h>

// ------------------------------------------------------------------------
extern int WINAPI FP_Message(unsigned int Flags,LPCSTR HelpTopic,LPCSTR *Items,int ItemsNumber,int ButtonsNumber, LPBOOL Delayed = NULL);

// --------------------------------------------------------------
/** @defgroup GetMsg Language messages
    @{

    [fstd_Msg.cpp]
    Wrapers for FAR language API.
*/
#define FMSG( v )       ((LPCSTR)(INT_PTR)(v))
#define FISMSG(v)      ((v) != NULL && (DWORD_PTR)(v) > FAR_MAX_LANGID)
#define FGETID( v )    Abs((int)LO_WORD((DWORD)(DWORD_PTR)(v)))

#if !defined(__FP_NOT_FUNCTIONS__)
typedef LPCSTR(WINAPI *FP_GetMsgINT_t)(int MsgId);
typedef LPCSTR(WINAPI *FP_GetMsgSTR_t)(LPCSTR Msg);

extern FP_GetMsgINT_t FP_GetMsgINT;
extern FP_GetMsgSTR_t FP_GetMsgSTR;

inline LPCSTR FP_GetMsg(int MsgId)   { return FP_GetMsgINT(MsgId); }
inline LPCSTR FP_GetMsg(LPCSTR Msg) { return FP_GetMsgSTR(Msg); }
#endif
/**@}*/

// --------------------------------------------------------------
/** @defgroup FarRegXX Regestry manipulations.
    @{

    [fstd_RegXX.cpp]
    Wrapers for regestry Win API.
*/
#if !defined(__FP_NOT_FUNCTIONS__)
extern int   WINAPI FP_GetRegKey(LPCSTR Key,LPCSTR ValueName,DWORD Default);
extern BYTE *WINAPI FP_GetRegKey(LPCSTR Key,LPCSTR ValueName,BYTE *ValueData,const BYTE * Default,DWORD DataMaxSize);
extern char *WINAPI FP_GetRegKey(LPCSTR Key,LPCSTR ValueName,char *ValueData,LPCSTR Default,DWORD DataMaxSize);
inline int   WINAPI FP_GetRegKey(LPCSTR ValueName,DWORD Default)                                                { return FP_GetRegKey(NULL,ValueName,Default); }
inline char *WINAPI FP_GetRegKey(LPCSTR ValueName,char *ValueData,LPCSTR Default,DWORD DataSize)              { return FP_GetRegKey(NULL,ValueName,ValueData,Default,DataSize); }
inline BYTE *WINAPI FP_GetRegKey(LPCSTR ValueName,BYTE *ValueData,const BYTE * Default,DWORD DataSize)               { return FP_GetRegKey(NULL,ValueName,ValueData,Default,DataSize); }

extern BOOL  WINAPI FP_SetRegKey(LPCSTR Key,LPCSTR ValueName,DWORD ValueData);
extern BOOL  WINAPI FP_SetRegKey(LPCSTR Key,LPCSTR ValueName,const BYTE * ValueData,DWORD ValueSize);
extern BOOL  WINAPI FP_SetRegKey(LPCSTR Key,LPCSTR ValueName,LPCSTR ValueData);
inline BOOL  WINAPI FP_SetRegKey(LPCSTR ValueName,DWORD ValueData)                               { return FP_SetRegKey(NULL,ValueName,ValueData); }
inline BOOL  WINAPI FP_SetRegKey(LPCSTR ValueName,LPCSTR ValueData)                            { return FP_SetRegKey(NULL,ValueName,ValueData); }
inline BOOL  WINAPI FP_SetRegKey(LPCSTR ValueName,const BYTE * ValueData,DWORD ValueSize)             { return FP_SetRegKey(NULL,ValueName,ValueData,ValueSize); }

extern HKEY  WINAPI FP_CreateRegKey(LPCSTR Key);
extern HKEY  WINAPI FP_OpenRegKey(LPCSTR Key);
extern BOOL  WINAPI FP_DeleteRegKey(LPCSTR Key);
extern BOOL  WINAPI FP_CheckRegKey(LPCSTR Key);
extern BOOL  WINAPI FP_DeleteRegKeyFull(LPCSTR Key);     //!!Do not uses FP_PluginRootKey - absolute path from HKCU

extern BOOL  WINAPI FP_DeleteRegKeyAll(HKEY BaseKey, LPCSTR SubKeyName);
extern BOOL  WINAPI FP_DeleteRegKeyAll(LPCSTR hParentKey,LPCSTR Key);                 // HKCU + hParentKey + Key
extern BOOL  WINAPI FP_DeleteRegKeyAll(LPCSTR Key);                                     // HKCU + PluginKey + Key

extern HANDLE WINAPI FP_PushKey(LPCSTR Subkey);
extern void   WINAPI FP_PopKey(HANDLE PushedKey);
#endif
/**@}*/


// --------------------------------------------------------------
/** @defgroup Clipboard Clipboard operations.
    @{

    [fstd_ClpS.cpp]
    Wrapers for clipboard Win API.
*/
#if !defined(__FP_NOT_FUNCTIONS__)
extern BOOL WINAPI FP_CopyToClipboard(LPVOID Data, SIZE_T DataSize);
extern BOOL WINAPI FP_GetFromClipboard(LPVOID& Data, SIZE_T& DataSize);    //The calles should call `free()` to recvd data
#endif
/**@}*/

// --------------------------------------------------------------
/** @defgroup Other Other utilities functions.
    @{

*/
#if !defined(__FP_NOT_FUNCTIONS__)
extern int WINAPI FP_ConWidth(void);
extern int WINAPI FP_ConHeight(void);
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

    @def CHK_INITED
      Assertly checks if FSTD initialized.

    @def IS_SILENT( <OpMode> )
       Checks if OpMode is `silent` or interactive mode.

    @def SET_SILENT( <OpMode> )
       Sets "silent" bits to OpMode.
*/
#if !defined(__FP_NOT_FUNCTIONS__)
extern void     WINAPI FP_SetStartupInfo(const PluginStartupInfo *Info,const char *KeyName);
extern BOOL     WINAPI FP_PluginStartup(DWORD Reason);
extern LPCSTR WINAPI FP_GetPluginLogName(void);

inline BOOL     isFARWin9x(void) { return FP_WinVer->dwPlatformId != VER_PLATFORM_WIN32_NT; }
inline BOOL     isFARWinNT(void) { return FP_WinVerDW < 0x80000000UL; }

extern BOOL     WINAPI FP_InPattern(LPCSTR ListOfPatterns,LPCSTR NameToFitInPattern);
extern int      WINAPI FP_CheckKeyPressed(int *keys = NULL);

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
class FP_Screen
{
	public:
		FP_Screen(void) { Save(); }
		~FP_Screen()      { Restore(); }

		static void WINAPI Save(void);                   //Save console screen, inc save counter
		static void WINAPI Restore(void);                //Dec save counter, Restore console screen on zero
		static void WINAPI RestoreWithoutNotes(void);    //Restore screen without counter changes
		static void WINAPI FullRestore(void);            //Dec counter to zero and restore screen
		static int  WINAPI isSaved(void);                //Save counter value
};
#endif
// ------------------------------------------------------------------------
/** @brief
    []
*/
#if !defined(__FP_NOT_FUNCTIONS__)
class FPOpMode
{
		int OpMode;
	public:
		FPOpMode(int mode) { OpMode = FP_LastOpMode; FP_LastOpMode = mode; }
		~FPOpMode()          { FP_LastOpMode = OpMode; }
};
#endif
// ------------------------------------------------------------------------
/** @brief
    []
*/
struct SRect: public SMALL_RECT
{
	int Width(void)                     const { return Right-Left; }
	int Height(void)                    const { return Bottom-Top; }
	int X(void)                         const { return Left; }
	int Y(void)                         const { return Top; }
	int X1(void)                        const { return Right; }
	int Y1(void)                        const { return Bottom; }

	void Set(int x,int y,int x1,int y1)       { Left = x; Top = y; Right = x1; Bottom = y1; }
	void Set(int x,int y)                     { Left = x; Top = y; Right = x; Bottom = y; }

	bool isEmpty(void)                  const { return Right-Left == 0 && Bottom-Top == 0; }
	bool isNull(void)                   const { return Right == 0 && Left == 0 && Bottom == 0 && Top == 0; }

	void Empty(void)                          { Left = Top = Right = Bottom = 0; }
	void Normalize(void)
	{
		if(Top>Bottom)                { Swap(Bottom,Top); Swap(Left,Right); }

		if(Top==Bottom && Left>Right) Swap(Left,Right);
	}

	operator SMALL_RECT*()                      { return (SMALL_RECT*)this; }
};

// ------------------------------------------------------------------------
/** @brief
    []
*/
struct SaveConsoleTitle
{
		char      SaveTitle[FAR_MAX_TITLE];
		BOOL      NeedToRestore;
		int       Usage;

	public:
		DWORD LastChange;

	public:
		SaveConsoleTitle(BOOL NeedToRestore = TRUE);
		~SaveConsoleTitle();

		static void Text(LPCSTR buff);

		void          Set(LPCSTR Buff);
		void          Using(void);
		void          Restore(void);
		double Changed(void);
};

// ------------------------------------------------------------------------
/** @brief
    []
*/
struct SaveLastError
{
		DWORD Error;

	public:
		SaveLastError(void) { Error = GetLastError(); }
		~SaveLastError()      { SetLastError(Error); }
};

extern LPSTR    WINAPI StrFromOEMDup(LPCSTR str,int num = 0 /*0|1*/);
extern LPSTR    WINAPI StrToOEMDup(LPCSTR str,int num = 0 /*0|1*/);
extern LPSTR    WINAPI StrFromOEM(LPSTR str,int sz /*=-1*/);
extern LPSTR    WINAPI StrToOEM(LPSTR str,int num /*0|1*/);

//FUtils
extern int WINAPI CheckForKeyPressed(WORD *Codes,int NumCodes);

#endif
