#include <memory>

#define WIN32_NO_STATUS
#include <windows.h>
#undef WIN32_NO_STATUS

#include <wbemidl.h>
#include "Proclist.hpp"
#include "perfthread.hpp"

class bstr
{
public:
	explicit bstr(const wchar_t* const Str):
		m_Str(SysAllocString(Str))
	{
	}

	bstr(const bstr&) = delete;
	bstr& operator=(const bstr&) = delete;

	~bstr()
	{
		SysFreeString(m_Str);
	}

	operator BSTR() const
	{
		return m_Str;
	}

private:
	BSTR m_Str;
};

static auto ProcessPath(DWORD const Pid)
{
	wchar_t Str[64];
	wsprintfW(Str, L"Win32_Process.Handle=%u", Pid);
	return bstr(Str);
}

WMIConnection::WMIConnection()
{
	token.Enable();
}

WMIConnection::~WMIConnection()
{
	Disconnect();
}

std::wstring WMIConnection::GetProcessExecutablePath(DWORD dwPID)
{
	hrLast = WBEM_S_NO_ERROR;
	IWbemClassObject* Object = {};
	hrLast = pIWbemServices->GetObject(ProcessPath(dwPID), 0, {}, &Object, {});

	if (!Object)
		return {};

	VARIANT pVal;
	hrLast = Object->Get(L"ExecutablePath", 0, &pVal, {}, {});

	std::wstring Result;
	if (hrLast==WBEM_S_NO_ERROR && pVal.vt!=VT_NULL)
	{
		Result = pVal.bstrVal;
		VariantClear(&pVal);
	}

	Object->Release();
	return Result;
}

DWORD WMIConnection::GetProcessPriority(DWORD dwPID)
{
	hrLast = WBEM_S_NO_ERROR;
	IWbemClassObject* Object = {};
	hrLast = pIWbemServices->GetObject(ProcessPath(dwPID), 0, {}, &Object, {});

	if (hrLast != WBEM_S_NO_ERROR || !Object)
		return 0;

	DWORD rc = 0;
	VARIANT pVal;
	hrLast = Object->Get(L"Priority", 0, &pVal, {}, {});

	if (hrLast==WBEM_S_NO_ERROR)
	{
		if (pVal.vt==VT_I4)
			rc = pVal.lVal;

		VariantClear(&pVal);
	}

	Object->Release();
	return rc;
}

std::wstring WMIConnection::GetProcessOwner(DWORD dwPID, std::wstring* Domain)
{
	hrLast = WBEM_S_NO_ERROR;
	IWbemClassObject* pOutParams = {};

	if (Domain)
		Domain->clear();

	if ((hrLast=pIWbemServices->ExecMethod(ProcessPath(dwPID), bstr(L"GetOwner"), 0, {}, {}, &pOutParams, {}))!=WBEM_S_NO_ERROR || !pOutParams)
		return {};

	VARIANT pVal;

	std::wstring User;
	if ((hrLast=pOutParams->Get(L"User", 0, &pVal, {}, {}))==WBEM_S_NO_ERROR && pVal.vt!=VT_NULL)
	{
		User = pVal.bstrVal;
		VariantClear(&pVal);
	}

	if (Domain && (hrLast=pOutParams->Get(L"Domain", 0, &pVal, {}, {}))==WBEM_S_NO_ERROR && pVal.vt!=VT_NULL)
	{
		*Domain = pVal.bstrVal;
		VariantClear(&pVal);
	}

	pOutParams->Release();
	return User;
}

std::wstring WMIConnection::GetProcessUserSid(DWORD dwPID)
{
	hrLast = WBEM_S_NO_ERROR;
	IWbemClassObject* pOutParams = {};

	if ((hrLast = pIWbemServices->ExecMethod(ProcessPath(dwPID), bstr(L"GetOwnerSid"), 0, {}, {}, &pOutParams, {})) != WBEM_S_NO_ERROR || !pOutParams)
		return {};

	VARIANT pVal;
	std::wstring UserSid;

	if ((hrLast=pOutParams->Get(L"Sid", 0, &pVal, {}, {}))==WBEM_S_NO_ERROR && pVal.vt!=VT_NULL)
	{
		UserSid = pVal.bstrVal;
		VariantClear(&pVal);
	}

	pOutParams->Release();
	return UserSid;
}

