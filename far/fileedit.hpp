#ifndef __FILEEDITOR_HPP__
#define __FILEEDITOR_HPP__
/*
fileedit.hpp

Редактирование файла - надстройка над editor.cpp

*/

/* Revision: 1.14 31.05.2001 $ */

/*
Modify:
  31.05.2001 OT
    ! константы для ExitCode объединены с XC_QUIT и перенесены в farconst.cpp
  27.05.2001 DJ
    + константы для ExitCode
  15.05.2001 OT
    ! NWZ -> NFZ
  14.05.2001 OT
    - Борьба с F4 -> ReloadAgain
  12.05.2001 DJ
    ! отрисовка по OnChangeFocus перенесена в Frame
  10.05.2001 DJ
    + OnDestroy(), DisableHistory, DisableF6
  07.05.2001 DJ
    + добавлен NameList (пока только для передачи обратно во вьюер при
      повторном нажатии F6)
  06.05.2001 DJ
    ! перетрях #include
  07.05.2001 ОТ
    - Избавимся от "дублирования" ExitCode здесь и во Frame :)
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
#include "editor.hpp"
#include "keybar.hpp"

class NamesList;

class FileEditor:public Frame
{
  private:
    typedef class Frame inherited;
    void Show();
    void DisplayObject();
    int ProcessQuitKey();

    Editor FEdit;
    KeyBar EditKeyBar;

    /* $ 07.05.2001 DJ */
    NamesList *EditNamesList;
    /* DJ $ */
    char FileName[NM];
    char FullFileName[NM];
    char StartDir[NM];
    char NewTitle[NM];
    int FullScreen;
    /* $ 10.05.2001 DJ */
    int DisableHistory;
    int EnableF6;
    /* DJ $ */
  public:
    FileEditor(char *Name,int CreateNewFile,int EnableSwitch,
               int StartLine=-1,int StartChar=-1,int DisableHistory=FALSE,
               char *PluginData=NULL);
    FileEditor(char *Name,int CreateNewFile,int EnableSwitch,
               int StartLine,int StartChar,char *Title,
               int X1,int Y1,int X2,int Y2);
    /* $ 07.05.2001 DJ */
    virtual ~FileEditor();
    /* DJ $ */
    void Init(char *Name,int CreateNewFile,int EnableSwitch,
              int StartLine,int StartChar,int DisableHistory,char *PluginData);
    /* $ 07.08.2000 SVS
       Функция инициализации KeyBar Labels
    */
    void InitKeyBar(void);
    /* SVS $ */
    int ProcessKey(int Key);
    int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
    int GetTypeAndName(char *Type,char *Name);
    void ShowConsoleTitle();
    int IsFileChanged() {return(FEdit.IsFileChanged());};
    virtual int IsFileModified() {return(FEdit.IsFileModified());};
    /* $ 28.06.2000 tran
       NT Console resize - resize editor */
    virtual void SetScreenPosition();
    /* tran $ */
    virtual char *GetTypeName(){return "[FileEdit]";}; ///
    virtual int GetType() { return MODALTYPE_EDITOR; }

    /* $ 10.05.2001 DJ */
    virtual void OnDestroy();
    /* DJ $ */

    /* $ 07.05.2001 DJ */
    void SetNamesList (NamesList *Names);
    /* DJ $ */
    /* $ 10.05.2001 DJ */
    void SetEnableF6 (int AEnableF6) { EnableF6 = AEnableF6; }
    /* DJ $ */
    int GetCanLoseFocus(int DynamicMode=FALSE);

};

#endif	// __FILEEDITOR_HPP__
