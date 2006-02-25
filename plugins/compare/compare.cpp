#define _FAR_NO_NAMELESS_UNIONS
#define _FAR_USE_FARFINDDATA
#include "plugin.hpp"
#include "crt.hpp"

#if defined(__GNUC__)

#ifdef __cplusplus
extern "C"{
#endif
  BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved);
#ifdef __cplusplus
};
#endif

BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved)
{
  (void) lpReserved;
  (void) dwReason;
  (void) hDll;
  return TRUE;
}
#endif

DWORD bufSize;
char *ABuf, *PBuf;

/****************************************************************************
 * Константы для извлечения строк из .lng файла
 ****************************************************************************/
enum CompareLng
{
  MOK=0,
  MCancel,

  MCompare,

  MCmpTitle,
  MProcessBox,
  MProcessSubfolders,
  MProcessSelected,
  MCompareBox,
  MCompareTime,
  MCompareLowPrecision,
  MCompareSize,
  MCompareContents,
  MDisplayBox,
  MMessageWhenNoDiff,

  MFilePanelsRequired,

  MComparing,
  MComparingWith,

  MComparingFiles,

  MNoDiffTitle,
  MNoDiffBody,
  MNoMemTitle,
  MNoMemBody,
  MOldFARTitle,
  MOldFARBody,

  MMaxLngStringNumber,

  MNoLngStringDefined = -1,
};

/****************************************************************************
 * Текущие настройки плагина
 ****************************************************************************/
struct Options
{
  int ProcessSubfolders,
      ProcessSelected,
      CompareTime,
      LowPrecisionTime,
      CompareSize,
      CompareContents,
      MessageWhenNoDiff,
      ProcessHidden;
 } Opt;

/****************************************************************************
 * Копии стандартных структур FAR
 ****************************************************************************/
static struct PluginStartupInfo Info;
static struct FarStandardFunctions FSF;
static char *PluginRootKey=NULL;

void WFD2FFD(WIN32_FIND_DATA &wfd, FAR_FIND_DATA &ffd)
{
  ffd.dwFileAttributes=wfd.dwFileAttributes;
  ffd.ftCreationTime=wfd.ftCreationTime;
  ffd.ftLastAccessTime=wfd.ftLastAccessTime;
  ffd.ftLastWriteTime=wfd.ftLastWriteTime;
  ffd.nFileSizeHigh=wfd.nFileSizeHigh;
  ffd.nFileSizeLow=wfd.nFileSizeLow;
  ffd.dwReserved0=wfd.dwReserved0;
  ffd.dwReserved1=wfd.dwReserved1;
  lstrcpy(ffd.cFileName,wfd.cFileName);
  lstrcpy(ffd.cAlternateFileName,wfd.cAlternateFileName);
}

/****************************************************************************
 * Обёртка сервисной функции FAR: получение строки из .lng-файла
 ****************************************************************************/
static const char *GetMsg(int MsgId)
{
  return Info.GetMsg(Info.ModuleNumber, MsgId);
}

static bool bOldFAR = false;

/****************************************************************************
 * Эту функцию плагина FAR вызывает в первую очередь
 ****************************************************************************/
void WINAPI _export SetStartupInfo(const struct PluginStartupInfo *Info)
{
  const char *cpPlugRegKey = "\\AdvCompare";
  ::Info = *Info;
  if (Info->StructSize >= (int)sizeof(struct PluginStartupInfo))
    FSF = *Info->FSF;
  else
    bOldFAR = true;
  if (PluginRootKey)
  {
    free(PluginRootKey);
    PluginRootKey = NULL;
  }
  if (PluginRootKey = (char *)malloc(lstrlen(Info->RootKey) + lstrlen(cpPlugRegKey) + 1))
  {
    lstrcpy(PluginRootKey, Info->RootKey);
    lstrcat(PluginRootKey, cpPlugRegKey);
  }
  else
  {
    const char *MsgItems[] = { GetMsg(MNoMemTitle), GetMsg(MNoMemBody), GetMsg(MOK) };
    ::Info.Message(::Info.ModuleNumber, FMSG_WARNING, NULL, MsgItems, sizeof(MsgItems) / sizeof(MsgItems[0]), 1);
  }
}

