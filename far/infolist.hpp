#ifndef __INFOLIST_HPP__
#define __INFOLIST_HPP__
/*
infolist.hpp

Информационная панель

*/

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
  virtual ~DizViewer() {}
  virtual int ProcessKey(int Key)
  {
    InRecursion=1;
    int res=Viewer::ProcessKey(Key);
    InRecursion=0;
    return res;
  }
  virtual int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent)
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
    virtual void DisplayObject();
    void ShowDirDescription();
    void ShowPluginDescription();

    void PrintText(const wchar_t *Str);
    void PrintText(int MsgID);
    void PrintInfo(const wchar_t *Str);
    void PrintInfo(int MsgID);


    int  OpenDizFile(const wchar_t *DizFile);
    void SetMacroMode(int Restore = FALSE);
    void DynamicUpdateKeyBar();

  public:
    InfoList();
    virtual ~InfoList();

  public:
    virtual int ProcessKey(int Key);
    virtual int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
    virtual void Update(int Mode);
    virtual void SetFocus();
    virtual void KillFocus();
    virtual void GetTitle(string &Title,int SubLen=-1,int TruncSize=0);
    virtual BOOL UpdateKeyBar();
    virtual void CloseFile();
    virtual int GetCurName(string &strName, string &strShortName);
};

#endif  // __INFOLIST_HPP__
