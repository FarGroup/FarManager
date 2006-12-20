#define _WIN32_WINNT 0x0400

#define CMDOFFSET 50

#define BOUNDS( x )  (sizeof( (x) ) / sizeof( (x)[ 0 ] ))

#include <windows.h>
#include <shlobj.h>
//#include <atlbase.h>

HINSTANCE g_dllHandle;
HANDLE g_threadHandle;
BOOL g_started = FALSE;
unsigned long g_threadid = 0;
char PanelItem[40];

char **g_sendToCache = NULL;
int g_sendToCacheCount;

HANDLE g_thread_ready = NULL;
HANDLE g_thread_finished = NULL;

#ifndef _DEBUG

extern "C" long __stdcall _DllMainCRTStartup( HINSTANCE pDLLInstance, DWORD
                                             pReason, LPVOID )
{
    if (pReason == DLL_PROCESS_ATTACH)
        g_dllHandle = pDLLInstance;

    return TRUE;
}

#endif

void *my_malloc (size_t size)
{
  return HeapAlloc (GetProcessHeap(), 0, size);
}

void my_free (void *memblock)
{
  HeapFree (GetProcessHeap(), 0, memblock);
}

BOOL APIENTRY DllMain(HINSTANCE pDLLInstance, DWORD pReason,
		      LPVOID /*pReserved */ )
{
  if (pReason == DLL_PROCESS_ATTACH)
    g_dllHandle = pDLLInstance;

  return TRUE;
}

#include "plugin.hpp"
#include "emenulng.hpp"
#include "emenu.hpp"
#include "reg.cpp"
#include "mix.cpp"
#include "resource.h"

BOOL DoMenu();
UINT GetItemCount(LPITEMIDLIST pidl);
LPITEMIDLIST GetNextItem(LPITEMIDLIST pidl);
LPITEMIDLIST GetNextItem(LPITEMIDLIST pidl);
LPITEMIDLIST DuplicateItem(LPMALLOC pMalloc, LPITEMIDLIST pidl);

unsigned long __stdcall thread_proc(void *);
void StartOLEThread();
void StopOLEThread();

int WINAPI _export Configure(int /*ItemNumber */ )
{
  struct InitDialogItem InitItems[] = {
    {DI_DOUBLEBOX, 3, 1, 41, 7, 0, 0, 0, 0, (char *) MTitle},
    {DI_CHECKBOX, 5, 2, 0, 0, TRUE, 0, 0, 0, (char *) MMessage},
    {DI_TEXT, 0, 3, 0, 0, 0, 0, DIF_BOXCOLOR | DIF_SEPARATOR, 0, ""},
    {DI_CHECKBOX, 5, 4, 0, 0, 0, 0, DIF_3STATE, 0, (char *) MShowGUI},
    {DI_TEXT, 0, 5, 0, 0, 0, 0, DIF_BOXCOLOR | DIF_SEPARATOR, 0, ""},
    {DI_BUTTON, 0, 6, 0, 0, 0, 0, DIF_CENTERGROUP, 1, (char *) MSave},
    {DI_BUTTON, 0, 6, 0, 0, 0, 0, DIF_CENTERGROUP, 0, (char *) MCancel}
  };

  struct FarDialogItem DialogItems[BOUNDS(InitItems)];

  InitDialogItems(InitItems, DialogItems, BOUNDS(InitItems));

  DialogItems[1].Selected = WaitToContinue;
  DialogItems[3].Selected = UseGUI;

  if (5 !=
      g_Info.Dialog(g_Info.ModuleNumber, -1, -1, 45, 9, "Config",
		    DialogItems, BOUNDS(DialogItems)))
    {
      return 0;
    }

  WaitToContinue = DialogItems[1].Selected;
  UseGUI = DialogItems[3].Selected;
  SetRegKey(HKEY_CURRENT_USER, "", REGStr.WaitToContinue, WaitToContinue);
  SetRegKey(HKEY_CURRENT_USER, "", REGStr.UseGUI, UseGUI);
  return 1;
}

