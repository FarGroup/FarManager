#ifndef __FAREXCPT_HPP__
#define __FAREXCPT_HPP__
/*
exception.cpp

Все про исключения

*/

/* Revision: 1.04 25.01.2002 $ */

/*
Modify:
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

#define FLOG_SYSINFO     0x00000001 // информация о системе
#define FLOG_REGISTERS   0x00000002 // текущее состояние регистров
#define FLOG_EXCEPTION   0x00000004 // про исключение
#define FLOG_STACK       0x00000008 // "раскрученный стек"
#define FLOG_PLUGIN      0x00000010 // информация о плагине
#define FLOG_FARAREA     0x00000020 // "где мы сейчас находимся?"
#define FLOG_MACRO       0x00000040 // Макросы
#define FLOG_RAWDARA     0x00000080 // произвольные данные
#define FLOG_FAULTCODE   0x00000100 // кусок кода
#define FLOG_PLUGINSINFO 0x00000200 // информация о плагинах
#define FLOG_ALL         0xFFFFFFFF

enum {
  RTYPE_HEADER       =0,// заголовок дампа
  RTYPE_SYSINFO      =1,// информация о системе
  RTYPE_CONTEXT      =2,// текущее состояние регистров
  RTYPE_EXCEPTION    =3,// про исключение
  RTYPE_FAULTCODE    =4,// кусок кода
  RTYPE_STACK        =5,// "раскрученный стек"
  RTYPE_PLUGIN       =6,// информация о плагине
  RTYPE_FARAREA      =7,// "где мы сейчас находимся?"
  RTYPE_MACRO        =8,// Макросы
  RTYPE_RAWDARA      =9,// произвольные данные
};

/*

Структура файла "farevent.dmp"

FILEHEADER FileHeader;

Record {
    RECHEADER RecHeader;  // заголовок
    // RECHEADER Rec[0];  // количеством RecHeader.RecordsCount
} [FileHeader.CountRecords];

*/

struct FILEHEADER{
  DWORD ID;               // 'FLOG'
  DWORD CountRecords;     // количество записей
  DWORD Version;          // Версия "писателя"
};

struct RECHEADER{         // заголовок рекорда
  WORD TypeRec;           // Тип записи
  WORD SizeRec;           // Размер данных Data (байт), если =0 - Data отсутствует
  // Data                 // Данные размером SizeRec
};

struct DUMPHEADER{        // заголовок дампа
  WORD  TypeRec;          // Тип записи = RTYPE_HEADER
  WORD  SizeRec;          // Размер данных = sizeof(struct DUMPHEADER)-sizeof(WORD)*2
  DWORD DumpFlags;        // дополнительные флаги (пока =0)
  DWORD DumpSize;         // общий размер текущего дампа
  SYSTEMTIME DumpTime;    // the system time is expressed in Coordinated Universal Time (UTC))
  WORD  RecordsCount;     // Общее количество рекордов RECHEADER за исключением заголовка
  WORD  CountException;   // из них - количество записей под исключения
  WORD  CountStack;       // из них - количество записей под стек
  WORD  Reserved[5];      // резерв
};

struct SYSINFOHEADER{     // информация о системе
  WORD  TypeRec;          // Тип записи = RTYPE_SYSINFO
  WORD  SizeRec;          // Размер данных = sizeof(struct DUMPHEADER)-sizeof(WORD)*2
  OSVERSIONINFO WinVer;   // версия виндов
  DWORD FARVersion;       // версия FAR Manager в формате FAR_VERSION
  DWORD Reserved[3];      // резерв
};

struct CONTEXTRECORD{     // текущее состояние регистров
  WORD  TypeRec;          // Тип записи = RTYPE_CONTEXT
  WORD  SizeRec;          // Размер данных = sizeof(CONTEXT)
  CONTEXT Regs;           // Регистры процессора
#if defined(__BORLANDC__) && (__BORLANDC__ < 0x0550)
  //
  // This section is specified/returned if the ContextFlags word
  // contains the flag CONTEXT_EXTENDED_REGISTERS.
  // The format and contexts are processor specific
  //
#define MAXIMUM_SUPPORTED_EXTENSION     512
#define CONTEXT_EXTENDED_REGISTERS  (CONTEXT_i386 | 0x00000020L) // cpu specific extensions
  BYTE    ExtendedRegisters[MAXIMUM_SUPPORTED_EXTENSION];
#endif
};

struct MACRORECORD{       // Макросы
  WORD  TypeRec;          // Тип записи = RTYPE_MACRO
  WORD  SizeRec;          // Размер данных
  WORD  MacroStatus;      // 0 - не в режиме макро, 1 - Recording, 2 - Executing
  WORD  MacroPos;         // текущая позиция в MacroKeyBuffer
  DWORD MacroFlags;       // флаги - младшее слово = MACRO_AREA
  DWORD MacroKey;         // назначенная макроклавиша
  DWORD MacroBufferSize;  // размер макропоследовательности MacroKeyBuffer
  // DWORD MacroKeyBuffer[0];// макро-последовательность
};

