/*
manager.cpp

Переключение между несколькими file panels, viewers, editors, dialogs

*/

/* Revision: 1.83 08.04.2003 $ */

/*
Modify:
  08.04.2003 SVS
    - Висюн ФАРа, если в плагиновом StartupInfo до старта манагера вызывается что-то активное
  21.01.2003 SVS
    + xf_malloc,xf_realloc,xf_free - обертки вокруг malloc,realloc,free
      Просьба блюсти порядок и прописывать именно xf_* вместо простых.
  10.01.2003 SVS
    + Для отработки процесса исключений самого ФАРа (эмуляция) в manager.cpp
      добавлен код, вызывающий менюху по Ctrl-Alt-Apps с последующим
      исполнением "плохого" кода. ЭТОТ будет только для "закрытых" альф
      (только для тестеров).
      Если кто знает как остальные баги проэмульгировать - добавляйте
  07.11.2002 SVS
    ! Не компиляция под Borland 5.5
  08.10.2002 SVS
    - BugZ#675 - Неправильно вычисляется ширина меню со списком окон
  26.09.2002 SVS
    + Вынесем код по ресайзу в функцию ResizeAllFrame()
  20.06.2002 SVS
    ! Для хелпа и диалога по F12 не кажим список скринов.
  18.06.2002 SVS
    + В манагер добавлена переменная StartManager, отвечающая на вопрос
      "Манагер уже стартовал?"
  24.05.2002 SVS
    + Обработка KEY_ALTINS (временно закомменчено, до выяснения обстоятельств)
  22.05.2002 SKV
    + удалён институт semimodal фрэймов, и всё что с ним связано.
    + У ExecuteNonModal своё цикл работы.
  14.05.2002 SKV
    - Куча багов связанных с переключениями между полумодальными
      редакторами и вьюерами.
  15.05.2002 SVS
    ! Сделаем виртуальный метод Frame::InitKeyBar и будем его вызывать
      для всех Frame в методе Manager::InitKeyBar.
  28.04.2002 KM
    - Баг с зацикливанием:
      F12 F1 F12 F1 F12 F1... и так далее из любого фрейма
      приводило к бякам с перерисовкой.
    - Bug#485
      ===
      Вызываем UserMenu: F2 Alt-F4 и попадаем в модальный редактор.

      Далее:
      1. Ctrl-O - видим User Screen
      2. CAS    - (no comment)

      ImmediateHide говорите. Хе!
      ===
  18.04.2002 SKV
    - фикс деактиватора.
  13.04.2002 KM
    - Устранён важный момент неперерисовки, когда один
      над другим находится несколько объектов VMenu. Пример:
      настройка цветов. Теперь AltF9 в диалоге настройки
      цветов корректно перерисовывает меню.
  07.04.2002 KM
    - Надо бы ещё и прорефрешить меню, находящееся над диалогом,
      иначе при активации ещё одного диалога поверх меню, с
      последующим его закрытием, меню перемещалось под первый
      диалог (точнее первый диалог рисовался поверх меню).
  04.04.2002 KM
    - Удавлен ещё один артефакт, приводящий к неперерисовке
      меню, вызванного из модального объекта. Проверялось так:
      до этого патча делаем AltF7, вызываем меню дисков (AltD),
      затем хелп (F1), затем AltF9, Esc - меню исчезло (!).
    - Устранено мелькание заголовка консоли при использовании
      CAS (Ctrl-Alt-Shift).
  30.03.2002 OT
    - После исправления бага №314 (патч 1250) отвалилось закрытие
      фара по кресту.
  27.03.2002 OT
    - Дыра в ДелитКоммит(). Не корректно удалялся фрейм, которого
      не было ни в модальном стеке, ни в списке немодальных фреймов.
      Проявлялось при попытке открыть залоченный файл из поиска файлов.
  22.03.2002 SVS
    - strcpy - Fuck!
    ! Уточнение для BugZ#386 - F12 вызывается всегда, но!
      Но для модального редактора (или вьювера из поиска)
      все пункты меню задисаблены.
  22.03.2002 SVS
    - BugZ#386 - Поиск файлов и редактор
  03.03.2002 SVS
    ! Если для VC вставить ключ /Gr, то видим кучу багов :-/
  15.02.2002 SVS
    ! Вызов ShowProcessList() вынесен в манагер
  29.01.2002 OT
    - падение фара в рефрешКоммит()
  08.01.2002 SVS
    - Бага с макросом, в котором есть Alt-F9 (смена режима)
  02.01.2002 IS
    ! Вывод правильной помощи по Shift-F1 в меню плагинов в редакторе/вьюере
    ! Если на панели QVIEW или INFO открыт файл, то считаем, что это
      полноценный вьюер и запускаем с соответствующим параметром плагины
  13.12.2001 OT
    ФАР виснет при закрытии по [X] при модальном фрейме. (Bug 175)
  15.11.2001 SVS
    - BugZ#66 - порча командной строки после двойного AltF9
      Добавив немного Sleep(1) избавляемся от бага...
  11.10.2001 IS
    + CountFramesWithName
  04.10.2001 OT
    Запуск немодального фрейма в модальном режиме
  21.09.2001 SVS
    ! расширим диалог
  18.09.2001 SVS
    ! Безхозный "_D(" заменен на "_OT(".
      2OT: Ты не против? А то для _ALGO этот макрос всю масть сбивает :-)
  31.08.2001 OT
    - Закрытие ExitAll() при far -e и изменении оного
  23.08.2001 OT
    - Исправление зависания фара по F10 из панелей с несохраненными файлами
      в редакторе. Криво была написана функция ExitAll()
  08.08.2001 OT
    - Исправление CtrlF10 в редакторе/вьюере
  28.07.2001 OT
    - Исправление зацикливания Reversi
  27.07.2001 SVS
    - При обновлении часиков учитывать тот факт, что они могут быть
      отключены (к тому же не в той функции вызов стоял)
    ! KEY_CTRLALTSHIFTPRESS уже не привелегированная
  26.07.2001 OT
    - Исправление far /e - AltF9
  26.07.2001 SVS
    ! VFMenu уничтожен как класс
  25.07.2001 SVS
    ! Принудительно вызовем ShowTime(1); в ActivateCommit() для того, чтобы
      редравить часы (ибо теперь они могут быть трех цветов :-)
  25.07.2001 SVS
    ! Во время назначения макроса не юзаем некотрое количество клавиш :-)
      Об этом нам говорит флаг IsProcessAssignMacroKey
  24.07.2001 OT
    - Исправление отрисовки CAS при 2-х и более модальных диалогах
  24.07.2001 SVS
    ! Заюзаем флаг NotUseCAS - чтобы не гасилось ничего для одиночного
      редатора/вьювера (far /e)
  23.07.2001 SVS
    ! Закомментим пока WaitInFastFind - они здесь как бы не нужны.
  19.07.2001 OT
    + Добавились новые члены и методв типа UnmodalizeХХХ
  18.07.2001 OT
    ! VFMenu
  11.07.2001 OT
    ! Перенос CtrlAltShift в Manager
  07.07.2001 IS
    + При выборе фреймов (F12), если фреймов больше 10, то используем для
      горячих клавиш буквы латинского алфавита, т.о. получаем всего не 10,
      а 36 горячих клавиш.
  29.06.2001 OT
    - Зацикливание при UpdateCommit()
  26.06.2001 SKV
    + PluginCommit(); вызов ACTL_COMMIT
  23.06.2001 OT
    - Решение проблемы "старика Мюллера"
    - некие проблемы при far -r. FramePos теперь инициализируется значение -1.
  14.06.2001 OT
    ! "Бунт" ;-)
  06.06.2001 OT
    - Исправлены баги приводившие к утечкам памяти в ситуации: AltF7->Enter->F3->F6->F6->:((
    - Перемудрил зачем-то с ExecuteFrame()...
  04.06.2001 OT
     Подпорка для "естественного" обновления экрана
  30.05.2001 OT
    - Баг типа memory leak после F6 в редакторе/вьюере. Исправлена функция UpdateCommit()
    - Приведение CloseAll() и ExitAll() к канонами NFZ.
    - Исправление ActivateCommit(). При некоторых обстоятельствах
      бестолку "проглатывался" ActivatedFrame.
    + AltF9 работет теперь не только в панелях, но и ... везде :)
  28.05.2001 OT
    - Исправление "Файл после F3 остается залоченным" (переписан DeleteCommit())
  26.05.2001 OT
    - Исправление ExucuteModal()
    + Новые методы ExecuteComit(), ExecuteFrame(), IndexOfStack()
    + Новый член Frame *ExecutedFrame;
    ! Исправление функций DeleteCommit(), UpdateCommit(), связанных с появлением ExecuteFrame
    ! Исправление поведения RefreshCommit() с учетом блокировок.
  25.05.2001 DJ
    - Исправлен трап при закрытии Alt-F7.
  23.05.2001 OT
    - Исправление бага - удаление Frame, не внесенного в список FrameList
  22.05.2001 OT
    + Добавился RefreshedFrame
  22.05.2001 DJ
    ! в ExecuteModal() прежде всего делаем явный commit (если остались
      подвисшие операции, возможны разные глюки)
  21.05.2001 DJ
    ! чистка внутренностей; в связи с появлением нового типа фреймов
      выкинуто GetFrameTypesCount(); не посылалось OnChangeFocus(0)
  21.05.2001 SVS
    ! struct MenuData|MenuItem
      Поля Selected, Checked, Separator и Disabled преобразованы в DWORD Flags
    ! Константы MENU_ - в морг
  16.05.2001 DJ
    ! возвращение ExecuteModal()
  15.05.2001 OT
    ! NWZ -> NFZ
  14.05.2001 OT
    - Борьба с F4 -> ReloadAgain
  12.05.2001 DJ
    ! Убран ModalFrame.Show() в Manager::ExecuteModal()
  12.05.2001 DJ
    ! FrameManager оторван от CtrlObject
    ! объединены ExecuteModal() и ExecuteModalPtr() (и о чем я думал, когда
      делал две функции?)
    ! ReplaceCurrentFrame() заменена на более универсальную ReplaceFrame()
      (с подачи ОТ)
  11.05.2001 OT
    ! Отрисовка Background
  10.05.2001 DJ
    + SwitchToPanels()
    * GetFrameTypesCount() не учитывает фрейм, который мы собрались удалять
    + ModalStack
    - всякие перетряхи логики DestroyFrame() и иже с ними
  07.05.2001 SVS
    ! SysLog(); -> _D(SysLog());
  07.05.2001 DJ
    ! приведены в порядок CloseAll() и ExitAll()
  06.05.2001 DJ
    ! перетрях #include
    + ReplaceCurrentFrame()
  07.05.2001 ОТ
    - Баг с порядком индекса текущего фрейма FramePos при удалении
      какого-нибудь из списка :)
  06.05.2001 ОТ
    ! Переименование Window в Frame :)
  05.05.2001 DJ
    + перетрях NWZ
  04.05.2001 OT
    + Неверно формировалось меню плагинов по F11 (NWZ)
      Изменился PluginSet::CommandsMenu()
  29.04.2001 ОТ
    + Внедрение NWZ от Третьякова
  29.12.2000 IS
    + Метод ExitAll - аналог CloseAll, но разрешает продолжение полноценной
      работы в фаре, если пользователь продолжил редактировать файл.
      Возвращает TRUE, если все закрыли и можно выходить из фара.
  28.07.2000 tran 1.04
    + косметика при выводе списка окон -
      измененные файлы в редакторе маркируются "*"
  13.07.2000 SVS
    ! Некоторые коррекции при использовании new/delete/realloc
  11.07.2000 SVS
    ! Изменения для возможности компиляции под BC & VC
  28.06.2000 tran
    - NT Console resize
      add class member ActiveModal
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#include "headers.hpp"
#pragma hdrstop

#include "manager.hpp"
#include "global.hpp"
#include "fn.hpp"
#include "lang.hpp"
#include "keys.hpp"
#include "frame.hpp"
#include "vmenu.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "savescr.hpp"
#include "cmdline.hpp"
#include "ctrlobj.hpp"

Manager *FrameManager;

Manager::Manager()
{
  FrameList=NULL;
  FrameCount=FrameListSize=0;
  FramePos=-1;
  ModalStack=NULL;
  FrameList=(Frame **)xf_realloc(FrameList,sizeof(*FrameList)*(FrameCount+1));

  ModalStack=NULL;
  ModalStackSize = ModalStackCount = 0;
  EndLoop = FALSE;
  RefreshedFrame=NULL;

  CurrentFrame  = NULL;
  InsertedFrame = NULL;
  DeletedFrame  = NULL;
  ActivatedFrame= NULL;
  DeactivatedFrame=NULL;
  ModalizedFrame=NULL;
  UnmodalizedFrame=NULL;
  ExecutedFrame=NULL;
  //SemiModalBackFrames=NULL; //Теперь это массив
  //SemiModalBackFramesCount=0;
  //SemiModalBackFramesSize=0;
  ModalEVCount=0;
  StartManager=FALSE;
}

Manager::~Manager()
{
  if (FrameList)
    xf_free(FrameList);
  if (ModalStack)
    xf_free (ModalStack);
  /*if (SemiModalBackFrames)
    xf_free(SemiModalBackFrames);*/
}