void WINAPI _export SetStartupInfo(const struct PluginStartupInfo *Info)
{
  StartOLEThread();

  g_Info = *Info;
  IsOldFAR = TRUE;

  if (Info->StructSize >= sizeof(struct PluginStartupInfo))
  {
    DWORD FarVER;
    g_Info.AdvControl(g_Info.ModuleNumber, ACTL_GETFARVERSION, &FarVER);

    IsOldFAR = !(HIWORD(FarVER) >= 321);
  }

  if (!IsOldFAR)
  {
    g_FSF = *Info->FSF;
    g_Info.FSF = &g_FSF;
  }

  strcpy(PluginRootKey, Info->RootKey);
  strcat(PluginRootKey, "\\ExplorerMenu");
  UseGUI = GetRegKey(HKEY_CURRENT_USER, "", REGStr.UseGUI, 0);
  WaitToContinue = GetRegKey(HKEY_CURRENT_USER, "", REGStr.WaitToContinue, 0);
}

void WINAPI _export GetPluginInfo(struct PluginInfo *Info)
{
  if(IsOldFAR) return;

  static const char *PluginMenuStrings[1], *PluginConfigStrings[1];

  StartOLEThread();

  PluginMenuStrings[0] = GetMsg(MTitle);
  PluginConfigStrings[0] = GetMsg(MTitle);
  Info->StructSize = sizeof(*Info);
  Info->Flags = 0;
  Info->DiskMenuStringsNumber = 0;
  Info->PluginMenuStrings = PluginMenuStrings;
  Info->PluginMenuStringsNumber = BOUNDS(PluginMenuStrings);
  Info->PluginConfigStrings = PluginConfigStrings;
  Info->PluginConfigStringsNumber = BOUNDS(PluginConfigStrings);
}


HANDLE WINAPI _export OpenPlugin(int /*OpenFrom */ , int /*Item */ )
{
  SetFileApisToANSI();

  if (DoMenu())
    {
      const char *MsgItems[3];

      MsgItems[0] = GetMsg(MTitle);
      MsgItems[1] = GetMsg(MClose);
      MsgItems[2] = GetMsg(MOk);

      if (0 != WaitToContinue)
	{
	  g_Info.Message(g_Info.ModuleNumber, NULL, "Close", MsgItems,
			 BOUNDS(MsgItems), 1);
	}
    }
  SetFileApisToOEM();

  g_Info.Control(INVALID_HANDLE_VALUE, FCTL_UPDATEPANEL, (void *) 1);
  g_Info.Control(INVALID_HANDLE_VALUE, FCTL_REDRAWPANEL, NULL);

  return INVALID_HANDLE_VALUE;
}

void WINAPI _export ExitFAR()
{
  StopOLEThread();
}


int RealLength(const char *text)
{
  int rc = 0;
  const char *ptr = text;

  while (*ptr != '\0')
    {
      if (*ptr != '&')
	{
	  rc++;
	}
      ptr++;
    }
  return rc;
}

void TrimSub(char *dest, char *src)
{
  int src_len = strlen(src);

  if (src[src_len - 1] == '>')
    {
      char *end = src + src_len - 2;

      while (*end == ' ')
	{
	  end--;
	}

      memcpy(dest, src, end - src + 1);
      dest[end - src + 1] = '\0';
    }
}

