/*
fileedit.cpp

Редактирование файла - надстройка над editor.cpp

*/

/* Revision: 1.72 26.11.2001 $ */

/*
Modify:
  26.11.2001 VVM
    ! Использовать полное имя файла при CTRL+F10
  14.11.2001 SVS
    ! Ctrl-F10 не выходит, а только позиционирует
  02.11.2001 IS
    - отрицательные координаты левого верхнего угла заменяются на нулевые
  29.10.2001 IS
    + Обновим настройки "сохранять позицию файла" и "сохранять закладки" после
      смены настроек по alt-shift-f9.
  28.10.2001 SVS
    - Не чистится экран после отмены открытия редактора
  19.10.2001 OT
    - Исправление ошибки HyperViewer
  15.10.2001 SVS
    + _KEYMACRO()
  10.10.2001 IS
    + обработка DeleteOnClose
  04.10.2001 OT
    - исправлен баг в fileEditor, когда на вопрос How to open this file? ответить Current не удалался созданный, уже не нужный никому пустой фрейм.
  27.09.2001 IS
    - Левый размер при использовании strncpy
  26.09.2001 SVS
    - Бага с Ctrl-F10, когда указывался корень диска.
  08.09.2001 IS
    + Дополнительный параметр у второго конструктора: DisableHistory
  17.08.2001 KM
    + Добавлена функция SetSaveToSaveAs для установки дефолтной реакции
      на клавишу F2 в вызов ShiftF2 для поиска, в случае редактирования
      найденного файла из архива.
    ! Изменён конструктор и функция Init для работы SaveToSaveAs.
    - Убрана в KeyBar надпись на клавишу F12 при CanLoseFocus=TRUE
  01.08.2001 tran
    - bug с Shift-F2, существующее имя, esc. F2
  23.07.2001 SVS
    ! Изменен порядок кнопок  MNewOpen и MReload
  22.07.2001 SVS
    + Добавлен хелп для месага про "релоад"
    ! Имя файла в месага про "релоад" усекается.
  11.07.2001 OT
    ! Перенос CtrlAltShift в Manager
  06.07.2001 IS
    - При создании файла с нуля так же посылаем плагинам событие EE_READ, дабы
      не нарушать однообразие.
  25.06.2001 IS
    ! Внедрение const
  14.06.2001 OT
    ! "Бунт" ;-)
  07.06.2001 IS
    - Баг (сохранение файла): нужно сначала убирать пробелы, а только потом
      кавычки
  06.06.2001 IS
    - мелкий фикс моего последнего патча
  05.06.2001 IS
    + посылаем подальше всех, кто пытается отредактировать каталог
  27.05.2001 DJ
    ! используются константы для кодов возврата
  26.05.2001 OT
    - Выпрямление логики вызовов в NFZ
    - Редактор возможно запускать в модальном режиме
  23.05.2001 OT
    + Опция AllowReedit
    ! Исправление бага из-за которго файл трапался по отмене ReloadAgain
    - Выпрямление логики вызовов в NFZ
  19.05.2001 DJ
    ! лечим последствия NFZ
  15.05.2001 OT
    ! NWZ -> NFZ
  14.05.2001 OT
    - Борьба с F4 -> ReloadAgain
  12.05.2001 DJ
    ! отрисовка по OnChangeFocus перенесена в Frame
  11.05.2001 OT
    ! Отрисовка Background
  10.05.2001 DJ
    + OnDestroy() (не работало добавление во view/edit history)
    + FileEditor::DisableHistory, DisableF6
    + Alt-F11 - показать историю
    + при Ctrl-F10 переключаемся на панели
  07.05.2001 SVS
    ! SysLog(); -> _D(SysLog());
  07.05.2001 DJ
    + добавлен NameList (пока только для передачи обратно во вьюер при
      повторном нажатии F6)
    - кейбар не обновлялся
  06.05.2001 DJ
    ! перетрях #include
    + обработка F6
  07.05.2001 ОТ
    - Избавимся от "дублирования" ExitCode здесь и во Frame :)
  06.05.2001 ОТ
    ! Переименование Window в Frame :)
  05.05.2001 DJ
    + перетрях NWZ
  04.05.2001 DJ
    - В процессе наложения 623-го был выкинут патч 616:
      "не передаем KEY_MACRO* в ProcessEditorInput()"
      Вернул его обратно.
  29.04.2001 ОТ
    + Внедрение NWZ от Третьякова
  28.04.2001 VVM
    + KeyBar тоже умеет обрабатывать клавиши.
  19.04.2001 SVS
    ! Диалог SaveAs некорректно работал при нажатии Ctrl-Enter
  10.04.2001 IS
    ! Не делаем SetCurDir при ctrl-f10, если нужный путь уже есть на открытых
      панелях, тем самым добиваемся того, что выделение с элементов
      панелей не сбрасывается.
  05.04.2001 SVS
    + Добавлен вызов топика "FileSaveAs" для диалога SaveAs
  28.03.2001 SVS
    ! Передадим в SaveFile новый параметр - SaveAs?
  22.03.2001 SVS
    - "Залипание" кейбара после исполнения макроса
  18.03.2001 IS
    ! Поменял местами проверку при открытии на "только для чтения" и
      "уже открыт", тем самым избавились от ситуации, когда задавался вопрос
      "вы уверены, что хотите редактировать r/o файл" для ужЕ открытых файлов.
  01.03.2001 IS
    - Баг: не учитывалось, закрылся ли файл на самом деле по ctrl-f10
  27.02.2001 SVS
    + Добавки по поводу базы вывода.
  26.02.2001 IS
    + В прошлый раз я не все доделал :(
      Теперь на самом деле большинство переменных, редактируемых в редакторе по
      alt-shift-f9, локальные, кроме настроек внешнего редактора и опций
      "Сохранять позицию файла", "Сохранять закладки"
  21.02.2001 IS
    + При обработке alt-shift-f9 работаем с локальными переменными
  15.02.2001 IS
    + Обновим "постоянные блоки" и "del удаляет блоки"
      при смене настроек редактора по AltShiftF9
  15.02.2001 IS
    + Обновим размер табуляции и режим "Пробелы вместо табуляции" при смене
      настроек редактора по AltShiftF9
  01.02.2001 IS
    ! Открываем по F6 вьюер с указанием длинного имени файла, а не короткого
  03.01.2001 SVS
    ! для KEY_ALTSHIFTF9 забыли сделать Show()
  19.12.2000 SVS
    + Alt-Shift-F9 - Вызов диалога настроек (с подачи IS)
  16.12.2000 tran 1.15
    ! Ctrl-F10 смотрит на пассивную панель
  15.12.2000 SVS
    - Shift-F4, новый файл. Выдает сообщение :-(
  03.12.2000 SVS
    + "Если файл имеет атрибут ReadOnly..." здесь System и Hidden - задаются
      отдельно.
  29.11.2000 SVS
    + Если файл имеет атрибут ReadOnly или System или Hidden,
      И параметр на запрос выставлен, то сначала спросим.
  03.11.2000 OT
    ! Введение проверки возвращаемого значения
  02.11.2000 OT
    ! Введение проверки на длину буфера, отведенного под имя файла.
  16.10.2000 SVS
    ! Отмена 1.08 (#229)
  13.10.2000 tran 1.08
    ! код возврата опредеяется по IsFileModified вместо IsFileChanged()
  27.09.2000 SVS
    + Печать файла/блока с использованием плагина PrintMan
    ! Ctrl-Alt-Shift - реагируем, если надо.
  27.09.2000 SKV
    + Для правильного функционирования макро с Ctrl-O делается FEdit.Hide()
  24.08.2000 SVS
    + Добавляем реакцию показа бакграунда на клавишу CtrlAltShift
  07.08.2000 SVS
    + добавил названия расширенных функциональных клавиш
    + Функция инициализации KeyBar Labels - InitKeyBar()
  21.07.2000 SKV
    + выход с позиционированием на редактируемом файле по CTRLF10
  29.06.2000 tran
    + названия всех функциональных клавиш
  28.06.2000 tran
    - (NT Console resize bug)
      adding FileEditor::SetScreenPosition
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#include "headers.hpp"
#pragma hdrstop

#include "fileedit.hpp"
#include "global.hpp"
#include "fn.hpp"
#include "lang.hpp"
#include "keys.hpp"
#include "ctrlobj.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "dialog.hpp"
#include "fileview.hpp"
#include "ctrlobj.hpp"
#include "manager.hpp"
#include "namelist.hpp"
#include "history.hpp"
#include "cmdline.hpp"
#include "savescr.hpp"

FileEditor::FileEditor(const char *Name,int CreateNewFile,int EnableSwitch,
                       int StartLine,int StartChar,int DisableHistory,
                       char *PluginData,int ToSaveAs)
{
  ScreenObject::SetPosition(0,0,ScrX,ScrY);
  FullScreen=TRUE;
  Init(Name,CreateNewFile,EnableSwitch,StartLine,StartChar,
       DisableHistory,PluginData,ToSaveAs,FALSE);
}


FileEditor::FileEditor(const char *Name,int CreateNewFile,int EnableSwitch,
            int StartLine,int StartChar,const char *Title,
            int X1,int Y1,int X2,int Y2,int DisableHistory, BOOL DeleteOnClose)
{
  /* $ 02.11.2001 IS
       отрицательные координаты левого верхнего угла заменяются на нулевые
  */
  if(X1<0) X1=0;
  if(Y1<0) Y1=0;
  /* IS $ */
  ScreenObject::SetPosition(X1,Y1,X2,Y2);
  FullScreen=(X1==0 && Y1==0 && X2==ScrX && Y2==ScrY);
  FEdit.SetTitle(Title);
  Init(Name,CreateNewFile,EnableSwitch,StartLine,StartChar,DisableHistory,"",
       FALSE,DeleteOnClose);
}


