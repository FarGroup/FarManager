#include "Proclist.hpp"
#include "perfthread.hpp"

#include <wbemidl.h>

#include <smart_ptr.hpp>

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
	return bstr(far::format(L"Win32_Process.Handle={0}"sv, Pid).c_str());
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

wmi_result<std::wstring> WMIConnection::GetProcessExecutablePath(DWORD const Pid) const
{
	return GetProcessString(Pid, L"ExecutablePath");
}

wmi_result<std::wstring> WMIConnection::GetProcessCommandLine(DWORD const Pid) const
{
	return GetProcessString(Pid, L"CommandLine");
}

HRESULT WMIConnection::AttachDebuggerToProcess(DWORD const Pid) const
{
	return ExecMethod(Pid, L"AttachDebugger");
}

wmi_result<DWORD> WMIConnection::GetProcessPriority(DWORD const Pid) const
{
	return GetProcessInt(Pid, L"Priority");
}

static HRESULT wbem_error_to_win32_error(unsigned const WbemError)
{
	switch (WbemError)
	{
	case 0:  return HRESULT_FROM_WIN32(ERROR_SUCCESS);            // Successful completion
	case 2:  return HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED);      // Access denied
	case 3:  return HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED);      // Insufficient privilege
	case 8:  return HRESULT_FROM_WIN32(ERROR_GEN_FAILURE);        // Unknown failure
	case 9:  return HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND);     // Path not found
	case 21: return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);  // Invalid parameter
	case 22: return HRESULT_FROM_WIN32(ERROR_GEN_FAILURE);        // Other
	default: return WbemError;
	}
}

static HRESULT get_return_value(com_ptr<IWbemClassObject> const& OutParams)
{
	VARIANT Variant;
	if (const auto Result = OutParams->Get(L"ReturnValue", 0, &Variant, {}, {}); FAILED(Result))
		return Result;

	assert(Variant.vt == VT_I4);

	const auto Result = wbem_error_to_win32_error(Variant.lVal);
	VariantClear(&Variant);

	return Result;
}

std::pair<wmi_result<std::wstring>, wmi_result<std::wstring>> WMIConnection::GetProcessOwner(DWORD const Pid) const
{
	com_ptr<IWbemClassObject> OutParams;
	if (const auto Result = pIWbemServices->ExecMethod(ProcessPath(Pid), bstr(L"GetOwner"), 0, {}, {}, &ptr_setter(OutParams), {}); FAILED(Result))
		return { Result, Result };

	std::pair<wmi_result<std::wstring>, wmi_result<std::wstring>> Pair;

	auto& [User, Domain] = Pair;

	VARIANT Variant;

	if (const auto Result = OutParams->Get(L"User", 0, &Variant, {}, {}); FAILED(Result))
		User = Result;
	else
	{
		if (Variant.vt == VT_NULL)
			User = get_return_value(OutParams);
		else
		{
			assert(Variant.vt == VT_BSTR);
			User = Variant.bstrVal;
		}
		VariantClear(&Variant);
	}

	if (const auto Result = OutParams->Get(L"Domain", 0, &Variant, {}, {}); FAILED(Result))
		Domain = Result;
	else
	{
		if (Variant.vt == VT_NULL)
			Domain = get_return_value(OutParams);
		else
		{
			assert(Variant.vt == VT_BSTR);
			Domain = Variant.bstrVal;
		}
		VariantClear(&Variant);
	}

	return Pair;
}

wmi_result<std::wstring> WMIConnection::GetProcessUserSid(DWORD Pid) const
{
	com_ptr<IWbemClassObject> OutParams;
	if (const auto Result = pIWbemServices->ExecMethod(ProcessPath(Pid), bstr(L"GetOwnerSid"), 0, {}, {}, &ptr_setter(OutParams), {}); FAILED(Result))
		return Result;

	VARIANT Variant;

	wmi_result<std::wstring> UserSid;
	if (const auto Result = OutParams->Get(L"Sid", 0, &Variant, {}, {}); FAILED(Result))
		UserSid = Result;
	else
	{
		if (Variant.vt == VT_NULL)
			UserSid = get_return_value(OutParams);
		else
		{
			assert(Variant.vt == VT_BSTR);
			UserSid = Variant.bstrVal;
		}
		VariantClear(&Variant);
	}

	return UserSid;
}

wmi_result<DWORD> WMIConnection::GetProcessSessionId(DWORD const Pid) const
{
	return GetProcessInt(Pid, L"SessionId");
}