/****************************************************************************
 * Эту функцию FAR вызывает перед выгрузкой плагина
 ****************************************************************************/
void WINAPI _export ExitFAR(void)
{
  if (PluginRootKey)
  {
    free(PluginRootKey);
    PluginRootKey = NULL;
  }
}

/****************************************************************************
 * Эту функцию плагина FAR вызывает во вторую очередь
 ****************************************************************************/
void WINAPI _export GetPluginInfo(struct PluginInfo *Info)
{
  Info->StructSize = sizeof(*Info);
  Info->Flags = 0;
  Info->DiskMenuStringsNumber = 0;
  static const char *PluginMenuStrings[1];
  PluginMenuStrings[0] = GetMsg(MCompare);
  Info->PluginMenuStrings = PluginMenuStrings;
  Info->PluginMenuStringsNumber = sizeof(PluginMenuStrings) / sizeof(PluginMenuStrings[0]);
  Info->PluginConfigStringsNumber = 0;
}

static int iTruncLen;

static void TrunCopy(char *cpDest, const char *cpSrc)
{
  int iLen = lstrlen(FSF.TruncStr(lstrcpy(cpDest, cpSrc), iTruncLen));
  if (iLen < iTruncLen)
  {
    memset(&cpDest[iLen], ' ', iTruncLen - iLen);
    cpDest[iTruncLen] = 0;
  }
}

static bool bStart;

/****************************************************************************
 * Показывает сообщение о сравнении двух файлов
 ****************************************************************************/
static void ShowMessage(const char *Name1, const char *Name2)
{
  static DWORD dwTicks;
  DWORD dwNewTicks = GetTickCount();
  if (dwNewTicks - dwTicks < 500)
    return;
  dwTicks = dwNewTicks;
  char TruncName1[NM], TruncName2[NM];
  const char *MsgItems[] = { GetMsg(MCmpTitle), GetMsg(MComparing), TruncName1, GetMsg(MComparingWith), TruncName2 };
  TrunCopy(TruncName1, Name1);
  TrunCopy(TruncName2, Name2);
  Info.Message(Info.ModuleNumber, bStart? FMSG_LEFTALIGN : FMSG_LEFTALIGN|FMSG_KEEPBACKGROUND, NULL, MsgItems, sizeof(MsgItems) / sizeof(MsgItems[0]), 0);
  bStart = false;
}

/****************************************************************************
 * Читает настройки из реестра, показывает диалог с опциями сравнения,
 * заполняет структуру Opt, сохраняет (если надо) новые настройки в реестре,
 * возвращает true, если пользователь нажал OK
 ****************************************************************************/
