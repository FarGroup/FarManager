/*
keyboard.cpp

Функций, имеющие отношение к клавитуре

*/

/* Revision: 1.63 05.02.2002 $ */

/*
Modify:
  05.02.2002 SVS
    ! технологический патч - про сислоги
    + SpecKeyName - доступно только при отладке (SysLog)
  04.02.2002 SVS
    + IsNavKey(), IsShiftKey()
  29.01.2002 SVS
    - Была неверная отложенная установка заголовка консоли (неверное,
      старое значение выставлялось)
  10.01.2002 SVS
    ! Устем правило Opt.HotkeyRules для Alt-буква
  27.12.2001 KM
    ! Обнулим KeyText в KeyToText (как и было раньше), в противном случае
      в буфере клавиши возвращался мусор!
  26.12.2001 SVS
    ! При закрытии окна "по кресту"... теперь настраиваемо!
  25.12.2001 SVS
    ! В режиме WaitInMainLoop=1 Alt-символ и Alt-Shift-символ транслируем как
      есть (чтобы корректно работал FastFind). В остальных случаях - берем
      абс.значения кода клавиши.
    ! немного оптимизации
  21.12.2001 SVS
    + DOUBLE_CLICK as Macro
  08.12.2001 SVS
    - Бага в KeyNameToKey() - например, для "ShiftFooBar" выдаст KEY_SHIFT,
      Лечится путем сравнения Pos и Len, т.е. в нормальной ситуации должно
      быть Pos == Len
  26.11.2001 SVS
    + MouseEventFlags, PreMouseEventFlags - типы эвентов мыши
  20.11.2001 SVS
    ! В обработку FOCUS_EVENT добавим также обнуление LButtonPressed,
      RButtonPressed и MButtonPressed
  15.11.2001 SVS
    - BugZ#66 - порча командной строки после двойного AltF9
      Добавив немного Sleep(1) избавляемся от бага...
  24.10.2001 SVS
    ! CheckForEsc() - нажатие Esc в диалоге равносильно нажатию кнопки Yes
      Плюс - запрещение Alt-F9 и скринсавер.
  21.10.2001 SVS
    + PrevScrX,PrevScrY - предыдущие размеры консоли (для позиционирования
      диалогов)
  15.10.2001 SVS
    + _KEYMACRO()
  11.10.2001 SVS
    - BugZ#79. Некоректное преобразование имен клавиш в коды (при ошибках в
      написании названия клавиш)
  20.09.2001 SVS
    - бага с Alt-цифровая клавиша.
    ! Параметр у InputRecordToKey "const"
  18.09.2001 SVS
    ! временно отменим "...теперь даже для макроса корректно заполняется..."
  14.09.2001 SVS
    - Бага в TranslateKeyToVK() - неверно формировалось поле для Shift-клавиш
    ! теперь даже для макроса корректно заполняется структура INPUT_RECORD
      в функции GetInputRecord()
  30.08.2001 IS
    ! При принудительном закрытии Фара пытаемся вести себя так же, как и при
      нажатии на F10 в панелях, только не запрашиваем подтверждение закрытия,
      если это возможно.
  24.07.2001 SVS
    ! Если ждем KEY_CTRLALTSHIFTRELEASE, то гасим курсор (!)
  23.07.2001 SVS
    - Не работала комбинация Alt (не отпуская, в панеля) sp_
      (не набирался симол подчеркивания при нажатой Alt)
  01.07.2001 KM
    - При отпускании Shift-Enter в диалоге назначения
      вылазил Shift после отпускания клавиш.
  28.06.2001 SVS
    - Для Win9x при нажатом NumLock и юзании курсорных клавиш
      получаем в диалоге назначения ерундистику.
  25.06.2001 IS
    ! Внедрение const
  23.06.2001 OT
    - far -r
  21.06.2001 SVS
    ! Удалена функция WriteSequenceInput() за ненадобностью
  14.06.2001 OT
    ! "Бунт" ;-)
  08.06.2001 SVS
    ! Исправление ситуации (влоб) с блокировкой мыши при запуске некоторых
      PE приложений!
    ! ENABLE_MOUSE_INPUT -> ENABLE_WINDOW_INPUT с подаче ER ;-)
  06.06.2001 SVS
    ! Уточнение в функции WriteInput - теперь wVirtualScanCode
      корректно транслируется.
    ! W-функции юзаем пока только в режиме USE_WFUNC
  06.06.2001 SVS
    ! Уточнение в функции TranslateKeyToVK - теперь wVirtualScanCode
      корректно транслируется.
  23.05.2001 SVS
    ! Макрос на Alt+Shift+Цифра
  17.05.2001 OT
    + при изменении размера консоли генерим KEY_CONSOLE_BUFFER_RESIZE.
  07.05.2001 SVS
    ! SysLog(); -> _D(SysLog());
  07.05.2001 SVS
    - При быстром поиске Alt-Shift-'-' не начинает поиск файлов, начинающихся
      с '_', а начинает с '-'. Раньше работало нормально.
  06.05.2001 DJ
    ! перетрях #include
  06.05.2001 ОТ
    ! Переименование Window в Frame :)
  05.05.2001 DJ
    + перетрях NWZ
  29.04.2001 ОТ
    + Внедрение NWZ от Третьякова
  28.04.2001 vvm
    + KEY_FOCUS_CHANGED для прорисовки кейбара.
  27.04.2001 SVS
    + Добавлены:
       "MsWheelDown" для KEY_MSWHEEL_DOWN и
       "MsWheelUp" для KEY_MSWHEEL_UP
      в массив FKeys1[]
    ! Не были учтены шифтовые клавиши при прокрутке колеса, из-за чего
      нельзя было использовать в макросах нечто вроде "ShiftMsWheelUp"
  26.04.2001 VVM
    - Выкинул нафиг MouseWheeled
    + Обработка спецклавиш KEY_MSWHEEL_XXXX
  24.04.2001 SVS
    + MouseWheeled - признак того, что крутанули колесо.
  16.04.2001 VVM
    + Прокрутка с шифтом подменяется на PgUp/PgDn
    + Opt.MouseWheelDelta - задает смещение для прокрутки. Сколько раз посылать UP/DOWN
  13.04.2001 VVM
    + Обработка колесика мышки под 2000.
  12.03.2001 SVS
    ! К терапевту предыдущие изменения - неудобно...
  07.03.2001 SVS
    ! Небольшое изменение - может быть и временное (как работает не пойму пока
      и на что потом повлияет - тоже не знаю, но Shift-F4 Shift-Enter, не
      отпуская Shift - работает корректно :-)
  28.02.2001 IS
    ! "CtrlObject->CmdLine." -> "CtrlObject->CmdLine->"
  25.02.2001 SVS
    ! Уточнения для Alt-Shift-...
  09.02.2001 IS
    + Подтверждение нажатия Esc в CheckForEsc (опционально)
  06.02.2001 SVS
    - ОНИ... :-)
      Приведение в порядок "непослушных" Divide & Multiple на цифровой клаве.
  05.02.2001 SVS
    - Снова про клавиши... :-( Все переводилось в Upper.
  01.02.2001 SVS
    - бага - неверно конвертилось имя клавиши :-(
  01.02.2001 SVS
    - Неверное преобразование клавиш в VK_* в функции TranslateKeyToVK()
      из-за чего не отрабатывал ENTER, etc в плагинах.
    - Не отрабатывались комбинации Alt-Shift-* и Alt-Shift-? при быстром
      поиске в панелях.
  28.01.2001 SVS
    ! Снова про... WriteInput (с учетом данных на VK_F16)
  24.01.2001 SVS
    ! Вернем взад содержимое WriteInput
  23.01.2001 SVS
    ! CalcKeyCode - дополнительный параметр - проверка на макросы.
    ! Исключения вызовов макросов при указании "не использовать макросы"
  21.01.2001 SVS
    ! Уточнения в WriteInput!
    ! WriteInput теперь возвращает результат в виде FALASE/TRUE.
    + WriteSequenceInput
  17.01.2001 SVS
    + Opt.ShiftsKeyRules - Правило на счет выбора механизма трансляции
      Alt-Буква для нелатинским буковок
  09.01.2001 SVS
    ! Грабер должен быть раньше Alt-Number-ввода
  05.01.2001 SVS
    - И снова CalcKeyCode - не работал Alt-Ins
  05.01.2001 SVS
    - База в "вычислителе" клавиш :-(
  04.01.2001 SVS
    + Переделка алгоритмов декодирования клавиш...
    + TranslateKeyToVK
  26.12.2000 SVS
    + вызов KeyMacroToText() (функция определена в macro.cpp)
  22.12.2000 SVS
    + Выделение в качестве самостоятельного модуля, после чего можно смело
      ваять интерфейс пвсевдоклавиатурного драйвера :-)
*/

#include "headers.hpp"
#pragma hdrstop

#include "keys.hpp"
#include "farqueue.hpp"
#include "global.hpp"
#include "fn.hpp"
#include "plugin.hpp"
#include "lang.hpp"
#include "ctrlobj.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "cmdline.hpp"
#include "grabber.hpp"
#include "manager.hpp"
#include "scrbuf.hpp"
#include "savescr.hpp"

