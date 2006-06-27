#ifndef __FILEVIEWER_HPP__
#define __FILEVIEWER_HPP__
/*
fileview.hpp

Просмотр файла - надстройка над viewer.cpp

*/

/* Revision: 1.27 04.06.2006 $ */

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
      Добавлено для поиска по AltF7. При редактировании найденного файла из
      архива для клавиши F2 сделать вызов ShiftF2.
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
       Функция инициализации KeyBar Labels
    */
    void InitKeyBar(void);
    /* SVS $ */
    int ProcessKey(int Key);
    int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
    void ShowConsoleTitle();
    /* $ 14.06.2002 IS
       Параметр DeleteFolder - удалить не только файл, но и каталог, его
       содержащий (если каталог пуст). По умолчанию - TRUE (получаем
       поведение SetTempViewName такое же, как и раньше)
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
/* $ Введена для нужд CtrlAltShift OT */
    int FastHide();

    /* $ 17.08.2001 KM
      Добавлено для поиска по AltF7. При редактировании найденного файла из
      архива для клавиши F2 сделать вызов ShiftF2.
    */
    void SetSaveToSaveAs(int ToSaveAs) { SaveToSaveAs=ToSaveAs; InitKeyBar(); }
    /* KM $ */
    int  ViewerControl(int Command,void *Param);
    BOOL IsFullScreen(){return FullScreen;}
    virtual void GetTitle(string &Title,int SubLen=-1,int TruncSize=0);
};

#endif  // __FILEVIEWER_HPP__
