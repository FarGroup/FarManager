#define TITLE_SIZE          64
#define PROCESS_SIZE        16

//
// task list structure
//
typedef struct _TASK_LIST {
    DWORD       dwProcessId;
    DWORD       dwProcessPriority;
    DWORD       dwInheritedFromProcessId;
    BOOL        flags;
    HWND        hwnd;
    CHAR        ProcessName[PROCESS_SIZE];
    CHAR        WindowTitle[TITLE_SIZE];
} TASK_LIST, *PTASK_LIST;

typedef struct _TASK_LIST_ENUM {
    PTASK_LIST  tlist;
    DWORD       numtasks;
} TASK_LIST_ENUM, *PTASK_LIST_ENUM;

static DWORD GetTaskList(PTASK_LIST pTask,DWORD dwNumTasks);
static VOID GetWindowTitles(PTASK_LIST_ENUM te);
static BOOL CALLBACK EnumWindowsProcNT(HWND hwnd,LPARAM lParam);

//
// manafest constants
//
#define INITIAL_SIZE        51200
#define EXTEND_SIZE         25600
#define REGKEY_PERF         "software\\microsoft\\windows nt\\currentversion\\perflib"
#define REGSUBKEY_COUNTERS  "Counters"
#define PROCESS_COUNTER     "process"
#define PROCESSID_COUNTER   "id process"
#define PROCESSPRIORITY_COUNTER   "priority base"
#define UNKNOWN_TASK        "unknown"

#define MAX_TASKS           1024

int GetListNT(PluginPanelItem **pPanelItem,int *pItemsNumber)
{
  DWORD numTasks;
  TASK_LIST_ENUM te;
  TASK_LIST *tlist;
  if ((tlist=new TASK_LIST[MAX_TASKS])==NULL)
    return(FALSE);
  memset(tlist,0,sizeof(TASK_LIST)*MAX_TASKS);
  numTasks = GetTaskList(tlist,MAX_TASKS);
  te.tlist = tlist;
  te.numtasks = numTasks;
  GetWindowTitles( &te );

  *pPanelItem=(PluginPanelItem *)realloc(*pPanelItem,numTasks*sizeof(PluginPanelItem));
  if (*pPanelItem==NULL)
  {
    delete tlist;
    return(FALSE);
  }
  *pItemsNumber=numTasks;
  for (int I=0;I<numTasks;I++)
  {
    PluginPanelItem *CurItem=*pPanelItem+I;
    memset(CurItem,0,sizeof(PluginPanelItem));
    CurItem->Flags|=PPIF_USERDATA;
    strcpy(CurItem->FindData.cFileName,tlist[I].ProcessName);
    CurItem->FindData.nFileSizeLow=tlist[I].dwProcessPriority;
    CurItem->PackSize=tlist[I].dwProcessId;
    CurItem->Description=new char[strlen(tlist[I].WindowTitle)+1];
    strcpy(CurItem->Description,tlist[I].WindowTitle);
    CharToOem(CurItem->Description,CurItem->Description);
    CurItem->UserData=(DWORD)(new ProcessData);
    ProcessData pdata;
    pdata.Size=sizeof(pdata);
    pdata.hwnd=tlist[I].hwnd;
    memcpy((void *)CurItem->UserData,&pdata,sizeof(pdata));
  }
  delete tlist;
  return(TRUE);
}


DWORD GetTaskList(PTASK_LIST pTask,DWORD dwNumTasks)
/*++

Routine Description:

    Provides an API for getting a list of tasks running at the time of the
    API call.  This function uses the registry performance data to get the
    task list and is therefor straight WIN32 calls that anyone can call.

Arguments:

    dwNumTasks       - maximum number of tasks that the pTask array can hold

Return Value:

    Number of tasks placed into the pTask array.

--*/

