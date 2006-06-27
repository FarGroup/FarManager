#if !defined(EDITOR2)

#ifndef __EDITOR_HPP__
#define __EDITOR_HPP__
/*
editor.hpp

��������

*/

/* Revision: 1.55 25.05.2006 $ */

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

struct EditorUndoData
{
  int Type;
  int UndoNext;
  int StrPos;
  int StrNum;
  wchar_t *Str;
};

// ������� ���� (����� 0xFF) ������� ������� ScreenObject!!!
enum FLAGS_CLASS_EDITOR{
  FEDITOR_DELETEONCLOSE         = 0x00000100,   // 10.10.2001 IS: ���� TRUE, �� �������
                                                // � ����������� ���� ������ � ���������
                                                // (���� ��� ����)
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
  FEDITOR_CURPOSCHANGEDBYPLUGIN = 0x00100000,   // TRUE, ���� ������� � ��������� ���� ��������
                                                // �������� (ECTL_SETPOSITION)
  FEDITOR_TABLECHANGEDBYUSER    = 0x00200000,
  FEDITOR_OPENFAILED            = 0x00400000,
  FEDITOR_ISRESIZEDCONSOLE      = 0x00800000,

  /* $ 14.06.2002 IS
     ���� ���� ������� � ��� FEDITOR_DELETEONCLOSE, �� ������� ������ ����
  */
  FEDITOR_DELETEONLYFILEONCLOSE = 0x01000000,
  /* IS $ */
  FEDITOR_PROCESSCTRLQ          = 0x02000000, // ������ Ctrl-Q � ���� ������� ������� ���� �������
};

struct EditList;

class Editor:public ScreenObject
{
  friend class FileEditor;
  private:

    /* $ 04.11.2003 SKV
      �� ����� ������ ���� ���� ������ ������ ���������,
      � ��� ��� "�����" (������� 0-� ������), �� ��� ���� ������.
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

    struct EditList *TopList;
    struct EditList *EndList;
    struct EditList *TopScreen;
    struct EditList *CurLine;

    struct EditorUndoData *UndoData;  // $ 03.12.2001 IS: ������ ���������, �.�. ������ ����� ��������
    int UndoDataPos;
    int UndoOverflow;
    int UndoSavePos;

    int LastChangeStrPos;
    int NumLastLine;
    int NumLine;
    /* $ 26.02.2001 IS
         ���� �������� ������ ��������� � � ���������� ����� ������������ ���,
         � �� Opt.TabSize
    */
    struct EditorOptions EdOpt;
    /* IS $ */
    int Pasting;
    wchar_t GlobalEOL[10];

    struct EditList *BlockStart;
    int BlockStartLine;
    struct EditList *VBlockStart;
    int VBlockX;
    int VBlockSizeX;
    int VBlockY;
    int VBlockSizeY;
    int BlockUndo;

    int MaxRightPos;

    string strLastSearchStr;
    /* $ 30.07.2000 KM
       ����� ���������� ��� ������ "Whole words"
    */
    int LastSearchCase,LastSearchWholeWords,LastSearchReverse;
    /* $ KM */

    struct CharTableSet TableSet;
    int UseDecodeTable;
    int TableNum;
    int AnsiText;

    int StartLine;
    int StartChar;

    struct InternalEditorBookMark SavePos;

    int EditorID;

    FileEditor *HostFileEditor;

  private:
    void DisplayObject();
    void ShowEditor(int CurLineOnly);
    void DeleteString(struct EditList *DelPtr,int DeleteLast,int UndoLine);
    void InsertString();
    void Up();
    void Down();
    void ScrollDown();
    void ScrollUp();
    BOOL Search(int Next);
    /* $ 05.07.2000 tran
       ! ������� ��� �������� � GoToLine() � 'void ' �� 'int'
       ������������ �������� - ��� �������, ��������� �������������
       ������������ ������ � ����� ����� - � ��������� Alt-F8
    */
    /* $ 21.07.2000 tran
       GotoLine ����� ��� ������ void
       � ���������� GoToPosition
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

    int  CalcDistance(struct EditList *From,struct EditList *To,int MaxDist);
    void Paste(const wchar_t *Src=NULL);
    void Copy(int Append);
    void DeleteBlock();
    void UnmarkBlock();
    /* $ 07.03.2002 IS
         ������� ���������, ���� ��� ������ (�������� ���� �������� � ������)
    */
    void UnmarkEmptyBlock();
    /* IS $ */
    void AddUndoData(const wchar_t *Str,int StrNum,int StrPos,int Type);
    void Undo();
    void SelectAll();
    void SetStringsTable();
    void BlockLeft();
    void BlockRight();
    void DeleteVBlock();
    void VCopy(int Append);
    void VPaste(const wchar_t *ClipText);
    void VBlockShift(int Left);
    struct EditList * GetStringByNumber(int DestLine);
    static void EditorShowMsg(const wchar_t *Title,const wchar_t *Msg, const wchar_t* Name);

    int SetBookmark(DWORD Pos);
    int GotoBookmark(DWORD Pos);

  public:
    Editor();
    ~Editor();

  public:
    int ReadFile(const wchar_t *Name,int &UserBreak);               // �������������� �� ����� � ������

    int ReadData(LPCSTR SrcBuf,int SizeSrcBuf);                  // �������������� �� ������ � ������
    int SaveData(char **DestBuf,int& SizeDestBuf,int TextFormat); // �������������� �� ������ � �����

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
         ������� ������/������������ ������� �������� ��������������
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
         ������ � ����������� "��������� ������� �����" �
         "��������� ��������" ����� ����� �������� �� alt-shift-f9.
    */
    void GetSavePosMode(int &SavePos, int &SaveShortPos);

    // ����������� � �������� �������� ��������� "-1" ��� ���������,
    // ������� �� ����� ������
    void SetSavePosMode(int SavePos, int SaveShortPos);
    /* IS $ */

    void GetRowCol(const wchar_t *argv,int *row,int *col);

    /* $ 21.07.2000 tran
       ��� ����� ������*/
    int  GetLineCurPos();
    void BeginVBlockMarking();
    void AdjustVBlock(int PrevX);
    /* tran 21.07.2000 $ */
    void Xlat();
    static void PR_EditorShowMsg(void);

    /* $ 28.01.2002 VVM
      + ���������� ��� ������� ������ */
    void FreeAllocatedData();
    /* VVM $ */
};

#endif // __EDITOR_HPP__
#endif //defined(EDITOR2)