static int AltValue=0,ReturnAltValue,KeyCodeForALT_LastPressed=0;
static int ShiftPressedLast=FALSE,AltPressedLast=FALSE,CtrlPressedLast=FALSE;
static int RightAltPressedLast=FALSE,RightCtrlPressedLast=FALSE;
#if defined(MOUSEKEY)
static int PrePreMouseEventFlags;
#endif
static BOOL IsKeyCASPressed=FALSE; // CtrlAltShift - нажато или нет?
static clock_t PressedLastTime,KeyPressedLastTime;


/* ----------------------------------------------------------------- */
static struct TTable_KeyToVK{
  int Key;
  int VK;
} Table_KeyToVK[]={
//   {KEY_PGUP,          VK_PRIOR},
//   {KEY_PGDN,          VK_NEXT},
//   {KEY_END,           VK_END},
//   {KEY_HOME,          VK_HOME},
//   {KEY_LEFT,          VK_LEFT},
//   {KEY_UP,            VK_UP},
//   {KEY_RIGHT,         VK_RIGHT},
//   {KEY_DOWN,          VK_DOWN},
//   {KEY_INS,           VK_INSERT},
//   {KEY_DEL,           VK_DELETE},
//   {KEY_LWIN,          VK_LWIN},
//   {KEY_RWIN,          VK_RWIN},
//   {KEY_APPS,          VK_APPS},
//   {KEY_MULTIPLY,      VK_MULTIPLY},
//   {KEY_ADD,           VK_ADD},
//   {KEY_SUBTRACT,      VK_SUBTRACT},
//   {KEY_DIVIDE,        VK_DIVIDE},
//   {KEY_F1,            VK_F1},
//   {KEY_F2,            VK_F2},
//   {KEY_F3,            VK_F3},
//   {KEY_F4,            VK_F4},
//   {KEY_F5,            VK_F5},
//   {KEY_F6,            VK_F6},
//   {KEY_F7,            VK_F7},
//   {KEY_F8,            VK_F8},
//   {KEY_F9,            VK_F9},
//   {KEY_F10,           VK_F10},
//   {KEY_F11,           VK_F11},
//   {KEY_F12,           VK_F12},
   {KEY_BREAK,         VK_CANCEL},
   {KEY_BS,            VK_BACK},
   {KEY_TAB,           VK_TAB},
   {KEY_ENTER,         VK_RETURN},
   {KEY_ESC,           VK_ESCAPE},
   {KEY_SPACE,         VK_SPACE},
   {KEY_NUMPAD5,       VK_CLEAR},
};


struct TFKey3{
  DWORD Key;
  int   Len;
  char *Name;
};

static struct TFKey3 FKeys1[]={
  { KEY_CTRLALTSHIFTRELEASE, 19, "CtrlAltShiftRelease"},
  { KEY_CTRLALTSHIFTPRESS,   17, "CtrlAltShiftPress"},
#if defined(MOUSEKEY)
  { KEY_MSLDBLCLICK,         11, "MsLDblClick"},
  { KEY_MSRDBLCLICK,         11, "MsRDblClick"},
#endif
  { KEY_MSWHEEL_DOWN,        11, "MsWheelDown"},
  { KEY_MSWHEEL_UP,           9, "MsWheelUp"},
  { KEY_BACKSLASH,            9, "BackSlash"},
  { KEY_SUBTRACT,             8, "Subtract"},
  { KEY_MULTIPLY,             8, "Multiply"},
  { KEY_DIVIDE,               6, "Divide"},
  { KEY_BREAK,                5, "Break"},
  { KEY_CLEAR,                5, "Clear"},
  { KEY_NUMPAD5,              5, "Clear"},
  { KEY_RIGHT,                5, "Right"},
  { KEY_ENTER,                5, "Enter"},
  { KEY_SPACE,                5, "Space"},
  { KEY_PGUP,                 4, "PgUp"},
  { KEY_HOME,                 4, "Home"},
  { KEY_LEFT,                 4, "Left"},
  { KEY_DOWN,                 4, "Down"},
  { KEY_PGDN,                 4, "PgDn"},
  { KEY_APPS,                 4, "Apps"},
  { KEY_LWIN,                 4 ,"LWin"},
  { KEY_RWIN,                 4 ,"RWin"},
  { KEY_END,                  3, "End"},
  { KEY_INS,                  3, "Ins"},
  { KEY_DEL,                  3, "Del"},
  { KEY_ADD,                  3, "Add"},
  { KEY_F10,                  3, "F10"},
  { KEY_F11,                  3, "F11"},
  { KEY_F12,                  3, "F12"},
  { KEY_TAB,                  3, "Tab"},
  { KEY_ESC,                  3, "Esc"},
  { KEY_F1,                   2, "F1"},
  { KEY_F2,                   2, "F2"},
  { KEY_F3,                   2, "F3"},
  { KEY_F4,                   2, "F4"},
  { KEY_F5,                   2, "F5"},
  { KEY_F6,                   2, "F6"},
  { KEY_F7,                   2, "F7"},
  { KEY_F8,                   2, "F8"},
  { KEY_F9,                   2, "F9"},
  { KEY_UP,                   2, "Up"},
  { KEY_BS,                   2, "BS"},
  { KEY_BRACKET,              1, "["},
  { KEY_BACKBRACKET,          1, "]"},
  { KEY_COMMA,                1, ","},
  { KEY_QUOTE,                1, "\""},
  { KEY_COLON,                1, ":"},
  { KEY_DOT,                  1, "."},
  { KEY_SLASH,                1, "/"},
};

static struct TFKey3 ModifKeyName[]={
  { KEY_CTRL   ,4 ,"Ctrl"},
  { KEY_RCTRL  ,5 ,"RCtrl"},
  { KEY_ALT    ,3 ,"Alt"},
  { KEY_RALT   ,4 ,"RAlt"},
  { KEY_SHIFT  ,5 ,"Shift"},
//  { KEY_LCTRL  ,5 ,"LCtrl"},
//  { KEY_LALT   ,4 ,"LAlt"},
//  { KEY_LSHIFT ,6 ,"LShift"},
//  { KEY_RSHIFT ,6 ,"RShift"},
};

#if defined(SYSLOG)
static struct TFKey3 SpecKeyName[]={
  { KEY_CONSOLE_BUFFER_RESIZE,19, "ConsoleBufferResize"},
  { KEY_FOCUS_CHANGED        ,12, "FocusChanged"},
  { KEY_LOCKSCREEN           ,10, "LockScreen"},
  { KEY_DRAGCOPY             , 8, "DragCopy"},
  { KEY_DRAGMOVE             , 8, "DragMove"},
  { KEY_NONE                 , 4, "None"},
  { KEY_IDLE                 , 4, "Idle"},
};
#endif

/* ----------------------------------------------------------------- */

/* tran 31.08.2000 $
  FarInputRecordToKey */
int WINAPI InputRecordToKey(const INPUT_RECORD *r)
{
  if(r)
  {
    INPUT_RECORD Rec=*r; // НАДО!, т.к. внутри CalcKeyCode
                         //   структура INPUT_RECORD модифицируется!
    return CalcKeyCode(&Rec,TRUE);
  }
  return KEY_NONE;
}
/* tran 31.08.2000 $ */


int IsMouseButtonPressed()
{
  INPUT_RECORD rec;
  if (PeekInputRecord(&rec))
    GetInputRecord(&rec);
  Sleep(20);
  if (LButtonPressed)
    return(1);
  if (RButtonPressed)
    return(2);
  /* $ 23.08.2000 SVS
     + Дополнительно - для средней клавиши мыши
  */
  if(MButtonPressed)
    return(3);
  /* SVS $ */
  return(0);
}

