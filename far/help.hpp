#ifndef __HELP_HPP__
#define __HELP_HPP__
/*
help.hpp

������

*/

/* Revision: 1.33 15.03.2006 $ */

#include "frame.hpp"
#include "keybar.hpp"
#include "farconst.hpp"
#include "unicodestring.hpp"

class CallBackStack;

#define HELPMODE_CLICKOUTSIDE  0x20000000 // ���� ������� ���� ��� �����?

struct StackHelpData
{
  DWORD Flags;                  // �����
  int   TopStr;                 // ����� ������� ������� ������ ����
  int   CurX,CurY;              // ���������� (???)

  string strHelpMask;           // �������� �����
  string strHelpPath;           // ���� � ������
  string strHelpTopic;         // ������� �����
  string strSelTopic;          // ���������� ����� (???)
};

enum HELPDOCUMENTSHELPTYPE{
  HIDX_PLUGINS,                 // ������ ��������
  HIDX_DOCUMS,                  // ������ ����������
};

enum {
  FHELPOBJ_ERRCANNOTOPENHELP  = 0x80000000,
};

class Help:public Frame
{
  private:
    BOOL  ErrorHelp;            // TRUE - ������! �������� - ��� ������ ������
    SaveScreen *TopScreen;      // ������� ���������� ��� ������
    KeyBar      HelpKeyBar;     // ������
    CallBackStack *Stack;       // ���� ��������

    struct StackHelpData StackData;
    wchar_t *HelpData;             // "����" � ������.
    int   StrCount;             // ���������� ����� � ����
    int   FixCount;             // ���������� ����� ���������������� �������
    int   FixSize;              // ������ ���������������� �������
    int   TopicFound;           // TRUE - ����� ������
    int   IsNewTopic;           // ��� ����� �����?
    int   MouseDown;

    string strCtrlColorChar;    // CtrlColorChar - �����! ��� �����������-
                                //   ������� - ��� ���������
    int   CurColor;             // CurColor - ������� ���� ���������
    int   CtrlTabSize;          // CtrlTabSize - �����! ������ ���������

    int   PrevMacroMode;        // ���������� ����� �������

    /* $ 29.11.2001 DJ
       ������ PluginContents (��� ����������� � ���������)
    */
    string strCurPluginContents;
    /* DJ $ */

    DWORD LastStartPos;
    DWORD StartPos;

    string strCtrlStartPosChar;

#if defined(WORK_HELP_FIND)
  private:
    DWORD LastSearchPos;
    unsigned char LastSearchStr[SEARCHSTRINGBUFSIZE];
    int LastSearchCase,LastSearchWholeWords,LastSearchReverse;

  private:
    int Search(int Next);
    void KeepInitParameters();
#endif

  private:
    void DisplayObject();
    int  ReadHelp(const wchar_t *Mask=NULL);
    void AddLine(const wchar_t *Line);
    void AddTitle(const wchar_t *Title);
    void HighlightsCorrection(wchar_t *Str);
    void FastShow();
    /* $ 29.11.2001 DJ
       �������� �� FastShow
    */
    void DrawWindowFrame();
    /* DJ $ */
    void OutString(const wchar_t *Str);
    int  StringLen(const wchar_t *Str);
    void CorrectPosition();
    int  IsReferencePresent();
    void MoveToReference(int Forward,int CurScreen);
    void ReadDocumentsHelp(int TypeIndex);
    int  JumpTopic(const wchar_t *JumpTopic=NULL);

  public:
    Help(const wchar_t *Topic,const wchar_t *Mask=NULL,DWORD Flags=0);
    ~Help();

  public:
    void Hide();
    int  ProcessKey(int Key);
    int  ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
    void InitKeyBar(void);
    BOOL GetError() {return ErrorHelp;}
    /* $ 28.06.2000 tran NT Console resize - resize help */
    virtual void SetScreenPosition();
    void OnChangeFocus(int focus); // ���������� ��� ����� ������
    void ResizeConsole();
    /* $ ������� ��� ���� CtrlAltShift OT */
    int  FastHide();

    virtual const wchar_t *GetTypeName() {return L"[Help]";}
    virtual int GetTypeAndName(string &strType, string &strName);
    virtual int GetType() { return MODALTYPE_HELP; }

    static string &MkTopic(int PluginNumber,const wchar_t *HelpTopic,string &strTopic);
};

#endif  // __HELP_HPP__
