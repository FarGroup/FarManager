#ifndef __INFOLIST_HPP__
#define __INFOLIST_HPP__
/*
infolist.hpp

Информационная панель

*/

/* Revision: 1.14 09.05.2006 $ */

#include "panel.hpp"
#include "viewer.hpp"
//class Viewer;

/* $ 12.10.2001 SKV
  заврапим Viewer что бы отслеживать рекурсивность вызова
  методов DizView и случайно не удалить его во время вызова.
*/
class DizViewer: public Viewer
{
public:
  int InRecursion;
  DizViewer():InRecursion(0){}
  int ProcessKey(int Key)
  {
    InRecursion=1;
    int res=Viewer::ProcessKey(Key);
    InRecursion=0;
    return res;
  }
  int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent)
  {
    InRecursion=1;
    int res=Viewer::ProcessMouse(MouseEvent);
    InRecursion=0;
    return res;
  }
};
/* SKV$*/

class InfoList:public Panel
{
  private:
    DizViewer *DizView;
    int  PrevMacroMode;
    int  DizPresent;
    int  OldWrapMode;
    int  OldWrapType;
    string strDizFileName;

  private:
    void DisplayObject();
    void ShowDirDescription();
    void ShowPluginDescription();

    void PrintTextW(const wchar_t *Str);
    void PrintTextW(int MsgID);
    void PrintInfoW(const wchar_t *Str);
    void PrintInfoW(int MsgID);


    int  OpenDizFile(const wchar_t *DizFile);
    void SetMacroMode(int Restore = FALSE);
    void DynamicUpdateKeyBar();

  public:
    InfoList();
    ~InfoList();

  public:
    int ProcessKey(int Key);
    int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
    void Update(int Mode);
    virtual void SetFocus();
    virtual void KillFocus();
    virtual void GetTitle(string &Title,int SubLen=-1,int TruncSize=0);
    virtual BOOL UpdateKeyBar();
    virtual void CloseFile();
    virtual int GetCurNameW(string &strName, string &strShortName);
};

#endif  // __INFOLIST_HPP__
