#ifndef __VIEWER_HPP__
#define __VIEWER_HPP__
/*
viewer.hpp

Internal viewer

*/

#include "scrobj.hpp"
#include "namelist.hpp"
#include "plugin.hpp"
#include "struct.hpp"

/* $ 10.07.2000 tran
   ! modified MAXSCRY from 120 to 300
   on win200, with Console height FAR work, but trap on viewer... */
#define  MAXSCRY     300
/* tran 10.07.2000 $ */

/* $ 12.07.2000 SVS
  - из-за увеличения длины строки до 0x800 вылетал FAR
    по Alt-F7. Сократим MAX_VIEWLINE до 1024 (0x400)
*/
#define MAX_VIEWLINE  0x800 // 0x400
#define MAX_VIEWLINEB 0x80f // 0x40f
/* SVS $ */

#define VIEWER_UNDO_COUNT   64

/* $ 12.07.2000 SVS
   + Константы для WrapMode во вьювере.
*/
enum {VIEW_UNWRAP=0,VIEW_WRAP=1, VIEW_WORDWRAP=2};
/* SVS $ */

class FileViewer;
class KeyBar;

struct ViewerString
{
	char *lpData /*[MAX_VIEWLINEB]*/;
	__int64 nFilePos;
	bool bSelection;
	__int64 nSelStart;
	__int64 nSelEnd;
};

struct InternalViewerBookMark
{
	__int64 SavePosAddr[BOOKMARK_COUNT];
	__int64 SavePosLeft[BOOKMARK_COUNT];
};

struct ViewerUndoData
{
	__int64 UndoAddr;
	__int64 UndoLeft;
};

/* $ 03.02.2003 VVM
   Разные флаги для поиска */
enum SEARCH_FLAGS
{
	SEARCH_MODE2   = 0x00000001,
	REVERSE_SEARCH = 0x00000002
};
/* VVM $ */

enum SHOW_MODES
{
	SHOW_RELOAD,
	SHOW_HEX,
	SHOW_UP,
	SHOW_DOWN
};

class Viewer:public ScreenObject
{
		friend class FileViewer;
	private:

		BitFlags SearchFlags;

		struct ViewerOptions ViOpt;  // $ 29.03.2001 IS - Часть локальных настроек переехала в ViewerOptions
		WORD FirstWord;              // $ 14.06.2000 SVS - + Переменная FirstWord - первое слово из файла (для автоопределения Unicode)

		NamesList ViewNamesList;
		KeyBar *ViewKeyBar;


		ViewerString *Strings[MAXSCRY+1];

		char FileName[NM];
		char FullFileName[NM];
		FILE *ViewFile;
		WIN32_FIND_DATA ViewFindData;

		unsigned char LastSearchStr[SEARCHSTRINGBUFSIZE];

		// $ 30.07.2000 KM - Новая переменная для поиска
		int LastSearchCase,LastSearchWholeWords,LastSearchReverse,LastSearchHex;

		struct CharTableSet TableSet;
		struct ViewerMode VM;        // $ 27.09.2000 SVS - Переменные "mode" вогнаны под одну крышу

		__int64 FilePos;
		__int64 SecondPos;
		__int64 LastScrPos;
		__int64 FileSize;
		__int64 LastSelPos;

		__int64 LeftPos;
		__int64 LastPage;
		int CRSym;
		__int64 SelectPos,SelectSize;
		DWORD SelectFlags;
		/* $ 06.02.2001 IS
		   Используется для коррекции позиции выделения в юникодных файлах
		*/
		__int64 SelectPosOffSet;
		/* IS $ */
		int ShowStatusLine,HideCursor;
		char TempViewName[NM];
		/* $ 14.06.2002 IS
		   DeleteFolder - удалит не только файл TempViewName, но и каталог,
		   в котором он лежит
		*/
		BOOL DeleteFolder;
		/* IS */

		char Title[512];
		char PluginData[NM*2];
		int TableChangedByUser;
		int ReadStdin;
		int InternalKey;

		struct InternalViewerBookMark BMSavePos;
		struct ViewerUndoData UndoData[VIEWER_UNDO_COUNT];

		int LastKeyUndo;
		int Width,XX2;
		int ViewerID;
		bool OpenFailed;
		bool bVE_READ_Sent;
		FileViewer *HostFileViewer;
		bool AdjustSelPosition;

		bool m_bQuickView;

	private:
		virtual void DisplayObject();

		void ShowPage(int nMode);