static bool ShowDialog(bool bPluginPanels, bool bSelectionPresent)
{
  static struct InitDialogItem
  {
    unsigned char Type;
    unsigned char X1, Y1, X2, Y2;
    int Data;
    unsigned int SelectedDefault;
    char *SelectedRegValue;
    unsigned int Flags;
    int *StoreTo;
  } InitItems[] =
  {
    /*  0 */ { DI_DOUBLEBOX, 3,  1, 48, 16, MCmpTitle,            0, NULL,                            0, NULL },
    /*  1 */ { DI_SINGLEBOX, 5,  2, 46,  5, MProcessBox,          0, NULL,                 DIF_LEFTTEXT, NULL },
    /*  2 */ { DI_CHECKBOX,  7,  3,  0,  0, MProcessSubfolders,   0, "ProcessSubfolders",             0, &Opt.ProcessSubfolders },
    /*  3 */ { DI_CHECKBOX,  7,  4,  0,  0, MProcessSelected,     0, "ProcessSelected",               0, &Opt.ProcessSelected },
    /*  4 */ { DI_SINGLEBOX, 5,  6, 46, 11, MCompareBox,          0, NULL,                 DIF_LEFTTEXT, NULL },
    /*  5 */ { DI_CHECKBOX,  7,  7,  0,  0, MCompareTime,         1, "CompareTime",                   0, &Opt.CompareTime },
    /*  6 */ { DI_CHECKBOX, 11,  8,  0,  0, MCompareLowPrecision, 1, "LowPrecisionTime",              0, &Opt.LowPrecisionTime },
    /*  7 */ { DI_CHECKBOX,  7,  9,  0,  0, MCompareSize,         1, "CompareSize",                   0, &Opt.CompareSize },
    /*  8 */ { DI_CHECKBOX,  7, 10,  0,  0, MCompareContents,     0, "CompareContents",               0, &Opt.CompareContents },
    /*  9 */ { DI_SINGLEBOX, 5, 12, 46, 14, MDisplayBox,          0, NULL,                 DIF_LEFTTEXT, NULL },
    /* 10 */ { DI_CHECKBOX,  7, 13,  0,  0, MMessageWhenNoDiff,   0, "MessageWhenNoDiff",             0, &Opt.MessageWhenNoDiff },
    /* 11 */ { DI_BUTTON,    0, 15,  0,  0, MOK,                  0, NULL,              DIF_CENTERGROUP, NULL },
    /* 12 */ { DI_BUTTON,    0, 15,  0,  0, MCancel,              0, NULL,              DIF_CENTERGROUP, NULL }
  };
  struct FarDialogItem DialogItems[sizeof(InitItems) / sizeof(InitItems[0])];
  HKEY hKey;
  if (!PluginRootKey || RegOpenKeyEx(HKEY_CURRENT_USER, PluginRootKey, 0, KEY_QUERY_VALUE, &hKey) != ERROR_SUCCESS)
    hKey = 0;
  size_t i;
  for (i = 0; i < sizeof(InitItems) / sizeof(InitItems[0]); i++)
  {
    DWORD dwSelected, dwSize = sizeof(DWORD);
    DialogItems[i].Type  = InitItems[i].Type;
    DialogItems[i].X1    = InitItems[i].X1;
    DialogItems[i].Y1    = InitItems[i].Y1;
    DialogItems[i].X2    = InitItems[i].X2;
    DialogItems[i].Y2    = InitItems[i].Y2;
    DialogItems[i].Focus = FALSE;
    DialogItems[i].Flags = InitItems[i].Flags;
    DialogItems[i].Param.Selected = (hKey && InitItems[i].SelectedRegValue && RegQueryValueEx(hKey, InitItems[i].SelectedRegValue, NULL, NULL,
        (unsigned char *)&dwSelected, &dwSize) == ERROR_SUCCESS) ? dwSelected : InitItems[i].SelectedDefault;
    switch (InitItems[i].Data)
    {
      case MProcessSubfolders:
      case MCompareContents:
        if (bPluginPanels)
        {
          DialogItems[i].Flags |= DIF_DISABLE;
          DialogItems[i].Param.Selected = 0;
        }
        break;
      case MProcessSelected:
        if (!bSelectionPresent)
        {
          DialogItems[i].Flags |= DIF_DISABLE;
          DialogItems[i].Param.Selected = 0;
        }
        break;
    }
    DialogItems[i].DefaultButton = (InitItems[i].Data == MOK);
    lstrcpy(DialogItems[i].Data.Data, (InitItems[i].Data == MNoLngStringDefined) ? "" : GetMsg(InitItems[i].Data));
  }
  for (i = 0; i < sizeof(InitItems) / sizeof(InitItems[0]); i++)
    if (InitItems[i].Type == DI_CHECKBOX && !(DialogItems[i].Flags & DIF_DISABLE))
      DialogItems[i].Focus = TRUE;
  if (hKey)
    RegCloseKey(hKey);
  int ExitCode = Info.Dialog(Info.ModuleNumber, -1, -1, 52, 18, "Contents", DialogItems, sizeof(DialogItems) / sizeof(DialogItems[0]));
  if (ExitCode == (sizeof(InitItems) / sizeof(InitItems[0]) - 1) || ExitCode == -1)
    return false;
  for (i = 0; i < sizeof(InitItems) / sizeof(InitItems[0]); i++)
    if (InitItems[i].StoreTo)
      *InitItems[i].StoreTo = DialogItems[i].Param.Selected;
  DWORD dwDisposition;
  if (PluginRootKey && RegCreateKeyEx(HKEY_CURRENT_USER, PluginRootKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dwDisposition) == ERROR_SUCCESS)
  {
    for (i = 0; i < sizeof(InitItems) / sizeof(InitItems[0]); i++)
      if (!(DialogItems[i].Flags & DIF_DISABLE) && InitItems[i].SelectedRegValue)
      {
        DWORD dwValue = (DWORD)DialogItems[i].Param.Selected;
        RegSetValueEx(hKey, InitItems[i].SelectedRegValue, 0, REG_DWORD, (BYTE *)&dwValue, sizeof(dwValue));
      }
    RegCloseKey(hKey);
  }
  if (bPluginPanels)
  {
    Opt.ProcessSubfolders = FALSE;
    Opt.CompareContents = FALSE;
  }
  Opt.ProcessHidden = (Info.AdvControl(::Info.ModuleNumber,ACTL_GETPANELSETTINGS,NULL) & FPS_SHOWHIDDENANDSYSTEMFILES) != 0;
  return true;
}

