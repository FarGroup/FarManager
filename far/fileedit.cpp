/*
fileedit.cpp

Редактирование файла - надстройка над editor.cpp

*/

/* Revision: 1.106 27.05.2002 $ */

/*
Modify:
  27.05.2002 SVS
    ! В некоторых местах в Init() явно не стояло выставление кода возврата.
  24.05.2002 SVS
    ! Уточнения в FileEditor::EditorControl для логов
  22.05.2002 SVS
    + SetTitle()
    ! В Init добавлен вторым параметром - Title
    ! FEdit из объекта превращается в указатель - контроля с нашей стороны
      поболее будет
  18.05.2002 SVS
    - BugZ#515 - AltF5 не приходит в ProcessEditorInput
  18.05.2002 SVS
    ! ФЛАГИ
  13.05.2002 VVM
    + Перерисуем заголовок консоли после позиционирования на файл.
  11.05.2002 SVS
    - BugZ#468 - File saving after, cancelation.
  29.04.2002 SVS
    - BugZ#488 - Shift=enter
  22.03.2002 SVS
    - strcpy - Fuck!
  21.03.2002 SVS
    ! Запретим показ амп. в проптере диалога Save As
  20.03.2002 SVS
    ! GetCurrentDirectory -> FarGetCurDir
  19.03.2002 SVS
    - BugZ#371 - F2 -> разное поведение.
    + Save As... "Mac format (CR)"
  19.03.2002 SVS
    - BugZ#373 - F4 Ctrl-O - виден курсор
  18.03.2002 SVS
    + SetLockEditor() - возможноть программно лочить редактор
  26.02.2002 VVM
    ! При поиске папки (запись файла) учтем корень диска или "текущий каталог"
  09.02.2002 VVM
    + Обновить панели, если писали в текущий каталог
  05.02.2002 SVS
    ! Технологический патч - про сислоги
  04.02.2002 SVS
    - проблемы с текущим каталогом
  28.01.2002 OT
    При неудачном открытии файла не удалялся фрейм (частичная отмена 1210)
  28.01.2002 VVM
    ! Если не прочитали файл - освободить память.
  23.01.2002 SVS
    ! MEditSavedChangedNonFile2
  21.01.2002 SVS
    - Bug#255 - Alt-Shift-Ins - каталог с другой панели
  16.01.2002 SVS
    - Вах. Забыли поставить "return TRUE" в FileEditor::SetFileName()
  15.01.2002 SVS
    ! Первая серия по отучиванию класса Editor слову "Файл"
    + FileEditor::EditorControl() - первая стадия по отучению класса Editor
      от слова File
    + SetFileName() - установить переменные в имя редактируемого файла
    ! ProcessEditorInput ушел в FileEditor (в диалога плагины не...)
    + ReadFile() - постепенно сюды переносить код из Editor::ReadFile
    + SaveFile() - постепенно сюды переносить код из Editor::SaveFile
  14.01.2002 IS
    ! chdir -> FarChDir
  12.01.2002 IS
    ! ExitCode=0 -> ExitCode=XC_OPEN_ERROR
  10.01.2002 SVS
    - Bugz#213 - Не туда сохраняется файл
  28.12.2001 DJ
    ! обработка Ctrl-F10 вынесена в единую функцию
  28.12.2001 SVS
    - BugZ#213 Не туда сохраняется файл
  26.12.2001 SVS
    + внедрение FEOPMODE_*
  25.12.2001 SVS
    + ResizeConsole()
  17.12.2001 KM
    ! Если !GetCanLoseFocus() тогда на Alt-F11 рисуем пустую строку.
  08.12.2001 OT
    Bugzilla #144 Заходим в архив, F4 на файле, Ctrl-F10.
  27.11.2001 DJ
    + Local в EditorConfig
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
    - Избавимся от "дублирования" УчшеСщву здесь и во Frame :)
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
                       char *PluginData,int ToSaveAs, int OpenModeExstFile)
{
  _KEYMACRO(SysLog("FileEditor::FileEditor(1)"));
  _KEYMACRO(SysLog(1));
  ScreenObject::SetPosition(0,0,ScrX,ScrY);
  FullScreen=TRUE;
  Init(Name,NULL,CreateNewFile,EnableSwitch,StartLine,StartChar,
       DisableHistory,PluginData,ToSaveAs,FALSE,OpenModeExstFile);
}


FileEditor::FileEditor(const char *Name,int CreateNewFile,int EnableSwitch,
            int StartLine,int StartChar,const char *Title,
            int X1,int Y1,int X2,int Y2,int DisableHistory, BOOL DeleteOnClose,
            int OpenModeExstFile)
{
  _KEYMACRO(SysLog("FileEditor::FileEditor(1)"));
  _KEYMACRO(SysLog(1));
  /* $ 02.11.2001 IS
       отрицательные координаты левого верхнего угла заменяются на нулевые
  */
  if(X1<0) X1=0;
  if(Y1<0) Y1=0;
  /* IS $ */
  ScreenObject::SetPosition(X1,Y1,X2,Y2);
  FullScreen=(X1==0 && Y1==0 && X2==ScrX && Y2==ScrY);
  Init(Name,Title,CreateNewFile,EnableSwitch,StartLine,StartChar,DisableHistory,"",
       FALSE,DeleteOnClose,OpenModeExstFile);
}