/* $ 29.12.2000 IS
  Аналог CloseAll, но разрешает продолжение полноценной работы в фаре,
  если пользователь продолжил редактировать файл.
  Возвращает TRUE, если все закрыли и можно выходить из фара.
*/
BOOL Manager::ExitAll()
{
  int i;
  for (i=this->ModalStackCount-1; i>=0; i--){
    Frame *iFrame=this->ModalStack[i];
    if (!iFrame->GetCanLoseFocus(TRUE)){
      int PrevFrameCount=ModalStackCount;
      iFrame->ProcessKey(KEY_ESC);
      Commit();
      if (PrevFrameCount==ModalStackCount){
        return FALSE;
      }
    }
  }
  for (i=FrameCount-1; i>=0; i--){
    Frame *iFrame=FrameList[i];
    if (!iFrame->GetCanLoseFocus(TRUE)){
      ActivateFrame(iFrame);
      Commit();
      int PrevFrameCount=FrameCount;
      iFrame->ProcessKey(KEY_ESC);
      Commit();
      if (PrevFrameCount==FrameCount){
        return FALSE;
      }
    }
  }
  return TRUE;
}
/* IS $ */

void Manager::CloseAll()
{
  int i;
  Frame *iFrame;
  for (i=ModalStackCount-1;i>=0;i--){
    iFrame=ModalStack[i];
    DeleteFrame(iFrame);
    DeleteCommit();
    DeletedFrame=NULL;
  }
  for (i=FrameCount-1;i>=0;i--){
    iFrame=(*this)[i];
    DeleteFrame(iFrame);
    DeleteCommit();
    DeletedFrame=NULL;
  }
  /* $ 13.07.2000 SVS
     Здесь было "delete ModalList;", но перераспределение массива ссылок
     идет через realloc...
  */
  xf_free(FrameList);
  /* SVS $ */
  FrameList=NULL;
  FrameCount=FramePos=0;
}

