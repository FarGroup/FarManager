#ifndef __FILEVIEWER_HPP__
#define __FILEVIEWER_HPP__
/*
fileview.hpp

Просмотр файла - надстройка над viewer.cpp

*/

/* Revision: 1.17 13.05.2002 $ */

/*
Modify:
  13.05.2002 VVM
    + Перерисуем заголовок консоли после позиционирования на файл.
  02.11.2001 SVS
    ! возвращаемое значение у GetTypeName() - модификатор const
  08.09.2001 IS
    + Дополнительный параметр у второго конструктора: DisableHistory
  17.08.2001 KM
    + Добавлена функция SetSaveToSaveAs для установки дефолтной реакции
      на клавишу F2 в вызов ShiftF2 для поиска, в случае редактирования
      найденного файла из архива.
    ! Изменён конструктор и функция Init для работы SaveToSaveAs.
  11.07.2001 OT
    Перенос CtrlAltShift в Manager
  25.06.2001 IS
   ! Внедрение const
  14.06.2001 OT
    ! "Бунт" ;-)
  06.06.2001 OT
    ! отменен OnChangeFocus за отсутствием состава ... необходимости :)
    + добавлен деструктор ~FileViewer()... с косметическими целями
  05.06.2001 tran
    + класс FileView - добавлен OnChangeFocus
  15.05.2001 OT
    ! NWZ -> NFZ
  12.05.2001 DJ
    ! отрисовка по OnChangeFocus перенесена в Frame
    ! убран дублирующийся ExitCode
    + SetEnableF6()
  06.05.2001 DJ
    ! перетрях #include
  06.05.2001 ОТ
    ! Переименование Window в Frame :)
  05.05.2001 DJ
    + Перетрях NWZ
  29.04.2001 ОТ
    + Внедрение NWZ от Третьякова
  07.08.2000 SVS
    + Функция инициализации KeyBar Labels - InitKeyBar()
  28.06.2000 tran
    - NT Console resize bug
      adding SetScreenPosition method
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

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
    char Name[NM];
    typedef class Frame inherited;
    /* $ 17.08.2001 KM
      Добавлено для поиска по AltF7. При редактировании найденного файла из
      архива для клавиши F2 сделать вызов ShiftF2.
    */
    int SaveToSaveAs;
    /* KM $ */

  public:
    FileViewer(const char *Name,int EnableSwitch=FALSE,int DisableHistory=FALSE,
               int DisableEdit=FALSE,long ViewStartPos=-1,char *PluginData=NULL,
               NamesList *ViewNamesList=NULL,int ToSaveAs=FALSE);
    FileViewer(const char *Name,int EnableSwitch,int DisableHistory,
               const char *Title,int X1,int Y1,int X2,int Y2);
    ~FileViewer();

  public:
    void Init(const char *Name,int EnableSwitch,int DisableHistory,
              long ViewStartPos,char *PluginData,NamesList *ViewNamesList,int ToSaveAs);
    /* $ 07.08.2000 SVS
       Функция инициализации KeyBar Labels
    */
    void InitKeyBar(void);
    /* SVS $ */
    int ProcessKey(int Key);
    int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
    void ShowConsoleTitle();
    void SetTempViewName(const char *Name);
    virtual void OnDestroy();

    virtual int GetTypeAndName(char *Type,char *Name);
    virtual const char *GetTypeName(){return "[FileView]";}; ///
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
};

#endif  // __FILEVIEWER_HPP__
