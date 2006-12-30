#if !defined(EDITOR2)

#ifndef __EDITOR_HPP__
#define __EDITOR_HPP__
/*
editor.hpp

Редактор

*/

#include "scrobj.hpp"
#include "struct.hpp"
#include "plugin.hpp"
#include "farconst.hpp"
#include "bitflags.hpp"
#include "fn.hpp"

class FileEditor;
class KeyBar;

struct InternalEditorBookMark{
  DWORD Line[BOOKMARK_COUNT];
  DWORD Cursor[BOOKMARK_COUNT];
  DWORD ScreenLine[BOOKMARK_COUNT];
  DWORD LeftPos[BOOKMARK_COUNT];
};


struct EditorCacheParams {
	int Line;
	int LinePos;
	int ScreenLine;
	int LeftPos;
	int Table; //CODEPAGE!!! //BUGBUG

	InternalEditorBookMark SavePos;
};



struct EditorUndoData
{
  int Type;
  int UndoNext;
  int StrPos;
  int StrNum;
  wchar_t EOL[10];
  wchar_t *Str;
};

// Младший байт (маска 0xFF) юзается классом ScreenObject!!!
enum FLAGS_CLASS_EDITOR{
  FEDITOR_MODIFIED              = 0x00000200,
  FEDITOR_JUSTMODIFIED          = 0x00000400,   // 10.08.2000 skv: need to send EE_REDRAW 2.
                                                // set to 1 by TextChanged, no matter what
                                                // is value of State.
  FEDITOR_MARKINGBLOCK          = 0x00000800,
  FEDITOR_MARKINGVBLOCK         = 0x00001000,
  FEDITOR_WASCHANGED            = 0x00002000,
  FEDITOR_OVERTYPE              = 0x00004000,
  FEDITOR_UNDOOVERFLOW          = 0x00008000,
  FEDITOR_NEWUNDO               = 0x00010000,
  FEDITOR_DISABLEUNDO           = 0x00040000,
  FEDITOR_LOCKMODE              = 0x00080000,
  FEDITOR_CURPOSCHANGEDBYPLUGIN = 0x00100000,   // TRUE, если позиция в редакторе была изменена
                                                // плагином (ECTL_SETPOSITION)
  FEDITOR_TABLECHANGEDBYUSER    = 0x00200000,
  FEDITOR_ISRESIZEDCONSOLE      = 0x00800000,
  FEDITOR_PROCESSCTRLQ          = 0x02000000, // нажата Ctrl-Q и идет процесс вставки кода символа
};

class Edit;



class Editor:public ScreenObject
{
  friend class FileEditor;
  private:

    /* $ 04.11.2003 SKV
      на любом выходе если была нажата кнопка выделения,
      и она его "сняла" (сделала 0-й ширины), то его надо убрать.
    */
    struct EditorBlockGuard{
      Editor& ed;
      void (Editor::*method)();
      bool needCheckUnmark;
      EditorBlockGuard(Editor& ed,void (Editor::*method)()):ed(ed),method(method),needCheckUnmark(false)
      {
      }
      ~EditorBlockGuard()
      {
        if(needCheckUnmark)(ed.*method)();
      }
    };
    /* SKV $ */

	Edit *TopList;
    Edit *EndList;
    Edit *TopScreen;
    Edit *CurLine;

    struct EditorUndoData *UndoData;  // $ 03.12.2001 IS: теперь указатель, т.к. размер может меняться
    int UndoDataPos;
    int UndoOverflow;
    int UndoSavePos;

    int LastChangeStrPos;
    int NumLastLine;
    int NumLine;
    /* $ 26.02.2001 IS
         Сюда запомним размер табуляции и в дальнейшем будем использовать его,
         а не Opt.TabSize
    */
    struct EditorOptions EdOpt;
    /* IS $ */
    int Pasting;
    wchar_t GlobalEOL[10];

    Edit *BlockStart;
    int BlockStartLine;
    Edit *VBlockStart;

    int VBlockX;
    int VBlockSizeX;
    int VBlockY;
    int VBlockSizeY;
    int BlockUndo;

    int MaxRightPos;

    string strLastSearchStr;
    /* $ 30.07.2000 KM
       Новая переменная для поиска "Whole words"
    */
    int LastSearchCase,LastSearchWholeWords,LastSearchReverse;

    int m_codepage; //BUGBUG

    int StartLine;
    int StartChar;

    struct InternalEditorBookMark SavePos;

    int EditorID;

    FileEditor *HostFileEditor;

  private:
    void DisplayObject();
    void ShowEditor(int CurLineOnly);
    void DeleteString(Edit *DelPtr,int DeleteLast,int UndoLine);
    void InsertString();
    void Up();
    void Down();
    void ScrollDown();
    void ScrollUp();
    BOOL Search(int Next);
    /* $ 05.07.2000 tran
       ! изменил тип возврата у GoToLine() с 'void ' на 'int'
       возвращаемое значение - это колонка, введенная пользователем
       используется только в одном месте - в обработке Alt-F8
    */
    /* $ 21.07.2000 tran
       GotoLine стала как раньше void
       и добавилась GoToPosition
       */

    void GoToLine(int Line);
    void GoToPosition();
    /* tran 21.07.2000 $ */

    /* tran 05.07.2000 $ */

