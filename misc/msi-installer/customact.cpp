#include <string>
#include <list>
#include <typeinfo>

#include <windows.h>
#include <shellapi.h>
#include <shlobj.h>
#include <msiquery.h>
#include <rpc.h>
#include <comdef.h>

using namespace std;

HMODULE g_h_module;
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD /*fdwReason*/, LPVOID /*lpvReserved*/) {
  g_h_module = hinstDLL;
  return TRUE;
}

#define CLEAN(type, object, code) \
  class Clean_##object { \
  private: \
    type& object; \
  public: \
    Clean_##object(type& object): object(object) { \
    } \
    ~Clean_##object() { \
      code; \
    } \
  }; \
  Clean_##object clean_##object(object);

struct Error {
  int code;
  const wchar_t* message;
  const char* file;
  int line;
};

#define FAIL(_code) { \
  Error error; \
  error.code = _code; \
  error.message = NULL; \
  error.file = __FILE__; \
  error.line = __LINE__; \
  throw error; \
}

#define FAIL_MSG(_message) { \
  Error error; \
  error.code = 0; \
  error.message = _message; \
  error.file = __FILE__; \
  error.line = __LINE__; \
  throw error; \
}

#define CHECK_SYS(code) { if (!(code)) FAIL(HRESULT_FROM_WIN32(GetLastError())); }
#define CHECK_ADVSYS(code) { DWORD __ret = (code); if (__ret != ERROR_SUCCESS) FAIL(HRESULT_FROM_WIN32(__ret)); }
#define CHECK_COM(code) { HRESULT __ret = (code); if (FAILED(__ret)) FAIL(__ret); }
#define CHECK(code) { if (!(code)) FAIL_MSG(L ## #code); }

class NonCopyable {
protected:
  NonCopyable() {}
  ~NonCopyable() {}
private:
  NonCopyable(const NonCopyable&);
  NonCopyable& operator=(const NonCopyable&);
};

template<typename Type> class Buffer: private NonCopyable {
private:
  Type* buffer;
  size_t buf_size;
public:
  Buffer(): buffer(NULL), buf_size(0) {
  }
  Buffer(size_t size) {
    buffer = new Type[size];
    buf_size = size;
  }
  ~Buffer() {
    delete[] buffer;
  }
  void resize(size_t size) {
    if (buffer) delete[] buffer;
    buffer = new Type[size];
    buf_size = size;
  }
  Type* data() {
    return buffer;
  }
  size_t size() const {
    return buf_size;
  }
};

template<class CharType>
static basic_string<CharType> strip(const basic_string<CharType>& str) {
  typename basic_string<CharType>::size_type hp = 0;
  typename basic_string<CharType>::size_type tp = str.size();
  while ((hp < str.size()) && ((static_cast<unsigned>(str[hp]) <= 0x20) || (str[hp] == 0x7F)))
    hp++;
  if (hp < str.size())
    while ((static_cast<unsigned>(str[tp - 1]) <= 0x20) || (str[tp - 1] == 0x7F))
      tp--;
  return str.substr(hp, tp - hp);
}

static wstring widen(const string& str) {
  return wstring(str.begin(), str.end());
}

static wstring int_to_str(int val) {
  wchar_t str[64];
  return _itow(val, str, 10);
}

static wchar_t hex(unsigned char v) {
  if (v >= 0 && v <= 9)
    return v + L'0';
  else
    return v - 10 + L'A';
}

static wstring hex(const void* data_ptr, size_t size) {
  wstring result;
  result.reserve(size * 2);
  const auto data = reinterpret_cast<const unsigned char*>(data_ptr);
  for (unsigned i = 0; i < size; i++) {
    unsigned char b = data[i];
    result += hex(b >> 4);
    result += hex(b & 0x0F);
  }
  return result;
}

static unsigned char unhex(wchar_t v) {
  if (v >= L'0' && v <= L'9')
    return static_cast<unsigned char>(v - L'0');
  else
    return static_cast<unsigned char>(v - L'A' + 10);
}

static void unhex(const wstring& str, Buffer<unsigned char>& buf) {
  buf.resize(str.size() / 2);
  for (unsigned i = 0; i < str.size(); i += 2) {
    unsigned char b = (unhex(str[i]) << 4) | unhex(str[i + 1]);
    buf.data()[i / 2] = b;
  }
}