static bool bBrokenByEsc;
static HANDLE hConInp = INVALID_HANDLE_VALUE;

/****************************************************************************
 * Проверка на Esc. Возвращает true, если пользователь нажал Esc
 ****************************************************************************/
static bool CheckForEsc(void)
{
  if (hConInp == INVALID_HANDLE_VALUE)
    return false;
  static DWORD dwTicks;
  DWORD dwNewTicks = GetTickCount();
  if (dwNewTicks - dwTicks < 500)
    return false;
  dwTicks = dwNewTicks;
  INPUT_RECORD rec;
  DWORD ReadCount;
  while (PeekConsoleInput(hConInp, &rec, 1, &ReadCount) && ReadCount)
  {
    ReadConsoleInput(hConInp, &rec, 1, &ReadCount);
    if (rec.EventType == KEY_EVENT && rec.Event.KeyEvent.wVirtualKeyCode == VK_ESCAPE)
      return bBrokenByEsc = true;
  }
  return false;
}

/****************************************************************************
 * Строит полное имя файла из пути и имени
 ****************************************************************************/
static char *BuildFullFilename(const char *cpDir, const char *cpFileName)
{
  static char cName[NM];
  FSF.AddEndSlash(lstrcpy(cName, cpDir));
  return lstrcat(cName, cpFileName);
}

struct FileIndex
{
  PluginPanelItem **ppi;
  int iCount;
};

/****************************************************************************
 * Функция сравнения имён файлов в двух структурах PluginPanelItem
 * для нужд qsort()
 ****************************************************************************/