BOOL ShowMenu(LPITEMIDLIST pidlMain, HWND hwnd, HMENU hMenu, char *title,
	      LPCONTEXTMENU pContextMenu, BOOL main, BOOL & BACKSPACEPRESSED,
	      LPCONTEXTMENU2 pContextMenu2)
{
  int ni, total = 0;
  int items = 0;
  BOOL rc = FALSE;
  size_t max_item_len = 0, item_len = 0;

  if (pContextMenu2)
    pContextMenu2->HandleMenuMsg(WM_INITMENUPOPUP, (WPARAM) hMenu, 0);

  int count = GetMenuItemCount(hMenu);

  BACKSPACEPRESSED = 0;

  if (count == 0)
    return rc;

  FarMenuItem *menu =
    (struct FarMenuItem *) my_malloc((count + 2) * sizeof(struct FarMenuItem));
  UINT *IDs = (UINT *) my_malloc((count + 2) * sizeof(UINT));
  HMENU *Subs = (HMENU *) my_malloc((count + 2) * sizeof(HMENU));
  char text[sizeof(menu[0].Text)];

  if (menu == NULL || IDs == NULL || Subs == NULL)
    return rc;

  memset(IDs, 0, (count + 2) * sizeof(UINT));
  memset(Subs, 0, (count + 2) * sizeof(HMENU));
  memset(menu, 0, (count + 2) * sizeof(struct FarMenuItem));

  MENUITEMINFO mi;

  for (ni = 0; ni < count; ni++)
    {
      memset(&mi, 0, sizeof(mi));

      mi.cbSize = sizeof(mi);
      mi.fMask = MIIM_TYPE | MIIM_ID | MIIM_SUBMENU;
      mi.dwTypeData = text;
      mi.cch = sizeof(text) - 3;	/* Compensate for
					   the possible submenu */

      if (!GetMenuItemInfo(hMenu, ni, TRUE, &mi))
	break;

      if (mi.fType == MFT_STRING)
	{
	  menu[ni].Separator = 0;
	  total++;
	}
      else if (mi.fType == MFT_OWNERDRAW && g_sendToCacheCount != 0)
	{
	  menu[ni].Separator = 0;
	  total++;

	  if (ni < g_sendToCacheCount)
	    strcpy(text, g_sendToCache[ni]);
	  else
	    strcpy(text, "-- owner drawn --");
	}
      else
	{
	  menu[ni].Separator = 1;
	}

      item_len = RealLength(text);
      max_item_len = max(max_item_len, item_len);
      CharToOem(text, menu[ni].Text);

      if (NULL == mi.hSubMenu)
	{
	  IDs[ni] = mi.wID;
	}
      else
	{
	  Subs[ni] = mi.hSubMenu;

          //pContextMenu2->HandleMenuMsg(WM_INITMENUPOPUP, (WPARAM) mi.hSubMenu, ni);
	}
    }

  items = total ? ni : 0;

  char newTitle[128];

  if (main)
    strcpy(newTitle, PanelItem);
  else
  {
    static char tmpTitle[128];
    int newTitlePos = 0, oldTitlePos = 0;

    while (title[oldTitlePos] && newTitlePos < 128)
    {
      if (title[oldTitlePos] != '&')
        tmpTitle[newTitlePos++] = title[oldTitlePos];
      oldTitlePos++;
    }
    tmpTitle[newTitlePos] = 0;
    strcpy (newTitle, PanelItem);
    strcat (newTitle, " / ");
    g_FSF.TruncStr (tmpTitle, 35);
    strcat (newTitle, tmpTitle);
  }

  max_item_len = max(max_item_len, strlen (newTitle));

  // Indicate submenus if any

  if (max_item_len > 0 && items)
    {
      for (ni = 0; ni < items; ni++)
	{
	  if (NULL != Subs[ni])
	    {
	      item_len = RealLength(menu[ni].Text);

	      for (size_t j = 0; j < (max_item_len + 2 - item_len); j++)
		strcat(menu[ni].Text, " ");

	      strcat(menu[ni].Text, ">");
	    }
	}
    }

  int BreakKeys[] = { VK_BACK, (PKF_CONTROL << 16) | VK_LEFT,
    (PKF_CONTROL << 16) | VK_PRIOR, VK_SPACE,
    (PKF_CONTROL << 16) | VK_RIGHT, (PKF_CONTROL << 16) | VK_NEXT,
    0
  }, BreakCode = -1, again = 1, index = 0, Forward, Back;

  while (again)
    {
      for (int i = 0; i < items; i++)
	menu[i].Selected = FALSE;

      menu[index].Selected = TRUE;

      index = g_Info.Menu(g_Info.ModuleNumber,
			  -1, -1, 0,
			  FMENU_WRAPMODE,
			  newTitle,
			  (main) ? NULL : ">",
			  "Contents", BreakKeys, &BreakCode, menu, items);
      again = Back = Forward = 0;

      if (BreakCode > -1)
	{
	  if (BreakCode < 3)
	    {
	      if (main)
		again = 1;
	      else
		Back = 1;
	    }
	  else
	    Forward = 1;
	}
      else if (index != -1)
	Forward = 1;

      if (Back)
	BACKSPACEPRESSED = 1;
      else if (Forward)		//if( index != -1 )
	{			// If a command was selected from the menu, execute it.
	  UINT nCmd = IDs[index];

	  if (NULL != Subs[index])
	    {
	      // Submenu - continue

	      TrimSub(text, menu[index].Text);

	      rc = ShowMenu(pidlMain, hwnd, Subs[index], text,
			    pContextMenu, FALSE, again, pContextMenu2);
	    }
	  else if (nCmd >= CMDOFFSET)
	    {
	      // Invoke item command

	      CMINVOKECOMMANDINFO ici;

	      ici.cbSize = sizeof(CMINVOKECOMMANDINFO);
	      ici.fMask = SEE_MASK_FLAG_DDEWAIT;
	      ici.hwnd = hwnd;
	      ici.lpVerb = MAKEINTRESOURCE(nCmd - CMDOFFSET);
	      ici.lpParameters = NULL;
	      ici.lpDirectory = NULL;
	      ici.nShow = SW_SHOWNORMAL;
	      ici.dwHotKey = 0;
	      ici.hIcon = NULL;

	      rc = (0 != SUCCEEDED(pContextMenu->InvokeCommand(&ici)));
	    }
	}
    }

  if (NULL != menu)
    {
      my_free(menu);
    }
  if (NULL != IDs)
    {
      my_free(IDs);
    }
  if (NULL != Subs)
    {
      my_free(Subs);
    }

  return rc;
}