int GetInputRecord(INPUT_RECORD *rec)
{
  static int LastEventIdle=FALSE;
  DWORD ReadCount;
  DWORD LoopCount=0,CalcKey;
  DWORD ReadKey=0;
  int NotMacros=FALSE;

  if (CtrlObject!=NULL)
  {
     _KEYMACRO(CleverSysLog SL("GetInputRecord()"));
    int VirtKey,ControlState;
    int MacroKey=CtrlObject->Macro.GetKey();
    if (MacroKey)
    {
      ScrBuf.Flush();
      TranslateKeyToVK(MacroKey,VirtKey,ControlState,rec);
      rec->EventType=0;
      _KEYMACRO(SysLog("MacroKey1 =%s",_FARKEY_ToName(MacroKey)));
//      memset(rec,0,sizeof(*rec));
      return(MacroKey);
    }
    if (CtrlObject->Cp()&&CtrlObject->Cp()->ActivePanel&&!CmdMode)
      CtrlObject->Macro.RunStartMacro();
    MacroKey=CtrlObject->Macro.GetKey();
    if (MacroKey)
    {
      ScrBuf.Flush();
      TranslateKeyToVK(MacroKey,VirtKey,ControlState,rec);
      rec->EventType=0;
      _KEYMACRO(SysLog("MacroKey2 =%s",_FARKEY_ToName(MacroKey)));
//      memset(rec,0,sizeof(*rec));
      return(MacroKey);
    }
  }

  if(KeyQueue && KeyQueue->Peek())
  {
    CalcKey=KeyQueue->Get();
    NotMacros=CalcKey&0x80000000?1:0;
    CalcKey&=~0x80000000;
    if (!NotMacros)
    {
      if (CtrlObject!=NULL && CtrlObject->Macro.ProcessKey(CalcKey))
        CalcKey=KEY_NONE;
    }
    return(CalcKey);
  }

  int EnableShowTime=Opt.Clock && (WaitInMainLoop || CtrlObject!=NULL &&
                     CtrlObject->Macro.GetMode()==MACRO_SEARCH);

  if (EnableShowTime)
    ShowTime(1);

  ScrBuf.Flush();

  if (!LastEventIdle)
    StartIdleTime=clock();
  LastEventIdle=FALSE;
  SetFarConsoleMode();
  while (1)
  {
#if defined(USE_WFUNC)
    if(WinVer.dwPlatformId == VER_PLATFORM_WIN32_NT)
      PeekConsoleInputW(hConInp,rec,1,&ReadCount);
    else
      PeekConsoleInputA(hConInp,rec,1,&ReadCount);
#else
    PeekConsoleInput(hConInp,rec,1,&ReadCount);
#endif
    /* $ 26.04.2001 VVM
       ! Убрал подмену колесика */
    if (ReadCount!=0)
    {
/*
      switch(rec->EventType)
      {
        case FOCUS_EVENT:
          SysLog("GetInputRecord(FOCUS_EVENT)");
          break;
        case WINDOW_BUFFER_SIZE_EVENT:
          SysLog("GetInputRecord(WINDOW_BUFFER_SIZE_EVENT)");
          break;
        case MENU_EVENT:
          SysLog("GetInputRecord(MENU_EVENT)");
          break;
        case KEY_EVENT:
          SysLog("GetInputRecord(KEY_EVENT), %s KState=0x%08X VK=0x%04X,",
               (rec->Event.KeyEvent.bKeyDown?"Up":"Dn"),
               rec->Event.KeyEvent.dwControlKeyState,
               rec->Event.KeyEvent.wVirtualKeyCode);
          break;
        case MOUSE_EVENT:
          //SysLog("GetInputRecord(MENU_EVENT)");
          break;
        default:
          SysLog("GetInputRecord(??????_EVENT)");
          break;

      }
*/
      break;
    }
    /* VVM $ */

    ScrBuf.Flush();

    Sleep(15);
    // Позволяет избежать ситуации блокирования мыши
    if(Opt.Mouse) // А нужно ли это условие???
      SetFarConsoleMode();

    if (CloseFAR)
    {
      CloseFAR=FALSE;
      /* $ 30.08.2001 IS
         При принудительном закрытии Фара пытаемся вести себя так же, как и при
         нажатии на F10 в панелях, только не запрашиваем подтверждение закрытия,
         если это возможно.
      */
      if(!Opt.CloseConsoleRule)
        FrameManager->IsAnyFrameModified(TRUE);
      else
        FrameManager->ExitMainLoop(FALSE);
      return KEY_NONE;
      /* IS $ */
    }

    if ((LoopCount & 15)==0)
    {
      clock_t CurTime=clock();
      clock_t TimeAfterExec=CurTime-StartExecTime;
      if (EnableShowTime)
        ShowTime(0);
      if (WaitInMainLoop)
      {
        if (Opt.InactivityExit && Opt.InactivityExitTime>0 &&
            CurTime-StartIdleTime>Opt.InactivityExitTime*60000 &&
            FrameManager->GetFrameCount()==1)
        {
          FrameManager->ExitMainLoop(FALSE);
          return(KEY_NONE);
        }
        if ((LoopCount & 63)==0)
        {
          static int Reenter=0;
          if (!Reenter)
          {
            Reenter++;
            int X,Y;
            GetRealCursorPos(X,Y);
            if (X==0 && Y==ScrY && CtrlObject->CmdLine->IsVisible())
            {
              while (1)
              {
                INPUT_RECORD tmprec;
                int Key=GetInputRecord(&tmprec);
                if ((DWORD)Key==KEY_NONE || (DWORD)Key!=KEY_SHIFT && tmprec.Event.KeyEvent.bKeyDown)
                  break;
              }
              CtrlObject->Cp()->SetScreenPosition();
              ScrBuf.ResetShadow();
              ScrBuf.Flush();
            }
            Reenter--;
          }
          static int UpdateReenter=0;
          if (!UpdateReenter && CurTime-KeyPressedLastTime>700)
          {
            UpdateReenter=TRUE;
            CtrlObject->Cp()->LeftPanel->UpdateIfChanged();
            CtrlObject->Cp()->RightPanel->UpdateIfChanged();
            UpdateReenter=FALSE;
          }
        }
      }
      if (StartExecTime!=0 && TimeAfterExec>2000)
      {
        StartExecTime=0;
        if (!IsWindowed() && !Opt.Mouse)
        {
          SetConsoleMode(hConInp,ENABLE_WINDOW_INPUT|ENABLE_MOUSE_INPUT);
          SetConsoleMode(hConInp,ENABLE_WINDOW_INPUT);
        }
        SetFarTitle(NULL);//LastFarTitle);
      }
      if (Opt.ScreenSaver && Opt.ScreenSaverTime>0 &&
          CurTime-StartIdleTime>Opt.ScreenSaverTime*60000)
        if (!ScreenSaver(WaitInMainLoop))
          return(KEY_NONE);
      if (!WaitInMainLoop && LoopCount==64)
      {
        LastEventIdle=TRUE;
        memset(rec,0,sizeof(*rec));
        rec->EventType=KEY_EVENT;
        return(KEY_IDLE);
      }
    }
    LoopCount++;
  }
  clock_t CurClock=clock();
  if (rec->EventType==FOCUS_EVENT)
  {
    /* $ 28.04.2001 VVM
      + Не только обработаем сами смену фокуса, но и передадим дальше */
    ShiftPressed=ShiftPressedLast=FALSE;
    CtrlPressed=CtrlPressedLast=RightCtrlPressedLast=FALSE;
    AltPressed=AltPressedLast=RightAltPressedLast=FALSE;
    LButtonPressed=RButtonPressed=MButtonPressed=FALSE;
    PressedLastTime=0;
#if defined(USE_WFUNC)
    if(WinVer.dwPlatformId == VER_PLATFORM_WIN32_NT)
      ReadConsoleInputW(hConInp,rec,1,&ReadCount);
    else
      ReadConsoleInputA(hConInp,rec,1,&ReadCount);
#else
    ReadConsoleInput(hConInp,rec,1,&ReadCount);
#endif
    rec->EventType=KEY_EVENT;
    return(KEY_FOCUS_CHANGED);
    /* VVM $ */
  }
  if (rec->EventType==KEY_EVENT)
  {
    DWORD CtrlState=rec->Event.KeyEvent.dwControlKeyState;

    /* $ 28.06.2001 SVS
       Для Win9x при нажатом NumLock и юзании курсорных клавиш
       получаем в диалоге назначения ерундистику.
    */
    if(CtrlObject && CtrlObject->Macro.IsRecording())
    {
      static WORD PrevVKKeyCode=0; // NumLock+Cursor
      WORD PrevVKKeyCode2=PrevVKKeyCode;
      PrevVKKeyCode=rec->Event.KeyEvent.wVirtualKeyCode;

      if(WinVer.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS &&
        (CtrlState&NUMLOCK_ON))
      {
        if((PrevVKKeyCode2 >= 0x21 && PrevVKKeyCode2 <= 0x28 ||
            PrevVKKeyCode2 >= 0x2D && PrevVKKeyCode2 <= 0x2E) &&
           PrevVKKeyCode == VK_SHIFT && rec->Event.KeyEvent.bKeyDown
           ||
           (PrevVKKeyCode >= 0x21 && PrevVKKeyCode <= 0x28 ||
            PrevVKKeyCode >= 0x2D && PrevVKKeyCode <= 0x2E) &&
           PrevVKKeyCode2 == VK_SHIFT && !rec->Event.KeyEvent.bKeyDown
          )
        {
          if(PrevVKKeyCode2 != VK_SHIFT)
          {
            INPUT_RECORD pinp;
            DWORD nread;
            // Удалим из очереди...
            ReadConsoleInput(hConInp, &pinp, 1, &nread);
            return KEY_NONE;
          }
        }
      }
      /* 1.07.2001 KM
        При отпускании Shift-Enter в диалоге назначения
        вылазил Shift после отпускания клавиш.
      */
      if((PrevVKKeyCode2==VK_SHIFT && PrevVKKeyCode==VK_RETURN &&
          rec->Event.KeyEvent.bKeyDown) ||
          (PrevVKKeyCode2==VK_RETURN && PrevVKKeyCode==VK_SHIFT &&
          !rec->Event.KeyEvent.bKeyDown))
      {
        if(PrevVKKeyCode2 != VK_SHIFT)
        {
          INPUT_RECORD pinp;
          DWORD nread;
          // Удалим из очереди...
          ReadConsoleInput(hConInp, &pinp, 1, &nread);
          return KEY_NONE;
        }
      }
      /* KM $ */
    }
    /* SVS $ */

    if (AltPressed && (CtrlState & (LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED))==0)
      DetectWindowedMode();
    CtrlPressed=(CtrlState & (LEFT_CTRL_PRESSED|RIGHT_CTRL_PRESSED));
    AltPressed=(CtrlState & (LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED));
    ShiftPressed=(CtrlState & SHIFT_PRESSED);
    KeyPressedLastTime=CurClock;
    /* $ 24.08.2000 SVS
       + Добавление на реакцию KEY_CTRLALTSHIFTRELEASE
    */
    if(IsKeyCASPressed && (!CtrlPressed || !AltPressed || !ShiftPressed))
    {
      IsKeyCASPressed=FALSE;
      return(KEY_CTRLALTSHIFTRELEASE);
    }
    /* SVS $ */
  }

  ReturnAltValue=FALSE;
  CalcKey=CalcKeyCode(rec,TRUE,&NotMacros);
//_SVS(SysLog("1) CalcKey=%s",_FARKEY_ToName(CalcKey)));
  if (ReturnAltValue && !NotMacros)
  {
    if (CtrlObject!=NULL && CtrlObject->Macro.ProcessKey(CalcKey))
      CalcKey=KEY_NONE;
    return(CalcKey);
  }
  int GrayKey=(CalcKey==KEY_ADD || CalcKey==KEY_SUBTRACT || CalcKey==KEY_MULTIPLY);
  if ((CalcKey>=' ' && CalcKey<256 || CalcKey==KEY_BS || GrayKey) &&
      CalcKey!=KEY_DEL && WinVer.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS)
  {
#if defined(USE_WFUNC)
    if(WinVer.dwPlatformId == VER_PLATFORM_WIN32_NT)
      ReadConsoleW(hConInp,&ReadKey,1,&ReadCount,NULL);
    else
      ReadConsoleA(hConInp,&ReadKey,1,&ReadCount,NULL);
#else
    ReadConsole(hConInp,&ReadKey,1,&ReadCount,NULL);
#endif
    if (ReadKey==13 && CalcKey!=KEY_ENTER)
    {
#if defined(USE_WFUNC)
      if(WinVer.dwPlatformId == VER_PLATFORM_WIN32_NT)
        ReadConsoleW(hConInp,&ReadKey,1,&ReadCount,NULL);
      else
        ReadConsoleA(hConInp,&ReadKey,1,&ReadCount,NULL);
#else
      ReadConsole(hConInp,&ReadKey,1,&ReadCount,NULL);
#endif
    }
    rec->Event.KeyEvent.uChar.AsciiChar=(char) ReadKey;
  }
  else
  {
#if defined(USE_WFUNC)
    if(WinVer.dwPlatformId == VER_PLATFORM_WIN32_NT)
      ReadConsoleInputW(hConInp,rec,1,&ReadCount);
    else
      ReadConsoleInputA(hConInp,rec,1,&ReadCount);
#else
    ReadConsoleInput(hConInp,rec,1,&ReadCount);
#endif
  }


  if (EnableShowTime)
    ShowTime(1);

  /*& 17.05.2001 OT Изменился размер консоли, генерим клавишу*/
  if (rec->EventType==WINDOW_BUFFER_SIZE_EVENT)
  {
    int PScrX=ScrX;
    int PScrY=ScrY;
    //_SVS(SysLog(1,"GetInputRecord(WINDOW_BUFFER_SIZE_EVENT)"));
    Sleep(1);
    GetVideoMode(CurScreenBufferInfo);
    if (PScrX+1 == CurScreenBufferInfo.dwSize.X &&
        PScrY+1 == CurScreenBufferInfo.dwSize.Y)
    {
      //_SVS(SysLog(-1,"GetInputRecord(WINDOW_BUFFER_SIZE_EVENT); return KEY_NONE"));
      return KEY_NONE;
    }
    else
    {
      PrevScrX=PScrX;
      PrevScrY=PScrY;
      //_SVS(SysLog(-1,"GetInputRecord(WINDOW_BUFFER_SIZE_EVENT); return KEY_CONSOLE_BUFFER_RESIZE"));
      Sleep(1);
      return(KEY_CONSOLE_BUFFER_RESIZE);
    }
  }
  /* 17.05.2001 $ */

  if (rec->EventType==KEY_EVENT)
  {
    DWORD CtrlState=rec->Event.KeyEvent.dwControlKeyState;
    DWORD KeyCode=rec->Event.KeyEvent.wVirtualKeyCode;
    CtrlPressed=(CtrlState & (LEFT_CTRL_PRESSED|RIGHT_CTRL_PRESSED));
    AltPressed=(CtrlState & (LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED));
    ShiftPressed=(CtrlState & SHIFT_PRESSED);

    if (KeyCode==VK_F16 && ReadKey==VK_F16 || KeyCode==0)
      return(KEY_NONE);

    if (!rec->Event.KeyEvent.bKeyDown &&
        (KeyCode==VK_SHIFT || KeyCode==VK_CONTROL || KeyCode==VK_MENU) &&
        CurClock-PressedLastTime<500)
    {
      int Key=-1;
      if (ShiftPressedLast && KeyCode==VK_SHIFT)
        Key=KEY_SHIFT;
      if (KeyCode==VK_CONTROL)
        if (CtrlPressedLast)
          Key=KEY_CTRL;
        else
          if (RightCtrlPressedLast)
            Key=KEY_RCTRL;
      if (KeyCode==VK_MENU)
        if (AltPressedLast)
          Key=KEY_ALT;
        else
          if (RightAltPressedLast)
            Key=KEY_RALT;
      if (Key!=-1 && !NotMacros && CtrlObject!=NULL && CtrlObject->Macro.ProcessKey(Key))
        Key=KEY_NONE;
      if (Key!=-1)
        return(Key);
    }

    ShiftPressedLast=FALSE;
    CtrlPressedLast=RightCtrlPressedLast=FALSE;
    AltPressedLast=RightAltPressedLast=FALSE;

    ShiftPressedLast=(KeyCode==VK_SHIFT && rec->Event.KeyEvent.bKeyDown) ||
         (KeyCode==VK_RETURN && ShiftPressed && !rec->Event.KeyEvent.bKeyDown);

    if (!ShiftPressedLast)
      if (KeyCode==VK_CONTROL && rec->Event.KeyEvent.bKeyDown)
      {
        if (CtrlState & RIGHT_CTRL_PRESSED)
          RightCtrlPressedLast=TRUE;
        else
          CtrlPressedLast=TRUE;
      }

    if (!ShiftPressedLast && !CtrlPressedLast && !RightCtrlPressedLast)
    {
      if (KeyCode==VK_MENU && rec->Event.KeyEvent.bKeyDown)
      {
        if (CtrlState & RIGHT_ALT_PRESSED)
          RightAltPressedLast=TRUE;
        else
          AltPressedLast=TRUE;
        PressedLastTime=CurClock;
      }
    }
    else
      PressedLastTime=CurClock;

    if (KeyCode==VK_SHIFT || KeyCode==VK_MENU || KeyCode==VK_CONTROL ||
        KeyCode==VK_NUMLOCK || KeyCode==VK_SCROLL)
    {
      /* $ 24.08.2000 SVS
         + Добавление на реакцию KEY_CTRLALTSHIFTPRESS
      */
      switch(KeyCode)
      {
        case VK_SHIFT:
        case VK_MENU:
        case VK_CONTROL:
          if(!IsKeyCASPressed && CtrlPressed && AltPressed && ShiftPressed)
          {
            IsKeyCASPressed=TRUE;
            return (KEY_CTRLALTSHIFTPRESS);
          }
      }
      /* SVS $ */
      return(KEY_NONE);
    }
    Panel::EndDrag();
  }
  if (rec->EventType==MOUSE_EVENT)
  {
    // проверка на Swap клавиш мыши
    static int SwapButton=GetSystemMetrics(SM_SWAPBUTTON);
#if defined(MOUSEKEY)
    PrePreMouseEventFlags=PreMouseEventFlags;
#endif
    PreMouseEventFlags=MouseEventFlags;
    MouseEventFlags=rec->Event.MouseEvent.dwEventFlags;

    if (SwapButton && WinVer.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS && !IsWindowed())
    {
      DWORD CtrlState=rec->Event.MouseEvent.dwButtonState;
      if (CtrlState & FROM_LEFT_1ST_BUTTON_PRESSED)
        rec->Event.MouseEvent.dwButtonState|=RIGHTMOST_BUTTON_PRESSED;
      else
        rec->Event.MouseEvent.dwButtonState&=~RIGHTMOST_BUTTON_PRESSED;
      if (CtrlState & RIGHTMOST_BUTTON_PRESSED)
        rec->Event.MouseEvent.dwButtonState|=FROM_LEFT_1ST_BUTTON_PRESSED;
      else
        rec->Event.MouseEvent.dwButtonState&=~FROM_LEFT_1ST_BUTTON_PRESSED;
    }
    DWORD CtrlState=rec->Event.MouseEvent.dwButtonState;
    /* $ 23.08.2000 SVS
       + Дополнительно - для средней клавиши мыши
    */
    MButtonPressed=(CtrlState & FROM_LEFT_2ND_BUTTON_PRESSED);
    /* SVS $ */
    LButtonPressed=(CtrlState & FROM_LEFT_1ST_BUTTON_PRESSED);
    RButtonPressed=(CtrlState & RIGHTMOST_BUTTON_PRESSED);
    PrevMouseX=MouseX;
    PrevMouseY=MouseY;
    MouseX=rec->Event.MouseEvent.dwMousePosition.X;
    MouseY=rec->Event.MouseEvent.dwMousePosition.Y;
#if defined(MOUSEKEY)
    if(PrePreMouseEventFlags == DOUBLE_CLICK)
    {
      rec->EventType = KEY_EVENT;
      return(KEY_NONE);
    }
    if (MouseEventFlags == DOUBLE_CLICK && (LButtonPressed || RButtonPressed))
    {
      CalcKey=LButtonPressed?KEY_MSLDBLCLICK:KEY_MSRDBLCLICK;
      DWORD SMState=rec->Event.MouseEvent.dwControlKeyState;
      CalcKey |= (SMState&SHIFT_PRESSED?KEY_SHIFT:0)|
                 (SMState&(LEFT_CTRL_PRESSED|RIGHT_CTRL_PRESSED)?KEY_CTRL:0)|
                 (SMState&(LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED)?KEY_ALT:0);
      rec->EventType = KEY_EVENT;
    }
    else
#endif
    /* $ 26.04.2001 VVM
       + Обработка колесика мышки под 2000. */
    if (MouseEventFlags == MOUSE_WHEELED)
    { // Обработаем колесо и заменим на спец.клавиши
      short zDelta = (short)HIWORD(rec->Event.MouseEvent.dwButtonState);
      CalcKey = (zDelta>0)?KEY_MSWHEEL_UP:KEY_MSWHEEL_DOWN;
      /* $ 27.04.2001 SVS
         Не были учтены шифтовые клавиши при прокрутке колеса, из-за чего
         нельзя было использовать в макросах нечто вроде "ShiftMsWheelUp"
      */
      DWORD SMState=rec->Event.MouseEvent.dwControlKeyState;
      CalcKey |= (SMState&SHIFT_PRESSED?KEY_SHIFT:0)|
                 (SMState&(LEFT_CTRL_PRESSED|RIGHT_CTRL_PRESSED)?KEY_CTRL:0)|
                 (SMState&(LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED)?KEY_ALT:0);
      /* SVS $ */
      rec->EventType = KEY_EVENT;
    } /* if */
    /* VVM $ */
  }
  if (ReadKey!=0 && !GrayKey)
    CalcKey=ReadKey;

  if (!NotMacros && CtrlObject!=NULL && CtrlObject->Macro.ProcessKey(CalcKey))
    CalcKey=KEY_NONE;

  return(CalcKey);
}

