#include "proclist.hpp"
#include <objbase.h>
#include <wbemidl.h>
#include "perfthread.hpp"

class BStr {
    BSTR bstr;
  public:
      BStr(LPCWSTR str) { bstr = str ? SysAllocString(str) : 0; }
      BStr(LPCSTR str);
      ~BStr() { if(bstr) SysFreeString(bstr); }
      operator BSTR() { return bstr; }
};
BStr::BStr(LPCSTR str)
{
    if(!str) {
        bstr = 0;
        return;
    }
    int nConvertedLen = lstrlen(str);
    bstr = ::SysAllocStringLen(NULL, nConvertedLen);
    MultiByteToWideChar(CP_OEMCP, 0, str, -1, bstr, nConvertedLen);
}

class ProcessPath {
    BSTR PathStr;
  public:
      ProcessPath(DWORD dwPID);
      ~ProcessPath() { SysFreeString(PathStr); }
      operator BSTR() { return PathStr; }
};

ProcessPath::ProcessPath(DWORD dwPID)
{
    wchar_t path[36];
    wsprintfW(path, L"Win32_Process.Handle=%d", dwPID);

    PathStr = SysAllocString(path);
}

void WMIConnection::GetProcessExecutablePath(DWORD dwPID, char* pPath)
{
    hrLast = WBEM_S_NO_ERROR;
    IWbemClassObject* pIWbemClassObject=NULL;

    hrLast = pIWbemServices->GetObject(ProcessPath(dwPID), 0,0, &pIWbemClassObject, 0);

    if(!pIWbemClassObject) return;

    *pPath = 0;
    VARIANT pVal;
    hrLast = pIWbemClassObject->Get(L"ExecutablePath", 0, &pVal, 0, 0);
    if(hrLast==WBEM_S_NO_ERROR && pVal.vt!=VT_NULL)
    {
        WideCharToMultiByte( CP_OEMCP, 0, pVal.bstrVal, -1,
            pPath, MAX_PATH, NULL, NULL);
        VariantClear(&pVal);
    }
    pIWbemClassObject->Release();
}

DWORD WMIConnection::GetProcessPriority(DWORD dwPID)
{
    hrLast = WBEM_S_NO_ERROR;
    IWbemClassObject* pIWbemClassObject=NULL;

    hrLast = pIWbemServices->GetObject(ProcessPath(dwPID), 0,0, &pIWbemClassObject, 0);

    if(hrLast != WBEM_S_NO_ERROR || !pIWbemClassObject) return 0;

    DWORD rc = 0;

    VARIANT pVal;
    hrLast = pIWbemClassObject->Get(L"Priority", 0, &pVal, 0, 0);
    if(hrLast==WBEM_S_NO_ERROR) {
        if(pVal.vt==VT_I4)
            rc = pVal.lVal;
        VariantClear(&pVal);
    }
    pIWbemClassObject->Release();
    return rc;
}

void WMIConnection::GetProcessOwner(DWORD dwPID, char* pUser, char* pDomain)
{
    hrLast = WBEM_S_NO_ERROR;
    IWbemClassObject* pOutParams=0;
    *pUser = 0;
    if(pDomain) *pDomain = 0;
    if((hrLast=pIWbemServices->ExecMethod(ProcessPath(dwPID), BStr(L"GetOwner"), 0, 0, 0, &pOutParams, 0))!=WBEM_S_NO_ERROR
        || !pOutParams )
        return;

    VARIANT pVal;
    if((hrLast=pOutParams->Get(L"User", 0, &pVal, 0, 0))==WBEM_S_NO_ERROR && pVal.vt!=VT_NULL)
    {
        WideCharToMultiByte( CP_OEMCP, 0, pVal.bstrVal, -1,
            pUser, MAX_PATH, NULL, NULL);
        VariantClear(&pVal);
    }
    if(pDomain && (hrLast=pOutParams->Get(L"Domain", 0, &pVal, 0, 0))==WBEM_S_NO_ERROR && pVal.vt!=VT_NULL)
    {
        WideCharToMultiByte( CP_OEMCP, 0, pVal.bstrVal, -1,
            pDomain, MAX_PATH, NULL, NULL);
        VariantClear(&pVal);
    }
    pOutParams->Release();
}

void WMIConnection::GetProcessUserSid(DWORD dwPID, char* pUserSid)
{
    hrLast = WBEM_S_NO_ERROR;
    IWbemClassObject* pOutParams=0;
    *pUserSid = 0;
    if((hrLast=pIWbemServices->ExecMethod(ProcessPath(dwPID), BStr(L"GetOwnerSid"), 0, 0, 0, &pOutParams, 0))!=WBEM_S_NO_ERROR
        || !pOutParams )
        return;

    VARIANT pVal;
    if((hrLast=pOutParams->Get(L"Sid", 0, &pVal, 0, 0))==WBEM_S_NO_ERROR
        && pVal.vt!=VT_NULL)
    {
        WideCharToMultiByte( CP_OEMCP, 0, pVal.bstrVal, -1,
            pUserSid, MAX_PATH, NULL, NULL);
        VariantClear(&pVal);
    }
    pOutParams->Release();
}