void FileEditor::Init(const char *Name,int CreateNewFile,int EnableSwitch,
                      int StartLine,int StartChar,int DisableHistory,
                      char *PluginData,int ToSaveAs,BOOL DeleteOnClose)
{
  /* $ 07.05.2001 DJ */
  EditNamesList = NULL;
  KeyBarVisible = TRUE;
  /* DJ $ */
  /* $ 10.05.2001 DJ */
  FileEditor::DisableHistory = DisableHistory;
  EnableF6 = EnableSwitch;
  /* DJ $ */
  /* $ 17.08.2001 KM
    Добавлено для поиска по AltF7. При редактировании найденного файла из
    архива для клавиши F2 сделать вызов ShiftF2.
  */
  SaveToSaveAs=ToSaveAs;
  /* KM $ */
  if (*Name==0)
    return;
  FEdit.SetPluginData(PluginData);
  FEdit.SetHostFileEditor(this);
  _OT(SysLog("Editor;:Editor(), EnableSwitch=%i",EnableSwitch));
  SetCanLoseFocus(EnableSwitch);
  GetCurrentDirectory(sizeof(StartDir),StartDir);
  strcpy(FileName,Name);
  if (sizeof(FullFileName)<=ConvertNameToFull(FileName,FullFileName, sizeof(FullFileName))) {
    return;
  }

  /*$ 11.05.2001 OT */
  if (EnableSwitch)
  {
    int FramePos=FrameManager->FindFrameByFile(MODALTYPE_EDITOR,FullFileName);
    if (FramePos!=-1)
    {
      int SwitchTo=FALSE;
      if (!(*FrameManager)[FramePos]->GetCanLoseFocus(TRUE) ||
          Opt.Confirm.AllowReedit)
      {
        char MsgFullFileName[NM];
        strcpy(MsgFullFileName,FullFileName);
        SetMessageHelp("EditorReload");
        int MsgCode=Message(0,3,MSG(MEditTitle),
              TruncPathStr(MsgFullFileName,ScrX-16),
              MSG(MAskReload),
              MSG(MCurrent),MSG(MNewOpen),MSG(MReload));
        switch(MsgCode)
        {
          case 0:
            SwitchTo=TRUE;
            FrameManager->DeleteFrame(this);
            break;
          case 2:
            FrameManager->DeleteFrame(FramePos);
            SetExitCode(-2);
            break;
          case 1:
            SwitchTo=FALSE;
            break;
          default:
            FrameManager->DeleteFrame(this);
            SetExitCode(XC_QUIT);
            return;
        }
      }
      else
      {
        SwitchTo=TRUE;
      }
      if (SwitchTo)
      {
        FrameManager->ActivateFrame(FramePos);
        SetExitCode(TRUE);
        return ;
      }
    }
  }
  /* 11.05.2001 OT $*/

  /* $ 29.11.2000 SVS
     Если файл имеет атрибут ReadOnly или System или Hidden,
     И параметр на запрос выставлен, то сначала спросим.
  */
  /* $ 03.12.2000 SVS
     System или Hidden - задаются отдельно
  */
  /* $ 15.12.2000 SVS
    - Shift-F4, новый файл. Выдает сообщение :-(
  */
  DWORD FAttr=GetFileAttributes(Name);
  /* $ 05.06.2001 IS
     + посылаем подальше всех, кто пытается отредактировать каталог
  */
  if(FAttr!=-1 && FAttr&FILE_ATTRIBUTE_DIRECTORY)
  {
    Message(MSG_WARNING,1,MSG(MEditTitle),MSG(MEditCanNotEditDirectory),
            MSG(MOk));
    return;
  }
  /* IS $ */
  if((Opt.EditorReadOnlyLock&2) &&
     FAttr != -1 &&
     (FAttr &
        (FILE_ATTRIBUTE_READONLY|
           /* Hidden=0x2 System=0x4 - располагаются во 2-м полубайте,
              поэтому применяем маску 0110.0000 и
              сдвигаем на свое место => 0000.0110 и получаем
              те самые нужные атрибуты  */
           ((Opt.EditorReadOnlyLock&0x60)>>4)
        )
     )
  )
  /* SVS $ */
  {
    if(Message(MSG_WARNING,2,MSG(MEditTitle),Name,MSG(MEditRSH),
                             MSG(MEditROOpen),MSG(MYes),MSG(MNo)))
    {
      //SetLastError(ERROR_ACCESS_DENIED);
      return;
    }
  }
  /* SVS 03.12.2000 $ */
  /* SVS $ */

  FEdit.SetPosition(X1,Y1,X2,Y2-1);
  FEdit.SetStartPos(StartLine,StartChar);
  FEdit.SetDeleteOnClose(DeleteOnClose);
  int UserBreak;
  /* $ 06.07.2001 IS
     При создании файла с нуля так же посылаем плагинам событие EE_READ, дабы
     не нарушать однообразие.
  */
  if (!FEdit.ReadFile(FileName,UserBreak))
  {
    if(!CreateNewFile || UserBreak)
    {
      if (UserBreak!=1)
      {
        Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MEditTitle),MSG(MEditCannotOpen),FileName,MSG(MOk));
        ExitCode=0;
      }
      else
        ExitCode=XC_LOADING_INTERRUPTED;
      CtrlObject->Cp()->Redraw();
      return;
    }
    CtrlObject->Plugins.CurEditor=&FEdit;
    CtrlObject->Plugins.ProcessEditorEvent(EE_READ,NULL);
  }
  /* IS $ */
  ShowConsoleTitle();
  EditKeyBar.SetOwner(this);
  EditKeyBar.SetPosition(X1,Y2,X2,Y2);

  /* $ 07.08.2000 SVS
    ! Код, касаемый KeyBar вынесен в отдельную функцию */
  InitKeyBar();
  /* SVS $*/

  MacroMode=MACRO_EDITOR;