static wstring hex_str(unsigned val) {
  wstring result(8, L'0');
  for (unsigned i = 0; i < 8; i++) {
    wchar_t c = hex(val & 0x0F);
    if (c) result[7 - i] = c;
    val >>= 4;
  }
  return result;
}

static wstring get_system_message(HRESULT hr) {
  wchar_t* sys_msg;
  DWORD len = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPWSTR>(&sys_msg), 0, NULL);
  wstring message;
  if (len) {
    CLEAN(wchar_t*, sys_msg, LocalFree(static_cast<HLOCAL>(sys_msg)));
    message = strip(wstring(sys_msg));
  }
  else {
    message = L"HRESULT:";
  }
  message.append(L" (0x").append(hex_str(hr)).append(L")");
  return message;
}

_COM_SMARTPTR_TYPEDEF(IShellLinkW, __uuidof(IShellLinkW));
_COM_SMARTPTR_TYPEDEF(IPersistFile, __uuidof(IPersistFile));
_COM_SMARTPTR_TYPEDEF(IShellLinkDataList, __uuidof(IShellLinkDataList));

static wstring get_shortcut_props(const wstring& file_name) {
  IShellLinkWPtr sl;
  CHECK_COM(sl.CreateInstance(CLSID_ShellLink));

  IPersistFilePtr pf(sl);
  CHECK_COM(pf->Load(file_name.c_str(), STGM_READ));

  IShellLinkDataListPtr sldl(sl);
  DATABLOCK_HEADER* dbh;
  CHECK_COM(sldl->CopyDataBlock(NT_CONSOLE_PROPS_SIG, reinterpret_cast<VOID**>(&dbh)));
  CLEAN(DATABLOCK_HEADER*, dbh, LocalFree(dbh));

  return hex(dbh, dbh->cbSize);
}

static void set_shortcut_props(const wstring& file_name, const wstring& props) {
  IShellLinkWPtr sl;
  CHECK_COM(sl.CreateInstance(CLSID_ShellLink));

  IPersistFilePtr pf(sl);
  CHECK_COM(pf->Load(file_name.c_str(), STGM_READWRITE));

  Buffer<unsigned char> db;
  unhex(props, db);
  const DATABLOCK_HEADER* dbh = reinterpret_cast<const DATABLOCK_HEADER*>(db.data());

  IShellLinkDataListPtr sldl(sl);
  sldl->RemoveDataBlock(dbh->dwSignature);
  CHECK_COM(sldl->AddDataBlock(db.data()));

  CHECK_COM(pf->Save(file_name.c_str(), TRUE));
}

static wstring get_field(MSIHANDLE h_record, unsigned field_idx) {
  DWORD size = 256;
  Buffer<wchar_t> buf(size);
  UINT res = MsiRecordGetStringW(h_record, field_idx, buf.data(), &size);
  if (res == ERROR_MORE_DATA) {
    size += 1;
    buf.resize(size);
    res = MsiRecordGetStringW(h_record, field_idx, buf.data(), &size);
  }
  CHECK_ADVSYS(res);
  return wstring(buf.data(), size);
}

static wstring get_property(MSIHANDLE h_install, const wstring& name) {
  DWORD size = 256;
  Buffer<wchar_t> buf(size);
  UINT res = MsiGetPropertyW(h_install, name.c_str(), buf.data(), &size);
  if (res == ERROR_MORE_DATA) {
    size += 1;
    buf.resize(size);
    res = MsiGetPropertyW(h_install, name.c_str(), buf.data(), &size);
  }
  CHECK_ADVSYS(res);
  return wstring(buf.data(), size);
}

static wstring add_trailing_slash(const wstring& path) {
  if ((path.size() == 0) || (path[path.size() - 1] == L'\\')) {
    return path;
  }
  else {
    return path + L'\\';
  }
}

static bool is_installed(UINT (WINAPI *MsiGetState)(MSIHANDLE, LPCWSTR, INSTALLSTATE*, INSTALLSTATE*), MSIHANDLE h_install, const wstring& name) {
  INSTALLSTATE st_inst, st_action;
  CHECK_ADVSYS(MsiGetState(h_install, name.c_str(), &st_inst, &st_action));
  INSTALLSTATE st = st_action;
  if (st <= 0) st = st_inst;
  if (st <= 0) return false;
  if ((st == INSTALLSTATE_REMOVED) || (st == INSTALLSTATE_ABSENT)) return false;
  return true;
}