static int __cdecl PICompare(const void *el1, const void *el2)
{
  const PluginPanelItem *ppi1 = *(const PluginPanelItem **)el1, *ppi2 = *(const PluginPanelItem **)el2;
  if (ppi1->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
  {
    if (!(ppi2->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
      return -1;
  }
  else
  {
    if (ppi2->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      return 1;
  }
  return -FSF.LStricmp(ppi1->FindData.cFileName, ppi2->FindData.cFileName);
}

/****************************************************************************
 * Построение сортированного списка файлов для быстрого сравнения
 ****************************************************************************/
static bool BuildPanelIndex(const struct PanelInfo *pInfo, struct FileIndex *pIndex)
{
  bool bProcessSelected;
  pIndex->ppi = NULL;
  pIndex->iCount = (bProcessSelected = (Opt.ProcessSelected && pInfo->SelectedItemsNumber && (pInfo->SelectedItems[0].Flags & PPIF_SELECTED)))? pInfo->SelectedItemsNumber: pInfo->ItemsNumber;
  if (!pIndex->iCount)
    return true;
  if (!(pIndex->ppi = (PluginPanelItem **)malloc(pIndex->iCount * sizeof(pIndex->ppi[0]))))
    return false;
  int j = 0;
  for (int i = pInfo->ItemsNumber - 1; i >= 0 && j < pIndex->iCount; i--)
    if ((Opt.ProcessSubfolders || !(pInfo->PanelItems[i].FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) && (!bProcessSelected || (pInfo->PanelItems[i].Flags & PPIF_SELECTED)) && lstrcmp(pInfo->PanelItems[i].FindData.cFileName, "..") && lstrcmp(pInfo->PanelItems[i].FindData.cFileName, "."))
      pIndex->ppi[j++] = &pInfo->PanelItems[i];
  if (pIndex->iCount = j)
    FSF.qsort(pIndex->ppi, j, sizeof(pIndex->ppi[0]), PICompare);
  else
  {
    free(pIndex->ppi);
    pIndex->ppi = NULL;
  }
  return true;
}

/****************************************************************************
 * Освобождение памяти
 ****************************************************************************/
static void FreePanelIndex(struct FileIndex *pIndex)
{
  if (pIndex->ppi)
    free(pIndex->ppi);
  pIndex->ppi = NULL;
  pIndex->iCount = 0;
}

/****************************************************************************
 * Замена сервисной функции Info.GetDirList(). В отличие от оной возвращает
 * список файлов только в каталоге Dir, без подкаталогов.
 ****************************************************************************/
static int GetDirList(const char *Dir, struct PluginPanelItem **pPanelItem, int *pItemsNumber)
{
  *pPanelItem = NULL;
  *pItemsNumber = 0;
  char cPathMask[MAX_PATH];
  WIN32_FIND_DATA wfdFindData;
  HANDLE hFind;
  if ((hFind = FindFirstFile(lstrcat(lstrcpy(cPathMask, Dir), "\\*"), &wfdFindData)) == INVALID_HANDLE_VALUE)
    return TRUE;
  int iRet = TRUE;
  do
  {
    if (!lstrcmp(wfdFindData.cFileName, ".") || !lstrcmp(wfdFindData.cFileName, ".."))
      continue;
    if ((wfdFindData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) && !Opt.ProcessHidden)
      continue;
    struct PluginPanelItem *pPPI;
    if (!(pPPI = (struct PluginPanelItem *)realloc(*pPanelItem, (*pItemsNumber + 1) * sizeof(*pPPI))))
    {
      free(*pPanelItem);
      *pItemsNumber = 0;
      iRet = FALSE;
      break;
    }
    *pPanelItem = pPPI;
    WFD2FFD(wfdFindData,(*pPanelItem)[(*pItemsNumber)++].FindData);
  } while (FindNextFile(hFind, &wfdFindData));
  FindClose(hFind);
  return iRet;
}

/****************************************************************************
 * Замена сервисной функции Info.FreeDirList().
 ****************************************************************************/
static void FreeDirList(struct PluginPanelItem *PanelItem)
{
  if (PanelItem)
    free(PanelItem);
}

static bool CompareDirs(const struct PanelInfo *AInfo, const struct PanelInfo *PInfo, bool bCompareAll);

/****************************************************************************
 * Сравнение атрибутов и прочего для двух одноимённых элементов (файлов или
 * подкаталогов).
 * Возвращает true, если они совпадают.
 ****************************************************************************/
static bool CompareFiles(const FAR_FIND_DATA *AData, const FAR_FIND_DATA *PData, const char *ACurDir, const char *PCurDir)
{
  // Здесь сравниваем два подкаталога
  if (AData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
  {
    if (Opt.ProcessSubfolders)
    {
      // Составим списки файлов в подкаталогах
      struct PanelInfo AInfo, PInfo;
      memset(&AInfo, 0, sizeof(AInfo));
      memset(&PInfo, 0, sizeof(PInfo));
      bool bEqual;
      if (!GetDirList(lstrcpy(AInfo.CurDir, BuildFullFilename(ACurDir, AData->cFileName)), &AInfo.PanelItems, &AInfo.ItemsNumber)
         || !GetDirList(lstrcpy(PInfo.CurDir, BuildFullFilename(PCurDir, PData->cFileName)), &PInfo.PanelItems, &PInfo.ItemsNumber))
      {
        bBrokenByEsc = true; // То ли юзер прервал, то ли ошибка чтения
        bEqual = false; // Остановим сравнение
      }
      else
        bEqual = CompareDirs(&AInfo, &PInfo, false);
      FreeDirList(AInfo.PanelItems);
      FreeDirList(PInfo.PanelItems);
      return bEqual;
    }
  }
  else // Здесь сравниваем два файла
  {
    if (Opt.CompareSize)
      if (AData->nFileSizeLow != PData->nFileSizeLow || AData->nFileSizeHigh != PData->nFileSizeHigh)
        return false;
    if (Opt.CompareTime)
    {
      if (Opt.LowPrecisionTime)
      {
        //сравним с точностью в 2 секунды
        if (AData->ftLastWriteTime.dwHighDateTime != PData->ftLastWriteTime.dwHighDateTime
            || (20000000<max(AData->ftLastWriteTime.dwLowDateTime,PData->ftLastWriteTime.dwLowDateTime)-min(AData->ftLastWriteTime.dwLowDateTime,PData->ftLastWriteTime.dwLowDateTime)))
          return false;
      }
      else if (AData->ftLastWriteTime.dwLowDateTime != PData->ftLastWriteTime.dwLowDateTime || AData->ftLastWriteTime.dwHighDateTime != PData->ftLastWriteTime.dwHighDateTime)
        return false;
    }
    if (Opt.CompareContents)
    {
      HANDLE hFileA, hFileP;
      char cpFileA[MAX_PATH], cpFileP[MAX_PATH];
      ShowMessage(lstrcpy(cpFileA, BuildFullFilename(ACurDir, AData->cFileName)), lstrcpy(cpFileP, BuildFullFilename(PCurDir, PData->cFileName)));
      if ((hFileA = CreateFile(cpFileA, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL)) == INVALID_HANDLE_VALUE)
        return false;
      if ((hFileP = CreateFile(cpFileP, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL)) == INVALID_HANDLE_VALUE)
      {
        CloseHandle(hFileA);
        return false;
      }

      bool bEqual = true;
      DWORD ReadSizeA, ReadSizeP;
      do
      {
        if (CheckForEsc() || !ReadFile(hFileA, ABuf, bufSize, &ReadSizeA, NULL) || !ReadFile(hFileP, PBuf, bufSize, &ReadSizeP, NULL) || (ReadSizeA != ReadSizeP))
        {
          bEqual = false;
          break;
        }
        if (!ReadSizeA)
          break;
        if (memcmp(ABuf, PBuf, ReadSizeA))
        {
          bEqual = false;
          break;
        }
      } while (ReadSizeA == bufSize);
      CloseHandle(hFileA);
      CloseHandle(hFileP);
      return bEqual;
    }
  }
  return true;
}

/****************************************************************************
 * Сравнение двух каталогов, описанных структурами AInfo и PInfo.
 * Возвращает true, если они совпадают.
 * Параметр bCompareAll определяет,
 * надо ли сравнивать все файлы и взводить PPIF_SELECTED (bCompareAll == true)
 * или просто вернуть false при первом несовпадении (bCompareAll == false).
 ****************************************************************************/
static bool CompareDirs(const struct PanelInfo *AInfo, const struct PanelInfo *PInfo, bool bCompareAll)
{
  // Строим индексы файлов для быстрого сравнения
  struct FileIndex sfiA, sfiP;
  char DirA[MAX_PATH], DirP[MAX_PATH];
  ShowMessage(lstrcpy(DirA, BuildFullFilename(AInfo->CurDir, "*")), lstrcpy(DirP, BuildFullFilename(PInfo->CurDir, "*")));
  if (!BuildPanelIndex(AInfo, &sfiA) || !BuildPanelIndex(PInfo, &sfiP))
  {
    const char *MsgItems[] = { GetMsg(MNoMemTitle), GetMsg(MNoMemBody), GetMsg(MOK) };
    ::Info.Message(::Info.ModuleNumber, FMSG_WARNING, NULL, MsgItems, sizeof(MsgItems) / sizeof(MsgItems[0]), 1);
    bBrokenByEsc = true;
    FreePanelIndex(&sfiA);
    FreePanelIndex(&sfiP);
    return true;
  }
  bool bDifferenceNotFound = true;
  int i = sfiA.iCount - 1, j = sfiP.iCount - 1;
  while (i >= 0 && j >= 0 && (bDifferenceNotFound || bCompareAll) && !bBrokenByEsc)
  {
    const int iMaxCounter = 256;
    static int iCounter = iMaxCounter;
    if (!--iCounter)
    {
      iCounter = iMaxCounter;
      if (CheckForEsc())
        break;
    }
    switch (PICompare(&sfiA.ppi[i], &sfiP.ppi[j]))
    {
      case 0: // Имена совпали - проверяем всё остальное
        if (CompareFiles(&sfiA.ppi[i]->FindData, &sfiP.ppi[j]->FindData, AInfo->CurDir, PInfo->CurDir))
        {
          // И остальное совпало
          sfiA.ppi[i--]->Flags &= ~PPIF_SELECTED;
          sfiP.ppi[j--]->Flags &= ~PPIF_SELECTED;
        }
        else
        {
          bDifferenceNotFound = false;
          sfiA.ppi[i--]->Flags |= PPIF_SELECTED;
          sfiP.ppi[j--]->Flags |= PPIF_SELECTED;
        }
        break;
      case 1: // Элемент sfiA.ppi[i] не имеет одноимённых в sfiP.ppi
        bDifferenceNotFound = false;
        sfiA.ppi[i--]->Flags |= PPIF_SELECTED;
        break;
      case -1: // Элемент sfiP.ppi[j] не имеет одноимённых в sfiA.ppi
        bDifferenceNotFound = false;
        sfiP.ppi[j--]->Flags |= PPIF_SELECTED;
        break;
    }
  }
  if (!bBrokenByEsc)
  {
    // Собственно сравнение окончено. Пометим то, что осталось необработанным
    // в массивах
    if (i >= 0)
    {
      bDifferenceNotFound = false;
      if (bCompareAll)
        for (; i >= 0; i--)
          sfiA.ppi[i]->Flags |= PPIF_SELECTED;
    }
    if (j >= 0)
    {
      bDifferenceNotFound = false;
      if (bCompareAll)
        for (; j >= 0; j--)
          sfiP.ppi[j]->Flags |= PPIF_SELECTED;
    }
  }
  FreePanelIndex(&sfiA);
  FreePanelIndex(&sfiP);
  return bDifferenceNotFound;
}

/****************************************************************************
 * Основная функция плагина. FAR её вызывает, когда пользователь зовёт плагин
 ****************************************************************************/
HANDLE WINAPI _export OpenPlugin(int OpenFrom, int Item)
{
  if (bOldFAR)
  {
    const char *MsgItems[] = { GetMsg(MOldFARTitle), GetMsg(MOldFARBody), GetMsg(MOK) };
    Info.Message(Info.ModuleNumber, FMSG_WARNING, NULL, MsgItems, sizeof(MsgItems) / sizeof(MsgItems[0]), 1);
    return INVALID_HANDLE_VALUE;
    }
  struct PanelInfo AInfo, PInfo;
  if (!Info.Control(INVALID_HANDLE_VALUE, FCTL_GETPANELINFO, &AInfo) || !Info.Control(INVALID_HANDLE_VALUE, FCTL_GETANOTHERPANELINFO, &PInfo))
    return INVALID_HANDLE_VALUE;
  if (AInfo.PanelType != PTYPE_FILEPANEL || PInfo.PanelType != PTYPE_FILEPANEL)
  {
    const char *MsgItems[] = { GetMsg(MCmpTitle), GetMsg(MFilePanelsRequired), GetMsg(MOK)};
    Info.Message(Info.ModuleNumber, FMSG_WARNING, NULL, MsgItems, sizeof(MsgItems) / sizeof(MsgItems[0]), 1);
    return INVALID_HANDLE_VALUE;
  }
  if (!ShowDialog(AInfo.Plugin || PInfo.Plugin,
     (AInfo.SelectedItemsNumber && (AInfo.SelectedItems[0].Flags & PPIF_SELECTED))
     || (PInfo.SelectedItemsNumber && (PInfo.SelectedItems[0].Flags & PPIF_SELECTED))))
    return INVALID_HANDLE_VALUE;
  HANDLE hScreen = Info.SaveScreen(0, 0, -1, -1);
  // Откроем консольный ввод для проверок на Esc
  hConInp = CreateFile("CONIN$", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
  HANDLE hConOut = CreateFile("CONOUT$", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
  CONSOLE_SCREEN_BUFFER_INFO csbiNfo;
  if (GetConsoleScreenBufferInfo(hConOut, &csbiNfo))
  {
    if ((iTruncLen = csbiNfo.dwSize.X - 20) > NM - 2)
      iTruncLen = NM - 2;
    else if (iTruncLen < 0)
      iTruncLen = csbiNfo.dwSize.X - csbiNfo.dwSize.X / 4;
  }
  else
    iTruncLen = 60;
  CloseHandle(hConOut);
  bBrokenByEsc = false;
  bStart = true;
  char cConsoleTitle[MAX_PATH], cBuffer[MAX_PATH];
  DWORD dwTitleSaved = GetConsoleTitle(cConsoleTitle, sizeof(cConsoleTitle));
  OSVERSIONINFO ovi;
  ovi.dwOSVersionInfoSize = sizeof(ovi);
  if (GetVersionEx(&ovi) && ovi.dwPlatformId != VER_PLATFORM_WIN32_NT)
    OemToChar(GetMsg(MComparingFiles), cBuffer);
  else
    lstrcpy(cBuffer, GetMsg(MComparingFiles));
  SetConsoleTitle(cBuffer);

  bool bDifferenceNotFound = false;
  HKEY hKey;
  if (!PluginRootKey || RegOpenKeyEx(HKEY_CURRENT_USER, PluginRootKey, 0, KEY_QUERY_VALUE, &hKey) != ERROR_SUCCESS)
    hKey = 0;
  DWORD dwSize = sizeof(DWORD);
  bufSize = (hKey && (RegQueryValueEx(hKey, "CompareBufferSize", NULL, NULL, (unsigned char *)&bufSize, &dwSize) == ERROR_SUCCESS) && (bufSize > 32767)) ? bufSize : 32768;
  if (hKey)
    RegCloseKey(hKey);
  ABuf = (char*)malloc(bufSize);
  PBuf = (char*)malloc(bufSize);
  if (ABuf && PBuf)
    bDifferenceNotFound = CompareDirs(&AInfo, &PInfo, true);
  free (ABuf);
  free (PBuf);

  CloseHandle(hConInp);
  Info.RestoreScreen(hScreen);
  if (!bBrokenByEsc)
  {
    Info.Control(INVALID_HANDLE_VALUE, FCTL_SETSELECTION, &AInfo);
    Info.Control(INVALID_HANDLE_VALUE, FCTL_SETANOTHERSELECTION, &PInfo);
    Info.Control(INVALID_HANDLE_VALUE, FCTL_REDRAWPANEL, NULL);
    Info.Control(INVALID_HANDLE_VALUE, FCTL_REDRAWANOTHERPANEL, NULL);
    if (bDifferenceNotFound && Opt.MessageWhenNoDiff)
    {
      const char *MsgItems[] = { GetMsg(MNoDiffTitle), GetMsg(MNoDiffBody), GetMsg(MOK)};
      Info.Message(Info.ModuleNumber, 0, NULL, MsgItems, sizeof(MsgItems) / sizeof(MsgItems[0]), 1);
    }
  }
  if (dwTitleSaved)
    SetConsoleTitle(cConsoleTitle);
  return INVALID_HANDLE_VALUE;
}