BOOL Manager::IsAnyFrameModified(int Activate)
{
  for (int I=0;I<FrameCount;I++)
    if (FrameList[I]->IsFileModified())
    {
      if (Activate)
      {
        ActivateFrame(I);
        Commit();
      }
      return(TRUE);
    }

  return(FALSE);
}

void Manager::InsertFrame(Frame *Inserted, int Index)
{
  _OT(SysLog("InsertFrame(), Inserted=%p, Index=%i",Inserted, Index));
  if (Index==-1)
    Index=FramePos;
  InsertedFrame=Inserted;
}

void Manager::DeleteFrame(Frame *Deleted)
{
  _OT(SysLog("DeleteFrame(), Deleted=%p",Deleted));
  for (int i=0;i<FrameCount;i++){
    Frame *iFrame=FrameList[i];
    if(iFrame->RemoveModal(Deleted)){
      return;
    }
  }
  if (!Deleted){
    DeletedFrame=CurrentFrame;
  } else {
    DeletedFrame=Deleted;
  }
}

void Manager::DeleteFrame(int Index)
{
  _OT(SysLog("DeleteFrame(), Index=%i",Index));
  DeleteFrame(this->operator[](Index));
}


void Manager::ModalizeFrame (Frame *Modalized, int Mode)
{
  _OT(SysLog("ModalizeFrame(), Modalized=%p",Modalized));
  ModalizedFrame=Modalized;
  ModalizeCommit();
}

void Manager::UnmodalizeFrame (Frame *Unmodalized)
{
  UnmodalizedFrame=Unmodalized;
  UnmodalizeCommit();
}

void Manager::ExecuteNonModal ()
{
  _OT(SysLog("ExecuteNonModal(), ExecutedFrame=%p, InsertedFrame=%p, DeletedFrame=%p",ExecutedFrame, InsertedFrame, DeletedFrame));
  Frame *NonModal=InsertedFrame?InsertedFrame:(ExecutedFrame?ExecutedFrame:ActivatedFrame);
  if (!NonModal) {
    return;
  }
  /* $ 14.05.2002 SKV
    Положим текущий фрэйм в список "родителей" полумодальных фрэймов
  */
  //Frame *SaveFrame=CurrentFrame;
  //AddSemiModalBackFrame(SaveFrame);
  /* SKV $ */
  int NonModalIndex=IndexOf(NonModal);
  if (-1==NonModalIndex){
    InsertedFrame=NonModal;
    ExecutedFrame=NULL;
    InsertCommit();
    InsertedFrame=NULL;
  } else {
    ActivateFrame(NonModalIndex);
  }

  //Frame* ModalStartLevel=NonModal;
  while (1){
    Commit();
    if (CurrentFrame!=NonModal){
      break;
    }
    ProcessMainLoop();
  }

  //ExecuteModal(NonModal);
  /* $ 14.05.2002 SKV
    ... и уберём его же.
  */
  //RemoveSemiModalBackFrame(SaveFrame);
  /* SKV $ */
}

void Manager::ExecuteModal (Frame *Executed)
{
  _OT(SysLog("ExecuteModal(), Executed=%p, ExecutedFrame=%p",Executed,ExecutedFrame));
  if (!Executed && !ExecutedFrame){
    return;
  }
  if (Executed){
    if (ExecutedFrame) {
      _OT(SysLog("Попытка в одном цикле запустить в модальном режиме два фрейма. Executed=%p, ExecitedFrame=%p",Executed, ExecutedFrame));
      return;// NULL; //?? Определить, какое значение правильно возвращать в этом случае
    } else {
      ExecutedFrame=Executed;
    }
  }

  int ModalStartLevel=ModalStackCount;
  int OriginalStartManager=StartManager;
  StartManager=TRUE;
  while (1){
    Commit();
    if (ModalStackCount<=ModalStartLevel){
      break;
    }
    ProcessMainLoop();
  }
  StartManager=OriginalStartManager;
  return;// GetModalExitCode();
}

int Manager::GetModalExitCode()
{
  return ModalExitCode;
}

/* $ 11.10.2001 IS
   Подсчитать количество фреймов с указанным именем.
*/
int Manager::CountFramesWithName(const char *Name, BOOL IgnoreCase)
{
   int Counter=0;
   typedef int (__cdecl *cmpfunc_t)(const char *s1, const char *s2);
   cmpfunc_t cmpfunc=IgnoreCase?(cmpfunc_t)LocalStricmp:(cmpfunc_t)strcmp;
   char Type[200],curName[NM];
   for (int I=0;I<FrameCount;I++)
   {
     FrameList[I]->GetTypeAndName(Type,curName);
     if(!cmpfunc(Name, curName)) ++Counter;
   }
   return Counter;
}
/* IS $ */

