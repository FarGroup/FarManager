#if !defined(EDITOR2)

#ifndef __EDITOR_HPP__
#define __EDITOR_HPP__
/*
editor.hpp

Редактор

*/

/* Revision: 1.36 18.05.2002 $ */

/*
Modify:
  18.05.2002 SVS
    ! ФЛАГИ - сведем в кучу двухпозиционные переменные
  07.03.2002 IS
    + UnmarkEmptyBlock(): удалить выделение, если оно пустое (выделено ноль
      символов в ширину)
  04.02.2002 SVS
    ! Editor::IsShiftKey() -> keyboard.cpp::IsShiftKey()
  28.01.2002 VVM
    + Освободить всю занятую память - void FreeAllocatedData()
  15.01.2002 SVS
    ! FileEditor - друг Editor`у (в последствии - для "отучения Editor от 'файл'")
    + GetHostFileEditor()
    ! Вместо кучи int`ов - битовые флаги FLAGS_CLASS_EDITOR
    ! ProcessEditorInput ушел в FileEditor (в диалога плагины не...)
  11.11.2002 IS
    + CurPosChangedByPlugin
  25.12.2001 SVS
    + ResizedConsole - при изменении консоли = 1
  14.12.2001 IS
    ! внедрение const в соответствии с изменениями класса Edit
  03.12.2001 IS
    ! UndoData теперь указатель, т.к. размер может меняться
    ! Убрал EDITOR_UNDO_COUNT, т.к. вместо него теперь Opt.EditorUndoSize
  29.10.2001 IS
    + GetSavePosMode/SetSavePosMode
  21.10.2001 SVS
    + CALLBACK-функция для избавления от BugZ#85
  10.10.2001 IS
    + обработка DeleteOnClose
  18.08.2001 SVS
    + параметр у функции Paste - для отработки $Date, у которой есть '%n'
  25.06.2001 IS
    ! Внедрение const
  25.06.2001 SVS
    - Падение ФАРа при поске в редакторе
  06.06.2001 SVS
    + EDITOR_UNDO_COUNT - "от чисел к символам" (для наглядности :-)
    ! SavePos* заменено на SavePos (одной структурой - InternalEditorBookMark)
  03.06.2001 OT
    - Не обновлялся StatusLine после DrawLine в редакторе
  27.05.2001 DJ
    + константы для кодов возврата Editor::SaveFile()
  07.05.2001 SVS
    ! Search теперь возвращает TRUE/FALSE
  06.05.2001 DJ
    ! перетрях #include
  28.03.2001 SVS
    + дополнительный параметр для SaveFile() - SaveAs
  27.02.2001 SVS
    + *CharCodeBase() - по поводу базы вывода
  26.02.2001 IS
    ! Часть самостоятельных переменных заменено соответствующими из
      EditorOptions. Надо было это сразу сделать, да я что-то стормозил :)
    + SetAutoIndent/GetAutoIndent
      SetAutoDetectTable/GetAutoDetectTable
      SetCursorBeyondEOL/GetCursorBeyondEOL
      SetBSLikeDel/GetBSLikeDel
  15.02.2001 IS
    + Локальные переменные, в которых запоминается то, что храниться в
      настройках редактора:
      DelRemovesBlocks - "Del удаляет блоки"
      PersistentBlocks - "Постоянные блоки"
    + Функции для управления их состоянием:
      SetDelRemovesBlocks/GetDelRemovesBlocks
      SetPersistentBlocks/GetPersistentBlocks
  15.02.2001 IS
    - Тело функции SetTabSize переехало в editor.cpp
    + За режима "Пробелы вместо табуляции" отвечает переменная ConvertTabs
    + GetConvertTabs и SetConvertTabs
  14.02.2001 IS
    + Размер табуляции хранится в TabSize, манипулировать им можно при помощи
      GetTabSize, SetTabSize
  13.02.2001 IS
    + Переменная AttrStr
    + Функция GetFileAttributes;
  12.02.2001 IS
    + FileAttributes
  24.09.2000 SVS
    + Функция Xlat
  10.08.2000 skv
    + добавлены int JustModied и void TextChanged(state);
  30.07.2000 KM 1.06
    + LastSearchWholeWords
  21.07.2000 tran
    - три новых метода  - Bug22
  21.07.2000 tran
    ! GotoLine стала как раньше void и добавилась GoToPosition
  17.07.2000 OT
    + Застолбить место под разработку "моего" редактора
  14.07.2000 tran
    ! функцию GetRowCol сделал методом класса
  05.07.2000 tran
    ! изменил тип возврата у GoToLine() с 'void ' на 'int'
      возвращаемое значение - это колонка, введенная пользователем
      используется только в одном месте - в обработке Alt-F8
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#include "scrobj.hpp"
#include "struct.hpp"
#include "plugin.hpp"
#include "farconst.hpp"
#include "bitflags.hpp"

class FileEditor;
class KeyBar;

struct InternalEditorBookMark{
  long Line[BOOKMARK_COUNT];
  long Cursor[BOOKMARK_COUNT];
  long ScreenLine[BOOKMARK_COUNT];
  long LeftPos[BOOKMARK_COUNT];
};

struct EditorUndoData
{
  int Type;
  int UndoNext;
  int StrPos;
  int StrNum;
  char *Str;
};

/* $ 27.05.2001 DJ
   коды возврата Editor::SaveFile()
*/