    /* $ 10.08.2000 skv
      Call this when text changed to set Modified to
      specified State and JustModified to 1
    */
    void TextChanged(int State);
    /* skv $*/

    int  CalcDistance(Edit *From, Edit *To,int MaxDist);
    void Paste(const wchar_t *Src=NULL);
    void Copy(int Append);
    void DeleteBlock();
    void UnmarkBlock();
    /* $ 07.03.2002 IS
         удалить выделение, если оно пустое (выделено ноль символов в ширину)
    */
    void UnmarkEmptyBlock();
    /* IS $ */
    void AddUndoData(const wchar_t *Str,const wchar_t *Eol,int StrNum,int StrPos,int Type);
    void Undo();
    void SelectAll();
    //void SetStringsTable();
    void BlockLeft();
    void BlockRight();
    void DeleteVBlock();
    void VCopy(int Append);
    void VPaste(const wchar_t *ClipText);
    void VBlockShift(int Left);
    Edit* GetStringByNumber(int DestLine);
    static void EditorShowMsg(const wchar_t *Title,const wchar_t *Msg, const wchar_t* Name);

    int SetBookmark(DWORD Pos);
    int GotoBookmark(DWORD Pos);

  public:
    Editor();
    ~Editor();

  public:

    void SetCacheParams (EditorCacheParams *pp);
    void GetCacheParams (EditorCacheParams *pp);

    bool SetCodePage (int codepage); //BUGBUG
    int GetCodePage (); //BUGBUG

    int ReadData(LPCSTR SrcBuf,int SizeSrcBuf);                  // преобразование из буфера в список
    int SaveData(char **DestBuf,int& SizeDestBuf,int TextFormat); // преобразование из списка в буфер

    int ProcessKey(int Key);
    int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
    void KeepInitParameters();
    void SetStartPos(int LineNum,int CharNum);
    int IsFileModified();
    int IsFileChanged();
    void SetTitle(const wchar_t *Title);
    long GetCurPos();
    int EditorControl(int Command,void *Param);
    void SetHostFileEditor(FileEditor *Editor) {HostFileEditor=Editor;};
    static void SetReplaceMode(int Mode);
    FileEditor *GetHostFileEditor() {return HostFileEditor;};
    void PrepareResizedConsole(){Flags.Set(FEDITOR_ISRESIZEDCONSOLE);}

    /* $ 26.02.2001 IS
         Функции чтения/установления текущих настроек редактирования
    */
    void SetTabSize(int NewSize);
    int  GetTabSize(void) const {return EdOpt.TabSize; }

    void SetConvertTabs(int NewMode);
    int  GetConvertTabs(void) const {return EdOpt.ExpandTabs; }

    void SetDelRemovesBlocks(int NewMode);
    int  GetDelRemovesBlocks(void) const {return EdOpt.DelRemovesBlocks; }

    void SetPersistentBlocks(int NewMode);
    int  GetPersistentBlocks(void) const {return EdOpt.PersistentBlocks; }

    void SetAutoIndent(int NewMode) { EdOpt.AutoIndent=NewMode; }
    int  GetAutoIndent(void) const {return EdOpt.AutoIndent; }

    void SetAutoDetectTable(int NewMode) { EdOpt.AutoDetectTable=NewMode; }
    int  GetAutoDetectTable(void) const {return EdOpt.AutoDetectTable; }

    void SetCursorBeyondEOL(int NewMode);
    int  GetCursorBeyondEOL(void) const {return EdOpt.CursorBeyondEOL; }

    void SetBSLikeDel(int NewMode) { EdOpt.BSLikeDel=NewMode; }
    int  GetBSLikeDel(void) const {return EdOpt.BSLikeDel; }
    /* IS $ */

    void SetCharCodeBase(int NewMode) { EdOpt.CharCodeBase=NewMode%3; }
    int  GetCharCodeBase(void) const {return EdOpt.CharCodeBase; }

    void SetWordDiv(const wchar_t *WordDiv) { EdOpt.strWordDiv = WordDiv; }
    const wchar_t *GetWordDiv() { return (const wchar_t*)EdOpt.strWordDiv; }
    /* $ 29.10.2001 IS
         Работа с настройками "сохранять позицию файла" и
         "сохранять закладки" после смены настроек по alt-shift-f9.
    */
    void GetSavePosMode(int &SavePos, int &SaveShortPos);

    // передавайте в качестве значения параметра "-1" для параметра,
    // который не нужно менять
    void SetSavePosMode(int SavePos, int SaveShortPos);
    /* IS $ */

    void GetRowCol(const wchar_t *argv,int *row,int *col);

    /* $ 21.07.2000 tran
       три новых метода*/
    int  GetLineCurPos();
    void BeginVBlockMarking();
    void AdjustVBlock(int PrevX);
    /* tran 21.07.2000 $ */
    void Xlat();
    static void PR_EditorShowMsg(void);

    /* $ 28.01.2002 VVM
      + Освободить всю занятую память */
    void FreeAllocatedData();
    /* VVM $ */

    Edit *CreateString (const wchar_t *lpwszStr, int nLength);
    Edit *InsertString (const wchar_t *lpwszStr, int nLength, Edit *pAfter = NULL);
};

#endif // __EDITOR_HPP__
#endif //defined(EDITOR2)