/*!
  \return Возвращает NULL если нажат "отказ" или если нажат текущий фрейм.
  Другими словами, если немодальный фрейм не поменялся.
  Если же фрейм поменялся, то тогда функция должна возвратить
  указатель на предыдущий фрейм.
*/
Frame *Manager::FrameMenu()
{
  /* $ 28.04.2002 KM
      Флаг для определения того, что меню переключения
      экранов уже активировано.
  */
  static int AlreadyShown=FALSE;

  if (AlreadyShown)
    return NULL;
  /* KM $ */

  int ExitCode, CheckCanLoseFocus=CurrentFrame->GetCanLoseFocus();
  {
    struct MenuItem ModalMenuItem;
    memset(&ModalMenuItem,0,sizeof(ModalMenuItem));
    VMenu ModalMenu(MSG(MScreensTitle),NULL,0,ScrY-4);
    ModalMenu.SetHelp("ScrSwitch");
    ModalMenu.SetFlags(VMENU_WRAPMODE);
    ModalMenu.SetPosition(-1,-1,0,0);

    if (!CheckCanLoseFocus)
      ModalMenuItem.SetDisable(TRUE);

    for (int I=0;I<FrameCount;I++)
    {
      char Type[200],Name[NM*2],NumText[100];
      FrameList[I]->GetTypeAndName(Type,Name);
      /* $ 07.07.2001 IS
         Если фреймов больше 10, то используем для горячих клавиш буквы
         латинского алфавита, т.о. получаем всего не 10, а 36 горячих клавиш.
      */
      if (I<10)
        sprintf(NumText,"&%d. ",I);
      else if (I<36)
        sprintf(NumText,"&%c. ",I+55); // 55='A'-10
      else
        strcpy(NumText,"&   ");
      /* IS $ */
      /* $ 28.07.2000 tran
         файл усекает по ширине экрана */
      TruncPathStr(Name,ScrX-24);
      ReplaceStrings(Name,"&","&&",-1);
      /*  добавляется "*" если файл изменен */
      sprintf(ModalMenuItem.Name,"%s%-10.10s %c %s",NumText,Type,(FrameList[I]->IsFileModified()?'*':' '),Name);
      /* tran 28.07.2000 $ */
      ModalMenuItem.SetSelect(I==FramePos);
      ModalMenu.AddItem(&ModalMenuItem);
    }
    /* $ 28.04.2002 KM */
    AlreadyShown=TRUE;
    ModalMenu.Process();
    AlreadyShown=FALSE;
    /* KM $ */
    ExitCode=ModalMenu.Modal::GetExitCode();
  }

  if(CheckCanLoseFocus)
  {
    if (ExitCode>=0)
    {
      ActivateFrame (ExitCode);
      return (ActivatedFrame==CurrentFrame || !CurrentFrame->GetCanLoseFocus()?NULL:CurrentFrame);
    }
    return (ActivatedFrame==CurrentFrame?NULL:CurrentFrame);
  }
  return NULL;
}


int Manager::GetFrameCountByType(int Type)
{
  int ret=0;
  for (int I=0;I<FrameCount;I++)
  {
    /* $ 10.05.2001 DJ
       не учитываем фрейм, который собираемся удалять
    */
    if (FrameList[I] == DeletedFrame || FrameList [I]->GetExitCode() == XC_QUIT)
      continue;
    /* DJ $ */
    if (FrameList[I]->GetType()==Type)
      ret++;
  }
  return ret;
}

void Manager::SetFramePos(int NewPos)
{
  _OT(SysLog("Manager::SetFramePos(), NewPos=%i",NewPos));
  FramePos=NewPos;
}

/*$ 11.05.2001 OT Теперь можно искать файл не только по полному имени, но и отдельно - путь, отдельно имя */
int  Manager::FindFrameByFile(int ModalType,char *FileName,char *Dir)
{
  char bufFileName[NM*2];
  char *FullFileName=FileName;
  if (Dir)
  {
    strcpy(bufFileName,Dir);
    AddEndSlash(bufFileName);
    strcat(bufFileName,FileName);
    FullFileName=bufFileName;
  }

  for (int I=0;I<FrameCount;I++)
  {
    char Type[200],Name[NM];
    if (FrameList[I]->GetTypeAndName(Type,Name)==ModalType)
      if (LocalStricmp(Name,FullFileName)==0)
        return(I);
  }
  return(-1);
}
/* 11.05.2001 OT $*/

void Manager::ShowBackground()
{
  if (!RegVer)
  {
    Message(MSG_WARNING,1,MSG(MWarning),MSG(MRegOnly),MSG(MOk));
    return;
  }
  CtrlObject->CmdLine->ShowBackground();
}


void Manager::ActivateFrame(Frame *Activated)
{
  _OT(SysLog("ActivateFrame(), Activated=%i",Activated));
  if(IndexOf(Activated)==-1 && IndexOfStack(Activated)==-1)
    return;

  if (!ActivatedFrame)
  {
    ActivatedFrame=Activated;
  }
}

void Manager::ActivateFrame(int Index)
{
  _OT(SysLog("ActivateFrame(), Index=%i",Index));
  ActivateFrame((*this)[Index]);
}

void Manager::DeactivateFrame (Frame *Deactivated,int Direction)
{
  _OT(SysLog("DeactivateFrame(), Deactivated=%p",Deactivated));
  if (Direction) {
    FramePos+=Direction;
    if (Direction>0){
      if (FramePos>=FrameCount){
        FramePos=0;
      }
    } else {
      if (FramePos<0) {
        FramePos=FrameCount-1;
      }
    }
    ActivateFrame(FramePos);
  } else {
    // Direction==0
    // Direct access from menu or (in future) from plugin
  }
  DeactivatedFrame=Deactivated;
}

void Manager::RefreshFrame(Frame *Refreshed)
{
  _OT(SysLog("RefreshFrame(), Refreshed=%p",Refreshed));

  if (ActivatedFrame)
    return;

  if (Refreshed)
  {
    RefreshedFrame=Refreshed;
  }
  else
  {
    RefreshedFrame=CurrentFrame;
  }

  if(IndexOf(Refreshed)==-1 && IndexOfStack(Refreshed)==-1)
    return;

  /* $ 13.04.2002 KM
    - Вызываем принудительный Commit() для фрейма имеющего члена
      NextModal, это означает что активным сейчас является
      VMenu, а значит Commit() сам не будет вызван после возврата
      из функции.
      Устраняет ещё один момент неперерисовки, когда один над
      другим находится несколько объектов VMenu. Пример:
      настройка цветов. Теперь AltF9 в диалоге настройки
      цветов корректно перерисовывает меню.
  */
  if (RefreshedFrame && RefreshedFrame->NextModal)
    Commit();
  /* KM $ */
}

void Manager::RefreshFrame(int Index)
{
  RefreshFrame((*this)[Index]);
}

void Manager::ExecuteFrame(Frame *Executed)
{
  _OT(SysLog("ExecuteFrame(), Executed=%p",Executed));
  ExecutedFrame=Executed;
}


/* $ 10.05.2001 DJ
   переключается на панели (фрейм с номером 0)
*/

void Manager::SwitchToPanels()
{
  ActivateFrame (0);
}

/* DJ $ */


int Manager::HaveAnyFrame()
{
    if ( FrameCount || InsertedFrame || DeletedFrame || ActivatedFrame || RefreshedFrame ||
         ModalizedFrame || DeactivatedFrame || ExecutedFrame || CurrentFrame)
        return 1;
    return 0;
}

void Manager::EnterMainLoop()
{
  WaitInFastFind=0;
  StartManager=TRUE;
  while (1)
  {
    Commit();
    if (EndLoop || !HaveAnyFrame()) {
      break;
    }
    ProcessMainLoop();
  }
}

void Manager::ProcessMainLoop()
{

  WaitInMainLoop=IsPanelsActive();

  //WaitInFastFind++;
  int Key=GetInputRecord(&LastInputRecord);
  //WaitInFastFind--;
  WaitInMainLoop=FALSE;
  if (EndLoop)
    return;
  if (LastInputRecord.EventType==MOUSE_EVENT)
    ProcessMouse(&LastInputRecord.Event.MouseEvent);
  else
    ProcessKey(Key);
}