enum {
    SAVEFILE_ERROR   = 0,         // пытались сохранять, не получилось
    SAVEFILE_SUCCESS = 1,         // либо успешно сохранили, либо сохранять было не надо
    SAVEFILE_CANCEL  = 2          // сохранение отменено, редактор не закрывать
};
/* DJ $ */

// Младший байт (маска 0xFF) юзается классом ScreenObject!!!
enum FLAGS_CLASS_EDITOR{
  FEDITOR_DELETEONCLOSE         = 0x00000100,   // 10.10.2001 IS: Если TRUE, то удалить
                                                // в деструкторе файл вместе с каталогом
                                                // (если тот пуст
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
  FEDITOR_OPENFAILED            = 0x00400000,
  FEDITOR_ISRESIZEDCONSOLE      = 0x00800000,
};

struct EditList;

class Editor:public ScreenObject
{
  friend class FileEditor;
  private:
    char Title[512];
    char PluginData[NM*2];
    char PluginTitle[512];

    KeyBar *EditKeyBar;
    struct EditList *TopList,*EndList,*TopScreen,*CurLine;
    /* $ 03.12.2001 IS теперь указатель, т.к. размер может меняться */
    struct EditorUndoData *UndoData;
    /* IS $ */
    int UndoDataPos;
    int UndoOverflow;
    int UndoSavePos;
    int LastChangeStrPos;
    char FileName[NM];
    int NumLastLine,NumLine;
    /* $ 12.02.2001 IS
         сюда запомним атрибуты файла при открытии, пригодятся где-нибудь...
    */
    DWORD FileAttributes;
    /* IS $ */
    /* $ 13.02.2001 IS
         Сюда запомним буквы атрибутов, чтобы не вычислять их много раз
    */
    char AttrStr[4];
    /* IS $ */
    /* $ 26.02.2001 IS
         Сюда запомним размер табуляции и в дальнейшем будем использовать его,
         а не Opt.TabSize
    */
    struct EditorOptions EdOpt;
    /* IS $ */
    int DisableOut;
    int Pasting;
    char GlobalEOL[10];
    struct EditList *BlockStart;
    int BlockStartLine;

    struct EditList *VBlockStart;
    int VBlockX;
    int VBlockSizeX;
    int VBlockY;
    int VBlockSizeY;

    int BlockUndo;

    int MaxRightPos;

    unsigned char LastSearchStr[SEARCHSTRINGBUFSIZE];
    /* $ 30.07.2000 KM
       Новая переменная для поиска "Whole words"
    */
    int LastSearchCase,LastSearchWholeWords,LastSearchReverse;
    /* $ KM */

    struct CharTableSet TableSet;
    int UseDecodeTable,TableNum,AnsiText;
    int StartLine,StartChar;

    struct InternalEditorBookMark SavePos;

    int EditorID;

    FileEditor *HostFileEditor;

  private:
    void DisplayObject();
    void ShowEditor(int CurLineOnly);
    void ShowStatus();
    void DeleteString(struct EditList *DelPtr,int DeleteLast,int UndoLine);
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

    /* $ 13.02.2001 IS
         Обертка вокруг одноименной функции из win32 api
    */
    DWORD GetFileAttributes(LPCTSTR);
    /* IS $ */
    int  CalcDistance(struct EditList *From,struct EditList *To,int MaxDist);
    void Paste(char *Src=NULL);
    void Copy(int Append);
    void DeleteBlock();
    void UnmarkBlock();
    /* $ 07.03.2002 IS
         удалить выделение, если оно пустое (выделено ноль символов в ширину)
    */
    void UnmarkEmptyBlock();
    /* IS $ */
    void ChangeEditKeyBar();
    void AddUndoData(const char *Str,int StrNum,int StrPos,int Type);
    void Undo();
    void SelectAll();
    void SetStringsTable();
    void BlockLeft();
    void BlockRight();
    void DeleteVBlock();
    void VCopy(int Append);
    void VPaste(char *ClipText);
    void VBlockShift(int Left);
    struct EditList * GetStringByNumber(int DestLine);
    static void EditorShowMsg(const char *Title,const char *Msg, const char* Name);

  public:
    Editor();
    ~Editor();

  public:
    int ReadFile(const char *Name,int &UserBreak);
    int SaveFile(const char *Name,int Ask,int TextFormat,int SaveAs,int NewFile=TRUE);
    int ProcessKey(int Key);
    int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
    void SetEditKeyBar(KeyBar *EditKeyBar);
    void KeepInitParameters();
    void SetStartPos(int LineNum,int CharNum);
    int IsFileModified();
    int IsFileChanged();
    void SetTitle(const char *Title);
    long GetCurPos();
    void SetPluginData(char *PluginData);
    void SetPluginTitle(char *PluginTitle);
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

    /* $ 10.10.2001 IS установка DeleteOnClose */
    void SetDeleteOnClose(BOOL NewMode) { Flags.Change(FEDITOR_DELETEONCLOSE,NewMode); }
    /* IS */

    /* $ 29.10.2001 IS
         Работа с настройками "сохранять позицию файла" и
         "сохранять закладки" после смены настроек по alt-shift-f9.
    */
    void GetSavePosMode(int &SavePos, int &SaveShortPos);

    // передавайте в качестве значения параметра "-1" для параметра,
    // который не нужно менять
    void SetSavePosMode(int SavePos, int SaveShortPos);
    /* IS $ */

    /* $ tran 14.07.2000
      + goto to percent support */
    void GetRowCol(char *argv,int *row,int *col);
    /* tran 14.07.2000 $ */

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
};

#endif // __EDITOR_HPP__
#endif //defined(EDITOR2)