int PeekInputRecord(INPUT_RECORD *rec)
{
  DWORD ReadCount;
  DWORD Key;
  ScrBuf.Flush();
  if(KeyQueue && (Key=KeyQueue->Peek()) != NULL)
  {
    int VirtKey,ControlState;
    ReadCount=TranslateKeyToVK(Key,VirtKey,ControlState,rec)?1:0;
  }
  else
  {
#if defined(USE_WFUNC)
    if(WinVer.dwPlatformId == VER_PLATFORM_WIN32_NT)
      PeekConsoleInputW(hConInp,rec,1,&ReadCount);
    else
      PeekConsoleInputA(hConInp,rec,1,&ReadCount);
#else
    PeekConsoleInput(hConInp,rec,1,&ReadCount);
#endif
  }
  if (ReadCount==0)
    return(0);
  return(CalcKeyCode(rec,TRUE));
}


/* $ 24.08.2000 SVS
 + Пераметр у фунции WaitKey - возможность ожидать конкретную клавишу
     Если KeyWait = -1 - как и раньше
*/
void WaitKey(int KeyWait)
{
  int Visible,Size;
  if(KeyWait == KEY_CTRLALTSHIFTRELEASE)
  {
    GetCursorType(Visible,Size);
    SetCursorType(0,10);
  }
  while (1)
  {
    INPUT_RECORD rec;
    int Key=GetInputRecord(&rec);
    if(KeyWait == -1)
    {
      if (Key!=KEY_NONE && Key!=KEY_IDLE)
        break;
    }
    else if(Key == KeyWait)
      break;
  }
  if(KeyWait == KEY_CTRLALTSHIFTRELEASE)
    SetCursorType(Visible,Size);
}
/* SVS $ */


