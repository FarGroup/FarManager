#ifndef __FAREXCPT_HPP__
#define __FAREXCPT_HPP__
/*
exception.cpp

��� ��� ����������

*/

/* Revision: 1.09 13.01.2003 $ */

/*
Modify:
  13.01.2003 SVS
    ! ������� � 1251 �������� ��������� ������ (���� ��������)
  04.11.2002 SVS
    + ����� EX_PLUGINITEMWORKFLAGS, EX_PLUGINITEMCALLFUNCFLAGS
    ! ���� PLUGINRECORD.Next ������� ���� ��� PLUGINRECORD.Reserved1
  19.02.2002 SVS
    ! ��� ������ ����� :-)
  25.01.2002 SVS
    ! �������������, ���� ��� :-((
    ! FAULTCODERECORD.Code = 128 ����
    ! ������ pXXXXXX �������� ������ ����� (FuncFlags)
    ! ���� ������ ��������� ;-)
  22.01.2002 SVS
    ! ��������� � PLUGINRECORD
  11.07.2001 SVS
    + FARAREARECORD.ScrWH - ������� ������ - ������, ������
  16.05.2001 SVS
    ! ��������� ���������������� ������� EVENTPROC � ��������� WriteEvent
    + PLUGINSINFORECORD
  16.05.2001 SVS
    ! Created
*/

#include "plugins.hpp"

#if defined(__BORLANDC__)
  #pragma option -a2
#elif defined(__GNUC__) || (defined(__WATCOMC__) && (__WATCOMC__ < 1100)) || defined(__LCC__)
  #pragma pack(2)
#else
  #pragma pack(push,2)
#endif

#define FAR_LOG_VERSION  1

#ifndef MAKEFOURCC
#define MAKEFOURCC(ch0, ch1, ch2, ch3)                  \
      ((DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1) << 8) |   \
      ((DWORD)(BYTE)(ch2) << 16) | ((DWORD)(BYTE)(ch3) << 24 ))
#endif

#define FLOG_SYSINFO     0x00000001 // ���������� � �������
#define FLOG_EXCEPTION   0x00000002 // ��� ����������
#define FLOG_PLUGIN      0x00000004 // ���������� � �������
#define FLOG_FARAREA     0x00000008 // "��� �� ������ ���������?"
#define FLOG_MACRO       0x00000010 // �������
#define FLOG_RAWDARA     0x00000020 // ������������ ������
#define FLOG_PLUGINSINFO 0x80000000 // ���������� � ��������
#define FLOG_ALL         0xFFFFFFFF

enum FARRECORDTYPE{
  RTYPE_SYSINFO      =MAKEFOURCC('S','Y','S','T'),// ���������� � �������
  RTYPE_EXCEPTION    =MAKEFOURCC('E','X','C','T'),// ��� ����������
  RTYPE_PLUGIN       =MAKEFOURCC('C','P','L','G'),// ���������� � ������� �������
  RTYPE_FARAREA      =MAKEFOURCC('A','R','E','A'),// "��� �� ������ ���������?"
  RTYPE_MACRO        =MAKEFOURCC('M','A','C','R'),// �������
  RTYPE_RAWDARA      =MAKEFOURCC('R','A','W','D'),// ������������ ������
};

struct RECHEADER{         // ��������� �������
  DWORD TypeRec;          // ��� ������
  DWORD SizeRec;          // ������ ���������
  struct RECHEADER *Next; // ��������� ������� � ������
  // Data                 // ������ �������� SizeRec
};

struct SYSINFOHEADER{     // ���������� � �������
  DWORD TypeRec;          // ��� ������ = RTYPE_SYSINFO
  DWORD SizeRec;          // ������ ������ = sizeof(struct DUMPHEADER)-sizeof(WORD)*2
  struct RECHEADER *Next; // ��������� ������� � ������
  DWORD DumpFlags;        // �������������� ����� (���� =0)
  DWORD FARVersion;       // ������ FAR Manager � ������� FAR_VERSION
  SYSTEMTIME DumpTime;    // the system time is expressed in Coordinated Universal Time (UTC))
  OSVERSIONINFO WinVer;   // ������ ������
};

// ����� ��� ���� PluginItem.WorkFlags
enum EX_PLUGINITEMWORKFLAGS{
  EXPIWF_CACHED        = 0x00000001, // ����������
  EXPIWF_PRELOADED     = 0x00000002, //
  EXPIWF_DONTLOADAGAIN = 0x00000004, // �� ��������� ������ �����, �������� �
                                   //   ���������� �������� ��������� ������ ����
};