/*& OT */
  if (EnableSwitch){
    FrameManager->InsertFrame(this);
  } else {
    FrameManager->ExecuteFrame(this);
  }
/* OT &*/

}

/* $ 07.08.2000 SVS
  Функция инициализации KeyBar Labels
*/
void FileEditor::InitKeyBar(void)
{
  /* $ 29.06.2000 tran
     добавил названия всех функциональных клавиш */
  /* $ 10.05.2001 DJ
     смотрим на EnableF6 вместо CanLoseFocus
  */
  char *FEditKeys[]={MSG(MEditF1),(SaveToSaveAs)?MSG(MEditShiftF2):MSG(MEditF2),MSG(MEditF3),MSG(MEditF4),MSG(MEditF5),EnableF6 ? MSG(MEditF6):"",MSG(MEditF7),MSG(MEditF8),MSG(MEditF9),MSG(MEditF10),MSG(MEditF11),(GetCanLoseFocus())?MSG(MEditF12):""};
  /* DJ $ */
  char *FEditShiftKeys[]={MSG(MEditShiftF1),MSG(MEditShiftF2),MSG(MEditShiftF3),MSG(MEditShiftF4),MSG(MEditShiftF5),MSG(MEditShiftF6),MSG(MEditShiftF7),MSG(MEditShiftF8),MSG(MEditShiftF9),MSG(MEditShiftF10),MSG(MEditShiftF11),MSG(MEditShiftF12)};
  char *FEditAltKeys[]={MSG(MEditAltF1),MSG(MEditAltF2),MSG(MEditAltF3),MSG(MEditAltF4),MSG(MEditAltF5),MSG(MEditAltF6),MSG(MEditAltF7),MSG(MEditAltF8),MSG(MEditAltF9),MSG(MEditAltF10),MSG(MEditAltF11),MSG(MEditAltF12)};
  char *FEditCtrlKeys[]={MSG(MEditCtrlF1),MSG(MEditCtrlF2),MSG(MEditCtrlF3),MSG(MEditCtrlF4),MSG(MEditCtrlF5),MSG(MEditCtrlF6),MSG(MEditCtrlF7),MSG(MEditCtrlF8),MSG(MEditCtrlF9),MSG(MEditCtrlF10),MSG(MEditCtrlF11),MSG(MEditCtrlF12)};

  if(CtrlObject->Plugins.FindPlugin(SYSID_PRINTMANAGER) == -1)
    FEditAltKeys[5-1]="";
  /* tran $ */
  /* $ 07.08.2000 SVS
     добавил названия расширенных функциональных клавиш */
  char *FEditAltShiftKeys[]={MSG(MEditAltShiftF1),MSG(MEditAltShiftF2),MSG(MEditAltShiftF3),MSG(MEditAltShiftF4),MSG(MEditAltShiftF5),MSG(MEditAltShiftF6),MSG(MEditAltShiftF7),MSG(MEditAltShiftF8),MSG(MEditAltShiftF9),MSG(MEditAltShiftF10),MSG(MEditAltShiftF11),MSG(MEditAltShiftF12)};
  char *FEditCtrlShiftKeys[]={MSG(MEditCtrlShiftF1),MSG(MEditCtrlShiftF2),MSG(MEditCtrlShiftF3),MSG(MEditCtrlShiftF4),MSG(MEditCtrlShiftF5),MSG(MEditCtrlShiftF6),MSG(MEditCtrlShiftF7),MSG(MEditCtrlShiftF8),MSG(MEditCtrlShiftF9),MSG(MEditCtrlShiftF10),MSG(MEditCtrlShiftF11),MSG(MEditCtrlShiftF12)};
  char *FEditCtrlAltKeys[]={MSG(MEditCtrlAltF1),MSG(MEditCtrlAltF2),MSG(MEditCtrlAltF3),MSG(MEditCtrlAltF4),MSG(MEditCtrlAltF5),MSG(MEditCtrlAltF6),MSG(MEditCtrlAltF7),MSG(MEditCtrlAltF8),MSG(MEditCtrlAltF9),MSG(MEditCtrlAltF10),MSG(MEditCtrlAltF11),MSG(MEditCtrlAltF12)};
  /* SVS $*/

  EditKeyBar.Set(FEditKeys,sizeof(FEditKeys)/sizeof(FEditKeys[0]));
  EditKeyBar.SetShift(FEditShiftKeys,sizeof(FEditShiftKeys)/sizeof(FEditShiftKeys[0]));
  EditKeyBar.SetAlt(FEditAltKeys,sizeof(FEditAltKeys)/sizeof(FEditAltKeys[0]));
  EditKeyBar.SetCtrl(FEditCtrlKeys,sizeof(FEditCtrlKeys)/sizeof(FEditCtrlKeys[0]));

  /* $ 07.08.2000 SVS
     добавил названия расширенных функциональных клавиш */
  EditKeyBar.SetCtrlAlt(FEditCtrlAltKeys,sizeof(FEditCtrlAltKeys)/sizeof(FEditCtrlAltKeys[0]));
  EditKeyBar.SetCtrlShift(FEditCtrlShiftKeys,sizeof(FEditCtrlShiftKeys)/sizeof(FEditCtrlShiftKeys[0]));
  EditKeyBar.SetAltShift(FEditAltShiftKeys,sizeof(FEditAltShiftKeys)/sizeof(FEditAltShiftKeys[0]));
  /* SVS $ */

  EditKeyBar.Show();
  SetKeyBar(&EditKeyBar);
  FEdit.SetEditKeyBar(&EditKeyBar);
}
/* SVS $ */