/* $ 07.05.2001 DJ
   в деструкторе грохаем EditNamesList, если он был создан, а в SetNamesList()
   создаем EditNamesList и копируем туда значения
*/
/*
  Вызов деструкторов идет так:
    FileEditor::~FileEditor()
    Editor::~Editor()
    ...
*/
FileEditor::~FileEditor()
{
  _OT(SysLog("[%p] FileEditor::~FileEditor()",this));
  if(FEdit)
    delete FEdit;

  CurrentEditor=NULL;
  if (EditNamesList)
    delete EditNamesList;

  _KEYMACRO(SysLog(-1));
  _KEYMACRO(SysLog("FileEditor::~FileEditor()"));
}

void FileEditor::Init(const char *Name,const char *Title,int CreateNewFile,int EnableSwitch,
                      int StartLine,int StartChar,int DisableHistory,
                      char *PluginData,int ToSaveAs,BOOL DeleteOnClose,
                      int OpenModeExstFile)
{
  FEdit=new Editor;

  if(!FEdit)
  {
    ExitCode=XC_OPEN_ERROR;
    return;
  }

  CurrentEditor=this;
  SetTitle(Title);
  RedrawTitle = FALSE;
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
  {
    ExitCode=XC_OPEN_ERROR;
    return;
  }

  FEdit->SetPluginData(PluginData);
  FEdit->SetHostFileEditor(this);
  _OT(SysLog("Editor;:Editor(), EnableSwitch=%i",EnableSwitch));
  SetCanLoseFocus(EnableSwitch);
  FarGetCurDir(sizeof(StartDir),StartDir);

  if(!SetFileName(Name))
  {
    ExitCode=XC_OPEN_ERROR;
    return;
  }

  /*$ 11.05.2001 OT */
  //int FramePos=FrameManager->FindFrameByFile(MODALTYPE_EDITOR,FullFileName);
  //if (FramePos!=-1)
  if (EnableSwitch)
  {
    //if (EnableSwitch)
    int FramePos=FrameManager->FindFrameByFile(MODALTYPE_EDITOR,FullFileName);
    if (FramePos!=-1)
    {
      int SwitchTo=FALSE;
      int MsgCode=0;
      if (!(*FrameManager)[FramePos]->GetCanLoseFocus(TRUE) ||
          Opt.Confirm.AllowReedit)
      {
        if(OpenModeExstFile == FEOPMODE_QUERY)
        {
          char MsgFullFileName[NM];
          strncpy(MsgFullFileName,FullFileName,sizeof(MsgFullFileName)-1);
          SetMessageHelp("EditorReload");
          MsgCode=Message(0,3,MSG(MEditTitle),
                TruncPathStr(MsgFullFileName,ScrX-16),
                MSG(MAskReload),
                MSG(MCurrent),MSG(MNewOpen),MSG(MReload));
        }
        else
        {
          MsgCode=(OpenModeExstFile==FEOPMODE_USEEXISTING)?0:
                        (OpenModeExstFile==FEOPMODE_NEWIFOPEN?1:-100);
        }
        switch(MsgCode)
        {
          case 0:         // Current
            SwitchTo=TRUE;
            FrameManager->DeleteFrame(this); //???
            break;
          case 1:         // NewOpen
            SwitchTo=FALSE;
            break;
          case 2:         // Reload
            FrameManager->DeleteFrame(FramePos);
            SetExitCode(-2);
            break;
          case -100:
            //FrameManager->DeleteFrame(this);  //???
            SetExitCode(XC_EXISTS);
            return;
          default:
            FrameManager->DeleteFrame(this);  //???
            SetExitCode(MsgCode == -100?XC_EXISTS:XC_QUIT);
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
        //FrameManager->PluginCommit();
        SetExitCode((OpenModeExstFile != FEOPMODE_QUERY)?XC_EXISTS:TRUE);
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
    Message(MSG_WARNING,1,MSG(MEditTitle),MSG(MEditCanNotEditDirectory),MSG(MOk));
    ExitCode=XC_OPEN_ERROR;
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
      ExitCode=XC_OPEN_ERROR;
      return;
    }
  }
  /* SVS 03.12.2000 $ */
  /* SVS $ */

  FEdit->SetPosition(X1,Y1,X2,Y2-1);
  FEdit->SetStartPos(StartLine,StartChar);
  FEdit->SetDeleteOnClose(DeleteOnClose);
  int UserBreak;
  /* $ 06.07.2001 IS
     При создании файла с нуля так же посылаем плагинам событие EE_READ, дабы
     не нарушать однообразие.
  */
  IsNewFile=CreateNewFile;
  if (!ReadFile(FullFileName,UserBreak))
  {
    if(!CreateNewFile || UserBreak)
    {
      if (UserBreak!=1)
      {
        Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MEditTitle),MSG(MEditCannotOpen),FileName,MSG(MOk));
        ExitCode=XC_OPEN_ERROR;
      }
      else
      {
        ExitCode=XC_LOADING_INTERRUPTED;
      }
      FrameManager->DeleteFrame(this);
      CtrlObject->Cp()->Redraw();
      return;
    }
    CtrlObject->Plugins.CurEditor=this;//&FEdit;
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
  if (EnableSwitch)
  {
    FrameManager->InsertFrame(this);
    //FrameManager->PluginCommit(); // НАДА! иначе нифига ничего не работает
  }
  else
  {
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
  char *FEditKeys[]={MSG(MEditF1),MSG(MEditF2),MSG(MEditF3),MSG(MEditF4),MSG(MEditF5),MSG(MEditF6),MSG(MEditF7),MSG(MEditF8),MSG(MEditF9),MSG(MEditF10),MSG(MEditF11),MSG(MEditF12)};
  if(SaveToSaveAs)
    FEditKeys[2-1]=MSG(MEditShiftF2);
  if(!EnableF6)
    FEditKeys[6-1]="";
  if(!GetCanLoseFocus())
    FEditKeys[12-1]="";
  /* DJ $ */
  char *FEditShiftKeys[]={MSG(MEditShiftF1),MSG(MEditShiftF2),MSG(MEditShiftF3),MSG(MEditShiftF4),MSG(MEditShiftF5),MSG(MEditShiftF6),MSG(MEditShiftF7),MSG(MEditShiftF8),MSG(MEditShiftF9),MSG(MEditShiftF10),MSG(MEditShiftF11),MSG(MEditShiftF12)};
  /* $ 17.12.2001 KM
     ! Если !GetCanLoseFocus() тогда на Alt-F11 рисуем пустую строку.
  */
  char *FEditAltKeys[]={MSG(MEditAltF1),MSG(MEditAltF2),MSG(MEditAltF3),MSG(MEditAltF4),MSG(MEditAltF5),MSG(MEditAltF6),MSG(MEditAltF7),MSG(MEditAltF8),MSG(MEditAltF9),MSG(MEditAltF10),MSG(MEditAltF11),MSG(MEditAltF12)};
  if(!GetCanLoseFocus())
    FEditAltKeys[11-1]="";
  /* KM $ */
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
  FEdit->SetEditKeyBar(&EditKeyBar);
}
/* SVS $ */

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
    FEdit->SetPosition(0,0,ScrX,ScrY-1);
  }
  ScreenObject::Show();
}


