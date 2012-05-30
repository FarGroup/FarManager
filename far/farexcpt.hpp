#pragma once

/*
exception.cpp

Все про исключения
*/
/*
Copyright © 1996 Eugene Roshal
Copyright © 2000 Far Group
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "plugins.hpp"

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

enum FARRECORDTYPE
{
	RTYPE_SYSINFO      =MAKEFOURCC('S','Y','S','T'),// информация о системе
	RTYPE_EXCEPTION    =MAKEFOURCC('E','X','C','T'),// про исключение
	RTYPE_PLUGIN       =MAKEFOURCC('C','P','L','G'),// информация о текущем плагине
	RTYPE_FARAREA      =MAKEFOURCC('A','R','E','A'),// "где мы сейчас находимся?"
	RTYPE_MACRO        =MAKEFOURCC('M','A','C','R'),// Макросы
	RTYPE_RAWDARA      =MAKEFOURCC('R','A','W','D'),// произвольные данные
};

struct RECHEADER          // заголовок рекорда
{
	DWORD TypeRec;          // Тип записи
	DWORD SizeRec;          // Размер структуры
	RECHEADER *Next; // Следующий элемент в списке
	// Data                 // Данные размером SizeRec
};

struct SYSINFOHEADER      // информация о системе
{
	DWORD TypeRec;          // Тип записи = RTYPE_SYSINFO
	DWORD SizeRec;          // Размер данных = sizeof(DUMPHEADER)-sizeof(WORD)*2
	RECHEADER *Next; // Следующий элемент в списке
	DWORD DumpFlags;        // дополнительные флаги (пока =0)
	VersionInfo FARVersion;       // версия FAR Manager в формате FAR_VERSION
	SYSTEMTIME DumpTime;    // the system time is expressed in Coordinated Universal Time (UTC))
	OSVERSIONINFO WinVer;   // версия виндов
};

// флаги для поля PluginItem.WorkFlags
enum EX_PLUGINITEMWORKFLAGS
{
	EXPIWF_CACHED        = 0x00000001, // кешируется
	EXPIWF_PRELOADED     = 0x00000002, //
	EXPIWF_DONTLOADAGAIN = 0x00000004, // не загружать плагин снова, ставится в
	//   результате проверки требуемой версии фара
	EXPIWF_DATALOADED    = 0x00000008, // LoadData успешно выполнилась
};


struct PLUGINRECORD       // информация о плагине
{
	DWORD TypeRec;          // Тип записи = RTYPE_PLUGIN
	DWORD SizeRec;          // Размер
	DWORD Reserved1;

	DWORD WorkFlags;        // рабочие флаги текущего плагина
	DWORD FuncFlags;        // битовые маски эксп.функций плагина (бит есть - ест и функция)
	DWORD CallFlags;        // битовые маски вызова эксп.функций плагина

	// DWORD SysID; GUID

	const wchar_t *ModuleName;

	DWORD Reserved2[2];    // разерв :-)

	DWORD SizeModuleName;
};

struct EXCEPTIONRECORD    // про исключение
{
	DWORD TypeRec;          // Тип записи = RTYPE_EXCEPTION
	DWORD SizeRec;          // Размер данных
	RECHEADER *Next; // Следующий элемент в списке

	EXCEPTION_POINTERS *Exception;
};

struct MACRORECORD        // Макросы
{
	DWORD TypeRec;          // Тип записи = RTYPE_MACRO
	DWORD SizeRec;          // Размер
	RECHEADER *Next; // Следующий элемент в списке
	WORD  MacroStatus;      // 0 - не в режиме макро, 1 - Recording, 2 - Executing
	WORD  MacroPos;         // текущая позиция в MacroKeyBuffer
	DWORD MacroFlags;       // флаги - младшее слово = MACRO_AREA
	DWORD MacroKey;         // назначенная макроклавиша
	DWORD MacroBufferSize;  // размер макропоследовательности MacroKeyBuffer
	// DWORD MacroKeyBuffer[0];// макро-последовательность
};

struct FARAREARECORD      // "где мы сейчас находимся?"
{
	DWORD TypeRec;          // Тип записи = RTYPE_FARAREA
	DWORD SizeRec;          // Размер данных
	RECHEADER *Next; // Следующий элемент в списке
	DWORD ObjectType;       // то, что возвращает CtrlObject->Cp()->GetType()
	COORD ScrWH;            // размеры экрана - ширина, высота
};

enum
{
	RAWTYPE_BINARY =0,
	RAWTYPE_STRING   =1,
};

struct RAWDARARECORD      // произвольные данные
{
	DWORD TypeRec;          // Тип записи = RTYPE_RAWDARA
	DWORD SizeRec;          // Размер данных
	RECHEADER *Next; // Следующий элемент в списке
	DWORD RawFlags;         // Дополнительные флаги для расширябильности :-)
	DWORD RawType;          // Тип данных = RAWTYPE_BINARY, RAWTYPE_STRING
	DWORD SizeData;         // Размер произвольных данных
	//BYTE Data[0];         // если SizeRec=0, то этого поля нету
};


//Фейковая структуря для быстрого доступа к данным
//после заголовка пакета
struct COMBINE_RECORD
{
	RECHEADER Header;
	char      Data[ 1 /*SizeRec*/ ];
};
//Фейковые структуры для доступа к статическим данным
//после пакета
struct MACRORECORD_t : public MACRORECORD
{
	DWORD MacroKeyBuffer[ 1 /*MacroBufferSize*/ ];
};
struct PLUGINRECORD_t : public PLUGINRECORD
{
	char ModuleName[ 1 /*SizeModuleName*/ ];
};
struct RAWDARARECORD_t : public RAWDARARECORD
{
	LPBYTE RawDataPtr;  //The pointer to allocated raw data can be placed here
};