static bool is_component_installed(MSIHANDLE h_install, const wstring& component) {
  return is_installed(MsiGetComponentStateW, h_install, component);
}

static bool is_feature_installed(MSIHANDLE h_install, const wstring& feature) {
  return is_installed(MsiGetFeatureStateW, h_install, feature);
}

static list<wstring> get_shortcut_list(MSIHANDLE h_install, const wchar_t* condition = NULL) {
  list<wstring> result;
  PMSIHANDLE h_db = MsiGetActiveDatabase(h_install);
  CHECK(h_db);
  PMSIHANDLE h_view;
  wstring query = L"SELECT Shortcut, Directory_, Name, Component_ FROM Shortcut";
  if (condition)
    query.append(L" WHERE ").append(condition);
  CHECK_ADVSYS(MsiDatabaseOpenViewW(h_db, query.c_str(), &h_view));
  CHECK_ADVSYS(MsiViewExecute(h_view, 0));
  PMSIHANDLE h_record;
  while (true) {
    UINT res = MsiViewFetch(h_view, &h_record);
    if (res == ERROR_NO_MORE_ITEMS) break;
    CHECK_ADVSYS(res);

    wstring component = get_field(h_record, 4);

    if (is_component_installed(h_install, component)) {
      wstring shortcut_id = get_field(h_record, 1);
      wstring directory_id = get_field(h_record, 2);
      wstring file_name = get_field(h_record, 3);
      size_t pos = file_name.find(L'|');
      if (pos != wstring::npos)
        file_name.erase(0, pos + 1);
      wstring directory_path = get_property(h_install, directory_id);
      wstring full_path = add_trailing_slash(directory_path) + file_name + L".lnk";
      result.push_back(full_path);
    }
  }
  return result;
}

static void init_progress(MSIHANDLE h_install, const wstring& action_name, const wstring& action_descr, size_t max_progress) {
  PMSIHANDLE h_progress_rec = MsiCreateRecord(4);
  MsiRecordSetInteger(h_progress_rec, 1, 0);
  MsiRecordSetInteger(h_progress_rec, 2, static_cast<unsigned>(max_progress));
  MsiRecordSetInteger(h_progress_rec, 3, 0);
  MsiRecordSetInteger(h_progress_rec, 4, 0);
  MsiProcessMessage(h_install, INSTALLMESSAGE_PROGRESS, h_progress_rec);
  MsiRecordSetInteger(h_progress_rec, 1, 1);
  MsiRecordSetInteger(h_progress_rec, 2, 1);
  MsiRecordSetInteger(h_progress_rec, 3, 1);
  MsiProcessMessage(h_install, INSTALLMESSAGE_PROGRESS, h_progress_rec);
  PMSIHANDLE h_action_start_rec = MsiCreateRecord(3);
  MsiRecordSetStringW(h_action_start_rec, 1, action_name.c_str());
  MsiRecordSetStringW(h_action_start_rec, 2, action_descr.c_str());
  MsiRecordSetStringW(h_action_start_rec, 3, L"[1]");
  MsiProcessMessage(h_install, INSTALLMESSAGE_ACTIONSTART, h_action_start_rec);
}

static void update_progress(MSIHANDLE h_install, const wstring& file_name) {
  PMSIHANDLE h_action_rec = MsiCreateRecord(1);
  MsiRecordSetStringW(h_action_rec, 1, file_name.c_str());
  MsiProcessMessage(h_install, INSTALLMESSAGE_ACTIONDATA, h_action_rec);
}

static wstring get_error_message(const Error& e) {
  wstring str;
  str.append(L"Error at ").append(widen(e.file)).append(L":").append(int_to_str(e.line)).append(L": ");
  if (e.code)
    str.append(get_system_message(e.code));
  if (e.message) {
    if (e.code)
      str.append(L": ");
    str.append(e.message);
  }
  return str;
}

static wstring get_error_message(const exception& e) {
  wstring str;
  str.append(widen(typeid(e).name())).append(L": ").append(widen(e.what()));
  return str;
}

static wstring get_error_message(const _com_error& e) {
  wstring str;
  str.append(L"_com_error: ").append(get_system_message(e.Error()));
  return str;
}

static void log_message(MSIHANDLE h_install, const wstring& message) {
  OutputDebugStringW((message + L'\n').c_str());
  PMSIHANDLE h_rec = MsiCreateRecord(0);
  MsiRecordSetStringW(h_rec, 0, message.c_str());
  MsiProcessMessage(h_install, INSTALLMESSAGE_INFO, h_rec);
}