BOOL CALLBACK MenuHandlerDlgProc(HWND hWnd, UINT msg, WPARAM wParam,
				 LPARAM lParam)
{
  //grab object pointer from window data -- we put it there in WM_CREATE
  IContextMenu2 *pcm = (IContextMenu2 *)::GetWindowLong(hWnd, GWL_USERDATA);

  switch (msg)
    {
      case WM_INITDIALOG:
	//get pointer to the IContextMenu2 on whose behalf we're acting
	pcm = (IContextMenu2 *) lParam;	//passed to CreateDialog()
	//save it in window info
	::SetWindowLong(hWnd, GWL_USERDATA, (LONG) pcm);
	::ShowWindow(hWnd, SW_HIDE);
	return TRUE;

      case WM_DRAWITEM:
      case WM_MEASUREITEM:
      case WM_INITMENUPOPUP:
	if (pcm != NULL)
	  pcm->HandleMenuMsg(msg, wParam, lParam);

	return (msg == WM_INITMENUPOPUP ? FALSE : TRUE);
    }

  return FALSE;
}

int ShowGuiMenu(LPCONTEXTMENU pContextMenu, HMENU hMenu)
{
  StopOLEThread();

  HWND hDlg = CreateDialogParam(g_dllHandle, MAKEINTRESOURCE(IDD_NULL), NULL,
				(DLGPROC) MenuHandlerDlgProc,
				(LPARAM) pContextMenu);
  POINT pt;

  ::GetCursorPos(&pt);

  ::SetForegroundWindow(hDlg);
  int idCmd =::TrackPopupMenu(hMenu,
                              TPM_LEFTALIGN | TPM_RETURNCMD | TPM_RIGHTBUTTON,
                              pt.x, pt.y, 0, hDlg, NULL);
  ::SendMessage(hDlg, WM_NULL, 0, 0);

  DestroyWindow(hDlg);
  DestroyMenu(hMenu);

  // check if the user pressed Esc in the menu
  if (idCmd == 0)
    return TRUE;

  StartOLEThread();

  CMINVOKECOMMANDINFO ici;

  ici.cbSize = sizeof(CMINVOKECOMMANDINFO);
  ici.fMask = SEE_MASK_FLAG_DDEWAIT;
  ici.hwnd = NULL;
  ici.lpVerb = MAKEINTRESOURCE(idCmd - CMDOFFSET);
  ici.lpParameters = NULL;
  ici.lpDirectory = NULL;
  ici.nShow = SW_SHOWNORMAL;
  ici.dwHotKey = 0;
  ici.hIcon = NULL;

  return (0 != SUCCEEDED(pContextMenu->InvokeCommand(&ici)));
}