struct FARExceptionState
{
	DWORD               StructSize;
	DWORD               Version;       // Версия "писателя"

	//FAR additional error info
	char RecomendedDumpFileName[_MAX_PATH];
	const char         *RootKey;

	//FAR error context
	RECHEADER   *Head;
};

/* $ 17.10.2000 SVS
   ИСКЛЮЧЕНИЯ!
*/
enum ExceptFunctionsType
{
	EXCEPT_KERNEL=-1,
	EXCEPT_GETGLOBALINFO,
	EXCEPT_SETSTARTUPINFO,
	EXCEPT_GETVIRTUALFINDDATA,
	EXCEPT_OPEN,
	EXCEPT_OPENFILEPLUGIN,
	EXCEPT_CLOSEPANEL,
	EXCEPT_GETPLUGININFO,
	EXCEPT_GETOPENPANELINFO,
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
	EXCEPT_PROCESSPANELINPUT,
	EXCEPT_PROCESSPANELEVENT,
	EXCEPT_PROCESSEDITOREVENT,
	EXCEPT_COMPARE,
	EXCEPT_PROCESSEDITORINPUT,
	EXCEPT_MINFARVERSION,
	EXCEPT_PROCESSVIEWEREVENT,
	EXCEPT_PROCESSVIEWERINPUT,
	EXCEPT_PROCESSDIALOGEVENT,
	EXCEPT_PROCESSSYNCHROEVENT,
	EXCEPT_ANALYSE,
	EXCEPT_GETCUSTOMDATA,
	EXCEPT_FREECUSTOMDATA,
	EXCEPT_CLOSEANALYSE,
#if defined(MANTIS_0000466)
	EXCEPT_PROCESSMACRO,
#endif
#if defined(MANTIS_0001687)
	EXCEPT_PROCESSCONSOLEINPUT,
#endif
};

typedef BOOL (WINAPI *FARPROCESSEVENT)(FARExceptionState * Context);

int WriteEvent(DWORD DumpType, // FLOG_*
               EXCEPTION_POINTERS *xp=nullptr,
               Plugin *Module=nullptr,
               void *RawData=nullptr,DWORD RawDataSize=0,
               DWORD RawDataFlags=0,DWORD RawType=RAWTYPE_BINARY);

DWORD WINAPI xfilter(
    int From,                 // откуда: 0 = OpenPlugin, 1 = OpenFilePlugin
    EXCEPTION_POINTERS *xp,   // данные ситуации
    Plugin *Module,// модуль, приведший к исключению.
    DWORD Flags);             // дополнительные флаги - пока только один
//        0x1 - спрашивать про выгрузку?