void Manager::ExitMainLoop(int Ask)
{
  if (CloseFAR)
  {
    CloseFAR=FALSE;
    CloseFARMenu=TRUE;
  };
  if (!Ask || !Opt.Confirm.Exit || Message(0,2,MSG(MQuit),MSG(MAskQuit),MSG(MYes),MSG(MNo))==0)
   /* $ 29.12.2000 IS
      + Проверяем, сохранены ли все измененные файлы. Если нет, то не выходим
        из фара.
   */
   if(ExitAll())
   {
   /* IS $ */
     if (!CtrlObject->Cp()->LeftPanel->ProcessPluginEvent(FE_CLOSE,NULL) && !CtrlObject->Cp()->RightPanel->ProcessPluginEvent(FE_CLOSE,NULL))
       EndLoop=TRUE;
   } else {
     CloseFARMenu=FALSE;
   }
}

#if defined(FAR_ALPHA_VERSION)
#include <float.h>

static void Test_EXCEPTION_STACK_OVERFLOW(char* target)
{
   char Buffer[1024]; /* чтобы быстрее рвануло */
   strcpy( Buffer, "zzzz" );
   Test_EXCEPTION_STACK_OVERFLOW( Buffer );
}
#endif


int  Manager::ProcessKey(int Key)
{
  int ret=FALSE;
  _OT(char kn[32]);
  _OT(KeyToText(Key,kn));
  //    _D(SysLog(1,"Manager::ProcessKey(), key=%i, '%s'",Key,kn));

  if ( CurrentFrame)
  {
    //      _D(SysLog("Manager::ProcessKey(), to CurrentFrame 0x%p, '%s'",CurrentFrame, CurrentFrame->GetTypeName()));
    int i=0;

#if defined(FAR_ALPHA_VERSION)
// сей код для проверки исключатор, просьба не трогать :-)
    if(GetRegKey("System\\Exception","Used",0) && Key == (KEY_APPS|KEY_CTRL|KEY_ALT))
    {
      struct __ECODE {
        DWORD Code;
        char *Name;
      } ECode[]={
        {EXCEPTION_ACCESS_VIOLATION,"Access Violation (Read)"},
        {EXCEPTION_ACCESS_VIOLATION,"Access Violation (Write)"},
        {EXCEPTION_INT_DIVIDE_BY_ZERO,"Divide by zero"},
        {EXCEPTION_ILLEGAL_INSTRUCTION,"Illegal instruction"},
        {EXCEPTION_STACK_OVERFLOW,"Stack Overflow"},
        {EXCEPTION_FLT_DIVIDE_BY_ZERO,"Floating-point divide by zero"},
/*
        {EXCEPTION_FLT_OVERFLOW,"EXCEPTION_FLT_OVERFLOW"},
        {EXCEPTION_DATATYPE_MISALIGNMENT,"EXCEPTION_DATATYPE_MISALIGNMENT",},
        {EXCEPTION_BREAKPOINT,"EXCEPTION_BREAKPOINT",},
        {EXCEPTION_SINGLE_STEP,"EXCEPTION_SINGLE_STEP",},
        {EXCEPTION_ARRAY_BOUNDS_EXCEEDED,"EXCEPTION_ARRAY_BOUNDS_EXCEEDED",},
        {EXCEPTION_FLT_DENORMAL_OPERAND,"EXCEPTION_FLT_DENORMAL_OPERAND",},
        {EXCEPTION_FLT_INEXACT_RESULT,"EXCEPTION_FLT_INEXACT_RESULT",},
        {EXCEPTION_FLT_INVALID_OPERATION,"EXCEPTION_FLT_INVALID_OPERATION",},
        {EXCEPTION_FLT_STACK_CHECK,"EXCEPTION_FLT_STACK_CHECK",},
        {EXCEPTION_FLT_UNDERFLOW,"EXCEPTION_FLT_UNDERFLOW",},
        {EXCEPTION_INT_OVERFLOW,"EXCEPTION_INT_OVERFLOW",0},
        {EXCEPTION_PRIV_INSTRUCTION,"EXCEPTION_PRIV_INSTRUCTION",0},
        {EXCEPTION_IN_PAGE_ERROR,"EXCEPTION_IN_PAGE_ERROR",0},
        {EXCEPTION_NONCONTINUABLE_EXCEPTION,"EXCEPTION_NONCONTINUABLE_EXCEPTION",0},
        {EXCEPTION_INVALID_DISPOSITION,"EXCEPTION_INVALID_DISPOSITION",0},
        {EXCEPTION_GUARD_PAGE,"EXCEPTION_GUARD_PAGE",0},
        {EXCEPTION_INVALID_HANDLE,"EXCEPTION_INVALID_HANDLE",0},
*/
      };

      struct MenuItem ModalMenuItem;
      memset(&ModalMenuItem,0,sizeof(ModalMenuItem));
      VMenu ModalMenu("Test Exceptions",NULL,0,ScrY-4);
      ModalMenu.SetFlags(VMENU_WRAPMODE);
      ModalMenu.SetPosition(-1,-1,0,0);

      for (int I=0;I<sizeof(ECode)/sizeof(ECode[0]);I++)
      {
        strcpy(ModalMenuItem.Name,ECode[I].Name);
        ModalMenu.AddItem(&ModalMenuItem);
      }

      ModalMenu.Process();
      int ExitCode=ModalMenu.Modal::GetExitCode();

      switch(ExitCode)
      {
        case 0:
          return *(int*)0;
        case 1:
          *(int*)0 = 0;
          break;
        case 2:
          return i / 0;
        case 3:
          #if !defined(SYSLOG)
          // у компилера под дебаг крышу сносит от такой наглости :-)
          ((void (*)(void))(void *)"\xF0\x0F\xC7\xC8\xCF")();
          #endif
          return 0;
        case 4:
          Test_EXCEPTION_STACK_OVERFLOW(NULL);
          return 0;
        case 5:
        {
          double a = 0;
          a = 1 / a;
          return 0;
        }
      }
      return TRUE;
    }
#endif

    /*** БЛОК ПРИВЕЛЕГИРОВАННЫХ КЛАВИШ ! ***/
    /***   КОТОРЫЕ НЕЛЬЗЯ НАМАКРОСИТЬ    ***/
    switch(Key)
    {
//      case (KEY_ALT|KEY_NUMPAD0):
//      case (KEY_ALT|KEY_INS):
//      {
//        RunGraber();
//        return TRUE;
//      }

      case KEY_CONSOLE_BUFFER_RESIZE:
        Sleep(1);
        ResizeAllFrame();
        return TRUE;
    }

    /*** А вот здесь - все остальное! ***/
    if(!IsProcessAssignMacroKey || IsProcessVE_FindFile)
       // в любом случае если кому-то ненужны все клавиши или
    {
      /* ** Эти клавиши разрешены для работы вьювера/редактора
            во время вызова онных из поиска файлов ** */
      switch(Key)
      {
        case KEY_CTRLW:
          ShowProcessList();
          return(TRUE);

        case KEY_F11:
          PluginsMenu();
          FrameManager->RefreshFrame();
          _OT(SysLog(-1));
          return TRUE;

        case KEY_ALTF9:
        {
          //_SVS(SysLog(1,"Manager::ProcessKey, KEY_ALTF9 pressed..."));
          Sleep(1);
          SetVideoMode(FarAltEnter(-2));
          Sleep(1);

          /* В процессе исполнения Alt-F9 (в нормальном режиме) в очередь
             консоли попадает WINDOW_BUFFER_SIZE_EVENT, формируется в
             ChangeVideoMode().
             В режиме исполнения макросов ЭТО не происходит по вполне понятным
             причинам.
          */
          if(CtrlObject->Macro.IsExecuting())
          {
            int PScrX=ScrX;
            int PScrY=ScrY;
            Sleep(1);
            GetVideoMode(CurScreenBufferInfo);
            if (PScrX+1 == CurScreenBufferInfo.dwSize.X &&
                PScrY+1 == CurScreenBufferInfo.dwSize.Y)
            {
              //_SVS(SysLog(-1,"GetInputRecord(WINDOW_BUFFER_SIZE_EVENT); return KEY_NONE"));
              return TRUE;
            }
            else
            {
              PrevScrX=PScrX;
              PrevScrY=PScrY;
              //_SVS(SysLog(-1,"GetInputRecord(WINDOW_BUFFER_SIZE_EVENT); return KEY_CONSOLE_BUFFER_RESIZE"));
              Sleep(1);

              return ProcessKey(KEY_CONSOLE_BUFFER_RESIZE);
            }
          }
          //_SVS(SysLog(-1));
          return TRUE;
        }

        case KEY_F12:
        {
          int TypeFrame=FrameManager->GetCurrentFrame()->GetType();
          if(TypeFrame != MODALTYPE_HELP && TypeFrame != MODALTYPE_DIALOG)
          {
            DeactivateFrame(FrameMenu(),0);
            _OT(SysLog(-1));
            return TRUE;
          }
          break; // отдадим F12 дальше по цепочке
        }
      }

      // а здесь то, что может быть запрещено везде :-)
      if(!IsProcessVE_FindFile)
      {
        switch(Key)
        {
          case KEY_CTRLALTSHIFTPRESS:
            if(!NotUseCAS)
            {
              if (CurrentFrame->FastHide()){
                ImmediateHide();
                WaitKey(KEY_CTRLALTSHIFTRELEASE);
                FrameManager->RefreshFrame();
              }
            }
            return TRUE;

          case KEY_CTRLTAB:
          case KEY_CTRLSHIFTTAB:
            if (CurrentFrame->GetCanLoseFocus()){
              DeactivateFrame(CurrentFrame,Key==KEY_CTRLTAB?1:-1);
            }
            _OT(SysLog(-1));
            return TRUE;
        }
      }
    }
    CurrentFrame->UpdateKeyBar();
    CurrentFrame->ProcessKey(Key);
  }
  _OT(SysLog(-1));
  return ret;
}