int WriteInput(int Key,DWORD Flags)
{
  if(Flags&SKEY_VK_KEYS)
  {
    INPUT_RECORD Rec;
    DWORD WriteCount;

    Rec.EventType=KEY_EVENT;
    Rec.Event.KeyEvent.bKeyDown=1;
    Rec.Event.KeyEvent.wRepeatCount=1;
    Rec.Event.KeyEvent.wVirtualKeyCode=Key;
    Rec.Event.KeyEvent.wVirtualScanCode=MapVirtualKey(
                    Rec.Event.KeyEvent.wVirtualKeyCode, 0);
    if (Key>255)
      Key=0;
    Rec.Event.KeyEvent.uChar.UnicodeChar=Rec.Event.KeyEvent.uChar.AsciiChar=Key;
    Rec.Event.KeyEvent.dwControlKeyState=0;
#if defined(USE_WFUNC)
    if(WinVer.dwPlatformId == VER_PLATFORM_WIN32_NT)
      return WriteConsoleInputW(hConInp,&Rec,1,&WriteCount);
    else
      return WriteConsoleInputA(hConInp,&Rec,1,&WriteCount);
#else
    return WriteConsoleInput(hConInp,&Rec,1,&WriteCount);
#endif
  }
  else if(KeyQueue)
  {
    return KeyQueue->Put(((DWORD)Key)|(Flags&SKEY_NOTMACROS?0x80000000:0));
  }
  else
    return 0;
}


/* $ 09.02.2001 IS
     Подтверждение нажатия Esc
*/
int CheckForEsc()
{
  INPUT_RECORD rec;
  int Key;
  if (!CtrlObject->Macro.IsExecuting() && PeekInputRecord(&rec) &&
      ((Key=GetInputRecord(&rec))==KEY_ESC || Key==KEY_BREAK))
  {
    SaveScreen SaveScr; // НУЖЕН! Избавляет от некоторых подводных багов
    BOOL rc=TRUE;
    IsProcessAssignMacroKey++; // запретим спец клавиши
                               // т.е. в этом диалоге нельзя нажать Alt-F9!
    if(Opt.Confirm.Esc && Message(MSG_WARNING,2,MSG(MKeyESCWasPressed),
                  MSG(MDoYouWantToStopWork),MSG(MYes),MSG(MNo)) == 1)
       rc=FALSE;
    IsProcessAssignMacroKey--;
    return rc;
  }
  return(FALSE);
}
/* IS $ */

/* $ 25.07.2000 SVS
    ! Функция KeyToText сделана самосотоятельной - вошла в состав FSF
*/
/* $ 01.08.2000 SVS
   ! дополнительный параметра у KeyToText - размер данных
   Size=0 - по максимуму!
*/
/* $ 10.09.2000 SVS
  ! KeyToText возвращает BOOL
*/

static char *GetShiftKeyName(char *Name, DWORD Key,int& Len)
{
  if((Key&KEY_RCTRL) == KEY_RCTRL)   strcat(Name,ModifKeyName[1].Name);
  else if(Key&KEY_CTRL)              strcat(Name,ModifKeyName[0].Name);
//  else if(Key&KEY_LCTRL)             strcat(Name,ModifKeyName[3].Name);

  if((Key&KEY_RALT) == KEY_RALT)     strcat(Name,ModifKeyName[3].Name);
  else if(Key&KEY_ALT)               strcat(Name,ModifKeyName[2].Name);
//  else if(Key&KEY_LALT)    strcat(Name,ModifKeyName[6].Name);

  if(Key&KEY_SHIFT)                  strcat(Name,ModifKeyName[4].Name);
//  else if(Key&KEY_LSHIFT)  strcat(Name,ModifKeyName[0].Name);
//  else if(Key&KEY_RSHIFT)  strcat(Name,ModifKeyName[1].Name);

  Len=strlen(Name);
  return Name;
}

/* $ 24.09.2000 SVS
 + Функция KeyNameToKey - получение кода клавиши по имени
   Если имя не верно или нет такого - возвращается -1
   Может и криво, но правильно и коротко!
*/
int WINAPI KeyNameToKey(const char *Name)
{
   if(!Name || !*Name)
     return -1;
   DWORD Key=0;

   // Это макроклавиша?
   if(Name[0] == '$' && Name[1])
     return KeyNameMacroToKey(Name);
//   if((Key=KeyNameMacroToKey(Name)) != (DWORD)-1)
//     return Key;

   int I, Pos, Len=strlen(Name);

   // пройдемся по всем модификаторам
   for(Pos=I=0; Pos < Len && I < sizeof(ModifKeyName)/sizeof(ModifKeyName[0]); ++I)
   {
     if(!memicmp(Name+Pos,ModifKeyName[I].Name,ModifKeyName[I].Len))
     {
       Pos+=ModifKeyName[I].Len;
       Key|=ModifKeyName[I].Key;
     }
   }
//_D(SysLog("Name=%s",Name));
   // если что-то осталось - преобразуем.
   if(Pos < Len)
   {
     // сначала - FKeys1
     const char* Ptr=Name+Pos;
     for (I=0;I<sizeof(FKeys1)/sizeof(FKeys1[0]);I++)
       if (!memicmp(Ptr,FKeys1[I].Name,FKeys1[I].Len))
       {
         Key|=FKeys1[I].Key;
         Pos+=FKeys1[I].Len;
         break;
       }
     if(I  == sizeof(FKeys1)/sizeof(FKeys1[0]) && (Len == 1 || Pos == Len-1))
     {
       WORD Chr=(WORD)(BYTE)Name[Pos];
       if (Chr > 0 && Chr < 256)
       {
//         if (Key&(0xFF000000-KEY_SHIFT))
//           Chr=LocalUpper(Chr);
         Key|=Chr;
         Pos++;
       }
     }
   }
//_SVS(SysLog("Key=0x%08X (%c) => '%s'",Key,(Key?Key:' '),Name));

   return (!Key || Pos < Len)? -1: Key;
}
/* SVS $*/

