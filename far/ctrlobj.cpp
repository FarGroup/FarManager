/*
ctrlobj.cpp

Управление остальными объектами, раздача сообщений клавиатуры и мыши

*/

/* Revision: 1.39 14.03.2002 $ */

/*
Modify:
  14.03.2002 SVS
    - Блин, уродская неразбериха с этими сраными понятиями :-((
      Типа "левая-правая" и "активная-пассивная"
  21.02.2002 SVS
    ! Покажим кейбар (если надо) и командную строку перед загрузкой
      плагинов, а мало того, что пустые панели, так они еще и подвешены
  31.01.2002 SVS
    - BugZ#194 - Не сохраняются позиции в фоновых вьюверах/редакторах при
      выходе из FARа
      Ну естественно, сначала кешь сохраняем, а потом ExitAll() делаем.
      Круто, блин.
  14.12.2001 SVS
    ! Сделаем сроллинг перед выводом "лейбака"
  15,11,2001 SVS
    ! Для каждого из *History назначим тип.
    - При выставленном Interface\ShowMenuBar=1 ФАР при старте падал,
      т.к. в panel.cpp кто-то (IS) залудил когда-то вызов TopMenuBar->Show(),
      и кто-то (уже и не знаю кто)... в общем Long History
  29.10.2001 IS
    ! SaveEditorPos переехала в EditorOptions
  24.10.2001 SVS
    ! подсократим "лишний" код
  26.09.2001 SVS
    - Бага: При старте неверно выставлен текущий каталог
  25.07.2001 SVS
    ! Copyright переехала в global.cpp.
  06.07.2001 OT
    - баг с зацикливанием при выдаче сообщения о неправильной загрузки плагина
  23.06.2001 OT
    - far -r
  30.05.2001 OT
    ! Очистка исходников от закомментареных ранее кусков
  16.05.2001 DJ
    ! proof-of-concept
  15.05.2001 OT
    ! NWZ -> NFZ
  12.05.2001 DJ
    ! FrameManager оторван от CtrlObject
    ! глобальный указатель на CtrlObject переехал сюда
  11.05.2001 OT
    ! Отрисовка Background
  07.05.2001 SVS
    ! SysLog(); -> _D(SysLog());
  06.05.2001 DJ
    ! перетрях #include
  06.05.2001 ОТ
    ! Переименование Window в Frame :)
  05.05.2001 DJ
    + перетрях NWZ
  29.04.2001 ОТ
    + Внедрение NWZ от Третьякова
  28.04.2001 VVM
    + KeyBar тоже умеет обрабатывать клавиши.
  22.04.2001 SVS
    ! Загрузка плагнов - после создания ВСЕХ основных объектов
  02.04.2001 VVM
    + Обработка Opt.FlagPosixSemantics
  28.02.2001 IS
    ! Т.е. CmdLine теперь указатель, то произведем замену
      "CmdLine." на "CmdLine->" и собственно создадим/удалим ее в конструкторе
      и деструкторе CtrlObject.
  09.02.2001 IS
    + восстановим состояние опций "помеченное вперед"
  09.01.2001 SVS
    + Учтем правило Opt.ShiftsKeyRules (WaitInFastFind)
  29.12.2000 IS
    + Проверяем при выходе, сохранены ли все измененные файлы. Если нет, то
      не выходим из фара.
  15.12.2000 SVS
    ! Метод ShowCopyright - public static & параметр Flags.
      Если Flags&1, то использовать printf вместо внутренних функций
  25.11.2000 SVS
    ! Copyright в 2 строки
  27.09.2000 SVS
    ! Ctrl-Alt-Shift - реагируем, если надо.
  19.09.2000 IS
    ! Повторное нажатие на ctrl-l|q|t всегда включает файловую панель
  19.09.2000 SVS
    + Opt.PanelCtrlAltShiftRule задает поведение Ctrl-Alt-Shift для панелей.
  19.09.2000 SVS
    + Добавляем реакцию показа бакграунда в панелях на CtrlAltShift
  07.09.2000 tran 1.05
    + Current File
  15.07.2000 tran
    + а я код раздуваю :) вводя новый метод Redraw
  13.07.2000 SVS
    ! Некоторые коррекция по сокращению кода ;-)
  11.07.2000 SVS
    ! Изменения для возможности компиляции под BC & VC
  29.06.2000 tran
    ! соощение о копирайте включается из copyright.inc
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#include "headers.hpp"
#pragma hdrstop

#include "ctrlobj.hpp"
#include "global.hpp"
#include "fn.hpp"
#include "lang.hpp"
#include "manager.hpp"
#include "cmdline.hpp"
#include "hilight.hpp"
#include "grpsort.hpp"
#include "poscache.hpp"
#include "history.hpp"
#include "treelist.hpp"
#include "filter.hpp"
#include "filepanels.hpp"

ControlObject *CtrlObject;

ControlObject::ControlObject()
{
  _OT(SysLog("[%p] ControlObject::ControlObject()", this));
  FPanels=0;
  CtrlObject=this;
  /* $ 06.05.2001 DJ
     создаем динамически (для уменьшения dependencies)
  */
  HiFiles = new HighlightFiles;
  GrpSort = new GroupSort;
  ViewerPosCache = new FilePositionCache;
  EditorPosCache = new FilePositionCache;
  FrameManager = new Manager;
  /* DJ $ */
  ReadConfig();
  /* $ 28.02.2001 IS
       Создадим обязательно только после того, как прочитали настройки
  */
  CmdLine=new CommandLine;
  /* IS $ */

  CmdHistory=new History(HISTORYTYPE_CMD,"SavedHistory",&Opt.SaveHistory,FALSE,FALSE);
  FolderHistory=new History(HISTORYTYPE_FOLDER,"SavedFolderHistory",&Opt.SaveFoldersHistory,FALSE,TRUE);
  ViewHistory=new History(HISTORYTYPE_VIEW,"SavedViewHistory",&Opt.SaveViewHistory,TRUE,TRUE);

  FolderHistory->SetAddMode(TRUE,2,TRUE);
  ViewHistory->SetAddMode(TRUE,Opt.FlagPosixSemantics?1:2,TRUE);
  if (Opt.SaveHistory)
    CmdHistory->ReadHistory();
  if (Opt.SaveFoldersHistory)
    FolderHistory->ReadHistory();
  if (Opt.SaveViewHistory)
    ViewHistory->ReadHistory();
  RegVer=-1;
}


