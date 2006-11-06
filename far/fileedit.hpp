#ifndef __FILEEDITOR_HPP__
#define __FILEEDITOR_HPP__
/*
fileedit.hpp

Редактирование файла - надстройка над editor.cpp

*/

/* Revision: 1.51 06.06.2006 $ */

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
  FFILEEDIT_ISNEWFILE             = 0x00400000,
  FFILEEDIT_SAVEWQUESTIONS        = 0x00800000,  // сохранить без вопросов
  FFILEEDIT_OPENFAILED            = 0x01000000,  // файл открыть не удалось
  FFILEEDIT_DELETEONCLOSE         = 0x02000000,  // удалить в деструкторе файл вместе с каталогом (если тот пуст)
  FFILEEDIT_DELETEONLYFILEONCLOSE = 0x04000000,  // удалить в деструкторе только файл
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
    string strFileName;
    string strFullFileName;

    string strStartDir;
    char NewTitle[NM];

    string strTitle;
    string strPluginTitle;

    string strPluginData;

    FAR_FIND_DATA_EX FileInfo;
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

    //28.04.2005 AY: true когда редактор закрываеться (т.е. в деструкторе)
    bool bClosing;

  public:
    FileEditor(const wchar_t *Name,int CreateNewFile,int EnableSwitch,
               int StartLine=-1,int StartChar=-1,int DisableHistory=FALSE,
               const wchar_t *PluginData=NULL,int ToSaveAs=FALSE,
               int OpenModeExstFile=FEOPMODE_QUERY);
    /* $ 14.06.2002 IS
       DeleteOnClose стал int:
         0 - не удалять ничего
         1 - удалять файл и каталог
         2 - удалять только файл
    */
    FileEditor(const wchar_t *Name,int CreateNewFile,int EnableSwitch,
               int StartLine,int StartChar,const wchar_t *Title,
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
    int ReProcessKey(int Key,int CalledFromControl=TRUE);

  public:
    /* $ 14.06.2002 IS
       DeleteOnClose стал int:
         0 - не удалять ничего
         1 - удалять файл и каталог
         2 - удалять только файл
    */
    void Init(const wchar_t *Name,const wchar_t *Title,int CreateNewFile,int EnableSwitch,
              int StartLine,int StartChar,int DisableHistory,const wchar_t *PluginData,
              int ToSaveAs, int DeleteOnClose,int OpenModeExstFile);
    /* IS $ */

    void InitKeyBar(void);                            // $ 07.08.2000 SVS - Функция инициализации KeyBar Labels
    int ProcessKey(int Key);
    int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
    void ShowConsoleTitle();
    int IsFileChanged() {return(FEdit->IsFileChanged());};
    virtual int IsFileModified() {return(FEdit->IsFileModified());};

    void SetScreenPosition();                         // $ 28.06.2000 tran - NT Console resize - resize editor

    virtual int GetTypeAndName(string &strType, string &strName);
    virtual const wchar_t *GetTypeName(){return L"[FileEdit]";};
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

    int ReadFile(const wchar_t *Name,int &UserBreak);
    int SaveFile(const wchar_t *Name,int Ask,int TextFormat,int SaveAs);
    int EditorControl(int Command,void *Param);
    void SetPluginTitle(const wchar_t *PluginTitle);
    void SetTitle(const wchar_t *Title);
    virtual void GetTitle(string &Title,int SubLen=-1,int TruncSize=0);
    BOOL SetFileName(const wchar_t *NewFileName);
    int ProcessEditorInput(INPUT_RECORD *Rec);
    void SetLockEditor(BOOL LockMode);
    BOOL IsFullScreen(){return Flags.Check(FFILEEDIT_FULLSCREEN);}
    void ChangeEditKeyBar();
    void ShowStatus();

    DWORD GetFileAttributes(const wchar_t *Name);                 // $ 13.02.2001 IS - Обертка вокруг одноименной функции из win32 api

    void SetPluginData(const wchar_t *PluginData);
    const wchar_t *GetPluginData(void){return (const wchar_t*)strPluginData;};

    void GetEditorOptions(struct EditorOptions& EdOpt);
    void SetEditorOptions(struct EditorOptions& EdOpt);

    bool LoadFromCache (EditorCacheParams *pp);
    void SaveToCache ();
};

#endif  // __FILEEDITOR_HPP__
