#ifndef __FILEEDITOR_HPP__
#define __FILEEDITOR_HPP__
/*
fileedit.hpp

Редактирование файла - надстройка над editor.cpp

*/

/* Revision: 1.38 22.04.2004 $ */

/*
Modify:
  22.04.2004 SVS
    ! Константы SAVEFILE_* уехали из editor.hpp в fileedit.hpp
  04.11.2003 SKV
    + FileAttributesModified - признак того, что надо восстанавливать аттрибут.
  09.10.2003 SVS
    + FFILEEDIT_SAVEWQUESTIONS - сохранить без вопросов
  25.07.2003 SVS
    ! новый член класса - DWORD SysErrorCode.
  11.12.2002 SVS
    ! Некоторые переменные класса заменены на флаги
  08.11.2002 SVS
    ! Editor::PluginData уехал в FileEditor::PluginData
    ! Editor::SetPluginData() уехал в FileEditor::SetPluginData()
    + GetPluginData()
    + UpdateFileList()
    ! Очередная порция отучения Editor от понятия "файл"
  04.09.2002 SVS
    + GetLastInfo() & FileInfo - информация о файле
  25.06.2002 SVS
    + IsFullScreen()
    ! классу Editor нафиг ненужен кейбар - это привелегия FileEditor
  14.06.2002 IS
    ! DeleteOnClose стал int:
      0 - не удалять ничего
      1 - удалять файл и каталог
      2 - удалять только файл
  22.05.2002 SVS
    + SetTitle()
    ! В Init добавлен вторым параметром - Title
    ! FEdit из объекта превращается в указатель - контроля с нашей стороны
      поболее будет
  13.05.2002 VVM
    + Перерисуем заголовок консоли после позиционирования на файл.
  18.03.2002 SVS
    + SetLockEditor() - возможноть программно лочить редактор
  15.01.2002 SVS
    - Метод Show() не может быть в привате, т.к. в ScreenObject он в
      паблик-секции
    ! Начинаем обучение класса новому слову "Файл", он конечно и сам знает,
      но не все (начало переноса кода ответственного за работу с файлом из
      Editor в FileEditor)
    + SetFileName() - установить переменные в имя редактируемого файла
    + ReadFile() - постепенно сюды переносить код из Editor::ReadFile
    + SaveFile() - постепенно сюды переносить код из Editor::SaveFile
    ! ProcessEditorInput ушел в FileEditor (в диалога плагины не...)
  10.01.2002 SVS
    + FEOPMODE_NEWIFOPEN
    ! FirstSave у ProcessQuitKey() - как параметр.
  26.12.2001 SVS
    + внедрение FEOPMODE_*
  25.12.2001 SVS
    + ResizeConsole()
  08.12.2001 OT
    - Bugzilla #144 Заходим в архив, F4 на файле, Ctrl-F10.
  02.11.2001 SVS
    ! возвращаемое значение у GetTypeName() - модификатор const
  10.10.2001 IS
    + DeleteOnClose
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

/* $ 27.05.2001 DJ
   коды возврата Editor::SaveFile()
*/
enum {
    SAVEFILE_ERROR   = 0,         // пытались сохранять, не получилось
    SAVEFILE_SUCCESS = 1,         // либо успешно сохранили, либо сохранять было не надо
    SAVEFILE_CANCEL  = 2          // сохранение отменено, редактор не закрывать
};
/* DJ $ */

enum FEOPMODEEXISTFILE{
  FEOPMODE_QUERY        =0,
  FEOPMODE_NEWIFOPEN    =1,
  FEOPMODE_USEEXISTING  =2,
  FEOPMODE_BREAKIFOPEN  =3,
};

enum FFILEEDIT_FLAGS{
  FFILEEDIT_NEW            = 0x00010000,  // Этот файл СОВЕРШЕННО! новый или его успели стереть! Нету такого и все тут.
  FFILEEDIT_REDRAWTITLE    = 0x00020000,  // Нужно редравить заголовок?
  FFILEEDIT_FULLSCREEN     = 0x00040000,  // Полноэкранный режим?
  FFILEEDIT_DISABLEHISTORY = 0x00080000,  // Запретить запись в историю?
  FFILEEDIT_ENABLEF6       = 0x00100000,  // Переключаться во вьювер можно?
  FFILEEDIT_SAVETOSAVEAS   = 0x00200000,  // $ 17.08.2001 KM  Добавлено для поиска по AltF7.
                                          //   При редактировании найденного файла из архива для
                                          //   клавиши F2 сделать вызов ShiftF2.
  FFILEEDIT_ISNEWFILE      = 0x00400000,
  FFILEEDIT_SAVEWQUESTIONS = 0x00800000,  // сохранить без вопросов
};


class FileEditor:public Frame
{
  private:
    typedef class Frame inherited;

    Editor *FEdit;
    KeyBar EditKeyBar;

    /* $ 07.05.2001 DJ */
    NamesList *EditNamesList;
    /* DJ $ */
    char FileName[NM];
    char FullFileName[NM];
    char StartDir[NM];
    char NewTitle[NM];

    char Title[512];
    char PluginTitle[512];
    char PluginData[NM*2];