/* $ 07.05.2001 DJ
   в деструкторе грохаем EditNamesList, если он был создан, а в SetNamesList()
   создаем EditNamesList и копируем туда значения
*/

FileEditor::~FileEditor()
{
  _OT(SysLog("[%p] FileEditor::~FileEditor()",this));
  if (EditNamesList)
    delete EditNamesList;
}

void FileEditor::SetNamesList (NamesList *Names)
{
  if (EditNamesList == NULL)
    EditNamesList = new NamesList;
  Names->MoveData (EditNamesList);
}

/* DJ $ */

void FileEditor::Show()
{
  if (FullScreen)
  {
    EditKeyBar.SetPosition(0,ScrY,ScrX,ScrY);
    EditKeyBar.Redraw();
    ScreenObject::SetPosition(0,0,ScrX,ScrY-1);
    FEdit.SetPosition(0,0,ScrX,ScrY-1);
  }
  ScreenObject::Show();
}


void FileEditor::DisplayObject()
{
  FEdit.Show();
}


int FileEditor::ProcessKey(int Key)
{
  switch(Key)
  {
    /* $ 24.08.2000 SVS
       + Добавляем реакцию показа бакграунда на клавишу CtrlAltShift
    */
    case KEY_CTRLO:
      /*$ 27.09.2000 skv
        To prevent redraw in macro with Ctrl-O
      */
      FEdit.Hide();
      /* skv$*/
      FrameManager->ShowBackground();
      WaitKey(-1);
      Show();
      return(TRUE);
/* $ KEY_CTRLALTSHIFTPRESS унесено в manager OT */
    case KEY_F2:
    case KEY_SHIFTF2:
      {
        static int TextFormat=0;
        int NameChanged=FALSE;
        if (Key==KEY_SHIFTF2 || SaveToSaveAs)
        {
          const char *HistoryName="NewEdit";
          static struct DialogData EditDlgData[]=
          {
            /* 0 */ DI_DOUBLEBOX,3,1,72,11,0,0,0,0,(char *)MEditTitle,
            /* 1 */ DI_TEXT,5,2,0,0,0,0,DIF_SHOWAMPERSAND,0,(char *)MEditSaveAs,
            /* 2 */ DI_EDIT,5,3,70,3,1,(DWORD)HistoryName,DIF_HISTORY,0,"",
            /* 3 */ DI_TEXT,3,4,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
            /* 4 */ DI_TEXT,5,5,0,0,0,0,0,0,(char *)MEditSaveAsFormatTitle,
            /* 5 */ DI_RADIOBUTTON,5,6,0,0,0,0,DIF_GROUP,0,(char *)MEditSaveOriginal,
            /* 6 */ DI_RADIOBUTTON,5,7,0,0,0,0,0,0,(char *)MEditSaveDOS,
            /* 7 */ DI_RADIOBUTTON,5,8,0,0,0,0,0,0,(char *)MEditSaveUnix,
            /* 8 */ DI_TEXT,3,9,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
            /* 9 */ DI_BUTTON,0,10,0,0,0,0,DIF_CENTERGROUP,1,(char *)MOk,
            /*10 */ DI_BUTTON,0,10,0,0,0,0,DIF_CENTERGROUP,0,(char *)MCancel,
          };
          MakeDialogItems(EditDlgData,EditDlg);
          strcpy(EditDlg[2].Data,FileName);
          EditDlg[5].Selected=EditDlg[6].Selected=EditDlg[7].Selected=0;
          EditDlg[5+TextFormat].Selected=TRUE;
          Dialog Dlg(EditDlg,sizeof(EditDlg)/sizeof(EditDlg[0]));
          Dlg.SetPosition(-1,-1,76,13);
          Dlg.SetHelp("FileSaveAs");
          Dlg.Process();
          if (Dlg.GetExitCode()!=9 || *EditDlg[2].Data==0)
            return(FALSE);
          /* $ 07.06.2001 IS
             - Баг: нужно сначала убирать пробелы, а только потом кавычки
          */
          RemoveTrailingSpaces(EditDlg[2].Data);
          Unquote(EditDlg[2].Data);
          /* IS $ */

          NameChanged=LocalStricmp(EditDlg[2].Data,FileName)!=0;
          /* $ 01.08.2001 tran
             этот кусок перенесен повыше и вместо FileName
             используеся EditDlg[2].Data */
          if (NameChanged && GetFileAttributes(EditDlg[2].Data)!=0xFFFFFFFF)
            if (Message(MSG_WARNING,2,MSG(MEditTitle),EditDlg[2].Data,MSG(MEditExists),
                         MSG(MEditOvr),MSG(MYes),MSG(MNo))!=0)
              return(TRUE);
          /* tran $ */

          strcpy(FileName,EditDlg[2].Data);
//          ConvertNameToFull(FileName,FullFileName, sizeof(FullFileName));
          if (ConvertNameToFull(FileName,FullFileName, sizeof(FullFileName)) >= sizeof(FullFileName)){
            return FALSE;
          }
          if (EditDlg[5].Selected)
            TextFormat=0;
          if (EditDlg[6].Selected)
            TextFormat=1;
          if (EditDlg[7].Selected)
            TextFormat=2;
        }
        ShowConsoleTitle();
        chdir(StartDir);


        while (FEdit.SaveFile(FileName,0,Key==KEY_SHIFTF2 ? TextFormat:0,Key==KEY_SHIFTF2)==0)
          if (Message(MSG_WARNING|MSG_ERRORTYPE,2,MSG(MEditTitle),MSG(MEditCannotSave),
                      FileName,MSG(MRetry),MSG(MCancel))!=0)
            break;
      }
      return(TRUE);
    case KEY_F6:
      /* $ 10.05.2001 DJ
         используем EnableF6
      */
      if (EnableF6 &&
          (FEdit.IsFileChanged() || GetFileAttributes(FullFileName)!=0xFFFFFFFF))
      {
        long FilePos=FEdit.GetCurPos();
        /* $ 01.02.2001 IS
           ! Открываем вьюер с указанием длинного имени файла, а не короткого
        */
        if (ProcessQuitKey())
        {
          /* $ 11.10.200 IS
             не будем удалять файл, если было включено удаление, но при этом
             пользователь переключился во вьюер
          */
          FEdit.SetDeleteOnClose(FALSE);
          /* IS $ */
          /* $ 06.05.2001 DJ
             обработка F6 под NWZ
          */
          /* $ 07.05.2001 DJ
             сохраняем NamesList
          */
          FileViewer *Viewer = new FileViewer (FullFileName, GetCanLoseFocus(), FALSE,
            FALSE, FilePos, NULL, EditNamesList, SaveToSaveAs);
          /* DJ $ */
//OT          FrameManager->InsertFrame (Viewer);
          /* DJ $ */
        }
        /* IS $ */
        ShowTime(2);
      }
      /* DJ $ */
      return(TRUE);
    /*$ 21.07.2000 SKV
        + выход с позиционированием на редактируемом файле по CTRLF10
    */
    case KEY_CTRLF10:
      {
        SaveScreen Sc;
        if(strchr(FileName,'\\') || strchr(FileName,'/'))
        {
          char DirTmp[NM],ADir[NM],PDir[NM],NameFile[NM],*NameTmp;
          /* 26.11.2001 VVM
            ! Использовать полное имя файла */
          strncpy(DirTmp,FullFileName,sizeof(DirTmp)-1);
          /* VVM $ */
          NameTmp=PointToName(DirTmp);
          if(NameTmp>DirTmp)
          {
            strcpy(NameFile,NameTmp);
            *NameTmp=0;
          }
          CtrlObject->Cp()->GetAnotherPanel(CtrlObject->Cp()->ActivePanel)->GetCurDir(PDir);
          CtrlObject->Cp()->ActivePanel->GetCurDir(ADir);
          /* $ 10.04.2001 IS
               Не делаем SetCurDir, если нужный путь уже есть на открытых
               панелях, тем самым добиваемся того, что выделение с элементов
               панелей не сбрасывается.
          */
          BOOL AExist=LocalStricmp(ADir,DirTmp)==0,
               PExist=LocalStricmp(PDir,DirTmp)==0;
          // если нужный путь есть на пассивной панели
          if ( !AExist && PExist)
          {
            CtrlObject->Cp()->ProcessKey(KEY_TAB);
          }
          if(!AExist && !PExist)
              CtrlObject->Cp()->ActivePanel->SetCurDir(DirTmp,TRUE);
          /* IS */
          CtrlObject->Cp()->ActivePanel->GoToFile(NameFile);
        }
        else
        {
          CtrlObject->Cp()->ActivePanel->SetCurDir(StartDir,TRUE);
          CtrlObject->Cp()->ActivePanel->GoToFile(FileName);
        }
      }
      return (TRUE);
    /* SKV $*/

    case KEY_SHIFTF10:
      ProcessKey(KEY_F2);
    case KEY_ESC:
    case KEY_F10:
      ProcessQuitKey();
      return(TRUE);

    /* $ 27.09.2000 SVS
       Печать файла/блока с использованием плагина PrintMan
    */
    case KEY_ALTF5:
    {
      if(CtrlObject->Plugins.FindPlugin(SYSID_PRINTMANAGER) != -1)
        CtrlObject->Plugins.CallPlugin(SYSID_PRINTMANAGER,OPEN_EDITOR,0); // printman
      return TRUE;
    }
    /* SVS $*/

    /* $ 19.12.2000 SVS
       Вызов диалога настроек (с подачи IS)
    */
    case KEY_ALTSHIFTF9:
      /* $ 26.02.2001 IS
           Работа с локальной копией EditorOptions
      */
      struct EditorOptions EdOpt;

      EdOpt.TabSize=FEdit.GetTabSize();
      EdOpt.ExpandTabs=FEdit.GetConvertTabs();
      EdOpt.PersistentBlocks=FEdit.GetPersistentBlocks();
      EdOpt.DelRemovesBlocks=FEdit.GetDelRemovesBlocks();
      EdOpt.AutoIndent=FEdit.GetAutoIndent();
      EdOpt.AutoDetectTable=FEdit.GetAutoDetectTable();
      EdOpt.CursorBeyondEOL=FEdit.GetCursorBeyondEOL();
      EdOpt.CharCodeBase=FEdit.GetCharCodeBase();
      FEdit.GetSavePosMode(EdOpt.SavePos, EdOpt.SaveShortPos);
      //EdOpt.BSLikeDel=FEdit.GetBSLikeDel();

      EditorConfig(EdOpt);
      EditKeyBar.Show(); //???? Нужно ли????

      FEdit.SetTabSize(EdOpt.TabSize);
      FEdit.SetConvertTabs(EdOpt.ExpandTabs);
      FEdit.SetPersistentBlocks(EdOpt.PersistentBlocks);
      FEdit.SetDelRemovesBlocks(EdOpt.DelRemovesBlocks);
      FEdit.SetAutoIndent(EdOpt.AutoIndent);
      FEdit.SetAutoDetectTable(EdOpt.AutoDetectTable);
      FEdit.SetCursorBeyondEOL(EdOpt.CursorBeyondEOL);
      FEdit.SetCharCodeBase(EdOpt.CharCodeBase);
      FEdit.SetSavePosMode(EdOpt.SavePos, EdOpt.SaveShortPos);
      //FEdit.SetBSLikeDel(EdOpt.BSLikeDel);
      /* IS $ */
      FEdit.Show();
      return TRUE;
    /* SVS $ */

    /* $ 10.05.2001 DJ
       Alt-F11 - показать view/edit history
    */
    case KEY_ALTF11:
      if (GetCanLoseFocus())
        CtrlObject->CmdLine->ShowViewEditHistory();
      return TRUE;
    /* DJ $ */

    default:
    {
      /* $ 28.04.2001 DJ
         не передаем KEY_MACRO* плагину - поскольку ReadRec в этом случае
         никак не соответствует обрабатываемой клавише, возникают разномастные
         глюки
      */
      if(Key&KEY_MACROSPEC_BASE) // исключаем MACRO
         return(FEdit.ProcessKey(Key));
      /* DJ $ */
      _KEYMACRO(CleverSysLog SL("FileEditor::ProcessKey()"));
      _KEYMACRO(SysLog("Key=0x%08X Macro.IsExecuting()=%d",Key,CtrlObject->Macro.IsExecuting()));
      if (CtrlObject->Macro.IsExecuting() ||
        !FEdit.ProcessEditorInput(FrameManager->GetLastInputRecord()))
      {
        /* $ 22.03.2001 SVS
           Это помогло от залипания :-)
        */
        if (FullScreen && !CtrlObject->Macro.IsExecuting())
          EditKeyBar.Show();
        /* SVS $ */
        if (!EditKeyBar.ProcessKey(Key))
          return(FEdit.ProcessKey(Key));
      }
      return(TRUE);
    }
  }
}