#define BEGIN_ERROR_HANDLER \
  try { \
    try {

#define END_ERROR_HANDLER \
    } \
    catch (const Error& e) { \
      log_message(h_install, get_error_message(e)); \
    } \
    catch (const exception& e) { \
      log_message(h_install, get_error_message(e)); \
    } \
    catch (const _com_error& e) { \
      log_message(h_install, get_error_message(e)); \
    } \
    catch (...) { \
      log_message(h_install, L"unknown exception"); \
    } \
  } \
  catch (...) { \
  }

class ComInit: private NonCopyable {
private:
  HRESULT hr;
public:
  ComInit() {
    hr = CoInitialize(NULL);
  }
  ~ComInit() {
    if (SUCCEEDED(hr))
      CoUninitialize();
  }
};

static NT_CONSOLE_PROPS template_props
{
	{
		204,
		0xa0000002
	},
	0x07,              // wFillAttribute
	0xf5,              // wPopupFillAttribute
	{80, 3000},        // dwScreenBufferSize
	{80, 25},          // dwWindowSize
	{0, 0},            // dwWindowOrigin
	0,                 // nFont
	0,                 // nInputBufferSize
	{0, 16},           // dwFontSize
	54,                // uFontFamily
	400,               // uFontWeight
	L"Lucida Console", // FaceName
	25,                // uCursorSize
	0,                 // bFullScreen
	0,                 // bQuickEdit
	1,                 // bInsertMode
	1,                 // bAutoPosition
	50,                // uHistoryBufferSize
	4,                 // uNumberOfHistoryBuffers
	0,                 // bHistoryNoDup
	{                  // ColorTable
		//  BBGGRR
		0x00000000,    // black
		0x00800000,    // blue
		0x00008000,    // green
		0x00808000,    // cyan
		0x00000080,    // red
		0x00800080,    // magenta
		0x00008080,    // yellow
		0x00C0C0C0,    // white
		0x00808080,    // bright black
		0x00FF0000,    // bright blue
		0x0000FF00,    // bright green
		0x00FFFF00,    // bright cyan
		0x000000FF,    // bright red
		0x00FF00FF,    // bright magenta
		0x0000FFFF,    // bright yellow
		0x00FFFFFF     // white
	}
};

static void save_shortcut_props(MSIHANDLE h_install) {
  ComInit com_init;
  wstring data;
  list<wstring> shortcut_list = get_shortcut_list(h_install, L"Target = '[#Far.exe]'");
  init_progress(h_install, L"SaveShortcutProps", L"Saving shortcut properties", shortcut_list.size());
  list<wstring> props_list;
  bool has_empty_props = false;
  for (const auto& file_path: shortcut_list) {
    update_progress(h_install, file_path);
    wstring props;
    BEGIN_ERROR_HANDLER
    props = get_shortcut_props(file_path);
    END_ERROR_HANDLER
    if (props.empty()) has_empty_props = true;
    props_list.push_back(props);
  }
  wstring default_props;
  if (has_empty_props) {
    if (MsiEvaluateCondition(h_install, L"VersionNT >= 601") == MSICONDITION_TRUE) {
      template_props.dwFontSize.Y = 20;
      wcscpy(template_props.FaceName, L"Consolas");
    }
    default_props = hex(&template_props, sizeof(template_props));
  }
  list<wstring>::const_iterator props = props_list.begin();
  for (const auto& file_path: shortcut_list) {
    data.append(file_path).append(L"\n");
    if (props->empty())
      data.append(default_props);
    else
      data.append(*props);
    data.append(L"\n");
  }
  CHECK_ADVSYS(MsiSetPropertyW(h_install, L"RestoreShortcutProps", data.c_str()));
}

static list<wstring> split(const wstring& str, wchar_t sep) {
  list<wstring> result;
  size_t begin = 0;
  while (begin < str.size()) {
    size_t end = str.find(sep, begin);
    if (end == wstring::npos)
      end = str.size();
    wstring sub_str = str.substr(begin, end - begin);
    result.push_back(sub_str);
    begin = end + 1;
  }
  return result;
}