    WIN32_FIND_DATA FileInfo;
    /* $ 13.02.2001 IS
         Сюда запомним буквы атрибутов, чтобы не вычислять их много раз
    */
    char AttrStr[4];
    /* IS $ */
    /* $ 12.02.2001 IS
         сюда запомним атрибуты файла при открытии, пригодятся где-нибудь...
    */
    DWORD FileAttributes;
    /* IS $ */
    /* $ 04.11.2003 SKV
      надо ли восстанавливать аттрибуты при save
    */
    BOOL  FileAttributesModified;
    /* SKV $ */
    DWORD SysErrorCode;

  public:
    FileEditor(const char *Name,int CreateNewFile,int EnableSwitch,
               int StartLine=-1,int StartChar=-1,int DisableHistory=FALSE,
               char *PluginData=NULL,int ToSaveAs=FALSE,
               int OpenModeExstFile=FEOPMODE_QUERY);
    /* $ 14.06.2002 IS
       DeleteOnClose стал int:
         0 - не удалять ничего
         1 - удалять файл и каталог
         2 - удалять только файл
    */
    FileEditor(const char *Name,int CreateNewFile,int EnableSwitch,
               int StartLine,int StartChar,const char *Title,
               int X1,int Y1,int X2,int Y2, int DisableHistory,
               int DeleteOnClose=0,
               int OpenModeExstFile=FEOPMODE_QUERY);
    /* IS $ */
    /* $ 07.05.2001 DJ */
    virtual ~FileEditor();
    /* DJ $ */

  private:
    void DisplayObject();
    int  ProcessQuitKey(int FirstSave,BOOL NeedQuestion=TRUE);
    BOOL GetLastInfo(const char *Name,WIN32_FIND_DATA *FInfo);
    BOOL UpdateFileList();
    /* $ 10.10.2001 IS установка DeleteOnClose */
    /* $ 14.06.2002 IS
        DeleteOnClose стал int:
          0 - не удалять ничего
          1 - удалять файл и каталог
          2 - удалять только файл
    */
    void SetDeleteOnClose(int NewMode);
    /* IS 14.06.2002 */
    /* IS 10.10.2001 */

  public:
    /* $ 14.06.2002 IS
       DeleteOnClose стал int:
         0 - не удалять ничего
         1 - удалять файл и каталог
         2 - удалять только файл
    */
    void Init(const char *Name,const char *Title,int CreateNewFile,int EnableSwitch,
              int StartLine,int StartChar,int DisableHistory,char *PluginData,
              int ToSaveAs, int DeleteOnClose,int OpenModeExstFile);
    /* IS $ */

    void InitKeyBar(void);                            // $ 07.08.2000 SVS - Функция инициализации KeyBar Labels
    int ProcessKey(int Key);
    int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
    void ShowConsoleTitle();
    int IsFileChanged() {return(FEdit->IsFileChanged());};
    virtual int IsFileModified() {return(FEdit->IsFileModified());};

    void SetScreenPosition();                         // $ 28.06.2000 tran - NT Console resize - resize editor

    virtual int GetTypeAndName(char *Type,char *Name);
    virtual const char *GetTypeName(){return "[FileEdit]";};
    virtual int GetType() { return MODALTYPE_EDITOR; }


    virtual void OnDestroy();                         // $ 10.05.2001 DJ
    void SetNamesList (NamesList *Names);             // $ 07.05.2001 DJ

    int GetCanLoseFocus(int DynamicMode=FALSE);

    int FastHide();                                   // $ OT - Введена для нужд CtrlAltShift

    void SetEnableF6 (int AEnableF6) { Flags.Change(FFILEEDIT_ENABLEF6,AEnableF6); InitKeyBar(); }  // $ 10.05.2001 DJ
    /* $ 17.08.2001 KM
      Добавлено для поиска по AltF7. При редактировании найденного файла из
      архива для клавиши F2 сделать вызов ShiftF2.
    */
    void SetSaveToSaveAs(int ToSaveAs) { Flags.Change(FFILEEDIT_SAVETOSAVEAS,ToSaveAs); InitKeyBar(); }
    /* KM $ */

    /* $ 08.12.2001 OT
      возвращает признак того, является ли файл временным
      используется для принятия решения переходить в каталог по CtrlF10*/
    BOOL isTemporary();
    void ResizeConsole();
    void Show();

    int ReadFile(const char *Name,int &UserBreak);
    int SaveFile(const char *Name,int Ask,int TextFormat,int SaveAs);
    int EditorControl(int Command,void *Param);
    void SetPluginTitle(const char *PluginTitle);
    void SetTitle(const char *Title);
    BOOL SetFileName(const char *NewFileName);
    int ProcessEditorInput(INPUT_RECORD *Rec);
    void SetLockEditor(BOOL LockMode);
    BOOL IsFullScreen(){return Flags.Check(FFILEEDIT_FULLSCREEN);}
    void ChangeEditKeyBar();
    void ShowStatus();

    DWORD GetFileAttributes(LPCTSTR);                 // $ 13.02.2001 IS - Обертка вокруг одноименной функции из win32 api

    void SetPluginData(char *PluginData);
    char *GetPluginData(void){return PluginData;};
};

#endif  // __FILEEDITOR_HPP__