{
    DWORD                        rc;
    HKEY                         hKeyNames=NULL;
    DWORD                        dwType;
    DWORD                        dwSize;
    LPBYTE                       buf = NULL;
    static CHAR                  szSubKey[1024];
    LANGID                       lid;
    LPSTR                        p;
    LPSTR                        p2;
    PPERF_DATA_BLOCK             pPerf;
    PPERF_OBJECT_TYPE            pObj;
    PPERF_INSTANCE_DEFINITION    pInst;
    PPERF_COUNTER_BLOCK          pCounter;
    PPERF_COUNTER_DEFINITION     pCounterDef;
    DWORD                        i;
    static DWORD                 dwProcessIdTitle;
    static DWORD                 dwPriorityTitle;
    DWORD                        dwProcessIdCounter;
    DWORD                        dwPriorityCounter;
    CHAR                         szProcessName[MAX_PATH];
    DWORD                        dwLimit = dwNumTasks - 1;

    int Found;


    if (*szSubKey==0)
    {

      //
      // Look for the list of counters.  Always use the neutral
      // English version, regardless of the local language.  We
      // are looking for some particular keys, and we are always
      // going to do our looking in English.  We are not going
      // to show the user the counter names, so there is no need
      // to go find the corresponding name in the local language.
      //
      lid = MAKELANGID( LANG_ENGLISH, SUBLANG_NEUTRAL );
      sprintf( szSubKey, "%s\\%03x", REGKEY_PERF, lid );
      rc = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                         szSubKey,
                         0,
                         KEY_READ,
                         &hKeyNames
                       );
      if (rc != ERROR_SUCCESS) {
          goto exit;
      }

      //
      // get the buffer size for the counter names
      //
      rc = RegQueryValueEx( hKeyNames,
                            REGSUBKEY_COUNTERS,
                            NULL,
                            &dwType,
                            NULL,
                            &dwSize
                          );

      if (rc != ERROR_SUCCESS) {
          goto exit;
      }

      //
      // allocate the counter names buffer
      //
      buf = (LPBYTE) malloc( dwSize );
      if (buf == NULL) {
          goto exit;
      }
      memset( buf, 0, dwSize );

      //
      // read the counter names from the registry
      //
      rc = RegQueryValueEx( hKeyNames,
                            REGSUBKEY_COUNTERS,
                            NULL,
                            &dwType,
                            buf,
                            &dwSize
                          );

      if (rc != ERROR_SUCCESS) {
          goto exit;
      }

      //
      // now loop thru the counter names looking for the following counters:
      //
      //      1.  "Process"           process name
      //      2.  "ID Process"        process id
      //
      // the buffer contains multiple null terminated strings and then
      // finally null terminated at the end.  the strings are in pairs of
      // counter number and counter name.
      //

      p = buf;
      while (*p)
      {
        if (p > buf)
        {
          for( p2=p-2; isdigit(*p2); p2--) ;
        }
        if (stricmp(p, PROCESS_COUNTER) == 0)
        {
          //
          // look backwards for the counter number
          //
          for( p2=p-2; isdigit(*p2); p2--) ;
          strcpy( szSubKey, p2+1 );
        }
        else
          if (stricmp(p, PROCESSID_COUNTER) == 0)
          {
            //
            // look backwards for the counter number
            //
            for( p2=p-2; isdigit(*p2); p2--) ;
            dwProcessIdTitle = atol( p2+1 );
          }
          else
            if (stricmp(p, PROCESSPRIORITY_COUNTER) == 0)
            {
              for( p2=p-2; isdigit(*p2); p2--) ;
              dwPriorityTitle = atol( p2+1 );
            }
        //
        // next string
        //
        p += (strlen(p) + 1);
      }

      //
      // free the counter names buffer
      //
      free( buf );
    }

    //
    // allocate the initial buffer for the performance data
    //
    dwSize = INITIAL_SIZE;
    buf = (LPBYTE) malloc( dwSize );
    if (buf == NULL) {
        goto exit;
    }
    memset( buf, 0, dwSize );


    while (TRUE) {

        rc = RegQueryValueEx( HKEY_PERFORMANCE_DATA,
                              szSubKey,
                              NULL,
                              &dwType,
                              buf,
                              &dwSize
                            );

        pPerf = (PPERF_DATA_BLOCK) buf;

        //
        // check for success and valid perf data block signature
        //
        if ((rc == ERROR_SUCCESS) &&
            (dwSize > 0) &&
            (pPerf)->Signature[0] == (WCHAR)'P' &&
            (pPerf)->Signature[1] == (WCHAR)'E' &&
            (pPerf)->Signature[2] == (WCHAR)'R' &&
            (pPerf)->Signature[3] == (WCHAR)'F' ) {
            break;
        }

        //
        // if buffer is not big enough, reallocate and try again
        //
        if (rc == ERROR_MORE_DATA) {
            dwSize += EXTEND_SIZE;
            buf = (LPBYTE) realloc( buf, dwSize );
            memset( buf, 0, dwSize );
        }
        else {
            goto exit;
        }
    }

    //
    // set the perf_object_type pointer
    //
    pObj = (PPERF_OBJECT_TYPE) ((DWORD)pPerf + pPerf->HeaderLength);

    //
    // loop thru the performance counter definition records looking
    // for the process id counter and then save its offset
    //
    pCounterDef = (PPERF_COUNTER_DEFINITION) ((DWORD)pObj + pObj->HeaderLength);
    Found=0;
    for (i=0; i<(DWORD)pObj->NumCounters; i++)
    {
      if (pCounterDef->CounterNameTitleIndex == dwProcessIdTitle)
      {
        dwProcessIdCounter = pCounterDef->CounterOffset;
        if (++Found==2)
          break;
      }
      else
        if (pCounterDef->CounterNameTitleIndex == dwPriorityTitle)
        {
          dwPriorityCounter = pCounterDef->CounterOffset;
          if (++Found==2)
            break;
        }
      pCounterDef++;
    }

    dwNumTasks = min( dwLimit, (DWORD)pObj->NumInstances );

    pInst = (PPERF_INSTANCE_DEFINITION) ((DWORD)pObj + pObj->DefinitionLength);

    //
    // loop thru the performance instance data extracting each process name
    // and process id
    //
    for (i=0; i<dwNumTasks; i++) {
        //
        // pointer to the process name
        //
        p = (LPSTR) ((DWORD)pInst + pInst->NameOffset);

        //
        // convert it to ascii
        //
        rc = WideCharToMultiByte( CP_ACP,
                                  0,
                                  (LPCWSTR)p,
                                  -1,
                                  szProcessName,
                                  sizeof(szProcessName),
                                  NULL,
                                  NULL
                                );

        if (!rc) {
            //
	    // if we cant convert the string then use a default value
            //
            strcpy( pTask->ProcessName, UNKNOWN_TASK );
        }

        if (strlen(szProcessName)+4 <= sizeof(pTask->ProcessName)) {
            strcpy( pTask->ProcessName, szProcessName );
            strcat( pTask->ProcessName, ".exe" );
        }

        //
        // get the process id
        //
        pCounter = (PPERF_COUNTER_BLOCK) ((DWORD)pInst + pInst->ByteLength);
        pTask->flags = 0;
        pTask->dwProcessId = *((LPDWORD) ((DWORD)pCounter + dwProcessIdCounter));
//        if (pTask->dwProcessId == 0)
//          pTask->dwProcessId = (DWORD)-2;
        pTask->dwProcessPriority = *((LPDWORD) ((DWORD)pCounter + dwPriorityCounter));

        //
        // next process
        //
        pTask++;
        pInst = (PPERF_INSTANCE_DEFINITION) ((DWORD)pCounter + pCounter->ByteLength);
    }