struct PLUGINRECORD{      // информация о плагине
  WORD  TypeRec;          // Тип записи = RTYPE_PLUGIN
  WORD  SizeRec;          // Размер данных

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
    char     cFileName[NM];
    char     cAlternateFileName[14];
  } FindData;

  DWORD Reserved[4];    // разерв :-)

  DWORD SizeModuleName;
  //char ModuleName[0];
};

struct PLUGINSINFORECORD{ // информация о плагинах
  WORD  TypeRec;          // Тип записи = FLOG_PLUGINSINFO
  WORD  SizeRec;          // Размер данных
  DWORD PluginsCount;     // количество записей
}; // следом за этим рекордом идут PluginsCount рекорды PLUGINRECORD

struct EXCEPTIONRECORD{   // про исключение
  WORD  TypeRec;          // Тип записи = RTYPE_EXCEPTION
  WORD  SizeRec;          // Размер данных
  WORD  CurItem;          // теущий индекс исключения в текущем дампе
                          //   (если они валились каскадно)
  WORD  Reserved;
  // сокращенный EXCEPTION_RECORD
  DWORD ExceptionCode;    // код исключения
  DWORD ExceptionFlags;   // флаги исключения
  DWORD ExceptionAddress; // адрес исключения
  DWORD Section;          //  секция  \ Fault address - дополнение к
  DWORD Offset;           //  смещение/                 ExceptionAddress
  DWORD NumberParameters; // количество параметров доп. информации
  DWORD ExceptionInformation[EXCEPTION_MAXIMUM_PARAMETERS]; // доп. информация
  DWORD SizeModuleName;   // размер имени модуля
  //char  ModuleName[0];    // если SizeModuleName=0, то этого поля нету.
};

struct FAULTCODERECORD{   // кусок кода
  WORD  TypeRec;          // Тип записи = RTYPE_FAULTCODE
  WORD  SizeRec;          // Размер данных
  DWORD ExceptionAddress; // адрес исключения
  DWORD SuccessCode;      // =0 - данные в Code невалидны
  BYTE  Code[128];        // первые 64 байта перед тем местом, где был трап
                          // 64-й байт - это и есть то самое место!
                          // эти 128 байта могут быть заполнены 0, если не
                          // получилось считать данные из памяти (нет доступа)
  DWORD SuccessData;
  BYTE  Data[128];
};

struct STACKRECORD{       // "раскрученный стек"
  WORD  TypeRec;          // Тип записи = RTYPE_STACK
  WORD  SizeRec;          // Размер данных
  WORD  CurItem;          // теущий индекс стека в текущем дампе
  WORD  Reserved;
  DWORD EIP;              // адрес
  DWORD EBP;
  DWORD Section;          // секция
  DWORD Offset;           // смещение

  DWORD SizeModuleName;   // размер имени модуля
  // char  ModuleName[0];   // если SizeModuleName=0, то этого поля нету.
};


struct FARAREARECORD{     // "где мы сейчас находимся?"
  WORD  TypeRec;          // Тип записи = RTYPE_FARAREA
  WORD  SizeRec;          // Размер данных
  DWORD ObjectType;       // то, что возвращает CtrlObject->Cp()->GetType()
  COORD ScrWH;            // размеры экрана - ширина, высота

  DWORD Reserved[20];
};

struct RAWDARARECORD{     // произвольные данные
  WORD  TypeRec;          // Тип записи = RTYPE_RAWDARA
  WORD  SizeRec;          // Размер данных
  DWORD RawFlags;         // Дополнительные флаги для расширябильности :-)
  //BYTE Data[0];         // если SizeRec=0, то этого поля нету
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

/* Функция для записи порции рекордов под эгидой одного эвента :-)
   Вызывается до тех пор, пока не вернет FALSE.
   Функция должна добавлять к параметру SizeRec размер записанных данных.
   Iteration - номер очередной итерации (начинается с 0)
*/
typedef BOOL (WINAPI *EVENTPROC)(HANDLE hFile,DWORD *SizeRec,int Iteration);

int WriteEvent(DWORD DumpType, // FLOG_*
               EXCEPTION_POINTERS *xp,
               struct PluginItem *Module,
               void *RawData,DWORD RawDataSize,
               DWORD RawDataFlags=0,
               EVENTPROC CallBackProc=NULL);
int xfilter(
    int From,                 // откуда: 0 = OpenPlugin, 1 = OpenFilePlugin
    EXCEPTION_POINTERS *xp,   // данные ситуации
    struct PluginItem *Module,// модуль, приведший к исключению.
    DWORD Flags);             // дополнительные флаги - пока только один
                              //        0x1 - спрашивать про выгрузку?

#endif // __FAREXCPT_HPP__
