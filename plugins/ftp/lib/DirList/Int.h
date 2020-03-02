#ifndef __FTP_PROGRESS_INTERNAL
#define __FTP_PROGRESS_INTERNAL

#include "fstdlib.h"         //FAR plugin stdlib
#include "../Plugin.h"

//------------------------------------------------------------------------
#define NET_TO_UPPER(x) ((((unsigned int) (x)) > 0x7f) ? x : toupper(x))
#define NET_TO_LOWER(x) ((((unsigned int) (x)) > 0x7f) ? x : tolower(x))

#define NET_IS_ALPHA(x) ((((unsigned int) (x)) > 0x7f) ? 0 : isalpha(x))
#define NET_IS_DIGIT(x) ((((unsigned int) (x)) > 0x7f) ? 0 : isdigit(x))
#define NET_IS_SPACE(x) ((((unsigned int) (x)) > 0x7f) ? 0 : isspace(x))

#define NOT_TIME  ((__int64)-1)
typedef const FILETIME *LPCFILETIME;

#define CHECK( v, ret ) if v { Log(( "Parser failed: [" #v "] at " __FILE__ " in %d", __LINE__ )); return ret; }

struct Time_t
{
	union
	{
		FILETIME FileTime;
		__int64    Value;
	};

	bool operator==(__int64 v)      { return Value == v; }
	void operator=(__int64 v)       { Value = v; }
	void operator=(const Time_t& v) { Value = v.Value; }

	operator LPCFILETIME()            { return &FileTime; }
	operator LPFILETIME()             { return &FileTime; }
};

struct NET_FileEntryInfo: public FTPFileInfo
{
	Time_t date;       //Last write time
	Time_t cr_date;    //Time of ceation
	Time_t acc_date;   //Time of last access
	__int64  size;

	NET_FileEntryInfo(void) { memset(this,0,sizeof(*this)); }
};

//------------------------------------------------------------------------
extern char *SkipSpace(char *l);
extern char *SkipDigit(char *l);
extern char *SkipNSpace(char *l);
extern char *SkipNX(char *l, char ch);

/*
 * Get month number (-1 on error).
 */
extern BOOL TwoDigits(LPCSTR s,WORD& val);

/* remove front and back white space
 * modifies the original string
 */
extern char *XP_StripLine(char *string);

/*
 * Get month number by month name in range [1..12] or MAX_WORD on error.
 */
extern WORD NET_MonthNo(LPCSTR month);

/*
 * Converts PNET_FileEntryInfo fields to FTPFileInfo*.
 */
extern BOOL ConvertEntry(NET_FileEntryInfo* inf, FTPFileInfo* p);

/*
 * Checks if line starts with one of specified lines.
 */
BOOL StartsWith(LPCSTR line, LPCSTR *lines);

/*
 * Check if string starts from std unix symbols
 */
BOOL is_unix_start(char *s, int len, int *off = NULL);

//------------------------------------------------------------------------
const char PCTCP_PWD_Title[] = "Current working directory is ";
#define PCTCP_PWD_TITLE_LEN (sizeof(PCTCP_PWD_Title)-1)

extern BOOL net_convert_unix_date(LPSTR& datestr, Time_t& decoded);

extern BOOL WINAPI idPRParceCMS(const FTPServerInfo* Server, FTPFileInfo* p, char *entry, int entry_len);
extern BOOL WINAPI idPRParceDos(const FTPServerInfo* Server, FTPFileInfo* p, char *entry, int entry_len);
extern BOOL WINAPI idPRParceEPLF(const FTPServerInfo* Server, FTPFileInfo* p, char *entry, int entry_len);
extern BOOL WINAPI idPRParceNETWARE(const FTPServerInfo* Server, FTPFileInfo* p, char *entry, int entry_len);
extern BOOL WINAPI idPRParceOS2(const FTPServerInfo* Server, FTPFileInfo* p, char *entry, int entry_len);
extern BOOL WINAPI idPRParcePCTCP(const FTPServerInfo* Server, FTPFileInfo* p, char *entry, int entry_len);
extern BOOL WINAPI idDirParcePCTCP(const FTPServerInfo* Server, LPCSTR Line, char *CurDir, size_t CurDirSize);
extern BOOL WINAPI idPRParceSkirdin(const FTPServerInfo* Server, FTPFileInfo* p, char *entry, int entry_len);
extern BOOL WINAPI idDirParceSkirdin(const FTPServerInfo* Server, LPCSTR Line, char *CurDir, size_t CurDirSize);
extern BOOL WINAPI idPRParceTCPC(const FTPServerInfo* Server, FTPFileInfo* p, char *entry, int entry_len);
extern BOOL WINAPI idPRParceUnix(const FTPServerInfo* Server, FTPFileInfo* p, char *entry, int entry_len);
extern BOOL WINAPI idPRParceVMS(const FTPServerInfo* Server, FTPFileInfo* p, char *entry, int entry_len);
extern BOOL WINAPI idDirPRParceVMS(const FTPServerInfo* Server, LPCSTR Line, char *CurDir, size_t CurDirSize);
extern BOOL WINAPI idPRParceVX_DOS(const FTPServerInfo* Server, FTPFileInfo* p, char *entry, int entry_len);
extern BOOL WINAPI idPRParceOS400(const FTPServerInfo* Server, FTPFileInfo* p, char *entry, int entry_len);
extern BOOL WINAPI idPRParceMVS(const FTPServerInfo* Server, FTPFileInfo* p, char *entry, int entry_len);
extern BOOL WINAPI idDirParceMVS(const FTPServerInfo* Server, LPCSTR Line, char *CurDir, size_t CurDirSize);

#endif