exit:
    if (buf) {
        free( buf );
    }

    if (hKeyNames)
      RegCloseKey( hKeyNames );

    return dwNumTasks;
}


BOOL KillProcessNT(DWORD pid,HWND hwnd)
{
  HANDLE hProcess;
  hProcess = OpenProcess( PROCESS_TERMINATE, FALSE, pid );
  if (hProcess)
  {
    if (hProcess == NULL)
      return FALSE;

    if (!TerminateProcess( hProcess, 1 ))
    {
      CloseHandle( hProcess );
      return FALSE;
    }

    CloseHandle( hProcess );
    return TRUE;
  }
  return FALSE;
}


VOID GetWindowTitles(PTASK_LIST_ENUM te)
{
    //
    // enumerate all windows
    //
    EnumWindows( EnumWindowsProcNT, (LPARAM) te );
}



BOOL CALLBACK EnumWindowsProcNT(HWND hwnd,LPARAM lParam)

/*++

Routine Description:

    Callback function for window enumeration.

Arguments:

    hwnd             - window handle
    lParam           - ** not used **

Return Value:

    TRUE  - continues the enumeration

--*/

{
    DWORD             pid = 0;
    DWORD             i;
    CHAR              buf[TITLE_SIZE];
    PTASK_LIST_ENUM   te = (PTASK_LIST_ENUM)lParam;
    PTASK_LIST        tlist = te->tlist;
    DWORD             numTasks = te->numtasks;


    //
    // get the processid for this window
    //
    if (GetParent(hwnd)!=NULL || !IsWindowVisible(hwnd) ||
        !GetWindowThreadProcessId( hwnd, &pid ))
        return TRUE;

    //
    // look for the task in the task list for this window
    //
    for (i=0; i<numTasks; i++) {
        if (tlist[i].dwProcessId == pid) {
            tlist[i].hwnd = hwnd;
            //
	    // we found the task so lets try to get the
            // window text
            //
            if (GetWindowText( tlist[i].hwnd, buf, sizeof(buf) )) {
                //
		// got it, so lets save it
                //
                strcpy( tlist[i].WindowTitle, buf );
            }
            break;
        }
    }

    //
    // continue the enumeration
    //
    return TRUE;
}
