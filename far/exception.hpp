#ifndef __FAREXCPT_HPP__
#define __FAREXCPT_HPP__
/*
exception.cpp

Все про исключения

*/

/* Revision: 1.06 19.02.2002 $ */

/*
Modify:
  19.02.2002 SVS
    ! ВСЕ СОВСЕМ ИНАЧЕ :-)
  25.01.2002 SVS
    ! Выравниванием, мать его :-((
    ! FAULTCODERECORD.Code = 128 байт
    ! вместо pXXXXXX выставим нужные флаги (FuncFlags)
    ! Куча разных уточнений ;-)
  22.01.2002 SVS
    ! Уточнение в PLUGINRECORD
  11.07.2001 SVS
    + FARAREARECORD.ScrWH - размеры экрана - ширина, высота
  16.05.2001 SVS
    ! Добавлена пользовательская функция EVENTPROC в параметры WriteEvent
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

#define FLOG_SYSINFO     0x00000001 // информация о системе
#define FLOG_EXCEPTION   0x00000002 // про исключение
#define FLOG_PLUGIN      0x00000004 // информация о плагине
#define FLOG_FARAREA     0x00000008 // "где мы сейчас находимся?"
#define FLOG_MACRO       0x00000010 // Макросы
#define FLOG_RAWDARA     0x00000020 // произвольные данные
#define FLOG_PLUGINSINFO 0x80000000 // информация о плагинах
#define FLOG_ALL         0xFFFFFFFF

enum FARRECORDTYPE{
  RTYPE_SYSINFO      =MAKEFOURCC('S','Y','S','T'),// информация о системе
  RTYPE_EXCEPTION    =MAKEFOURCC('E','X','C','T'),// про исключение
  RTYPE_PLUGIN       =MAKEFOURCC('C','P','L','G'),// информация о текущем плагине
  RTYPE_FARAREA      =MAKEFOURCC('A','R','E','A'),// "где мы сейчас находимся?"
  RTYPE_MACRO        =MAKEFOURCC('M','A','C','R'),// Макросы
  RTYPE_RAWDARA      =MAKEFOURCC('R','A','W','D'),// произвольные данные
};

struct RECHEADER{         // заголовок рекорда
  DWORD TypeRec;          // Тип записи
  DWORD SizeRec;          // Размер структуры
  struct RECHEADER *Next; // Следующий элемент в списке
  // Data                 // Данные размером SizeRec
};

struct SYSINFOHEADER{     // информация о системе
  DWORD TypeRec;          // Тип записи = RTYPE_SYSINFO
  DWORD SizeRec;          // Размер данных = sizeof(struct DUMPHEADER)-sizeof(WORD)*2
  struct RECHEADER *Next; // Следующий элемент в списке
  DWORD DumpFlags;        // дополнительные флаги (пока =0)
  DWORD FARVersion;       // версия FAR Manager в формате FAR_VERSION
  SYSTEMTIME DumpTime;    // the system time is expressed in Coordinated Universal Time (UTC))
  OSVERSIONINFO WinVer;   // версия виндов
};

struct PLUGINRECORD{      // информация о плагине
  DWORD TypeRec;          // Тип записи = RTYPE_PLUGIN
  DWORD SizeRec;          // Размер
  struct RECHEADER *Next; // Следующий элемент в списке

  DWORD WorkFlags;      // рабочие флаги текущего плагина
  DWORD FuncFlags;      // битовые маски эксп.функций плагина (бит есть - ест и функция)
  DWORD CallFlags;      // битовые маски вызова эксп.функций плагина

  short CachePos;       // позиция в кеше
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

  DWORD Reserved[2];    // разерв :-)

  DWORD SizeModuleName;
  //char ModuleName[0];
};

struct EXCEPTIONRECORD{   // про исключение
  DWORD TypeRec;          // Тип записи = RTYPE_EXCEPTION
  DWORD SizeRec;          // Размер данных
  struct RECHEADER *Next; // Следующий элемент в списке

  EXCEPTION_POINTERS *Exception;
};

struct MACRORECORD{       // Макросы
  DWORD TypeRec;          // Тип записи = RTYPE_MACRO
  DWORD SizeRec;          // Размер
  struct RECHEADER *Next; // Следующий элемент в списке
  WORD  MacroStatus;      // 0 - не в режиме макро, 1 - Recording, 2 - Executing
  WORD  MacroPos;         // текущая позиция в MacroKeyBuffer
  DWORD MacroFlags;       // флаги - младшее слово = MACRO_AREA
  DWORD MacroKey;         // назначенная макроклавиша
  DWORD MacroBufferSize;  // размер макропоследовательности MacroKeyBuffer
  // DWORD MacroKeyBuffer[0];// макро-последовательность
};

struct FARAREARECORD{     // "где мы сейчас находимся?"
  DWORD TypeRec;          // Тип записи = RTYPE_FARAREA
  DWORD SizeRec;          // Размер данных
  struct RECHEADER *Next; // Следующий элемент в списке
  DWORD ObjectType;       // то, что возвращает CtrlObject->Cp()->GetType()
  COORD ScrWH;            // размеры экрана - ширина, высота
};

enum {
  RAWTYPE_BINARY =0,
  RAWTYPE_TEXT   =1,
};

struct RAWDARARECORD{     // произвольные данные
  DWORD TypeRec;          // Тип записи = RTYPE_RAWDARA
  DWORD SizeRec;          // Размер данных
  struct RECHEADER *Next; // Следующий элемент в списке
  DWORD RawFlags;         // Дополнительные флаги для расширябильности :-)
  DWORD RawType;          // Тип данных = RAWTYPE_BINARY, RAWTYPE_TEXT
  DWORD SizeData;         // Размер произвольных данных
  //BYTE Data[0];         // если SizeRec=0, то этого поля нету
};


//Фейковая структуря для быстрого доступа к данным
//после заголовка пакета
struct COMBINE_RECORD {
  RECHEADER Header;
  char      Data[ 1 /*SizeRec*/ ];
};
//Фейковые структуры для доступа к статическим данным
//после пакета
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
  DWORD               Version;       // Версия "писателя"

  //FAR additional error info
  char RecomendedDumpFileName[MAXPATH];
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
   ИСКЛЮЧЕНИЯ!
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
};

typedef BOOL (WINAPI *FARPROCESSEVENT)(struct FARExceptionState * Context);

int WriteEvent(DWORD DumpType, // FLOG_*
               EXCEPTION_POINTERS *xp=NULL,
               struct PluginItem *Module=NULL,
               void *RawData=NULL,DWORD RawDataSize=0,
               DWORD RawDataFlags=0,DWORD RawType=RAWTYPE_BINARY);

int xfilter(
    int From,                 // откуда: 0 = OpenPlugin, 1 = OpenFilePlugin
    EXCEPTION_POINTERS *xp,   // данные ситуации
    struct PluginItem *Module,// модуль, приведший к исключению.
    DWORD Flags);             // дополнительные флаги - пока только один
                              //        0x1 - спрашивать про выгрузку?

#endif // __FAREXCPT_HPP__