int FileEditor::ProcessQuitKey()
{
  int FirstSave=1;
  while (1)
  {
    chdir(StartDir);
    int SaveCode=FEdit.SaveFile(FileName,FirstSave,0,FALSE);
    if (SaveCode==SAVEFILE_CANCEL)
      break;
    if (SaveCode==SAVEFILE_SUCCESS)
    {
      FrameManager->DeleteFrame();
      SetExitCode (XC_QUIT);
      break;
    }
    if (Message(MSG_WARNING|MSG_ERRORTYPE,2,MSG(MEditTitle),MSG(MEditCannotSave),
                FileName,MSG(MRetry),MSG(MCancel))!=0)
      break;
    FirstSave=0;
  }
  return GetExitCode() == XC_QUIT;
}


int FileEditor::ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent)
{
  if (!EditKeyBar.ProcessMouse(MouseEvent))
    if (!FEdit.ProcessEditorInput(FrameManager->GetLastInputRecord()))
      if (!FEdit.ProcessMouse(MouseEvent))
        return(FALSE);
  return(TRUE);
}


int FileEditor::GetTypeAndName(char *Type,char *Name)
{
  if ( Type ) strcpy(Type,MSG(MScreensEdit));
  if ( Name ) strcpy(Name,FullFileName);
  return(MODALTYPE_EDITOR);
}