int  Manager::ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent)
{
    int ret=FALSE;
//    _D(SysLog(1,"Manager::ProcessMouse()"));
    if ( CurrentFrame)
        ret=CurrentFrame->ProcessMouse(MouseEvent);
//    _D(SysLog("Manager::ProcessMouse() ret=%i",ret));
    _OT(SysLog(-1));
    return ret;
}

void Manager::PluginsMenu()
{
  _OT(SysLog(1));
  int curType = CurrentFrame->GetType();
  if (curType == MODALTYPE_PANELS || curType == MODALTYPE_EDITOR || curType == MODALTYPE_VIEWER)
  {
    /* 02.01.2002 IS
       ! Вывод правильной помощи по Shift-F1 в меню плагинов в редакторе/вьюере
       ! Если на панели QVIEW или INFO открыт файл, то считаем, что это
         полноценный вьюер и запускаем с соответствующим параметром плагины
    */
    if(curType==MODALTYPE_PANELS)
    {
      int pType=CtrlObject->Cp()->ActivePanel->GetType();
      if(pType==QVIEW_PANEL || pType==INFO_PANEL)
      {
         char CurFileName[NM]="";
         CtrlObject->Cp()->GetTypeAndName(NULL,CurFileName);
         if(*CurFileName)
         {
           DWORD Attr=GetFileAttributes(CurFileName);
           // интересуют только обычные файлы
           if(Attr!=0xFFFFFFFF && !(Attr&FILE_ATTRIBUTE_DIRECTORY))
             curType=MODALTYPE_VIEWER;
         }
      }
    }

    // в редакторе или вьюере покажем свою помощь по Shift-F1
    char *Topic=curType==MODALTYPE_EDITOR?"Editor":
      curType==MODALTYPE_VIEWER?"Viewer":0;
    CtrlObject->Plugins.CommandsMenu(curType,0,Topic);
    /* IS $ */
  }
  _OT(SysLog(-1));
}

BOOL Manager::IsPanelsActive()
{
  if (FramePos>=0) {
    return CurrentFrame?CurrentFrame->GetType() == MODALTYPE_PANELS:FALSE;
  } else {
    return FALSE;
  }
}

Frame *Manager::operator[](int Index)
{
  if (Index<0||Index>=FrameCount ||FrameList==0){
    return NULL;
  }
  return FrameList[Index];
}

int Manager::IndexOfStack(Frame *Frame)
{
  int Result=-1;
  for (int i=0;i<ModalStackCount;i++)
  {
    if (Frame==ModalStack[i])
    {
      Result=i;
      break;
    }
  }
  return Result;
}

int Manager::IndexOf(Frame *Frame)
{
  int Result=-1;
  for (int i=0;i<FrameCount;i++)
  {
    if (Frame==FrameList[i])
    {
      Result=i;
      break;
    }
  }
  return Result;
}

BOOL Manager::Commit()
{
  _OT(SysLog(1));
  int Result = false;
  if (DeletedFrame && (InsertedFrame||ExecutedFrame)){
    UpdateCommit();
    DeletedFrame = NULL;
    InsertedFrame = NULL;
    ExecutedFrame=NULL;
    Result=true;
  } else if (ExecutedFrame) {
    ExecuteCommit();
    ExecutedFrame=NULL;
    Result=true;
  } else if (DeletedFrame){
    DeleteCommit();
    DeletedFrame = NULL;
    Result=true;
  } else if (InsertedFrame){
    InsertCommit();
    InsertedFrame = NULL;
    Result=true;
  } else if(DeactivatedFrame){
    DeactivateCommit();
    DeactivatedFrame=NULL;
    Result=true;
  } else if(ActivatedFrame){
    ActivateCommit();
    ActivatedFrame=NULL;
    Result=true;
  } else if (RefreshedFrame){
    RefreshCommit();
    RefreshedFrame=NULL;
    Result=true;
  } else if (ModalizedFrame){
    ModalizeCommit();
//    ModalizedFrame=NULL;
    Result=true;
  } else if (UnmodalizedFrame){
    UnmodalizeCommit();
//    UnmodalizedFrame=NULL;
    Result=true;
  }
  if (Result){
    Result=Commit();
  }
  _OT(SysLog(-1));
  return Result;
}