int WMIConnection::GetProcessSessionId(DWORD dwPID)
{
	hrLast = WBEM_S_NO_ERROR;
	IWbemClassObject* Object = {};
	hrLast = pIWbemServices->GetObject(ProcessPath(dwPID), 0, {}, &Object, {});

	if (!Object)
		return -1;

	int rc = -1;
	VARIANT pVal;

	if ((hrLast = Object->Get(L"SessionId", 0, &pVal, {}, {})) == WBEM_S_NO_ERROR)
	{
		if (pVal.vt==VT_I4)
			rc = pVal.lVal;

		VariantClear(&pVal);
	}

	Object->Release();
	return rc;
}

int WMIConnection::ExecMethod(DWORD dwPID, const wchar_t* wsMethod, const wchar_t* wsParamName, DWORD dwParam)
{
	hrLast = WBEM_S_NO_ERROR;
	IWbemClassObject* Object = {};
	hrLast = pIWbemServices->GetObject(bstr(L"Win32_Process"), WBEM_FLAG_DIRECT_READ, {}, &Object, {});

	if (!Object)
		return -1;

	IWbemClassObject* pInSignature = {};

	if (wsParamName)
		hrLast = Object->GetMethod(wsMethod, 0, &pInSignature, {});

	int rc = -1;

	if (pInSignature || !wsParamName)
	{
		IWbemClassObject* pInParams = {};

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

			IWbemClassObject* pOutParams = {};

			if ((hrLast=pIWbemServices->ExecMethod(ProcessPath(dwPID), bstr(wsMethod), 0, {}, pInParams, &pOutParams, {}))==WBEM_S_NO_ERROR)
			{
				rc = 0;

				if (pOutParams)
				{
					VARIANT pVal;

					if (pOutParams->Get(L"ReturnValue", 0, &pVal, {}, {}) == WBEM_S_NO_ERROR)
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

	Object->Release();
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

bool WMIConnection::Connect(const wchar_t* pMachineName, const wchar_t* pUser, const wchar_t* pPassword)
{
	if (pIWbemServices)
		return true;

	if (pUser && !*pUser)
		pUser = pPassword = {}; // Empty username means default security

	hrLast = WBEM_S_NO_ERROR;
	IWbemLocator* pIWbemLocator = {};

	if ((hrLast = CoCreateInstance(CLSID_WbemLocator, {}, CLSCTX_INPROC_SERVER, IID_IWbemLocator, IID_PPV_ARGS_Helper(&pIWbemLocator))) == S_OK)
	{
		if (!pMachineName || !*pMachineName)
			pMachineName = L".";

		if (norm_m_prefix(pMachineName))
			pMachineName += 2;
		wchar_t Namespace[128];
		wsprintfW(Namespace, L"\\\\%s\\root\\cimv2", pMachineName);

		if ((hrLast=pIWbemLocator->ConnectServer(bstr(Namespace), bstr(pUser), bstr(pPassword), {},0, {}, {}, &pIWbemServices)) == S_OK)
		{
			// We impersonate a token to enable SeDebugPrivilege privilege.
			// Enable static cloaking here to make sure the server sees it.
			//
			// Some privileged information (like the full EXE path) will not be
			// returned if this call fails. However it is not fatal so the error
			// can be ignored.
			pCoSetProxyBlanket(pIWbemServices,
			                   RPC_C_AUTHN_DEFAULT,
			                   RPC_C_AUTHN_DEFAULT,
			                   {},
			                   RPC_C_AUTHN_LEVEL_DEFAULT,
			                   RPC_C_IMP_LEVEL_IMPERSONATE,
			                   {},
			                   EOAC_STATIC_CLOAKING);
		}
		else
		{
			pIWbemServices = {};
		}

		pIWbemLocator->Release();
	}

	return pIWbemServices != nullptr;
}

void WMIConnection::Disconnect()
{
	if (pIWbemServices)
	{
		pIWbemServices->Release();
		pIWbemServices = {};
	}
}