void FileEditor::ShowConsoleTitle()
{
  char Title[NM+20];
  sprintf(Title,MSG(MInEditor),PointToName(FileName));
  SetFarTitle(Title);
}


/* $ 28.06.2000 tran
 (NT Console resize)
 resize editor */
void FileEditor::SetScreenPosition()
{
  if (FullScreen){
    SetPosition(0,0,ScrX,ScrY);
  }
}
/* tran $ */

/* $ 10.05.2001 DJ
   добавление в view/edit history
*/

void FileEditor::OnDestroy()
{
  _OT(SysLog("[%p] FileEditor::OnDestroy()",this));
  if (!DisableHistory)
    CtrlObject->ViewHistory->AddToHistory(FullFileName,MSG(MHistoryEdit),1);
  /* $ 19.10.2001 OT
  */
  if (CtrlObject->Plugins.CurEditor==&this->FEdit){
    CtrlObject->Plugins.CurEditor=NULL;
  }
}

int FileEditor::GetCanLoseFocus(int DynamicMode)
{
  if (DynamicMode) {
    if (FEdit.IsFileModified()){
      return FALSE;
    }
  } else {
    return CanLoseFocus;
  }
  return TRUE;
}

int FileEditor::FastHide()
{
  return Opt.AllCtrlAltShiftRule & CASR_EDITOR;
}