// ����� ��� ���� PluginItem.FuncFlags - ���������� �������
enum EX_PLUGINITEMCALLFUNCFLAGS{
  EXPICFF_LOADED               = 0x00000001, // DLL �������� ;-)
  EXPICFF_SETSTARTUPINFO       = 0x00000002, //
  EXPICFF_OPENPLUGIN           = 0x00000004, //
  EXPICFF_OPENFILEPLUGIN       = 0x00000008, //
  EXPICFF_CLOSEPLUGIN          = 0x00000010, //
  EXPICFF_GETPLUGININFO        = 0x00000020, //
  EXPICFF_GETOPENPLUGININFO    = 0x00000040, //
  EXPICFF_GETFINDDATA          = 0x00000080, //
  EXPICFF_FREEFINDDATA         = 0x00000100, //
  EXPICFF_GETVIRTUALFINDDATA   = 0x00000200, //
  EXPICFF_FREEVIRTUALFINDDATA  = 0x00000400, //
  EXPICFF_SETDIRECTORY         = 0x00000800, //
  EXPICFF_GETFILES             = 0x00001000, //
  EXPICFF_PUTFILES             = 0x00002000, //
  EXPICFF_DELETEFILES          = 0x00004000, //
  EXPICFF_MAKEDIRECTORY        = 0x00008000, //
  EXPICFF_PROCESSHOSTFILE      = 0x00010000, //
  EXPICFF_SETFINDLIST          = 0x00020000, //
  EXPICFF_CONFIGURE            = 0x00040000, //
  EXPICFF_EXITFAR              = 0x00080000, //
  EXPICFF_PROCESSKEY           = 0x00100000, //
  EXPICFF_PROCESSEVENT         = 0x00200000, //
  EXPICFF_PROCESSEDITOREVENT   = 0x00400000, //
  EXPICFF_COMPARE              = 0x00800000, //
  EXPICFF_PROCESSEDITORINPUT   = 0x01000000, //
  EXPICFF_MINFARVERSION        = 0x02000000, //
  EXPICFF_PROCESSVIEWEREVENT   = 0x04000000, //
};

struct PLUGINRECORD{      // ���������� � �������
  DWORD TypeRec;          // ��� ������ = RTYPE_PLUGIN
  DWORD SizeRec;          // ������
  DWORD Reserved1;

  DWORD WorkFlags;        // ������� ����� �������� �������
  DWORD FuncFlags;        // ������� ����� ����.������� ������� (��� ���� - ��� � �������)
  DWORD CallFlags;        // ������� ����� ������ ����.������� �������

  short CachePos;         // ������� � ����
  DWORD SysID;

  struct {
    DWORD    dwFileAttributes;
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
    DWORD    nFileSizeHigh;
    DWORD    nFileSizeLow;
    DWORD    dwReserved0;
    DWORD    dwReserved1;
    char     cFileName[MAX_PATH];
    char     cAlternateFileName[14];
  } FindData;

  DWORD Reserved2[2];    // ������ :-)

  DWORD SizeModuleName;
  //char ModuleName[0];
};

struct EXCEPTIONRECORD{   // ��� ����������
  DWORD TypeRec;          // ��� ������ = RTYPE_EXCEPTION
  DWORD SizeRec;          // ������ ������
  struct RECHEADER *Next; // ��������� ������� � ������

  EXCEPTION_POINTERS *Exception;
};

struct MACRORECORD{       // �������
  DWORD TypeRec;          // ��� ������ = RTYPE_MACRO
  DWORD SizeRec;          // ������
  struct RECHEADER *Next; // ��������� ������� � ������
  WORD  MacroStatus;      // 0 - �� � ������ �����, 1 - Recording, 2 - Executing
  WORD  MacroPos;         // ������� ������� � MacroKeyBuffer
  DWORD MacroFlags;       // ����� - ������� ����� = MACRO_AREA
  DWORD MacroKey;         // ����������� ������������
  DWORD MacroBufferSize;  // ������ ����������������������� MacroKeyBuffer
  // DWORD MacroKeyBuffer[0];// �����-������������������
};

struct FARAREARECORD{     // "��� �� ������ ���������?"
  DWORD TypeRec;          // ��� ������ = RTYPE_FARAREA
  DWORD SizeRec;          // ������ ������
  struct RECHEADER *Next; // ��������� ������� � ������
  DWORD ObjectType;       // ��, ��� ���������� CtrlObject->Cp()->GetType()
  COORD ScrWH;            // ������� ������ - ������, ������
};