HRESULT WMIConnection::ExecMethod(DWORD const Pid, const wchar_t* wsMethod, const wchar_t* wsParamName, DWORD dwParam) const
{
	com_ptr<IWbemClassObject> Object;
	if (const auto Result = pIWbemServices->GetObject(bstr(L"Win32_Process"), WBEM_FLAG_DIRECT_READ, {}, &ptr_setter(Object), {}); FAILED(Result))
		return Result;

	com_ptr<IWbemClassObject> InSignature;
	if (wsParamName)
	{
		if (const auto Result = Object->GetMethod(wsMethod, 0, &ptr_setter(InSignature), {}); FAILED(Result))
			return Result;
	}

	com_ptr<IWbemClassObject> InParams;
	if (InSignature)
	{
		if (const auto Result = InSignature->SpawnInstance(0, &ptr_setter(InParams)); FAILED(Result))
			return Result;
	}

	if (InParams)
	{
		VARIANT Variant;
		VariantInit(&Variant);

		Variant.vt = VT_I4;
		Variant.lVal = dwParam;
		if (const auto Result = InParams->Put(wsParamName, 0, &Variant, 0); FAILED(Result))
			return Result;
	}

	com_ptr<IWbemClassObject> OutParams;
	if (const auto Result = pIWbemServices->ExecMethod(ProcessPath(Pid), bstr(wsMethod), 0, {}, InParams.get(), &ptr_setter(OutParams), {}); FAILED(Result))
		return Result;

	return get_return_value(OutParams);
}

HRESULT WMIConnection::GetProcessProperty(DWORD const Pid, const wchar_t* const Name, const std::function<void(const VARIANT&)>& Getter) const
{
	com_ptr<IWbemClassObject> Object;
	if (const auto Result = pIWbemServices->GetObject(ProcessPath(Pid), 0, {}, &ptr_setter(Object), {}); FAILED(Result))
		return Result;

	VARIANT Variant;
	if (const auto Result = Object->Get(Name, 0, &Variant, {}, {}); FAILED(Result))
		return Result;

	Getter(Variant);

	return VariantClear(&Variant);
}

wmi_result<DWORD> WMIConnection::GetProcessInt(DWORD const Pid, const wchar_t* const Name) const
{
	DWORD Value;

	if (const auto Result = GetProcessProperty(Pid, Name, [&](const VARIANT& Variant)
	{
		if (Variant.vt == VT_I4)
			Value = Variant.lVal;
	}); FAILED(Result))
		return Result;

	return Value;
}

wmi_result<std::wstring> WMIConnection::GetProcessString(DWORD const Pid, const wchar_t* const Name) const
{
	std::wstring Value;
	if (const auto Result = GetProcessProperty(Pid, Name, [&](const VARIANT& Variant)
	{
		if (Variant.vt == VT_BSTR)
			Value = Variant.bstrVal;
	}); FAILED(Result))
		return Result;

	return Value;
}

HRESULT WMIConnection::SetProcessPriority(DWORD const Pid, DWORD const Priority) const
{
	return ExecMethod(Pid, L"SetPriority", L"Priority", Priority);
}

HRESULT WMIConnection::TerminateProcess(DWORD const Pid) const
{
	return ExecMethod(Pid, L"Terminate", L"Reason", ERROR_PROCESS_ABORTED);
}

HRESULT WMIConnection::Connect(const wchar_t* pMachineName, const wchar_t* pUser, const wchar_t* pPassword)
{
	if (pIWbemServices)
		return S_OK;

	if (pUser && !*pUser)
		pUser = pPassword = {}; // Empty username means default security

	com_ptr<IWbemLocator> IWbemLocator;
	if (const auto Result = CoCreateInstance(CLSID_WbemLocator, {}, CLSCTX_INPROC_SERVER, IID_IWbemLocator, IID_PPV_ARGS_Helper(&ptr_setter(IWbemLocator))); FAILED(Result))
		return Result;

	if (!pMachineName || !*pMachineName)
		pMachineName = L".";

	if (norm_m_prefix(pMachineName))
		pMachineName += 2;

	const auto Namespace = far::format(L"\\\\{0}\\root\\cimv2"sv, pMachineName);

	if (const auto Result = IWbemLocator->ConnectServer(bstr(Namespace.c_str()), bstr(pUser), bstr(pPassword), {}, 0, {}, {}, &pIWbemServices); FAILED(Result))
		return Result;

	// We impersonate a token to enable SeDebugPrivilege privilege.
	// Enable static cloaking here to make sure the server sees it.
	//
	// Some privileged information (like the full EXE path) will not be returned if this call fails.
	// However, it is not fatal, so the error can be ignored.
	CoSetProxyBlanket(
		pIWbemServices,
		RPC_C_AUTHN_DEFAULT,
		RPC_C_AUTHN_DEFAULT,
		{},
		RPC_C_AUTHN_LEVEL_DEFAULT,
		RPC_C_IMP_LEVEL_IMPERSONATE,
		{},
		EOAC_STATIC_CLOAKING
	);

	return S_OK;
}

void WMIConnection::Disconnect()
{
	if (pIWbemServices)
	{
		pIWbemServices->Release();
		pIWbemServices = {};
	}
}