BOOL WINAPI KeyToText(int Key0,char *KeyText0,int Size)
{
  if(!KeyText0)
     return FALSE;

  char KeyText[128];
  int I, Len;
  DWORD Key=(DWORD)Key0, FKey=(DWORD)Key0&0xFFFF;

  if(Key&KEY_MACRO_BASE)
    return KeyMacroToText(Key0,KeyText0,Size);

  /* $ 27.12.2001 KM
    ! Обнулим KeyText (как и было раньше), в противном случае
      в буфере возвращался мусор!
  */
  memset(KeyText,0,sizeof(KeyText));
  /* KM $ */
  GetShiftKeyName(KeyText,Key,Len);

  for (I=0;I<sizeof(FKeys1)/sizeof(FKeys1[0]);I++)
  {
    if (FKey==FKeys1[I].Key)
    {
      strcat(KeyText,FKeys1[I].Name);
      break;
    }
  }

  if(I  == sizeof(FKeys1)/sizeof(FKeys1[0]))
  {
#if defined(SYSLOG)
    for (I=0;I<sizeof(SpecKeyName)/sizeof(SpecKeyName[0]);I++)
      if (FKey==SpecKeyName[I].Key)
      {
        strcat(KeyText,SpecKeyName[I].Name);
        break;
      }
    if(I  == sizeof(SpecKeyName)/sizeof(SpecKeyName[0]))
#endif
    {
      FKey=(Key&0xFF)&(~0x20);
      if (FKey >= 'A' && FKey <= 'Z')
        KeyText[Len]=(char)Key&0xFF;
      else if ((Key&0xFF) > 0 && (Key&0xFF) < 256)
        KeyText[Len]=(char)Key&0xFF;
    }
  }

  if(!KeyText[0])
  {
    *KeyText0='\0';
    return FALSE;
  }

  if(Size > 0)
    strncpy(KeyText0,KeyText,Size);
  else
    strcpy(KeyText0,KeyText);
//_D(SysLog("KeyToText() 0x%08X %s",Key,KeyText));
  return TRUE;
}
/* SVS 10.09.2000 $ */
/* SVS $ */


int TranslateKeyToVK(int Key,int &VirtKey,int &ControlState,INPUT_RECORD *Rec)
{
  int FKey  =Key&0x0000FFFF;
  int FShift=Key&0x7F000000; // старший бит используется в других целях!
  int I;

  VirtKey=0;
  ControlState=(FShift&KEY_SHIFT?PKF_SHIFT:0)|
               (FShift&KEY_ALT?PKF_ALT:0)|
               (FShift&KEY_CTRL?PKF_CONTROL:0);

  for(I=0; I < sizeof(Table_KeyToVK)/sizeof(Table_KeyToVK[0]); ++I)
    if (FKey==Table_KeyToVK[I].Key)
    {
      VirtKey=Table_KeyToVK[I].VK;
      break;
    }

  if(I  == sizeof(Table_KeyToVK)/sizeof(Table_KeyToVK[0]))
  {
    if (FKey>='0' && FKey<='9' || FKey>='A' && FKey<='Z')
      VirtKey=FKey;
    else if(FKey > 0x100 && FKey < KEY_END_FKEY)
      VirtKey=FKey-0x100;
    else if(FKey < 0x100)
      VirtKey=VkKeyScan(FKey)&0xFF;
    else
      VirtKey=FKey;
  }
  if(Rec && VirtKey!=0)
  {
    Rec->EventType=KEY_EVENT;
    Rec->Event.KeyEvent.bKeyDown=1;
    Rec->Event.KeyEvent.wRepeatCount=1;
    Rec->Event.KeyEvent.wVirtualKeyCode=VirtKey;
    Rec->Event.KeyEvent.wVirtualScanCode = MapVirtualKey(
                    Rec->Event.KeyEvent.wVirtualKeyCode, 0);
    if (Key>255)
      Key=0;
    Rec->Event.KeyEvent.uChar.UnicodeChar=
        Rec->Event.KeyEvent.uChar.AsciiChar=Key;
    // здесь подход к Shift-клавишам другой, нежели для ControlState
    Rec->Event.KeyEvent.dwControlKeyState=
               (FShift&KEY_SHIFT?SHIFT_PRESSED:0)|
               (FShift&KEY_ALT?LEFT_ALT_PRESSED:0)|
               (FShift&KEY_CTRL?LEFT_CTRL_PRESSED:0)|
               (FShift&KEY_RALT?RIGHT_ALT_PRESSED:0)|
               (FShift&KEY_RCTRL?RIGHT_CTRL_PRESSED:0);
  }
  return(VirtKey!=0);
}


int IsNavKey(DWORD Key)
{
  static DWORD NavKeys[][2]={
    {1,KEY_LEFT},
    {1,KEY_RIGHT},
    {1,KEY_HOME},
    {1,KEY_END},
    {1,KEY_UP},
    {1,KEY_DOWN},
    {1,KEY_PGUP},
    {1,KEY_PGDN},
    {0,KEY_CTRLC},
    {0,KEY_INS},
    {0,KEY_CTRLINS},
  };

  for (int I=0; I < sizeof(NavKeys)/sizeof(NavKeys[0]); I++)
    if(!NavKeys[I][0] && Key==NavKeys[I][1] ||
       NavKeys[I][0] && (Key&0x00FFFFFF)==(NavKeys[I][1]&0x00FFFFFF))
      return TRUE;
  return FALSE;
}

int IsShiftKey(DWORD Key)
{
  /*
     29.06.2000 IG
     добавлены клавиши, чтобы не сбрасывалось выделение при их нажатии
  */
  static DWORD ShiftKeys[]={KEY_SHIFTLEFT,KEY_SHIFTRIGHT,KEY_SHIFTHOME,
                KEY_SHIFTEND,KEY_SHIFTUP,KEY_SHIFTDOWN,KEY_SHIFTPGUP,
                KEY_SHIFTPGDN,KEY_CTRLSHIFTHOME,KEY_CTRLSHIFTPGUP,
                KEY_CTRLSHIFTEND,KEY_CTRLSHIFTPGDN,
                KEY_CTRLSHIFTLEFT,KEY_CTRLSHIFTRIGHT,KEY_ALTSHIFTDOWN,
                KEY_ALTSHIFTLEFT,KEY_ALTSHIFTRIGHT,KEY_ALTSHIFTUP,
                KEY_ALTSHIFTEND,KEY_ALTSHIFTHOME,KEY_ALTSHIFTPGDN,
                KEY_ALTSHIFTPGUP,KEY_ALTUP,KEY_ALTLEFT,KEY_ALTDOWN,
                KEY_ALTRIGHT,KEY_ALTHOME,KEY_ALTEND,KEY_ALTPGUP,KEY_ALTPGDN,
                KEY_CTRLALTPGUP,KEY_CTRLALTHOME,KEY_CTRLALTPGDN,KEY_CTRLALTEND,
                KEY_CTRLALTLEFT, KEY_CTRLALTRIGHT
  };
  /* IG $ */

  for (int I=0;I<sizeof(ShiftKeys)/sizeof(ShiftKeys[0]);I++)
    if (Key==ShiftKeys[I])
      return TRUE;
  return FALSE;
}