void ControlObject::Init()
{
  TreeList::ClearCache(0);
  PanelFilter::InitFilter();

  SetColor(F_LIGHTGRAY|B_BLACK);
  GotoXY(0,ScrY-3);
  while (RegVer==-1)
    Sleep(0);
  ShowCopyright();
  GotoXY(0,ScrY-2);

  char TruncRegName[512];
  strcpy(TruncRegName,RegName);
  char *CountPtr=strstr(TruncRegName," - (");
  if (CountPtr!=NULL && isdigit(CountPtr[4]) && strchr(CountPtr+5,'/')!=NULL &&
      strchr(CountPtr+6,')')!=NULL)
    *CountPtr=0;
  if (RegVer)
    mprintf("%s: %s",MSG(MRegistered),TruncRegName);
  else
    Text(MShareware);

  CmdLine->SaveBackground(0,0,ScrX,ScrY);

  FPanels=new FilePanels();
  this->MainKeyBar=&(FPanels->MainKeyBar);
  this->TopMenuBar=&(FPanels->TopMenuBar);
  FPanels->Init();
  FPanels->SetScreenPosition();

  _beginthread(CheckVersion,0x10000,NULL);

  Cp()->LeftPanel->Update(0);
  Cp()->RightPanel->Update(0);
  /* $ 07.09.2000 tran
    + Config//Current File */
  if (Opt.AutoSaveSetup)
  {
      Cp()->LeftPanel->GoToFile(Opt.LeftCurFile);
      Cp()->RightPanel->GoToFile(Opt.RightCurFile);
  }
  /* tran 07.09.2000 $ */

  CmdLine->Show();
  if(Opt.ShowKeyBar)
    this->MainKeyBar->Show();

  Plugins.LoadPlugins();
  FrameManager->InsertFrame(FPanels);

  char StartCurDir[NM];
  Cp()->ActivePanel->GetCurDir(StartCurDir);
  chdir(StartCurDir);
  Cp()->ActivePanel->SetFocus();

//  _SVS(SysLog("ActivePanel->GetCurDir='%s'",StartCurDir));
//  _SVS(char PPP[NM];Cp()->GetAnotherPanel(Cp()->ActivePanel)->GetCurDir(PPP);SysLog("AnotherPanel->GetCurDir='%s'",PPP));
}

void ControlObject::CreateFilePanels()
{
  FPanels=new FilePanels();
}

ControlObject::~ControlObject()
{
  _OT(SysLog("[%p] ControlObject::~ControlObject()", this));
  if (Cp()&&Cp()->ActivePanel!=NULL)
  {
    if (Opt.AutoSaveSetup)
      SaveConfig(0);
    if (Cp()->ActivePanel->GetMode()!=PLUGIN_PANEL)
    {
      char CurDir[NM];
      Cp()->ActivePanel->GetCurDir(CurDir);
      FolderHistory->AddToHistory(CurDir,NULL,0);
    }
  }

  FrameManager->CloseAll();
  FPanels=NULL;

  Plugins.SendExit();
  PanelFilter::CloseFilter();
  delete CmdHistory;
  delete FolderHistory;
  delete ViewHistory;
  delete CmdLine;
  /* $ 06.05.2001 DJ
     удаляем то, что создали динамически
  */
  delete HiFiles;
  delete GrpSort;

  if (Opt.SaveViewerPos)
    ViewerPosCache->Save("Viewer\\LastPositions");
  delete ViewerPosCache;

  if (Opt.EdOpt.SavePos)
    EditorPosCache->Save("Editor\\LastPositions");

  delete EditorPosCache;
  delete FrameManager;
  /* DJ $ */
  Lang.Close();
  CtrlObject=NULL;
}


/* $ 25.11.2000 SVS
   Copyright в 2 строки
*/
/* $ 15.12.2000 SVS
 Метод ShowCopyright - public static & параметр Flags.
*/
void ControlObject::ShowCopyright(DWORD Flags)
{
  char Str[256];
  char *Line2=NULL;
  strcpy(Str,Copyright);
  char Xor=17;
  for (int I=0;Str[I];I++)
  {
    Str[I]=(Str[I]&0x7f)^Xor;
    Xor^=Str[I];
    if(Str[I] == '\n')
    {
      Line2=&Str[I+1];
      Str[I]='\0';
    }
  }
  if(Flags&1)
  {
    fprintf(stderr,"%s\n%s\n",Str,Line2);
  }
  else
  {
#ifdef BETA
    mprintf("Beta version %d.%02d.%d",BETA/1000,(BETA%1000)/10,BETA%10);
#else
    ScrollScreen(2+Line2?1:0);
    if(Line2)
    {
      GotoXY(0,ScrY-4);
      Text(Str);
      GotoXY(0,ScrY-3);
      Text(Line2);
    }
    else
      Text(Str);
#endif
  }
}
/* SVS $ */
/* SVS $ */


FilePanels* ControlObject::Cp()
{
  return FPanels;
}
