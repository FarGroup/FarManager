#include <CRT/crt.hpp>
#include <objbase.h>
#include <wbemidl.h>
#include "Proclist.hpp"
#include "perfthread.hpp"

class BStr
{
	private:
		BSTR bstr;

	public:
		BStr(LPCWSTR str) { bstr = str ? SysAllocString(str) : 0; }
		~BStr() { if (bstr) SysFreeString(bstr); }
		operator BSTR() { return bstr; }
};

class ProcessPath
{
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

WMIConnection::WMIConnection(): pIWbemServices(NULL), hrLast(0)
{
	token.Enable();
}

WMIConnection::~WMIConnection()
{
	Disconnect();
}

void WMIConnection::GetProcessExecutablePath(DWORD dwPID, wchar_t* pPath)
{
	hrLast = WBEM_S_NO_ERROR;
	IWbemClassObject* pIWbemClassObject=NULL;
	hrLast = pIWbemServices->GetObject(ProcessPath(dwPID), 0,0, &pIWbemClassObject, 0);

	if (!pIWbemClassObject) return;

	*pPath = 0;
	VARIANT pVal;
	hrLast = pIWbemClassObject->Get(L"ExecutablePath", 0, &pVal, 0, 0);

	if (hrLast==WBEM_S_NO_ERROR && pVal.vt!=VT_NULL)
	{
		lstrcpynW(pPath, pVal.bstrVal, MAX_PATH);
		VariantClear(&pVal);
	}

	pIWbemClassObject->Release();
}

DWORD WMIConnection::GetProcessPriority(DWORD dwPID)
{
	hrLast = WBEM_S_NO_ERROR;
	IWbemClassObject* pIWbemClassObject=NULL;
	hrLast = pIWbemServices->GetObject(ProcessPath(dwPID), 0,0, &pIWbemClassObject, 0);

	if (hrLast != WBEM_S_NO_ERROR || !pIWbemClassObject) return 0;

	DWORD rc = 0;
	VARIANT pVal;
	hrLast = pIWbemClassObject->Get(L"Priority", 0, &pVal, 0, 0);

	if (hrLast==WBEM_S_NO_ERROR)
	{
		if (pVal.vt==VT_I4)
			rc = pVal.lVal;

		VariantClear(&pVal);
	}

	pIWbemClassObject->Release();
	return rc;
}

void WMIConnection::GetProcessOwner(DWORD dwPID, wchar_t* pUser, wchar_t* pDomain)
{
	hrLast = WBEM_S_NO_ERROR;
	IWbemClassObject* pOutParams=0;
	*pUser = 0;

	if (pDomain) *pDomain = 0;

	if ((hrLast=pIWbemServices->ExecMethod(ProcessPath(dwPID), BStr(L"GetOwner"), 0, 0, 0, &pOutParams, 0))!=WBEM_S_NO_ERROR
	        || !pOutParams)
		return;

	VARIANT pVal;

	if ((hrLast=pOutParams->Get(L"User", 0, &pVal, 0, 0))==WBEM_S_NO_ERROR && pVal.vt!=VT_NULL)
	{
		lstrcpynW(pUser, pVal.bstrVal, MAX_PATH);
		VariantClear(&pVal);
	}

	if (pDomain && (hrLast=pOutParams->Get(L"Domain", 0, &pVal, 0, 0))==WBEM_S_NO_ERROR && pVal.vt!=VT_NULL)
	{
		lstrcpynW(pDomain, pVal.bstrVal, MAX_PATH);
		VariantClear(&pVal);
	}

	pOutParams->Release();
}

void WMIConnection::GetProcessUserSid(DWORD dwPID, wchar_t* pUserSid)
{
	hrLast = WBEM_S_NO_ERROR;
	IWbemClassObject* pOutParams=0;
	*pUserSid = 0;

	if ((hrLast=pIWbemServices->ExecMethod(ProcessPath(dwPID), BStr(L"GetOwnerSid"), 0, 0, 0, &pOutParams, 0))!=WBEM_S_NO_ERROR
	        || !pOutParams)
		return;

	VARIANT pVal;

	if ((hrLast=pOutParams->Get(L"Sid", 0, &pVal, 0, 0))==WBEM_S_NO_ERROR
	        && pVal.vt!=VT_NULL)
	{
		lstrcpynW(pUserSid, pVal.bstrVal, MAX_PATH);
		VariantClear(&pVal);
	}

	pOutParams->Release();
}

int WMIConnection::GetProcessSessionId(DWORD dwPID)
{
	hrLast = WBEM_S_NO_ERROR;
	IWbemClassObject* pIWbemClassObject=NULL;
	hrLast = pIWbemServices->GetObject(ProcessPath(dwPID), 0, 0, &pIWbemClassObject, 0);

	if (!pIWbemClassObject) return -1;

	int rc = -1;
	VARIANT pVal;

	if ((hrLast=pIWbemClassObject->Get(L"SessionId", 0, &pVal, 0, 0))==WBEM_S_NO_ERROR)
	{
		if (pVal.vt==VT_I4)
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

	if (!pIWbemClassObject) return -1;

	IWbemClassObject* pInSignature=0;

	if (wsParamName)
		hrLast = pIWbemClassObject->GetMethod(wsMethod, 0, &pInSignature, 0);

	int rc = -1;

	if (pInSignature || !wsParamName)
	{
		IWbemClassObject* pInParams=0;

		if (pInSignature)
			hrLast = pInSignature->SpawnInstance(0, &pInParams);

		if (pInParams || !wsParamName)
		{
			if (pInParams)
			{
				VARIANT var; VariantClear(&var);
				var.vt = VT_I4;
				var.lVal = dwParam;
				hrLast = pInParams->Put(wsParamName, 0, &var, 0);
			}

			IWbemClassObject* pOutParams=0;

			if ((hrLast=pIWbemServices->ExecMethod(ProcessPath(dwPID), BStr(wsMethod), 0, 0, pInParams, &pOutParams, 0))==WBEM_S_NO_ERROR)
			{
				rc = 0;

				if (pOutParams)
				{
					VARIANT pVal;

					if (pOutParams->Get(L"ReturnValue", 0, &pVal, 0, 0)==WBEM_S_NO_ERROR)
					{
						if (pVal.vt==VT_I4)
							rc = pVal.lVal;

						VariantClear(&pVal);
					}

					pOutParams->Release();
				}
			}

			if (pInParams)
				pInParams->Release();
		}

		if (pInSignature)
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

bool WMIConnection::Connect(LPCTSTR pMachineName, LPCTSTR pUser, LPCTSTR pPassword)
{
	if (pIWbemServices)
		return true;

	if (pUser && !*pUser)
		pUser = pPassword = 0; // Empty username means default security

	hrLast = WBEM_S_NO_ERROR;
	IWbemLocator *pIWbemLocator = NULL;

	if ((hrLast=CoCreateInstance(CLSID_WbemLocator,
	                             NULL, CLSCTX_INPROC_SERVER, IID_IWbemLocator,
	                             (LPVOID *) &pIWbemLocator)) == S_OK)
	{
		if (!pMachineName || !*pMachineName)
			pMachineName = _T(".");

		if (NORM_M_PREFIX(pMachineName))
			pMachineName += 2;
		wchar_t Namespace[128];
		wsprintfW(Namespace, L"\\\\%s\\root\\cimv2", pMachineName);

		if ((hrLast=pIWbemLocator->ConnectServer(Namespace, BStr(pUser), BStr(pPassword),0,0,0,0,
		            &pIWbemServices)) == S_OK)
		{
			// We impersonate a token to enable SeDebugPrivilege privilege.
			// Enable static cloacking here to make sure the server sees it.
			//
			// Some privileged information (like the full EXE path) will not be
			// returned if this call fails. However it is not fatal so the error
			// can be ignored.
			pCoSetProxyBlanket(pIWbemServices,
			                   RPC_C_AUTHN_DEFAULT,
			                   RPC_C_AUTHN_DEFAULT,
			                   NULL,
			                   RPC_C_AUTHN_LEVEL_DEFAULT,
			                   RPC_C_IMP_LEVEL_IMPERSONATE,
			                   NULL,
			                   EOAC_STATIC_CLOAKING);
		}
		else
		{
			pIWbemServices = 0;
		}

		pIWbemLocator->Release();
	}

	return pIWbemServices!=0;
}

void WMIConnection::Disconnect()
{
	if (pIWbemServices)
	{
		pIWbemServices->Release();
		pIWbemServices = 0;
	}
}
