

struct MpManagerPluginInfo {
    // Version 1.0
    int Size;
    char* PluginName;
    char* PluginAuthor;
    char* PluginURL;
    char* PluginEmail;
    char* PluginFullPath;
    int VersionMajor;
    int VersionMinor;
    int InterfaceMajor;
    int InterfaceMinor;
    BOOL Supported;
};

struct MpPluginInfo {
    // Version 1.0
    int Size;
    char* Name;
    char* Author;
    char* URL;
    char* Email;
    char* Major;
    char* Minor;
};

struct MpFormatInfo {
    // Version 1.0
    int Size;
    char* FileMask;
    char* GameName;
    __int64 Flags;
};

enum MpSupportFlags {
	SUPPORTFLAG_CREATE             = 0x0000000000000001,
	SUPPORTFLAG_IMPORT             = 0x0000000000000002,
	SUPPORTFLAG_EXPORT             = 0x0000000000000004,
	SUPPORTFLAG_DELETE             = 0x0000000000000008,
	SUPPORTFLAG_REPLACE            = 0x0000000000000010,
	SUPPORTFLAG_BYINDEX            = 0x0000000000000020,
	SUPPORTFLAG_BYNAME             = 0x0000000000000040,
	SUPPORTFLAG_HANDLEISTREAM      = 0x0000000000000080,
	SUPPORTFLAG_HANDLEFILE         = 0x0000000000000100,
	SUPPORTFLAG_TESTARCHIVE        = 0x0000000000000200,
	SUPPORTFLAG_EXPORTNAMEWILD     = 0x0000000000000400,
	SUPPORTFLAG_IMPORTNAMEWILD     = 0x0000000000000800,
};

enum MpOptionType {
	OPTIONTYPE_CREATE              = 0x00000001,
	OPTIONTYPE_EXPORT              = 0x00000002,
	OPTIONTYPE_IMPORT              = 0x00000003,
	OPTIONTYPE_FILEINFO            = 0x00000004,
	OPTIONTYPE_INVALIDFILENAMECHAR = 0x00000005,

	OPENFLAG_CREATENEW             = 0x00000001,
	OPENFLAG_OPENALWAYS            = 0x00000002,
	OPENFLAG_FOREXPORT             = 0x00000004,
	OPENFLAG_FORIMPORT             = 0x00000008,
};

enum MpError {
	pERROR_OK                      = 0x00000000,
	pERROR_INVALID_PARM_1          = 0x00000001,
	pERROR_INVALID_PARM_2          = 0x00000002,
	pERROR_INVALID_PARM_3          = 0x00000003,
	pERROR_INVALID_PARM_4          = 0x00000004,
	pERROR_INVALID_PARM_5          = 0x00000005,
	pERROR_INVALID_PARM_6          = 0x00000006,
	pERROR_INVALID_PARM_7          = 0x00000007,
	pERROR_INVALID_PARM_8          = 0x00000008,
	pERROR_INVALID_PARM_9          = 0x00000009,
	pERROR_FILE_NOT_EXISTS         = 0x0000000A,
	pERROR_FILE_CANT_OPEN          = 0x0000000B,
	pERROR_INVALID_FORMAT          = 0x0000000C,
	pERROR_STREAM_READ             = 0x0000000D,
	pERROR_INVALID_HANDLE          = 0x0000000E,
	pERROR_INVALID_INDEX           = 0x0000000F,
	pERROR_CREATE_ERROR            = 0x00000010,
	pERROR_ARCHIVE_READ_ERROR      = 0x00000011,
	pERROR_NO_MATCHES              = 0x00000012,
	pERROR_ARCHIVE_CLOSED          = 0x00000013,
	pERROR_INVALID_OPTION          = 0x00000014,
	pERROR_WILDCARDS_NOT_ALLOWED   = 0x00000015,
	pERROR_INVALID_ARCHIVE         = 0x00000016,
	pERROR_FILE_EXISTS             = 0x00000017,
	pERROR_UNSUPPORTED             = 0x00000018,

	pERROR_SPECIFICPLUGIN          = 0x80000000
};

//  InterfaceVersionMajor          = 1;
//  InterfaceVersionMinor          = 0;


typedef BOOL (__stdcall *MPGETINTERFACEVERSION)(DWORD& Major, DWORD &Minor);
typedef BOOL (__stdcall *MPGETPLUGININFO)(MpPluginInfo& Info);
typedef DWORD (__stdcall *MPGETFORMATCOUNT)();
typedef BOOL (__stdcall *MPGETFORMATINFO)(int FormatIndex, MpFormatInfo& FormatInfo);
typedef char* (__stdcall *MPGETOPTIONS)(int FormatIndex, int OptionType);
typedef BOOL (__stdcall *MPSETOPTION)(int FormatIndex, int OptionType, const char* Name, const char* Value);
typedef BOOL (__stdcall *MPOPENARCHIVE)(int& ArchiveHandle, int FormatIndex, const char* ArchiveName, DWORD Flags); 
typedef BOOL (__stdcall *MPOPENARCHIVEBINDSTREAM)(int& ArchiveHandle, int FormatIndex, IStream* Stream, DWORD Flags); 
typedef BOOL (__stdcall *MPCLOSEARCHIVE)(int ArchiveHandle); 
typedef int (__stdcall *MPINDEXCOUNT)(int ArchiveHandle); 
typedef const char* (__stdcall *MPINDEXEDINFO)(int ArchiveHandle, int Index, char* Item); 
typedef const char* (__stdcall *MPFINDINFO)(int Handle, const char* Field); 
typedef int (__stdcall *MPFINDFIRSTFILE)(int ArchiveHandle, const char* FileMask); 
typedef BOOL (__stdcall *MPFINDNEXTFILE)(int FindHandle); 
typedef BOOL (__stdcall *MPFINDCLOSE)(int FindHandle); 
typedef BOOL (__stdcall *MPISFILEANARCHIVE)(int FormatIndex, const char* Filename); 
typedef BOOL (__stdcall *MPISSTREAMANARCHIVE)(int FormatIndex, IStream* Stream); 
typedef BOOL (__stdcall *MPEXPORTFILEBYNAMETOFILE)(int ArchiveHandle, const char* ArchiveFile, const char* ExternalFile); 
typedef BOOL (__stdcall *MPEXPORTFILEBYINDEXTOFILE)(int ArchiveHandle, int FileIndex, const char* ExternalFile); 
typedef BOOL (__stdcall *MPEXPORTFILEBYNAMETOSTREAM)(int ArchiveHandle, const char* ArchiveFile, IStream* Stream); 
typedef BOOL (__stdcall *MPEXPORTFILEBYINDEXTOSTREAM)(int ArchiveHandle, int FileIndex, IStream* Stream); 
typedef BOOL (__stdcall *MPIMPORTFILEFROMFILE)(int ArchiveHandle, const char* ArchiveFile, const char* ExternalFile); 
typedef BOOL (__stdcall *MPIMPORTFILEFROMSTREAM)(int ArchiveHandle, const char* ArchiveFile, IStream* Stream); 
typedef BOOL (__stdcall *MPREMOVEFILEBYNAME)(int ArchiveHandle, const char* Filename); 
typedef BOOL (__stdcall *MPREMOVEFILEBYINDEX)(int ArchiveHandle, int FileIndex); 
typedef DWORD (__stdcall *MPGETLASTERROR)();
typedef char* (__stdcall *MPGETERRORTEXT)(DWORD Value);