void FileEditor::DisplayObject()
{
  FEdit->Show();
}


int FileEditor::ProcessKey(int Key)
{
  DWORD FNAttr;
  char *Ptr, Chr;

  if (RedrawTitle && ((Key & 0x00ffffff) < KEY_END_FKEY))
    ShowConsoleTitle();

  // BugZ#488 - Shift=enter
  if(ShiftPressed && Key == KEY_ENTER && !CtrlObject->Macro.IsExecuting())
  {
    Key=KEY_SHIFTENTER;
  }

  switch(Key)
  {
    /* $ 24.08.2000 SVS
       + Добавляем реакцию показа бакграунда на клавишу CtrlAltShift
    */
    case KEY_CTRLO:
    {
      /*$ 27.09.2000 skv
        To prevent redraw in macro with Ctrl-O
      */
      FEdit->Hide();
      /* skv$*/
      FrameManager->ShowBackground();
      SetCursorType(FALSE,0);
      WaitKey(-1);
      Show();
      return(TRUE);
    }
/* $ KEY_CTRLALTSHIFTPRESS унесено в manager OT */

    case KEY_F2:
    case KEY_SHIFTF2:
    {
      BOOL Done=FALSE;
      char OldCurDir[4096];
      FarGetCurDir(sizeof(OldCurDir),OldCurDir);

      while(!Done) // бьемся до упора
      {
        // проверим путь к файлу, может его уже снесли...
        Ptr=strrchr(FullFileName,'\\');
        if(Ptr)
        {
          Chr=*Ptr;
          *Ptr=0;
          // В корне?
          if (!((strlen(FullFileName)==2) && isalpha(FullFileName[0]) && (FullFileName[1]==':')))
          {
            // а дальше? каталог существует?
            if((FNAttr=GetFileAttributes(FullFileName)) == -1 ||
                              !(FNAttr&FILE_ATTRIBUTE_DIRECTORY)
                //|| LocalStricmp(OldCurDir,FullFileName)  // <- это видимо лишнее.
              )
              SaveToSaveAs=TRUE;
          }
          *Ptr=Chr;
        }

        if(Key == KEY_F2 &&
           (FNAttr=GetFileAttributes(FullFileName)) != -1 &&
           !(FNAttr&FILE_ATTRIBUTE_DIRECTORY))
            SaveToSaveAs=FALSE;

        static int TextFormat=0;
        int NameChanged=FALSE;
        if (Key==KEY_SHIFTF2 || SaveToSaveAs)
        {
          const char *HistoryName="NewEdit";
          static struct DialogData EditDlgData[]=
          {
            /* 0 */ DI_DOUBLEBOX,3,1,72,12,0,0,0,0,(char *)MEditTitle,
            /* 1 */ DI_TEXT,5,2,0,0,0,0,0,0,(char *)MEditSaveAs,
            /* 2 */ DI_EDIT,5,3,70,3,1,(DWORD)HistoryName,DIF_HISTORY,0,"",
            /* 3 */ DI_TEXT,3,4,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
            /* 4 */ DI_TEXT,5,5,0,0,0,0,0,0,(char *)MEditSaveAsFormatTitle,
            /* 5 */ DI_RADIOBUTTON,5,6,0,0,0,0,DIF_GROUP,0,(char *)MEditSaveOriginal,
            /* 6 */ DI_RADIOBUTTON,5,7,0,0,0,0,0,0,(char *)MEditSaveDOS,
            /* 7 */ DI_RADIOBUTTON,5,8,0,0,0,0,0,0,(char *)MEditSaveUnix,
            /* 8 */ DI_RADIOBUTTON,5,9,0,0,0,0,0,0,(char *)MEditSaveMac,
            /* 9 */ DI_TEXT,3,10,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
            /*10 */ DI_BUTTON,0,11,0,0,0,0,DIF_CENTERGROUP,1,(char *)MOk,
            /*11 */ DI_BUTTON,0,11,0,0,0,0,DIF_CENTERGROUP,0,(char *)MCancel,
          };
          MakeDialogItems(EditDlgData,EditDlg);
          strncpy(EditDlg[2].Data,(SaveToSaveAs?FullFileName:FileName),sizeof(EditDlg[2].Data)-1);
          EditDlg[5].Selected=EditDlg[6].Selected=EditDlg[7].Selected=EditDlg[8].Selected=0;
          EditDlg[5+TextFormat].Selected=TRUE;
          Dialog Dlg(EditDlg,sizeof(EditDlg)/sizeof(EditDlg[0]));
          Dlg.SetPosition(-1,-1,76,14);
          Dlg.SetHelp("FileSaveAs");
          Dlg.Process();
          if (Dlg.GetExitCode()!=10 || *EditDlg[2].Data==0)
            return(FALSE);
          /* $ 07.06.2001 IS
             - Баг: нужно сначала убирать пробелы, а только потом кавычки
          */
          RemoveTrailingSpaces(EditDlg[2].Data);
          Unquote(EditDlg[2].Data);
          /* IS $ */

          NameChanged=LocalStricmp(EditDlg[2].Data,(SaveToSaveAs?FullFileName:FileName))!=0;
          /* $ 01.08.2001 tran
             этот кусок перенесен повыше и вместо FileName
             используеся EditDlg[2].Data */
          if(!NameChanged)
            FarChDir(StartDir); // ПОЧЕМУ? А нужно ли???

          FNAttr=GetFileAttributes(EditDlg[2].Data);
          if (NameChanged && FNAttr != -1)
          {
            if (Message(MSG_WARNING,2,MSG(MEditTitle),EditDlg[2].Data,MSG(MEditExists),
                         MSG(MEditOvr),MSG(MYes),MSG(MNo))!=0)
            {
              FarChDir(OldCurDir);
              return(TRUE);
            }
          }
          /* tran $ */

          if(!SetFileName(EditDlg[2].Data))
          {
            if(!NameChanged)
              FarChDir(OldCurDir);
            return FALSE;
          }

          if (EditDlg[5].Selected)
            TextFormat=0;
          if (EditDlg[6].Selected)
            TextFormat=1;
          if (EditDlg[7].Selected)
            TextFormat=2;
          if (EditDlg[8].Selected)
            TextFormat=3;
          if(!NameChanged)
            FarChDir(OldCurDir);
        }
        ShowConsoleTitle();
        FarChDir(StartDir); //???

        if(SaveFile(FullFileName,0,Key==KEY_SHIFTF2 ? TextFormat:0,Key==KEY_SHIFTF2)==0)
        {
          if (Message(MSG_WARNING|MSG_ERRORTYPE,2,MSG(MEditTitle),MSG(MEditCannotSave),
                      FileName,MSG(MRetry),MSG(MCancel))!=0)
          {
            Done=TRUE;
            break;
          }
        }
        else
          Done=TRUE;
      }
      return(TRUE);
    }

    case KEY_F6:
    {
      /* $ 10.05.2001 DJ
         используем EnableF6
      */
      if (EnableF6)
      {
        int FirstSave=1, NeedQuestion=1;
        // проверка на "а может это говно удалили уже?"
        // возможно здесь она и не нужна!
        // хотя, раз уж были изменени, то
        if(FEdit->IsFileChanged() &&  // в текущем сеансе были изменения?
           GetFileAttributes(FullFileName) == -1) // а файл еще существует?
        {
          switch(Message(MSG_WARNING,2,MSG(MEditTitle),
                         MSG(MEditSavedChangedNonFile),
                         MSG(MEditSavedChangedNonFile2),
                         MSG(MEditSave),MSG(MCancel)))
          {
            case 0:
              if(ProcessKey(KEY_F2))
              {
                FirstSave=0;
                break;
              }
            default:
              return FALSE;
          }
        }

        if(!FirstSave || FEdit->IsFileChanged() || GetFileAttributes(FullFileName)!=-1)
        {
          long FilePos=FEdit->GetCurPos();
          /* $ 01.02.2001 IS
             ! Открываем вьюер с указанием длинного имени файла, а не короткого
          */
          if (ProcessQuitKey(FirstSave,NeedQuestion))
          {
            /* $ 11.10.200 IS
               не будем удалять файл, если было включено удаление, но при этом
               пользователь переключился во вьюер
            */
            FEdit->SetDeleteOnClose(FALSE);
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
        return(TRUE);
      }
      break; // отдадим F6 плагинам, если есть запрет на переключение
      /* DJ $ */
    }
    /*$ 21.07.2000 SKV
        + выход с позиционированием на редактируемом файле по CTRLF10
    */
    case KEY_CTRLF10:
    {
      {
        if (isTemporary())
        {
          return(TRUE);
        }
        /* 26.11.2001 VVM
          ! Использовать полное имя файла */
        /* $ 28.12.2001 DJ
           вынесем код в общую функцию
        */
        if(GetFileAttributes(FullFileName) == -1) // а сам файл то еще на месте?
        {
          return FALSE;
        }

        {
          SaveScreen Sc;
          CtrlObject->Cp()->GoToFile (FullFileName);
          RedrawTitle = TRUE;
        }
        /* DJ $ */
        /* VVM $ */
      }
      return (TRUE);
    }
    /* SKV $*/

    case KEY_SHIFTF10:
      if(!ProcessKey(KEY_F2)) // учтем факт того, что могли отказаться от сохранения
        return FALSE;
    case KEY_ESC:
    case KEY_F10:
    {
      int FirstSave=1, NeedQuestion=1;
      if(Key != KEY_SHIFTF10)    // KEY_SHIFTF10 не учитываем!
      {
        if(FEdit->IsFileChanged() &&  // в текущем сеансе были изменения?
           GetFileAttributes(FullFileName) == -1 && !IsNewFile) // а сам файл то еще на месте?
        {
          switch(Message(MSG_WARNING,3,MSG(MEditTitle),
                         MSG(MEditSavedChangedNonFile),
                         MSG(MEditSavedChangedNonFile2),
                         MSG(MEditSave),MSG(MEditNotSave),MSG(MEditContinue)))
          {
            case 0:
              if(!ProcessKey(KEY_F2))  // попытка сначала сохранить
                NeedQuestion=0;
              FirstSave=0;
              break;
            case 1:
              NeedQuestion=0;
              FirstSave=0;
              break;
            case 2:
            default:
              return FALSE;
          }
        }
      }
      ProcessQuitKey(FirstSave,NeedQuestion);
      return(TRUE);
    }

    /* $ 27.09.2000 SVS
       Печать файла/блока с использованием плагина PrintMan
    */
    case KEY_ALTF5:
    {
      if(CtrlObject->Plugins.FindPlugin(SYSID_PRINTMANAGER) != -1)
      {
        CtrlObject->Plugins.CallPlugin(SYSID_PRINTMANAGER,OPEN_EDITOR,0); // printman
        return TRUE;
      }
      break; // отдадим Alt-F5 на растерзание плагинам, если не установлен PrintMan
    }
    /* SVS $*/

    /* $ 19.12.2000 SVS
       Вызов диалога настроек (с подачи IS)
    */
    case KEY_ALTSHIFTF9:
    {
      /* $ 26.02.2001 IS
           Работа с локальной копией EditorOptions
      */
      struct EditorOptions EdOpt;

      EdOpt.TabSize=FEdit->GetTabSize();
      EdOpt.ExpandTabs=FEdit->GetConvertTabs();
      EdOpt.PersistentBlocks=FEdit->GetPersistentBlocks();
      EdOpt.DelRemovesBlocks=FEdit->GetDelRemovesBlocks();
      EdOpt.AutoIndent=FEdit->GetAutoIndent();
      EdOpt.AutoDetectTable=FEdit->GetAutoDetectTable();
      EdOpt.CursorBeyondEOL=FEdit->GetCursorBeyondEOL();
      EdOpt.CharCodeBase=FEdit->GetCharCodeBase();
      FEdit->GetSavePosMode(EdOpt.SavePos, EdOpt.SaveShortPos);
      //EdOpt.BSLikeDel=FEdit->GetBSLikeDel();

      /* $ 27.11.2001 DJ
         Local в EditorConfig
      */
      EditorConfig(EdOpt,1);
      /* DJ $ */
      EditKeyBar.Show(); //???? Нужно ли????

      FEdit->SetTabSize(EdOpt.TabSize);
      FEdit->SetConvertTabs(EdOpt.ExpandTabs);
      FEdit->SetPersistentBlocks(EdOpt.PersistentBlocks);
      FEdit->SetDelRemovesBlocks(EdOpt.DelRemovesBlocks);
      FEdit->SetAutoIndent(EdOpt.AutoIndent);
      FEdit->SetAutoDetectTable(EdOpt.AutoDetectTable);
      FEdit->SetCursorBeyondEOL(EdOpt.CursorBeyondEOL);
      FEdit->SetCharCodeBase(EdOpt.CharCodeBase);
      FEdit->SetSavePosMode(EdOpt.SavePos, EdOpt.SaveShortPos);
      //FEdit->SetBSLikeDel(EdOpt.BSLikeDel);
      /* IS $ */
      FEdit->Show();
      return TRUE;
    }
    /* SVS $ */

    /* $ 10.05.2001 DJ
       Alt-F11 - показать view/edit history
    */
    case KEY_ALTF11:
    {
      if (GetCanLoseFocus())
      {
        CtrlObject->CmdLine->ShowViewEditHistory();
        return TRUE;
      }
      break; // отдадим Alt-F11 на растерзание плагинам, если редактор модальный
    }
    /* DJ $ */
  }

  // Все сотальные необработанные клавиши пустим далее
  /* $ 28.04.2001 DJ
     не передаем KEY_MACRO* плагину - поскольку ReadRec в этом случае
     никак не соответствует обрабатываемой клавише, возникают разномастные
     глюки
  */
  if(Key&KEY_MACROSPEC_BASE) // исключаем MACRO
     return(FEdit->ProcessKey(Key));
  /* DJ $ */
  _KEYMACRO(CleverSysLog SL("FileEditor::ProcessKey()"));
  _KEYMACRO(SysLog("Key=%s Macro.IsExecuting()=%d",_FARKEY_ToName(Key),CtrlObject->Macro.IsExecuting()));
  if (CtrlObject->Macro.IsExecuting() ||
    !ProcessEditorInput(FrameManager->GetLastInputRecord()))
  {
    /* $ 22.03.2001 SVS
       Это помогло от залипания :-)
    */
    if (FullScreen && !CtrlObject->Macro.IsExecuting())
      EditKeyBar.Show();
    /* SVS $ */
    if (!EditKeyBar.ProcessKey(Key))
      return(FEdit->ProcessKey(Key));
  }
  return(TRUE);
}


int FileEditor::ProcessQuitKey(int FirstSave,BOOL NeedQuestion)
{
  char OldCurDir[4096];
  FarGetCurDir(sizeof(OldCurDir),OldCurDir);
  while (1)
  {
    FarChDir(StartDir); // ПОЧЕМУ? А нужно ли???
    int SaveCode=SAVEFILE_SUCCESS;
    if(NeedQuestion)
    {
      SaveCode=SaveFile(FullFileName,FirstSave,0,FALSE);
    }
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
  FarChDir(OldCurDir);
  return GetExitCode() == XC_QUIT;
}


// сюды плавно переносить код из Editor::ReadFile()
int FileEditor::ReadFile(const char *Name,int &UserBreak)
{
  return FEdit->ReadFile(Name,UserBreak);
}

// сюды плавно переносить код из Editor::SaveFile()
int FileEditor::SaveFile(const char *Name,int Ask,int TextFormat,int SaveAs)
{
  /* $ 11.10.2000 SVS
     Редактировали, залочили, при выходе - потеряли файл :-(
  */
  if (FEdit->Flags.Check(FEDITOR_LOCKMODE) && !FEdit->Flags.Check(FEDITOR_MODIFIED) && !SaveAs)
    return SAVEFILE_SUCCESS;
  /* SVS $ */

  if (Ask)
  {
    if(!FEdit->Flags.Check(FEDITOR_MODIFIED))
      return SAVEFILE_SUCCESS;

    if (Ask)
    {
      switch (Message(MSG_WARNING,3,MSG(MEditTitle),MSG(MEditAskSave),
              MSG(MEditSave),MSG(MEditNotSave),MSG(MEditContinue)))
      {
        case -1:
        case -2:
        case 2:  // Continue Edit
          return SAVEFILE_CANCEL;
        case 0:  // Save
          break;
        case 1:  // Not Save
          FEdit->TextChanged(0); // 10.08.2000 skv: TextChanged() support;
          return SAVEFILE_SUCCESS;
      }
    }
  }

  int NewFile=TRUE;
  if ((FEdit->FileAttributes=GetFileAttributes(Name))!=-1)
  {
    NewFile=FALSE;
    if (FEdit->FileAttributes & FA_RDONLY)
    {
      int AskOverwrite=Message(MSG_WARNING,2,MSG(MEditTitle),Name,MSG(MEditRO),
                           MSG(MEditOvr),MSG(MYes),MSG(MNo));
      if (AskOverwrite!=0)
        return SAVEFILE_CANCEL;

      SetFileAttributes(Name,FEdit->FileAttributes & ~FA_RDONLY); // сняты атрибуты
    }

    if (FEdit->FileAttributes & (FA_HIDDEN|FA_SYSTEM))
      SetFileAttributes(Name,0);
  }
  else
  {
    // проверим путь к файлу, может его уже снесли...
    char CreatedPath[4096];
    char *Ptr=strrchr(strncpy(CreatedPath,Name,sizeof(CreatedPath)-1),'\\'), Chr;
    if(Ptr)
    {
      Chr=*Ptr;
      *Ptr=0;
      DWORD FAttr=0;
      if(GetFileAttributes(CreatedPath) == -1)
      {
        // и попробуем создать.
        // Раз уж
        CreatePath(CreatedPath);
        FAttr=GetFileAttributes(CreatedPath);
      }
      *Ptr=Chr;
      if(FAttr == -1)
        return SAVEFILE_ERROR;
    }
  }

  int Ret=FEdit->SaveFile(Name,Ask,TextFormat,SaveAs);
  IsNewFile=0;

  /* $ 09.02.2002 VVM
    + Обновить панели, если писали в текущий каталог */
  if (Ret==SAVEFILE_SUCCESS)
  {
    Panel *ActivePanel = CtrlObject->Cp()->ActivePanel;
    char *FileName = PointToName((char *)Name);
    char FilePath[NM], PanelPath[NM];
    strncpy(FilePath, Name, FileName - Name);
    ActivePanel->GetCurDir(PanelPath);
    AddEndSlash(PanelPath);
    AddEndSlash(FilePath);
    if (!strcmp(PanelPath, FilePath))
      ActivePanel->Update(UPDATE_KEEP_SELECTION);
  }
  /* VVM $ */
  return Ret;
}

int FileEditor::ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent)
{
  if (!EditKeyBar.ProcessMouse(MouseEvent))
    if (!ProcessEditorInput(FrameManager->GetLastInputRecord()))
      if (!FEdit->ProcessMouse(MouseEvent))
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
  RedrawTitle = FALSE;
}


/* $ 28.06.2000 tran
 (NT Console resize)
 resize editor */
void FileEditor::SetScreenPosition()
{
  if (FullScreen)
  {
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
    CtrlObject->ViewHistory->AddToHistory(FullFileName,MSG(MHistoryEdit),
                  (FEdit->Flags.Check(FEDITOR_LOCKMODE)?4:1));
  /* $ 19.10.2001 OT
  */
  if (CtrlObject->Plugins.CurEditor==this)//&this->FEdit)
  {
    CtrlObject->Plugins.CurEditor=NULL;
  }
}

int FileEditor::GetCanLoseFocus(int DynamicMode)
{
  if (DynamicMode)
  {
    if (FEdit->IsFileModified())
    {
      return FALSE;
    }
  }
  else
  {
    return CanLoseFocus;
  }
  return TRUE;
}

void FileEditor::SetLockEditor(BOOL LockMode)
{
  if(LockMode)
    FEdit->Flags.Set(FEDITOR_LOCKMODE);
  else
    FEdit->Flags.Skip(FEDITOR_LOCKMODE);
}

int FileEditor::FastHide()
{
  return Opt.AllCtrlAltShiftRule & CASR_EDITOR;
}

BOOL FileEditor::isTemporary()
{
  return (!GetDynamicallyBorn());
}

void FileEditor::ResizeConsole()
{
  FEdit->PrepareResizedConsole();
}

int FileEditor::ProcessEditorInput(INPUT_RECORD *Rec)
{
  int RetCode;
  _KEYMACRO(CleverSysLog SL("FileEditor::ProcessEditorInput()"));
  _KEYMACRO(if(Rec->EventType == KEY_EVENT)SysLog("%cVKey=%s",(Rec->Event.KeyEvent.bKeyDown?0x19:0x18),_VK_KEY_ToName(Rec->Event.KeyEvent.wVirtualKeyCode)));

  CtrlObject->Plugins.CurEditor=this;
  RetCode=CtrlObject->Plugins.ProcessEditorInput(Rec);

  _KEYMACRO(SysLog("RetCode=%d",RetCode));
  return RetCode;
}

void FileEditor::SetPluginTitle(char *PluginTitle)
{
  FEdit->SetPluginTitle(PluginTitle);
}

BOOL FileEditor::SetFileName(const char *NewFileName)
{
  if (ConvertNameToFull(NewFileName,FullFileName, sizeof(FullFileName)) >= sizeof(FullFileName))
  {
    return FALSE;
  }
  strncpy(FileName,NewFileName,sizeof(FileName)-1);
  return TRUE;
}

void FileEditor::SetTitle(const char *Title)
{
  FEdit->SetTitle(NullToEmpty(Title));
}

int FileEditor::EditorControl(int Command,void *Param)
{
#if defined(SYSLOG_KEYMACRO)
  _KEYMACRO(CleverSysLog SL("FileEditor::EditorControl()"));
  if(Command == ECTL_READINPUT || Command == ECTL_PROCESSINPUT)
  {
    _KEYMACRO(SysLog("(Command=%s, Param=[%d/0x%08X]) Macro.IsExecuting()=%d",_ECTL_ToName(Command),(int)Param,Param,CtrlObject->Macro.IsExecuting()));
  }
#else
  _ECTLLOG(CleverSysLog SL("FileEditor::EditorControl()"));
  _ECTLLOG(SysLog("(Command=%s, Param=[%d/0x%08X])",_ECTL_ToName(Command),(int)Param,Param));
#endif
  return FEdit->EditorControl(Command,Param);
}
