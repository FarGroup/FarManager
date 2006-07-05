#ifndef __FILEVIEWER_HPP__
#define __FILEVIEWER_HPP__
/*
fileview.hpp

�������� ����� - ���������� ��� viewer.cpp

*/

/* Revision: 1.28 06.07.2006 $ */

#include "frame.hpp"
#include "viewer.hpp"
#include "keybar.hpp"

class FileViewer:public Frame
{
  private:
    void Show();
    void DisplayObject();
    Viewer View;
    int RedrawTitle;
    KeyBar ViewKeyBar;
    char NewTitle[NM];
    int F3KeyOnly;
    int FullScreen;
    int DisableEdit;
    int DisableHistory;

    string strName;

    typedef class Frame inherited;
    /* $ 17.08.2001 KM
      ��������� ��� ������ �� AltF7. ��� �������������� ���������� ����� ��
      ������ ��� ������� F2 ������� ����� ShiftF2.
    */
    int SaveToSaveAs;
    /* KM $ */

  public:
    FileViewer(const wchar_t *Name,int EnableSwitch=FALSE,int DisableHistory=FALSE,
               int DisableEdit=FALSE,long ViewStartPos=-1,const wchar_t *PluginData=NULL,
               NamesList *ViewNamesList=NULL,int ToSaveAs=FALSE);
    FileViewer(const wchar_t *Name,int EnableSwitch,int DisableHistory,
               const wchar_t *Title,int X1,int Y1,int X2,int Y2);
    ~FileViewer();

  public:
    void Init(const wchar_t *Name,int EnableSwitch,int DisableHistory,
              long ViewStartPos,const wchar_t *PluginData,NamesList *ViewNamesList,int ToSaveAs);
    /* $ 07.08.2000 SVS
       ������� ������������� KeyBar Labels
    */
    void InitKeyBar(void);
    /* SVS $ */
    int ProcessKey(int Key);
    int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
    void ShowConsoleTitle();
    /* $ 14.06.2002 IS
       �������� DeleteFolder - ������� �� ������ ����, �� � �������, ���
       ���������� (���� ������� ����). �� ��������� - TRUE (��������
       ��������� SetTempViewName ����� ��, ��� � ������)
    */
    void SetTempViewName(const wchar_t *Name,BOOL DeleteFolder=TRUE);
    /* IS $ */
    virtual void OnDestroy();

    virtual int GetTypeAndName(string &strType, string &strName);
    virtual const wchar_t *GetTypeName(){return L"[FileView]";}; ///
    virtual int GetType() { return MODALTYPE_VIEWER; }

    /* $ 12.05.2001 DJ */
    void SetEnableF6 (int AEnable) { DisableEdit = !AEnable; InitKeyBar(); }
    /* DJ $ */
/* $ ������� ��� ���� CtrlAltShift OT */
    int FastHide();

    /* $ 17.08.2001 KM
      ��������� ��� ������ �� AltF7. ��� �������������� ���������� ����� ��
      ������ ��� ������� F2 ������� ����� ShiftF2.
    */
    void SetSaveToSaveAs(int ToSaveAs) { SaveToSaveAs=ToSaveAs; InitKeyBar(); }
    /* KM $ */
    int  ViewerControl(int Command,void *Param);
    BOOL IsFullScreen(){return FullScreen;}
    virtual void GetTitle(string &Title,int SubLen=-1,int TruncSize=0);
    __int64 GetViewFileSize() const;
    __int64 GetViewFilePos() const;
};

#endif  // __FILEVIEWER_HPP__