// GetAsyncKeyState(VK_RSHIFT)
int CalcKeyCode(INPUT_RECORD *rec,int RealKey,int *NotMacros)
{
  unsigned int ScanCode,KeyCode,CtrlState,AsciiChar;
  CtrlState=rec->Event.KeyEvent.dwControlKeyState;
  ScanCode=rec->Event.KeyEvent.wVirtualScanCode;
  KeyCode=rec->Event.KeyEvent.wVirtualKeyCode;
  AsciiChar=rec->Event.KeyEvent.uChar.AsciiChar;

  if(NotMacros)
    *NotMacros=CtrlState&0x80000000?TRUE:FALSE;
//  CtrlState&=~0x80000000;

  if (!RealKey)
  {
    CtrlPressed=(CtrlState & (LEFT_CTRL_PRESSED|RIGHT_CTRL_PRESSED));
    AltPressed=(CtrlState & (LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED));
    ShiftPressed=(CtrlState & SHIFT_PRESSED);
  }

  if (rec->EventType!=KEY_EVENT)
    return(KEY_NONE);

  if (!rec->Event.KeyEvent.bKeyDown)
  {
    KeyCodeForALT_LastPressed=0;
    if (KeyCode==VK_MENU && AltValue!=0)
    {
      FlushInputBuffer();
      ReturnAltValue=TRUE;
      AltValue&=255;
      rec->Event.KeyEvent.uChar.AsciiChar=AltValue;
      //_SVS(SysLog("KeyCode==VK_MENU -> AltValue=%X (%c)",AltValue,AltValue));
      return(AltValue);
    }
    else
      return(KEY_NONE);
  }

  if ((CtrlState & 9)==9)
    if (AsciiChar!=0)
      return(AsciiChar);
    else
      CtrlPressed=0;

  if (Opt.AltGr && CtrlPressed && (rec->Event.KeyEvent.dwControlKeyState & RIGHT_ALT_PRESSED))
    if (AsciiChar=='\\')
      return(KEY_CTRLBACKSLASH);

  if (KeyCode==VK_MENU)
    AltValue=0;

  if (AsciiChar==0 && !CtrlPressed && !AltPressed)
  {
    if (KeyCode==0xc0)
      return(ShiftPressed ? '~':'`');
    if (KeyCode==0xde)
      return(ShiftPressed ? '"':'\'');
  }

  if (AsciiChar<32 && (CtrlPressed || AltPressed))
  {
    switch(KeyCode)
    {
      case 0xbc:
        AsciiChar=',';
        break;
      case 0xbe:
        AsciiChar='.';
        break;
      case 0xdb:
        AsciiChar='[';
        break;
      case 0xdc:
        AsciiChar='\\';
        break;
      case 0xdd:
        AsciiChar=']';
        break;
      case 0xde:
        AsciiChar='\"';
        break;
    }
  }

  /* $ 24.08.2000 SVS
     "Персональные 100 грамм" :-)
  */
  if(CtrlPressed && AltPressed && ShiftPressed)
  {
    switch(KeyCode)
    {
      case VK_SHIFT:
      case VK_MENU:
      case VK_CONTROL:
        return (IsKeyCASPressed?KEY_CTRLALTSHIFTPRESS:KEY_CTRLALTSHIFTRELEASE);
    }
  }
  /* SVS $*/
  DWORD Modif=(CtrlPressed?KEY_CTRL:0)|
        (AltPressed?KEY_ALT:0)|
        (ShiftPressed?KEY_SHIFT:0);

  if (KeyCode>=VK_F1 && KeyCode<=VK_F12)
//    return(Modif+KEY_F1+((KeyCode-VK_F1)<<8));
    return(Modif+KEY_F1+((KeyCode-VK_F1)));

  int NotShift=!CtrlPressed && !AltPressed && !ShiftPressed;

  if (AltPressed && !CtrlPressed && !ShiftPressed)
  {
    static int InGrabber=FALSE;
    if (AltValue==0 && !InGrabber && (KeyCode==VK_INSERT || KeyCode==VK_NUMPAD0))
    {
      InGrabber=TRUE;
      WaitInMainLoop=FALSE;
      Grabber Grabber;
      InGrabber=FALSE;
      return(KEY_NONE);
    }

    static unsigned int ScanCodes[]={82,79,80,81,75,76,77,71,72,73};
    for (int I=0;I<sizeof(ScanCodes)/sizeof(ScanCodes[0]);I++)
      if (ScanCodes[I]==ScanCode && (CtrlState & ENHANCED_KEY)==0)
      {
        if (RealKey && KeyCodeForALT_LastPressed != KeyCode)
        {
          AltValue=AltValue*10+I;
          KeyCodeForALT_LastPressed=KeyCode;
          //_SVS(SysLog("CalcKeyCode -> ScanCode=0x%0X AltValue=0x%0X (%c)",ScanCode,AltValue,AltValue));
        }
        return(KEY_NONE);
      }
  }

  /* ------------------------------------------------------------- */
  switch(KeyCode)
  {
    case VK_INSERT:
    case VK_NUMPAD0:
      if (NotShift && KeyCode == VK_NUMPAD0)
        return '0';
      return(Modif|KEY_INS);
    case VK_DOWN:
    case VK_NUMPAD2:
      if (NotShift && KeyCode == VK_NUMPAD2)
        return '2';
      return(Modif|KEY_DOWN);
    case VK_LEFT:
    case VK_NUMPAD4:
      if (NotShift && KeyCode == VK_NUMPAD4)
        return '4';
      return(Modif|KEY_LEFT);
    case VK_RIGHT:
    case VK_NUMPAD6:
      if (NotShift && KeyCode == VK_NUMPAD6)
        return '6';
      return(Modif|KEY_RIGHT);
    case VK_UP:
    case VK_NUMPAD8:
      if (NotShift && KeyCode == VK_NUMPAD8)
        return '8';
      return(Modif|KEY_UP);
    case VK_END:
    case VK_NUMPAD1:
      if (NotShift && KeyCode == VK_NUMPAD1)
        return '1';
      return(Modif|KEY_END);
    case VK_HOME:
    case VK_NUMPAD7:
      if (NotShift && KeyCode == VK_NUMPAD7)
        return '7';
      return(Modif|KEY_HOME);
    case VK_NEXT:
    case VK_NUMPAD3:
      if (NotShift && KeyCode == VK_NUMPAD3)
        return '3';
      return(Modif|KEY_PGDN);
    case VK_PRIOR:
    case VK_NUMPAD9:
      if (NotShift && KeyCode == VK_NUMPAD9)
        return '9';
      return(Modif|KEY_PGUP);
    case VK_CLEAR:
    case VK_NUMPAD5:
      if (NotShift)
      {
        if(KeyCode == VK_NUMPAD5)
          return '5';
      }
      return(Modif|KEY_NUMPAD5);
    case VK_DECIMAL:
      if(NotShift)
        return (CtrlState & NUMLOCK_ON) ? '.':KEY_DEL;
    case VK_DELETE:
      if(NotShift)
      {
        if ((CtrlState & NUMLOCK_ON) && (CtrlState & ENHANCED_KEY)==0 &&
           AsciiChar=='.')
          return('.');
        return(KEY_DEL);
      }
      return(Modif|KEY_DEL);
    case VK_APPS:
      return(Modif|KEY_APPS);
    case VK_LWIN:
      return(Modif|KEY_LWIN);
    case VK_RWIN:
      return(Modif|KEY_RWIN);
    case VK_RETURN:
      //  !!!!!!!!!!!!! - Если "!ShiftPressed", то Shift-F4 Shift-Enter, не
      //                  отпуская Shift...
      if (ShiftPressed && RealKey && !ShiftPressedLast && !CtrlPressed && !AltPressed)
        return(KEY_ENTER);
      return(Modif|KEY_ENTER);
    case VK_BACK:
      return(Modif|KEY_BS);
    case VK_SPACE:
      return(Modif|KEY_SPACE);
    case VK_TAB:
      return(Modif|KEY_TAB);
    case VK_ADD:
      return(Modif|KEY_ADD);
    case VK_SUBTRACT:
      return(Modif|KEY_SUBTRACT);
    case VK_ESCAPE:
      return(Modif|KEY_ESC);
  }

  /* ------------------------------------------------------------- */
  if (CtrlPressed && AltPressed)
  {
//_SVS(if(KeyCode!=VK_CONTROL && KeyCode!=VK_MENU) SysLog("%9s|0x%08X (%c)|0x%08X (%c)|","CtrlAlt",KeyCode,(KeyCode?KeyCode:' '),AsciiChar,(AsciiChar?AsciiChar:' ')));
    if (KeyCode>='A' && KeyCode<='Z')
      return(KEY_CTRL|KEY_ALT+KeyCode);
    if(Opt.ShiftsKeyRules) //???
      switch(KeyCode)
      {
        case 0xc0:
          return(KEY_CTRL+KEY_ALT+'~');
        case 0xbd:
          return(KEY_CTRL+KEY_ALT+'-');
        case 0xbb:
          return(KEY_CTRL+KEY_ALT+'=');
        case 0xdc:
          return(KEY_CTRL+KEY_ALT+KEY_BACKSLASH);
        case 0xdd:
          return(KEY_CTRL+KEY_ALT+KEY_BACKBRACKET);
        case 0xdb:
          return(KEY_CTRL+KEY_ALT+KEY_BRACKET);
        case 0xde:
          return(KEY_CTRL+KEY_ALT+KEY_QUOTE);
        case 0xba:
          return(KEY_CTRL+KEY_ALT+KEY_COLON);
        case 0xbf:
          return(KEY_CTRL+KEY_ALT+KEY_SLASH);
        case 0xbe:
          return(KEY_CTRL+KEY_ALT+KEY_DOT);
        case 0xbc:
          return(KEY_CTRL+KEY_ALT+KEY_COMMA);
      }
    switch(KeyCode)
    {
      case VK_DIVIDE:
        return(KEY_CTRLALT|KEY_DIVIDE);
      case VK_MULTIPLY:
        return(KEY_CTRLALT|KEY_MULTIPLY);
    }
    if (AsciiChar)
      return(KEY_CTRL|KEY_ALT+AsciiChar);
    if (!RealKey && (KeyCode==VK_CONTROL || KeyCode==VK_MENU))
      return(KEY_NONE);
    if (KeyCode)
      return(KEY_CTRL|KEY_ALT+KeyCode);
  }


  /* ------------------------------------------------------------- */
  if (AltPressed && ShiftPressed)
  {
//_SVS(if(KeyCode!=VK_MENU && KeyCode!=VK_SHIFT) SysLog("NotMacros=%d %9s|0x%08X (%c)|0x%08X (%c)|WaitInMainLoop=%d WaitInFastFind=%d",*NotMacros,"AltShift",KeyCode,(KeyCode?KeyCode:' '),AsciiChar,(AsciiChar?AsciiChar:' '),WaitInMainLoop,WaitInFastFind));
    if (KeyCode>='0' && KeyCode<='9')
    {
      if(WaitInFastFind>0 &&
        CtrlObject->Macro.GetCurRecord(NULL,NULL) != 1 &&
        CtrlObject->Macro.GetIndex(KEY_ALTSHIFT0+KeyCode-'0',-1) == -1)
      {
        return(KEY_ALT+KEY_SHIFT+AsciiChar);
      }
      else
        return(KEY_ALTSHIFT0+KeyCode-'0');
    }
    if (!WaitInMainLoop && KeyCode>='A' && KeyCode<='Z')
      return(KEY_ALTSHIFT+KeyCode);
    if(Opt.ShiftsKeyRules) //???
      switch(KeyCode)
      {
        case 0xc0:
          return(KEY_ALT+KEY_SHIFT+'~');
        case 0xbd:
          return(KEY_ALT+KEY_SHIFT+'_');
        case 0xbb:
          return(KEY_ALT+KEY_SHIFT+'=');
        case 0xdc:
          return(KEY_ALT+KEY_SHIFT+KEY_BACKSLASH);
        case 0xdd:
          return(KEY_ALT+KEY_SHIFT+KEY_BACKBRACKET);
        case 0xdb:
          return(KEY_ALT+KEY_SHIFT+KEY_BRACKET);
        case 0xde:
          return(KEY_ALT+KEY_SHIFT+KEY_QUOTE);
        case 0xba:
          return(KEY_ALT+KEY_SHIFT+KEY_COLON);
        case 0xbf:
          if(WaitInFastFind)
            return(KEY_ALT+KEY_SHIFT+'?');
          else
            return(KEY_ALT+KEY_SHIFT+KEY_SLASH);
        case 0xbe:
          return(KEY_ALT+KEY_SHIFT+KEY_DOT);
        case 0xbc:
          return(KEY_ALT+KEY_SHIFT+KEY_COMMA);
      }
    switch(KeyCode)
    {
      case VK_DIVIDE:
        if(WaitInFastFind)
          return(KEY_ALT+KEY_SHIFT+'/');
        else
          return(KEY_ALTSHIFT|KEY_DIVIDE);
      case VK_MULTIPLY:
        if(WaitInFastFind)
        {
          return(KEY_ALT+KEY_SHIFT+'*');
        }
        else
          return(KEY_ALTSHIFT|KEY_MULTIPLY);
    }
    if (AsciiChar)
    {
      if (Opt.AltGr && WinVer.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS)
        if (rec->Event.KeyEvent.dwControlKeyState & RIGHT_ALT_PRESSED)
          return(AsciiChar);
      return(KEY_ALT+KEY_SHIFT+AsciiChar);
    }
    if (!RealKey && (KeyCode==VK_MENU || KeyCode==VK_SHIFT))
      return(KEY_NONE);
    if (KeyCode)
      return(KEY_ALT+KEY_SHIFT+KeyCode);
  }


  /* ------------------------------------------------------------- */
  if (CtrlPressed && ShiftPressed)
  {
//_SVS(if(KeyCode!=VK_CONTROL && KeyCode!=VK_SHIFT) SysLog("%9s|0x%08X (%c)|0x%08X (%c)|","CtrlShift",KeyCode,(KeyCode?KeyCode:' '),AsciiChar,(AsciiChar?AsciiChar:' ')));
    if (KeyCode>='0' && KeyCode<='9')
      return(KEY_CTRLSHIFT0+KeyCode-'0');
    if (KeyCode>='A' && KeyCode<='Z')
      return(KEY_CTRLSHIFTA+KeyCode-'A');
    switch(KeyCode)
    {
      case 0xbe:
        return(KEY_CTRLSHIFTDOT);
      case 0xdb:
        return(KEY_CTRLSHIFTBRACKET);
      case 0xdd:
        return(KEY_CTRLSHIFTBACKBRACKET);
      case 0xbf:
        return(KEY_CTRLSHIFTSLASH);
      case 0xdc:
        return(KEY_CTRLSHIFTBACKSLASH);
      case VK_DIVIDE:
        return(KEY_CTRLSHIFT|KEY_DIVIDE);
      case VK_MULTIPLY:
        return(KEY_CTRLSHIFT|KEY_MULTIPLY);
    }
    if(Opt.ShiftsKeyRules) //???
      switch(KeyCode)
      {
        case 0xc0:
          return(KEY_CTRL+KEY_SHIFT+'~');
        case 0xbd:
          return(KEY_CTRL+KEY_SHIFT+'-');
        case 0xbb:
          return(KEY_CTRL+KEY_SHIFT+'=');
        case 0xde:
          return(KEY_CTRL+KEY_SHIFT+KEY_QUOTE);
        case 0xba:
          return(KEY_CTRL+KEY_SHIFT+KEY_COLON);
        case 0xbc:
          return(KEY_CTRL+KEY_SHIFT+KEY_COMMA);
      }
    if (AsciiChar)
      return(KEY_CTRL|KEY_SHIFT+AsciiChar);
    if (!RealKey && (KeyCode==VK_CONTROL || KeyCode==VK_SHIFT))
      return(KEY_NONE);
    if (KeyCode)
      return(KEY_CTRL|KEY_SHIFT+KeyCode);
  }


  /* ------------------------------------------------------------- */
  if ((CtrlState & RIGHT_CTRL_PRESSED)==RIGHT_CTRL_PRESSED)
  {
    if (KeyCode>='0' && KeyCode<='9')
      return(KEY_RCTRL0+KeyCode-'0');
  }


  /* ------------------------------------------------------------- */
  if (!CtrlPressed && !AltPressed && !ShiftPressed)
  {
    switch(KeyCode)
    {
      case VK_DIVIDE:
        return(KEY_DIVIDE);
      case VK_CANCEL:
        return(KEY_BREAK);
      case VK_MULTIPLY:
        return(KEY_MULTIPLY);
    }
  }

  /* ------------------------------------------------------------- */
  if (CtrlPressed)
  {
//_SVS(if(KeyCode!=VK_CONTROL) SysLog("%9s|0x%08X (%c)|0x%08X (%c)|","Ctrl",KeyCode,(KeyCode?KeyCode:' '),AsciiChar,(AsciiChar?AsciiChar:' ')));
    if (KeyCode>='0' && KeyCode<='9')
      return(KEY_CTRL0+KeyCode-'0');
    if (KeyCode>='A' && KeyCode<='Z')
      return(KEY_CTRL+KeyCode);
    switch(KeyCode)
    {
      case 0xbc:
        return(KEY_CTRLCOMMA);
      case 0xbe:
        return(KEY_CTRLDOT);
      case 0xbf:
        return(KEY_CTRLSLASH);
      case 0xdb:
        return(KEY_CTRLBRACKET);
      case 0xdc:
        return(KEY_CTRLBACKSLASH);
      case 0xdd:
        return(KEY_CTRLBACKBRACKET);
      case 0xde:
        return(KEY_CTRLQUOTE);
      case VK_MULTIPLY:
        return(KEY_CTRL|KEY_MULTIPLY);
      case VK_DIVIDE:
        return(KEY_CTRL|KEY_DIVIDE);
    }

    if(Opt.ShiftsKeyRules) //???
      switch(KeyCode)
      {
        case 0xc0:
          return(KEY_CTRL+'~');
        case 0xbd:
          return(KEY_CTRL+'-');
        case 0xbb:
          return(KEY_CTRL+'=');
        case 0xba:
          return(KEY_CTRL+KEY_COLON);
      }

    if (KeyCode)
    {
      if (!RealKey && KeyCode==VK_CONTROL)
        return(KEY_NONE);
      return(KEY_CTRL+KeyCode);
    }
  }

  /* ------------------------------------------------------------- */
  if (AltPressed)
  {
//_SVS(if(KeyCode!=VK_MENU) SysLog("%9s|0x%08X (%c)|0x%08X (%c)|","Alt",KeyCode,(KeyCode?KeyCode:' '),AsciiChar,(AsciiChar?AsciiChar:' ')));
    if(Opt.ShiftsKeyRules) //???
      switch(KeyCode)
      {
        case 0xc0:
          return(KEY_ALT+'~');
        case 0xbd:
          if(WaitInFastFind)
            return(KEY_ALT+KEY_SHIFT+'_');
          else
            return(KEY_ALT+'-');
        case 0xbb:
          return(KEY_ALT+'=');
        case 0xdc:
          return(KEY_ALT+KEY_BACKSLASH);
        case 0xdd:
          return(KEY_ALT+KEY_BACKBRACKET);
        case 0xdb:
          return(KEY_ALT+KEY_BRACKET);
        case 0xde:
          return(KEY_ALT+KEY_QUOTE);
        case 0xba:
          return(KEY_ALT+KEY_COLON);
        case 0xbf:
          return(KEY_ALT+KEY_SLASH);
      }
    switch(KeyCode)
    {
      case 0xbc:
        return(KEY_ALTCOMMA);
      case 0xbe:
        return(KEY_ALTDOT);
      case VK_DIVIDE:
        if(WaitInFastFind)
          return(KEY_ALT+KEY_SHIFT+'/');
        else
          return(KEY_ALT|KEY_DIVIDE);
      case VK_MULTIPLY:
//        if(WaitInFastFind)
//          return(KEY_ALT+KEY_SHIFT+'*');
//        else
          return(KEY_ALT|KEY_MULTIPLY);
    }
    if (AsciiChar)
    {
      if (Opt.AltGr && WinVer.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS)
        if (rec->Event.KeyEvent.dwControlKeyState & RIGHT_ALT_PRESSED)
          return(AsciiChar);
      if(!Opt.ShiftsKeyRules || WaitInFastFind > 0)
        return(LocalUpper(AsciiChar)+KEY_ALT);
      else if(WaitInMainLoop ||
              !Opt.HotkeyRules //????
           )
        return(KEY_ALT+AsciiChar);
    }
    if (KeyCode==VK_CAPITAL)
      return(KEY_NONE);
    if (!RealKey && KeyCode==VK_MENU)
      return(KEY_NONE);
    return(KEY_ALT+KeyCode);
  }

  if (AsciiChar)
    return(AsciiChar);
  return(KEY_NONE);
}