enum {
  RAWTYPE_BINARY =0,
  RAWTYPE_TEXT   =1,
};

struct RAWDARARECORD{     // ������������ ������
  DWORD TypeRec;          // ��� ������ = RTYPE_RAWDARA
  DWORD SizeRec;          // ������ ������
  struct RECHEADER *Next; // ��������� ������� � ������
  DWORD RawFlags;         // �������������� ����� ��� ���������������� :-)
  DWORD RawType;          // ��� ������ = RAWTYPE_BINARY, RAWTYPE_TEXT
  DWORD SizeData;         // ������ ������������ ������
  //BYTE Data[0];         // ���� SizeRec=0, �� ����� ���� ����
};


//�������� ��������� ��� �������� ������� � ������
//����� ��������� ������
struct COMBINE_RECORD {
  RECHEADER Header;
  char      Data[ 1 /*SizeRec*/ ];
};
//�������� ��������� ��� ������� � ����������� ������
//����� ������
struct MACRORECORD_t : public MACRORECORD {
  DWORD MacroKeyBuffer[ 1 /*MacroBufferSize*/ ];
};
struct PLUGINRECORD_t : public PLUGINRECORD {
  char ModuleName[ 1 /*SizeModuleName*/ ];
};
struct RAWDARARECORD_t : public RAWDARARECORD {
  LPBYTE RawDataPtr;  //The pointer to allocated raw data can be placed here
};

struct FARExceptionState {
  DWORD               StructSize;
  DWORD               Version;       // ������ "��������"

  //FAR additional error info
  char RecomendedDumpFileName[_MAX_PATH];
  const char         *RootKey;

  //FAR error context
  struct RECHEADER   *Head;
};

#if defined(__BORLANDC__)
  #pragma option -a.
#elif defined(__GNUC__) || (defined(__WATCOMC__) && (__WATCOMC__ < 1100)) || defined(__LCC__)
  #pragma pack()
#else
  #pragma pack(pop)
#endif

/* $ 17.10.2000 SVS
   ����������!
*/
enum ExceptFunctionsType{
  EXCEPT_SETSTARTUPINFO,
  EXCEPT_GETVIRTUALFINDDATA,
  EXCEPT_OPENPLUGIN,
  EXCEPT_OPENFILEPLUGIN,
  EXCEPT_OPENPLUGIN_FINDLIST,
  EXCEPT_CLOSEPLUGIN,
  EXCEPT_GETPLUGININFO,
  EXCEPT_GETPLUGININFO_DATA,
  EXCEPT_GETOPENPLUGININFO,
  EXCEPT_GETOPENPLUGININFO_DATA,
  EXCEPT_GETFINDDATA,
  EXCEPT_FREEFINDDATA,
  EXCEPT_FREEVIRTUALFINDDATA,
  EXCEPT_SETDIRECTORY,
  EXCEPT_GETFILES,
  EXCEPT_PUTFILES,
  EXCEPT_DELETEFILES,
  EXCEPT_MAKEDIRECTORY,
  EXCEPT_PROCESSHOSTFILE,
  EXCEPT_SETFINDLIST,
  EXCEPT_CONFIGURE,
  EXCEPT_EXITFAR,
  EXCEPT_PROCESSKEY,
  EXCEPT_PROCESSEVENT,
  EXCEPT_PROCESSEDITOREVENT,
  EXCEPT_COMPARE,
  EXCEPT_PROCESSEDITORINPUT,
  EXCEPT_MINFARVERSION,
  EXCEPT_PROCESSVIEWEREVENT,
  EXCEPT_PROCESSVIEWERINPUT,
  EXCEPT_FARDIALOG,
  EXCEPT_FAREDITOR,
};

typedef BOOL (WINAPI *FARPROCESSEVENT)(struct FARExceptionState * Context);

int WriteEvent(DWORD DumpType, // FLOG_*
               EXCEPTION_POINTERS *xp=NULL,
               struct PluginItem *Module=NULL,
               void *RawData=NULL,DWORD RawDataSize=0,
               DWORD RawDataFlags=0,DWORD RawType=RAWTYPE_BINARY);

DWORD WINAPI xfilter(
    int From,                 // ������: 0 = OpenPlugin, 1 = OpenFilePlugin
    EXCEPTION_POINTERS *xp,   // ������ ��������
    struct PluginItem *Module,// ������, ��������� � ����������.
    DWORD Flags);             // �������������� ����� - ���� ������ ����
                              //        0x1 - ���������� ��� ��������?

#endif // __FAREXCPT_HPP__
