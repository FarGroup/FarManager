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
	return bstr(format(FSTR(L"Win32_Process.Handle={0}"), Pid).c_str());
}

struct com_closer
{
	void operator()(IUnknown* Object) const
	{
		Object->Release();
	}
};

template<typename T>
using com_ptr = std::unique_ptr<T, com_closer>;

WMIConnection::WMIConnection()
{
	token.Enable();
}

WMIConnection::~WMIConnection()
{
	Disconnect();
}

std::wstring WMIConnection::GetProcessExecutablePath(DWORD const Pid)
{
	return GetProcessString(Pid, L"ExecutablePath");
}

std::wstring WMIConnection::GetProcessCommandLine(DWORD const Pid)
{
	return GetProcessString(Pid, L"CommandLine");
}

DWORD WMIConnection::GetProcessPriority(DWORD const Pid)
{
	return GetProcessInt(Pid, L"Priority");
}

std::wstring WMIConnection::GetProcessOwner(DWORD dwPID, std::wstring* Domain)
{
	com_ptr<IWbemClassObject> OutParams;

	if (Domain)
		Domain->clear();

	hrLast = pIWbemServices->ExecMethod(ProcessPath(dwPID), bstr(L"GetOwner"), 0, {}, {}, &ptr_setter(OutParams), {});
	if (hrLast != WBEM_S_NO_ERROR || !OutParams)
		return {};

	VARIANT pVal;

	std::wstring User;
	if ((hrLast = OutParams->Get(L"User", 0, &pVal, {}, {})) == WBEM_S_NO_ERROR && pVal.vt != VT_NULL)
	{
		User = pVal.bstrVal;
		VariantClear(&pVal);
	}

	if (Domain && (hrLast = OutParams->Get(L"Domain", 0, &pVal, {}, {})) == WBEM_S_NO_ERROR && pVal.vt != VT_NULL)
	{
		*Domain = pVal.bstrVal;
		VariantClear(&pVal);
	}

	return User;
}

std::wstring WMIConnection::GetProcessUserSid(DWORD dwPID)
{
	com_ptr<IWbemClassObject> OutParams;
	hrLast = pIWbemServices->ExecMethod(ProcessPath(dwPID), bstr(L"GetOwnerSid"), 0, {}, {}, &ptr_setter(OutParams), {});

	if (hrLast != WBEM_S_NO_ERROR || !OutParams)
		return {};

	VARIANT pVal;
	std::wstring UserSid;

	if ((hrLast = OutParams->Get(L"Sid", 0, &pVal, {}, {})) == WBEM_S_NO_ERROR && pVal.vt != VT_NULL)
	{
		UserSid = pVal.bstrVal;
		VariantClear(&pVal);
	}

	return UserSid;
}

DWORD WMIConnection::GetProcessSessionId(DWORD const Pid)
{
	return GetProcessInt(Pid, L"SessionId");
}

int WMIConnection::ExecMethod(DWORD dwPID, const wchar_t* wsMethod, const wchar_t* wsParamName, DWORD dwParam)
{
	com_ptr<IWbemClassObject> Object;
	hrLast = pIWbemServices->GetObject(bstr(L"Win32_Process"), WBEM_FLAG_DIRECT_READ, {}, &ptr_setter(Object), {});

	if (!Object)
		return -1;

	com_ptr<IWbemClassObject> InSignature;

	if (wsParamName)
		hrLast = Object->GetMethod(wsMethod, 0, &ptr_setter(InSignature), {});

	int rc = -1;

	if (InSignature || !wsParamName)
	{
		com_ptr<IWbemClassObject> InParams;

		if (InSignature)
			hrLast = InSignature->SpawnInstance(0, &ptr_setter(InParams));

		if (InParams || !wsParamName)
		{
			if (InParams)
			{
				VARIANT var; VariantClear(&var);
				var.vt = VT_I4;
				var.lVal = dwParam;
				hrLast = InParams->Put(wsParamName, 0, &var, 0);
			}

			com_ptr<IWbemClassObject> OutParams;

			if ((hrLast = pIWbemServices->ExecMethod(ProcessPath(dwPID), bstr(wsMethod), 0, {}, InParams.get(), &ptr_setter(OutParams), {})) == WBEM_S_NO_ERROR)
			{
				rc = 0;

				if (OutParams)
				{
					VARIANT pVal;

					if (OutParams->Get(L"ReturnValue", 0, &pVal, {}, {}) == WBEM_S_NO_ERROR)
					{
						if (pVal.vt==VT_I4)
							rc = pVal.lVal;

						VariantClear(&pVal);
					}
				}
			}
		}
	}

	return rc;
}

bool WMIConnection::GetProcessProperty(DWORD const Pid, const wchar_t* const Name, const std::function<void(const VARIANT&)>& Getter)
{
	com_ptr<IWbemClassObject> Object;
	hrLast = pIWbemServices->GetObject(ProcessPath(Pid), 0, {}, &ptr_setter(Object), {});

	if (hrLast != WBEM_S_NO_ERROR || !Object)
		return false;

	VARIANT pVal;

	hrLast = Object->Get(Name, 0, &pVal, {}, {});
	if (hrLast != WBEM_S_NO_ERROR)
		return false;

	Getter(pVal);

	VariantClear(&pVal);

	return true;
}

DWORD WMIConnection::GetProcessInt(DWORD const Pid, const wchar_t* const Name)
{
	DWORD Value = 0;
	GetProcessProperty(Pid, Name, [&](const VARIANT& Variant)
	{
		if (Variant.vt == VT_I4)
			Value = Variant.lVal;
	});
	return Value;
}

std::wstring WMIConnection::GetProcessString(DWORD const Pid, const wchar_t* const Name)
{
	std::wstring Value;
	GetProcessProperty(Pid, Name, [&](const VARIANT& Variant)
	{
		if (Variant.vt == VT_BSTR)
			Value = Variant.bstrVal;
	});
	return Value;

}

int WMIConnection::SetProcessPriority(DWORD dwPID, DWORD dwPri)
{
	return ExecMethod(dwPID, L"SetPriority", L"Priority", dwPri);
}

int WMIConnection::TerminateProcess(DWORD dwPID)
{
	return ExecMethod(dwPID, L"Terminate", L"Reason", ERROR_PROCESS_ABORTED);
}

bool WMIConnection::Connect(const wchar_t* pMachineName, const wchar_t* pUser, const wchar_t* pPassword)
{
	if (pIWbemServices)
		return true;

	if (pUser && !*pUser)
		pUser = pPassword = {}; // Empty username means default security

	hrLast = WBEM_S_NO_ERROR;
	com_ptr<IWbemLocator> IWbemLocator;

	if ((hrLast = CoCreateInstance(CLSID_WbemLocator, {}, CLSCTX_INPROC_SERVER, IID_IWbemLocator, IID_PPV_ARGS_Helper(&ptr_setter(IWbemLocator)))) == S_OK)
	{
		if (!pMachineName || !*pMachineName)
			pMachineName = L".";

		if (norm_m_prefix(pMachineName))
			pMachineName += 2;

		const auto Namespace = format(FSTR(L"\\\\{0}\\root\\cimv2"), pMachineName);

		if ((hrLast = IWbemLocator->ConnectServer(bstr(Namespace.c_str()), bstr(pUser), bstr(pPassword), {}, 0, {}, {}, &pIWbemServices)) == S_OK)
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