BOOL DoMenu()
{
  char name[MAX_PATH];
  char fullname[MAX_PATH];
  struct PanelInfo PInfo;
  BOOL rc = FALSE;
  BOOL dir = FALSE;
  HWND hwnd = NULL;

  g_Info.Control(INVALID_HANDLE_VALUE, FCTL_GETPANELINFO, &PInfo);

  if (PInfo.PanelType == PTYPE_FILEPANEL)
  {
    if (0 >= PInfo.ItemsNumber)
    {
        return rc;
    }
    dir =
        0 !=
        (PInfo.PanelItems[PInfo.CurrentItem].FindData.
        dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);

    strcpy(name, PInfo.PanelItems[PInfo.CurrentItem].FindData.cFileName);
    strcpy(fullname, PInfo.CurDir);

    if (!dir || (0 != strcmp(name, "..")))
    {
        if (fullname[strlen(fullname) - 1] != '\\')
        {
            strcat(fullname, "\\");
        }
        strcat(fullname, name);
    }
  }
  else if (PInfo.PanelType == PTYPE_TREEPANEL)
  {
    dir = TRUE;
    strcpy (fullname, PInfo.CurDir);
    strcpy (name, g_FSF.PointToName (PInfo.CurDir));
    if (!strlen (name))   // no name when invoking the menu for the root of the drive
      strcpy (name, fullname);
  }
  else
    return rc;

  // Prepare menu title

  if (dir)
    {
      PanelItem[0] = '<';
      g_FSF.TruncStr (name, 30);
      strcpy (PanelItem+1, name);
      strcat(PanelItem, ">");
    }
  else
  {
    g_FSF.TruncStr (name, 33);
    strcpy (PanelItem, name);
  }

  // Make sure the file name is in Unicode format.

  WCHAR wchPath[MAX_PATH];
  MultiByteToWideChar(CP_OEMCP, MB_ERR_INVALID_CHARS, fullname, -1,
		      wchPath, MAX_PATH);

  // Get pointers to the shell's IMalloc interface and the desktop's
  // IShellFolder interface.

  CComPtr<IMalloc> pMalloc;
  if (!SUCCEEDED(SHGetMalloc(&pMalloc)))
    return rc;

  CComPtr<IShellFolder> psfFolder;
  if (!SUCCEEDED(SHGetDesktopFolder(&psfFolder)))
    return rc;

  // Convert the path name into a pointer to an item ID list (pidl).

  ULONG ulCount, ulAttr;
  LPITEMIDLIST pidlMain;
  if (SUCCEEDED(psfFolder->ParseDisplayName(hwnd, NULL, wchPath, &ulCount,
					    &pidlMain, &ulAttr))
      && (pidlMain != NULL))
    {
      UINT nCount = GetItemCount(pidlMain);
      if (nCount)
	{
	  // Initialize psfFolder with a pointer to the IShellFolder
	  // interface of the folder that contains the item whose context
	  // menu we're after, and initialize pidlItem with a pointer to
	  // the item's item ID. If nCount > 1, this requires us to walk
	  // the list of item IDs stored in pidlMain and bind to each
	  // subfolder referenced in the list.

	  LPITEMIDLIST pidlItem = pidlMain;
	  while (--nCount)
	    {
	      // Create a 1-item item ID list for the next item in pidlMain.

	      LPITEMIDLIST pidlNextItem = DuplicateItem(pMalloc, pidlItem);
	      if (pidlNextItem == NULL)
		{
		  pMalloc->Free(pidlMain);
		  return rc;
		}

	      // Bind to the folder specified in the new item ID list.

              CComPtr<IShellFolder> psfNextFolder;
              if (!SUCCEEDED(psfFolder->BindToObject(pidlNextItem, NULL,
						     IID_IShellFolder,
						     (void **)
						     &psfNextFolder)))
		{
		  pMalloc->Free(pidlNextItem);
		  pMalloc->Free(pidlMain);
		  return rc;
		}

	      // Release the IShellFolder pointer to the parent folder
	      // and set psfFolder equal to the IShellFolder pointer for
	      // the current folder.

	      psfFolder = psfNextFolder;
              psfNextFolder = NULL;

	      // Release the storage for the 1-item item ID list we created
	      // just a moment ago and initialize pidlItem so that it points
	      // to the next item in pidlMain.

	      pMalloc->Free(pidlNextItem);
	      pidlItem = GetNextItem(pidlItem);
	    }

	  // Get a pointer to the item's IContextMenu interface and call
	  // IContextMenu::QueryContextMenu to initialize a context menu.

          LPITEMIDLIST *ppidl = &pidlItem;

          CComPtr<IContextMenu> pContextMenu;
          CComPtr<IContextMenu2> pContextMenu2;
          CComPtr<IContextMenu3> pContextMenu3;

          HRESULT hr = psfFolder->GetUIObjectOf(hwnd, 1, (LPCITEMIDLIST *) ppidl,
            IID_IContextMenu3, NULL,
            (void **) &pContextMenu3);
          if (SUCCEEDED (hr))
          {
            pContextMenu = CComQIPtr<IContextMenu, &IID_IContextMenu> (pContextMenu3);
            pContextMenu2 = CComQIPtr<IContextMenu2, &IID_IContextMenu2> (pContextMenu3);
          }
          else {
            hr = psfFolder->GetUIObjectOf(hwnd, 1, (LPCITEMIDLIST *) ppidl,
              IID_IContextMenu2, NULL,
              (void **) &pContextMenu2);
            if (SUCCEEDED(hr))
            {
              CComQIPtr<IContextMenu, &IID_IContextMenu> pIM (pContextMenu2);
              pContextMenu = pIM;
            }
            else {
              hr = psfFolder->GetUIObjectOf(hwnd, 1, (LPCITEMIDLIST *) ppidl,
                IID_IContextMenu, NULL,
                (void **) &pContextMenu);
              if (SUCCEEDED(hr))
              {
                CComQIPtr<IContextMenu2, &IID_IContextMenu2> pIM2 (pContextMenu);
                pContextMenu2 = pIM2;
              }
            }
          }

          if (pContextMenu)
            {
	      HMENU hMenu = CreatePopupMenu();

	      if (SUCCEEDED
		  (pContextMenu->
		   QueryContextMenu(hMenu, 0, CMDOFFSET, 0x7FFF, CMF_EXPLORE)))
		{
		  int GUI;
		  if(2==UseGUI)
		  {
		    static FarMenuItem fmi[2];
		    memset(&fmi,0,sizeof(fmi));
		    strcpy(fmi[0].Text, GetMsg(MGUI));
		    strcpy(fmi[1].Text, GetMsg(MCUI));
                    int EC = g_Info.Menu(g_Info.ModuleNumber,
			  -1, -1, 0,
			  FMENU_WRAPMODE,
			  PanelItem, NULL,
			  "Contents", NULL, NULL, fmi, 2);
                    if (EC == -1)
                      GUI = -1;
                    else
		      GUI = 0==EC;
		  }
		  else GUI=UseGUI;

                  if (GUI == 1)
		    {
		      ShowGuiMenu(pContextMenu2, hMenu);
		    }
		  else if (GUI == 0)
		    {
		      BOOL tmp;

		      rc = ShowMenu(pidlMain, hwnd, hMenu, "" /*title */ ,
				    pContextMenu, TRUE, tmp, pContextMenu2);
		    }
		}

	      DestroyMenu(hMenu);
	    }
	}
      pMalloc->Free(pidlMain);
    }

  // Clean up and return.
  return rc;
}