		void Up();
		void ShowHex();
		void ShowStatus();
		/* $ 27.04.2001 DJ
		   функции для рисования скроллбара, для корректировки ширины в
		   зависимости от наличия скроллбара и для корректировки позиции файла
		   на границу строки
		*/
		void DrawScrollbar();
		void AdjustWidth();
		void AdjustFilePos();
		/* DJ $ */
		void ReadString(ViewerString *pString, int MaxSize, int StrSize);
		int CalcStrSize(char *Str,int Length);
		void ChangeViewKeyBar();
		void SetCRSym();
		void Search(int Next,int FirstChar);
		void ConvertToHex(char *SearchStr,int &SearchLength);
		int HexToNum(int Hex);
		int vread(char *Buf,int Size,FILE *SrcFile);
		int vseek(FILE *SrcFile,__int64 Offset,int Whence);
		__int64 vtell(FILE *SrcFile);
		int vgetc(FILE *SrcFile);
		void SetFileSize();

	public:
		Viewer(bool bQuickView = false);
		virtual  ~Viewer();


	public:
		/* $ 04.07.2000 tran
		   + 'warning' parameter */
		int OpenFile(const char *Name,int warning);
		/* tran $ */
		void SetViewKeyBar(KeyBar *ViewKeyBar);

		virtual int ProcessKey(int Key);
		virtual int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
		virtual __int64 VMProcess(int OpCode,void *vParam=NULL,__int64 iParam=0);

		void SetStatusMode(int Mode);
		void EnableHideCursor(int HideCursor);
		int GetWrapMode();
		void SetWrapMode(int Wrap);
		int GetWrapType();
		void SetWrapType(int TypeWrap);
		void KeepInitParameters();
		void GetFileName(char *Name);
		virtual void ShowConsoleTitle();
		/* $ 14.06.2002 IS
		   DeleteFolder - удалит не только файл, но и каталог
		*/
		void SetTempViewName(const char *Name, BOOL DeleteFolder);
		/* IS $ */
		void SetTitle(const char *Title);
		const char *GetTitle(char *Title,int LenTitle,int TruncSize=0);

		void SetFilePos(__int64 Pos); // $ 18.07.2000 tran - change 'long' to 'unsigned long'
		__int64 GetFilePos() const { return FilePos; };
		__int64 GetViewFilePos() const { return FilePos; };
		__int64 GetViewFileSize() const { return FileSize; };

		void SetPluginData(char *PluginData);
		void SetNamesList(NamesList *List);
		/* $ 27.09.2000 SVS
		   "Ядро" будущего Viewer API :-)
		*/
		int  ViewerControl(int Command,void *Param);
		void SetHostFileViewer(FileViewer *Viewer) {HostFileViewer=Viewer;};
		/* SVS $ */

		void GoTo(int ShowDlg=TRUE,__int64 NewPos=0,DWORD Flags=0);
		void GetSelectedParam(__int64 &Pos, __int64 &Length, DWORD &Flags);
		// Функция выделения - как самостоятельная функция
		void SelectText(const __int64 &MatchPos,const __int64 &SearchLength, const DWORD Flags=0x1);
		/* $ 29.03.2001 IS
		     Манипуляции с ViewerOptions
		*/
		int GetTabSize() const { return ViOpt.TabSize; }
		void SetTabSize(int newValue) { ViOpt.TabSize=newValue; }

		int GetAutoDetectTable() const { return ViOpt.AutoDetectTable; }
		void SetAutoDetectTable(int newValue) { ViOpt.AutoDetectTable=newValue; }

		int GetShowScrollbar() const { return ViOpt.ShowScrollbar; }
		void SetShowScrollbar(int newValue) { ViOpt.ShowScrollbar=newValue; }

		int GetShowArrows() const { return ViOpt.ShowArrows; }
		void SetShowArrows(int newValue) { ViOpt.ShowArrows=newValue; }
		/* IS $ */
		int GetPersistentBlocks() const { return ViOpt.PersistentBlocks; }
		void SetPersistentBlocks(int newValue) { ViOpt.PersistentBlocks=newValue; }

		int GetAnsiMode() const { return VM.AnsiMode; }
		int GetHexMode() const { return VM.Hex; }


		NamesList *GetNamesList() { return &ViewNamesList; }

		/* $ 08.12.2001 OT
		  возвращает признак того, является ли файл временным
		  используется для принятия решения переходить в каталог по */
		BOOL isTemporary() const;

		int ProcessHexMode(int newMode, bool isRedraw=TRUE);
		int ProcessWrapMode(int newMode, bool isRedraw=TRUE);
		int ProcessTypeWrapMode(int newMode, bool isRedraw=TRUE);
};

#endif // __VIEWER_HPP__