int WMIConnection::GetProcessSessionId(DWORD dwPID)
{
    hrLast = WBEM_S_NO_ERROR;
    IWbemClassObject* pIWbemClassObject=NULL;

    hrLast = pIWbemServices->GetObject(ProcessPath(dwPID), 0, 0, &pIWbemClassObject, 0);
    if(!pIWbemClassObject) return -1;

    int rc = -1;
    VARIANT pVal;
    if((hrLast=pIWbemClassObject->Get(L"SessionId", 0, &pVal, 0, 0))==WBEM_S_NO_ERROR)
    {
        if(pVal.vt==VT_I4)
            rc = pVal.lVal;
        VariantClear(&pVal);
    }
    pIWbemClassObject->Release();
    return rc;
}

int WMIConnection::ExecMethod(DWORD dwPID, LPCWSTR wsMethod, LPCWSTR wsParamName, DWORD dwParam)
{
    hrLast = WBEM_S_NO_ERROR;
    IWbemClassObject* pIWbemClassObject=NULL;

    hrLast = pIWbemServices->GetObject(BStr(L"Win32_Process"), WBEM_FLAG_DIRECT_READ,0, &pIWbemClassObject, 0);

    if(!pIWbemClassObject) return -1;

    IWbemClassObject* pInSignature=0;
    if(wsParamName)
        hrLast = pIWbemClassObject->GetMethod(wsMethod, 0, &pInSignature, 0);
    int rc = -1;
    if(pInSignature || !wsParamName) {
        IWbemClassObject* pInParams=0;
        if(pInSignature)
            hrLast = pInSignature->SpawnInstance(0, &pInParams);
        if(pInParams || !wsParamName) {
            if(pInParams) {
                VARIANT var; VariantClear(&var);
                var.vt = VT_I4;
                var.lVal = dwParam;
                hrLast = pInParams->Put(wsParamName, 0, &var, 0);
            }
            IWbemClassObject* pOutParams=0;
            if((hrLast=pIWbemServices->ExecMethod(ProcessPath(dwPID), BStr(wsMethod), 0, 0, pInParams, &pOutParams, 0))==WBEM_S_NO_ERROR) {
                rc = 0;
                if(pOutParams) {
                    VARIANT pVal;
                    if(pOutParams->Get(L"ReturnValue", 0, &pVal, 0, 0)==WBEM_S_NO_ERROR)
                    {
                        if(pVal.vt==VT_I4)
                            rc = pVal.lVal;
                        VariantClear(&pVal);
                    }
                    pOutParams->Release();
                }
            }
            if(pInParams)
                pInParams->Release();
        }
        if(pInSignature)
            pInSignature->Release();
    }
    pIWbemClassObject->Release();
    return rc;
}

int WMIConnection::SetProcessPriority(DWORD dwPID, DWORD dwPri)
{
    return ExecMethod(dwPID, L"SetPriority", L"Priority", dwPri);
}

int WMIConnection::TerminateProcess(DWORD dwPID)
{
    return ExecMethod(dwPID, L"Terminate", L"Reason", 0xffffffff);
}

bool WMIConnection::Connect(LPCSTR pMachineName, LPCSTR pUser, LPCSTR pPassword)
{
    if (pIWbemServices)
        return true;

    if(pUser && !*pUser)
        pUser = pPassword = 0; // Empty username means default security

    hrLast = WBEM_S_NO_ERROR;
    CoInitialize(0);

    // It must called per thread, otherwise returns
    CoInitializeSecurity( 0, -1, 0, 0, RPC_C_AUTHN_LEVEL_DEFAULT,
                RPC_C_IMP_LEVEL_IMPERSONATE, 0, EOAC_NONE, 0);

    if(!pMachineName || !*pMachineName)
        pMachineName = ".";

    if(*(short*)pMachineName==0x5c5c || *(short*)pMachineName==0x2f2f)
        pMachineName+=2;

    IWbemLocator *pIWbemLocator = NULL;

    if((hrLast=CoCreateInstance(CLSID_WbemLocator,
        NULL, CLSCTX_INPROC_SERVER, IID_IWbemLocator,
        (LPVOID *) &pIWbemLocator)) == S_OK)
    {
        wchar_t Namespace[128];
        wsprintfW(Namespace, L"\\\\%S\\root\\cimv2", pMachineName);

        if((hrLast=pIWbemLocator->ConnectServer(Namespace, BStr(pUser), BStr(pPassword),0,0,0,0,
                        &pIWbemServices)) != S_OK)
                pIWbemServices = 0;

        pIWbemLocator->Release();
    }
    return pIWbemServices!=0;
}

void WMIConnection::Disconnect()
{
    if(pIWbemServices) {
        pIWbemServices->Release();
        pIWbemServices = 0;
    }
}