UINT GetItemCount(LPITEMIDLIST pidl)
{
  USHORT nLen;
  UINT nCount = 0;

  while ((nLen = pidl->mkid.cb) != 0)
    {
      pidl = GetNextItem(pidl);
      nCount++;
    }
  return nCount;
}


LPITEMIDLIST GetNextItem(LPITEMIDLIST pidl)
{
  USHORT nLen;

  if ((nLen = pidl->mkid.cb) == 0)
    {
      return NULL;
    }
  return (LPITEMIDLIST) (((LPBYTE) pidl) + nLen);
}

LPITEMIDLIST DuplicateItem(LPMALLOC pMalloc, LPITEMIDLIST pidl)
{
  LPITEMIDLIST pidlNew;
  USHORT nLen = pidl->mkid.cb;

  if (nLen == 0)
    {
      return NULL;
    }
  pidlNew = (LPITEMIDLIST) pMalloc->Alloc(nLen + sizeof(USHORT));
  if (pidlNew == NULL)
    {
      return NULL;
    }
  CopyMemory(pidlNew, pidl, nLen);

  *((USHORT *) (((LPBYTE) pidlNew) + nLen)) = 0;

  return pidlNew;
}

void StartOLEThread()
{
  if (!g_started)
    {
      g_thread_ready = CreateEvent(NULL, FALSE, FALSE, NULL);
      g_thread_finished = CreateEvent(NULL, FALSE, FALSE, NULL);

      g_threadHandle = CreateThread(NULL, 0, thread_proc, 0, 0, &g_threadid);
      if (g_threadHandle != NULL)
	{
	  WaitForSingleObject(g_thread_ready, INFINITE);
	  g_started = TRUE;
	}
      else
	{
	  CloseHandle(g_thread_ready);
	  g_thread_ready = NULL;
	  CloseHandle(g_thread_finished);
	  g_thread_ready = NULL;
	}
    }
}