static void restore_shortcut_props(MSIHANDLE h_install) {
  ComInit com_init;
  wstring data = get_property(h_install, L"CustomActionData");
  list<wstring> str_list = split(data, L'\n');
  CHECK(str_list.size() % 2 == 0);
  init_progress(h_install, L"RestoreShortcutProps", L"Restoring shortcut properties", str_list.size());
  for (auto str = str_list.begin(); str != str_list.end(); ++str) {
    BEGIN_ERROR_HANDLER
    const wstring& file_path = *str;
    str++;
    const wstring& props = *str;
    update_progress(h_install, file_path);
    set_shortcut_props(file_path, props);
    END_ERROR_HANDLER
  }
}

static void update_feature_state(MSIHANDLE h_install) {
  PMSIHANDLE h_db = MsiGetActiveDatabase(h_install);
  CHECK(h_db);
  PMSIHANDLE h_view;
  CHECK_ADVSYS(MsiDatabaseOpenView(h_db, L"SELECT Feature FROM Feature WHERE Display = 0", &h_view));
  CHECK_ADVSYS(MsiViewExecute(h_view, 0));
  PMSIHANDLE h_record;
  while (true) {
    UINT res = MsiViewFetch(h_view, &h_record);
    if (res == ERROR_NO_MORE_ITEMS) break;
    CHECK_ADVSYS(res);

    wstring feature_id = get_field(h_record, 1);

    list<wstring> sub_features = split(feature_id, L'.');
    CHECK(sub_features.size() > 1);

    bool inst = true;
    for (const auto& feature: sub_features) {
      if (feature == L"Ignore") break;
      if (!is_feature_installed(h_install, feature)) {
        inst = false;
        break;
      }
    }
    CHECK_ADVSYS(MsiSetFeatureStateW(h_install, feature_id.c_str(), inst ? INSTALLSTATE_LOCAL : INSTALLSTATE_ABSENT));
  }
}

static void launch_shortcut(MSIHANDLE h_install) {
  wstring launch_app = get_property(h_install, L"LAUNCHAPP");
  if (launch_app == L"1") { // launch Far shortcut
    list<wstring> shortcut_list = get_shortcut_list(h_install, L"Target = '[#Far.exe]'");
    CHECK(!shortcut_list.empty());
    launch_app = *shortcut_list.begin();
  }
  SHELLEXECUTEINFOW sei = { sizeof(SHELLEXECUTEINFOW) };
  sei.fMask = SEE_MASK_FLAG_NO_UI | SEE_MASK_NO_CONSOLE;
  sei.lpFile = launch_app.c_str();
  sei.nShow = SW_SHOWDEFAULT;
  CHECK_SYS(ShellExecuteExW(&sei));
}

// Find all hidden features with names like F1.F2...FN and set their state to installed
// when all the features F1, F2, ..., FN are installed.
// Example: feature Docs.Russian (Russian documentation) is installed only when feature Docs (Documentation)
// and feature Russian (Russian language support) are installed by user.
UINT WINAPI UpdateFeatureState(MSIHANDLE h_install) {
  BEGIN_ERROR_HANDLER
  update_feature_state(h_install);
  return ERROR_SUCCESS;
  END_ERROR_HANDLER
  return ERROR_INSTALL_FAILURE;
}

// Read console properties for all existing shortcuts and save them for later use by RestoreShortcutProps.
UINT WINAPI SaveShortcutProps(MSIHANDLE h_install) {
  BEGIN_ERROR_HANDLER
  save_shortcut_props(h_install);
  return ERROR_SUCCESS;
  END_ERROR_HANDLER
  return ERROR_INSTALL_FAILURE;
}

// Restore console properties saved by SaveShortcutProps.
// This action is run after shortcuts are recreated by upgrade/repair.
UINT WINAPI RestoreShortcutProps(MSIHANDLE h_install) {
  BEGIN_ERROR_HANDLER
  restore_shortcut_props(h_install);
  return ERROR_SUCCESS;
  END_ERROR_HANDLER
  return ERROR_INSTALL_FAILURE;
}

// Launch Far via installed shortcut or command specified by LAUNCHAPP property
UINT WINAPI LaunchShortcut(MSIHANDLE h_install) {
  BEGIN_ERROR_HANDLER
  launch_shortcut(h_install);
  return ERROR_SUCCESS;
  END_ERROR_HANDLER
  BEGIN_ERROR_HANDLER
  CHECK_ADVSYS(MsiDoAction(h_install, L"LaunchApp"));
  return ERROR_SUCCESS;
  END_ERROR_HANDLER
  return ERROR_INSTALL_FAILURE;
}