void Manager::DeactivateCommit()
{
  _OT(SysLog("DeactivateCommit(), DeactivatedFrame=%p",DeactivatedFrame));
  /*$ 18.04.2002 skv
    Если нечего активировать, то в общем-то не надо и деактивировать.
  */
  if (!DeactivatedFrame || !ActivatedFrame)
  {
    return;
  }
  /* skv $*/

  if (!ActivatedFrame)
  {
    _OT("WARNING!!!!!!!!");
  }

  if (DeactivatedFrame)
  {
    DeactivatedFrame->OnChangeFocus(0);
  }

  int modalIndex=IndexOfStack(DeactivatedFrame);
  if (-1 != modalIndex && modalIndex== ModalStackCount-1)
  {
    /*if (IsSemiModalBackFrame(ActivatedFrame))
    { // Является ли "родителем" полумодального фрэйма?
      ModalStackCount--;
    }
    else
    {*/
      if(IndexOfStack(ActivatedFrame)==-1)
      {
        ModalStack[ModalStackCount-1]=ActivatedFrame;
      }
      else
      {
        ModalStackCount--;
      }
//    }
  }
}


void Manager::ActivateCommit()
{
  _OT(SysLog("ActivateCommit(), ActivatedFrame=%p",ActivatedFrame));
  if (CurrentFrame==ActivatedFrame)
  {
    RefreshedFrame=ActivatedFrame;
    return;
  }

  int FrameIndex=IndexOf(ActivatedFrame);

  if (-1!=FrameIndex)
  {
    FramePos=FrameIndex;
  }
  /* 14.05.2002 SKV
    Если мы пытаемся активировать полумодальный фрэйм,
    то надо его вытащит на верх стэка модалов.
  */

  for(int I=0;I<ModalStackCount;I++)
  {
    if(ModalStack[I]==ActivatedFrame)
    {
      Frame *tmp=ModalStack[I];
      ModalStack[I]=ModalStack[ModalStackCount-1];
      ModalStack[ModalStackCount-1]=tmp;
      break;
    }
  }
  /* SKV $ */

  RefreshedFrame=CurrentFrame=ActivatedFrame;
}

void Manager::UpdateCommit()
{
  _OT(SysLog("UpdateCommit(), DeletedFrame=%p, InsertedFrame=%p, ExecutedFrame=%p",DeletedFrame,InsertedFrame, ExecutedFrame));
  if (ExecutedFrame){
    DeleteCommit();
    ExecuteCommit();
    return;
  }
  int FrameIndex=IndexOf(DeletedFrame);
  if (-1!=FrameIndex){
    ActivateFrame(FrameList[FrameIndex] = InsertedFrame);
    ActivatedFrame->FrameToBack=CurrentFrame;
    DeleteCommit();
  } else {
    _OT(SysLog("UpdateCommit(). ОШИБКА Не найден удаляемый фрейм"));
  }
}

//! Удаляет DeletedFrame изо всех очередей!
//! Назначает следующий активный, (исходя из своих представлений)
//! Но только в том случае, если активный фрейм еще не назначен заранее.
void Manager::DeleteCommit()
{
  _OT(SysLog("DeleteCommit(), DeletedFrame=%p",DeletedFrame));
  if (!DeletedFrame)
  {
    return;
  }

  // <ifDoubleInstance>
  //BOOL ifDoubI=ifDoubleInstance(DeletedFrame);
  // </ifDoubleInstance>
  int ModalIndex=IndexOfStack(DeletedFrame);
  if (ModalIndex!=-1)
  {
    /* $ 14.05.2002 SKV
      Надёжнее найти и удалить именно то, что
      нужно, а не просто верхний.
    */
    for(int i=0;i<ModalStackCount;i++)
    {
      if(ModalStack[i]==DeletedFrame)
      {
        for(int j=i+1;j<ModalStackCount;j++)
        {
          ModalStack[j-1]=ModalStack[j];
        }
        ModalStackCount--;
        break;
      }
    }
    /* SKV $ */
    if (ModalStackCount)
    {
      ActivateFrame(ModalStack[ModalStackCount-1]);
    }
  }

  for (int i=0;i<FrameCount;i++)
  {
    if (FrameList[i]->FrameToBack==DeletedFrame)
    {
      FrameList[i]->FrameToBack=CtrlObject->Cp();
    }
  }

  int FrameIndex=IndexOf(DeletedFrame);
  if (-1!=FrameIndex)
  {
    DeletedFrame->DestroyAllModal();
    for (int j=FrameIndex; j<FrameCount-1; j++ ){
      FrameList[j]=FrameList[j+1];
    }
    FrameCount--;
    if ( FramePos >= FrameCount ) {
      FramePos=0;
    }
    if (DeletedFrame->FrameToBack==CtrlObject->Cp()){
      ActivateFrame(FrameList[FramePos]);
    } else {
      ActivateFrame(DeletedFrame->FrameToBack);
    }
  }

  /* $ 14.05.2002 SKV
    Долго не мог понять, нужен всё же этот код или нет.
    Но вроде как нужен.
    SVS> Когда понадобится - в некоторых местах расскомментить куски кода
         помеченные скобками <ifDoubleInstance>

  if (ifDoubI && IsSemiModalBackFrame(ActivatedFrame)){
    for(int i=0;i<ModalStackCount;i++)
    {
      if(ModalStack[i]==ActivatedFrame)
      {
        break;
      }
    }

    if(i==ModalStackCount)
    {
      if (ModalStackCount == ModalStackSize){
        ModalStack = (Frame **) xf_realloc (ModalStack, ++ModalStackSize * sizeof (Frame *));
      }
      ModalStack[ModalStackCount++]=ActivatedFrame;
    }
  }
  */
  /* SKV $ */


  DeletedFrame->OnDestroy();
  if (DeletedFrame->GetDynamicallyBorn())
  {
    _tran(SysLog("delete DeletedFrame %p, CurrentFrame=%p",DeletedFrame,CurrentFrame));
    if ( CurrentFrame==DeletedFrame )
      CurrentFrame=0;
    /* $ 14.05.2002 SKV
      Так как в деструкторе фрэйма неявно может быть
      вызван commit, то надо подстраховаться.
    */
    Frame *tmp=DeletedFrame;
    DeletedFrame=NULL;
    delete tmp;
    /* SKV $ */
  }
  // Полагаемся на то, что в ActevateFrame не будет переписан уже
  // присвоенный  ActivatedFrame
  if (ModalStackCount){
    ActivateFrame(ModalStack[ModalStackCount-1]);
  } else {
    ActivateFrame(FramePos);
  }
}

void Manager::InsertCommit()
{
  _OT(SysLog("InsertCommit(), InsertedFrame=%p",InsertedFrame));
  if (InsertedFrame){
    if (FrameListSize <= FrameCount)
    {
      FrameList=(Frame **)xf_realloc(FrameList,sizeof(*FrameList)*(FrameCount+1));
      FrameListSize++;
    }
    InsertedFrame->FrameToBack=CurrentFrame;
    FrameList[FrameCount]=InsertedFrame;
    if (!ActivatedFrame){
      ActivatedFrame=InsertedFrame;
    }
    FrameCount++;
  }
}