void StopOLEThread()
{
  if (g_started)
    {
      if (PostThreadMessage(g_threadid, WM_QUIT, 0, 0))
	{
	  WaitForSingleObject(g_thread_finished, INFINITE);
	  g_started = FALSE;
	}
      CloseHandle(g_thread_ready);
      g_thread_ready = NULL;
      CloseHandle(g_thread_finished);
      g_thread_ready = NULL;
      CloseHandle(g_threadHandle);
      g_threadHandle = NULL;
    }
}

unsigned long __stdcall thread_proc(void *)
{
  unsigned rc = 0;

  if (S_OK == CoInitializeEx(NULL, COINIT_MULTITHREADED))
    {
      VARIANT var, var1;

      VariantInit(&var);
      VariantInit(&var1);

      BSTR str = SysAllocString(L"Magic");

      var.vt = VT_BSTR;
      var.bstrVal = str;

      VariantChangeType(&var, &var, 0, VT_I2);

      SetEvent(g_thread_ready);

      VariantClear(&var);
      VariantClear(&var1);

      MSG msg;

      while (GetMessage(&msg, NULL, 0, 0))
	{
	  TranslateMessage(&msg);
	  DispatchMessage(&msg);
	}
      rc = msg.wParam;

      CoUninitialize();

      SetEvent(g_thread_finished);
    }
  return rc;
}