void Manager::RefreshCommit()
{
  _OT(SysLog("RefreshCommit(), RefreshedFrame=%p,Refreshable()=%i",RefreshedFrame,RefreshedFrame->Refreshable()));
  if (!RefreshedFrame)
    return;

  if(IndexOf(RefreshedFrame)==-1 && IndexOfStack(RefreshedFrame)==-1)
    return;

  if (RefreshedFrame->Refreshable())
  {
    if (!IsRedrawFramesInProcess)
      RefreshedFrame->ShowConsoleTitle();
    RefreshedFrame->Refresh();
    if (!RefreshedFrame)
      return;
    CtrlObject->Macro.SetMode(RefreshedFrame->GetMacroMode());
  }
  if (Opt.ViewerEditorClock &&
      (RefreshedFrame->GetType() == MODALTYPE_EDITOR ||
      RefreshedFrame->GetType() == MODALTYPE_VIEWER)
      || WaitInMainLoop && Opt.Clock)
    ShowTime(1);
}

void Manager::ExecuteCommit()
{
  _OT(SysLog("ExecuteCommit(), ExecutedFrame=%p",ExecutedFrame));
  if (!ExecutedFrame) {
    return;
  }
  if (ModalStackCount == ModalStackSize){
    ModalStack = (Frame **) xf_realloc (ModalStack, ++ModalStackSize * sizeof (Frame *));
  }
  ModalStack [ModalStackCount++] = ExecutedFrame;
  ActivatedFrame=ExecutedFrame;
}

/*$ 26.06.2001 SKV
  Для вызова из плагинов посредством ACTL_COMMIT
*/
BOOL Manager::PluginCommit()
{
  return Commit();
}
/* SKV$*/

/* $ Введена для нужд CtrlAltShift OT */
void Manager::ImmediateHide()
{
  if (FramePos<0)
    return;

  // Сначала проверяем, есть ли у прятываемого фрейма SaveScreen
  if (CurrentFrame->HasSaveScreen())
  {
    CurrentFrame->Hide();
    return;
  }

  // Фреймы перерисовываются, значит для нижних
  // не выставляем заголовок консоли, чтобы не мелькал.
  if (ModalStackCount>0)
  {
    /* $ 28.04.2002 KM
        Проверим, а не модальный ли редактор или вьювер на вершине
        модального стека? И если да, покажем User screen.
    */
    if (ModalStack[ModalStackCount-1]->GetType()==MODALTYPE_EDITOR ||
        ModalStack[ModalStackCount-1]->GetType()==MODALTYPE_VIEWER)
    {
      CtrlObject->CmdLine->ShowBackground();
    }
    else
    {
      int UnlockCount=0;
      /* $ 07.04.2002 KM */
      IsRedrawFramesInProcess++;
      /* KM $ */

      while (!(*this)[FramePos]->Refreshable())
      {
        (*this)[FramePos]->UnlockRefresh();
        UnlockCount++;
      }
      RefreshFrame((*this)[FramePos]);

      Commit();
      for (int i=0;i<UnlockCount;i++)
      {
        (*this)[FramePos]->LockRefresh();
      }

      if (ModalStackCount>1)
      {
        for (int i=0;i<ModalStackCount-1;i++)
        {
          if (!(ModalStack[i]->FastHide() & CASR_HELP))
          {
            RefreshFrame(ModalStack[i]);
            Commit();
          }
          else
          {
            break;
          }
        }
      }
      /* $ 04.04.2002 KM
         Перерисуем заголовок только у активного фрейма.
         Этим мы предотвращаем мелькание заголовка консоли
         при перерисовке всех фреймов.
      */
      IsRedrawFramesInProcess--;
      CurrentFrame->ShowConsoleTitle();
      /* KM $ */
    }
    /* KM $ */
  }
  else
  {
    CtrlObject->CmdLine->ShowBackground();
  }
}

void Manager::ModalizeCommit()
{
  CurrentFrame->Push(ModalizedFrame);
  ModalizedFrame=NULL;
}

void Manager::UnmodalizeCommit()
{
  int i;
  Frame *iFrame;
  for (i=0;i<FrameCount;i++)
  {
    iFrame=FrameList[i];
    if(iFrame->RemoveModal(UnmodalizedFrame))
    {
      break;
    }
  }

  for (i=0;i<ModalStackCount;i++)
  {
    iFrame=ModalStack[i];
    if(iFrame->RemoveModal(UnmodalizedFrame))
    {
      break;
    }
  }
  UnmodalizedFrame=NULL;
}
/* OT $*/

/* $ 15.05.2002 SKV
  Чуток подправим логику.
*/

BOOL Manager::ifDoubleInstance(Frame *frame)
{
  // <ifDoubleInstance>
/*
  if (ModalStackCount<=0)
    return FALSE;
  if(IndexOfStack(frame)==-1)
    return FALSE;
  if(IndexOf(frame)!=-1)
    return TRUE;
*/
  // </ifDoubleInstance>
  return FALSE;
}

/* SKV $ */

/*  Вызов ResizeConsole для всех NextModal у
    модального фрейма. KM
*/
void Manager::ResizeAllModal(Frame *ModalFrame)
{
  if (!ModalFrame->NextModal)
    return;

  Frame *iModal=ModalFrame->NextModal;
  while (iModal)
  {
    iModal->ResizeConsole();
    iModal=iModal->NextModal;
  }
}
/* KM $ */

void Manager::ResizeAllFrame()
{
  int I;
  for (I=0; I < FrameCount; I++)
  {
    FrameList[I]->ResizeConsole();
  }

  for (I=0; I < ModalStackCount; I++)
  {
    ModalStack[I]->ResizeConsole();
    /* $ 13.04.2002 KM
      - А теперь проресайзим все NextModal...
    */
    ResizeAllModal(ModalStack[I]);
    /* KM $ */
  }
  ImmediateHide();
  FrameManager->RefreshFrame();
  //RefreshFrame();
}

void Manager::InitKeyBar(void)
{
  for (int I=0;I < FrameCount;I++)
    FrameList[I]->InitKeyBar();
}

/*void Manager::AddSemiModalBackFrame(Frame* frame)
{
  if(SemiModalBackFramesCount>=SemiModalBackFramesSize)
  {
    SemiModalBackFramesSize+=4;
    SemiModalBackFrames=
      (Frame**)xf_realloc(SemiModalBackFrames,sizeof(Frame*)*SemiModalBackFramesSize);

  }
  SemiModalBackFrames[SemiModalBackFramesCount]=frame;
  SemiModalBackFramesCount++;
}

BOOL Manager::IsSemiModalBackFrame(Frame *frame)
{
  if(!SemiModalBackFrames)return FALSE;
  for(int i=0;i<SemiModalBackFramesCount;i++)
  {
    if(SemiModalBackFrames[i]==frame)return TRUE;
  }
  return FALSE;
}

void Manager::RemoveSemiModalBackFrame(Frame* frame)
{
  if(!SemiModalBackFrames)return;
  for(int i=0;i<SemiModalBackFramesCount;i++)
  {
    if(SemiModalBackFrames[i]==frame)
    {
      for(int j=i+1;j<SemiModalBackFramesCount;j++)
      {
        SemiModalBackFrames[j-1]=SemiModalBackFrames[j];
      }
      SemiModalBackFramesCount--;
      return;
    }
  }
}
*/
