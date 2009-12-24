#include <windows.h>
#include <shlobj.h>
#include <msiquery.h>

#include <comdef.h>

#include <string>
#include <list>
#include <sstream>
#include <iomanip>
using namespace std;

#define CLEAN(type, object, code) \
  class Clean_##object { \
  private: \
    type object; \
  public: \
    Clean_##object(type object): object(object) { \
    } \
    ~Clean_##object() { \
      code; \
    } \
  }; \
  Clean_##object clean_##object(object);

struct Error {
  int code;
  wstring message;
  const char* file;
  int line;
};

#define FAIL(_code) { \
  Error error; \
  error.code = _code; \
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
#define CHECK(code) { if (!(code)) FAIL_MSG(L#code); }

template<class CharType> basic_string<CharType> strip(const basic_string<CharType>& str) {
  basic_string<CharType>::size_type hp = 0;
  basic_string<CharType>::size_type tp = str.size();
  while ((hp < str.size()) && ((static_cast<unsigned>(str[hp]) <= 0x20) || (str[hp] == 0x7F)))
    hp++;
  if (hp < str.size())
    while ((static_cast<unsigned>(str[tp - 1]) <= 0x20) || (str[tp - 1] == 0x7F))
      tp--;
  return str.substr(hp, tp - hp);
}

wstring widen(const string& str) {
  return wstring(str.begin(), str.end());
}

wstring get_system_message(HRESULT hr) {
  wchar_t* sys_msg;
  DWORD len = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPWSTR>(&sys_msg), 0, NULL);
  wostringstream st;
  wstring message;
  if (len) {
    CLEAN(wchar_t*, sys_msg, LocalFree(static_cast<HLOCAL>(sys_msg)));
    message = strip(wstring(sys_msg));
  }
  else {
    message = L"HRESULT:";
  }
  st << message << L" (0x" << hex << uppercase << setw(8) << setfill(L'0') << hr << L")";
  return st.str();
}

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
  Buffer(size_t size) {
    buffer = new Type[size];
    buf_size = size;
  }
  ~Buffer() {
    delete[] buffer;
  }
  void resize(size_t size) {
    delete[] buffer;
    buffer = new Type[size];
    buf_size = size;
  }
  Type* data() {
    return buffer;
  }
  size_t size() const {
    return buf_size;
  }
  void clear() {
    memset(buffer, 0, buf_size * sizeof(Type));
  }
};

_COM_SMARTPTR_TYPEDEF(IShellLink, __uuidof(IShellLink));
_COM_SMARTPTR_TYPEDEF(IPersistFile, __uuidof(IPersistFile));
_COM_SMARTPTR_TYPEDEF(IShellLinkDataList, __uuidof(IShellLinkDataList));

wchar_t hex(unsigned char v) {
  if (v >= 0 && v <= 9)
    return v + L'0';
  else
    return v - 10 + L'A';
}

wstring get_shortcut_props(const wstring& file_name) {
  IShellLinkPtr sl;
  CHECK_COM(sl.CreateInstance(CLSID_ShellLink));

  IPersistFilePtr pf(sl);
  CHECK_COM(pf->Load(file_name.c_str(), STGM_READ));

  IShellLinkDataListPtr sldl(sl);
  DATABLOCK_HEADER* dbh;
  CHECK_COM(sldl->CopyDataBlock(NT_CONSOLE_PROPS_SIG, reinterpret_cast<VOID**>(&dbh)));
  CLEAN(DATABLOCK_HEADER*, dbh, LocalFree(dbh));

  wstring result;
  result.reserve(dbh->cbSize * 2);
  for (unsigned i = 0; i < dbh->cbSize; i++) {
    unsigned char b = reinterpret_cast<unsigned char*>(dbh)[i];
    result += hex(b >> 4);
    result += hex(b & 0x0F);
  }
  return result;
}

unsigned char unhex(wchar_t v) {
  if (v >= L'0' && v <= L'9')
    return v - L'0';
  else
    return v - L'A' + 10;
}

void set_shortcut_props(const wstring& file_name, const wstring& props) {
  IShellLinkPtr sl;
  CHECK_COM(sl.CreateInstance(CLSID_ShellLink));

  IPersistFilePtr pf(sl);
  CHECK_COM(pf->Load(file_name.c_str(), STGM_READWRITE));
  
  string db;
  db.reserve(props.size() / 2);
  for (unsigned i = 0; i < props.size(); i += 2) {
    unsigned char b = (unhex(props[i]) << 4) | unhex(props[i + 1]);
    db += b;
  }
  const DATABLOCK_HEADER* dbh = reinterpret_cast<const DATABLOCK_HEADER*>(db.data());

  IShellLinkDataListPtr sldl(sl);
  sldl->RemoveDataBlock(dbh->dwSignature);
  CHECK_COM(sldl->AddDataBlock(const_cast<char*>(db.data())));

  CHECK_COM(pf->Save(file_name.c_str(), TRUE));
}

wstring get_field(MSIHANDLE h_record, unsigned field_idx) {
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

wstring get_property(MSIHANDLE h_install, const wstring& name) {
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

wstring add_trailing_slash(const wstring& path) {
  if ((path.size() == 0) || (path[path.size() - 1] == L'\\')) {
    return path;
  }
  else {
    return path + L'\\';
  }
}

list<wstring> get_shortcut_list(MSIHANDLE h_install) {
  list<wstring> result;
  PMSIHANDLE h_db = MsiGetActiveDatabase(h_install);
  CHECK(h_db);
  PMSIHANDLE h_view;
  CHECK_ADVSYS(MsiDatabaseOpenView(h_db, "SELECT Shortcut, Directory_, Name FROM Shortcut", &h_view));
  CHECK_ADVSYS(MsiViewExecute(h_view, 0));
  PMSIHANDLE h_record;
  while (true) {
    UINT res = MsiViewFetch(h_view, &h_record);
    if (res == ERROR_NO_MORE_ITEMS) break;
    CHECK_ADVSYS(res);

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
  return result;
}

void init_progress(MSIHANDLE h_install, const wstring& action_name, const wstring& action_descr, unsigned max_progress) {
  PMSIHANDLE h_progress_rec = MsiCreateRecord(4);
  MsiRecordSetInteger(h_progress_rec, 1, 0);
  MsiRecordSetInteger(h_progress_rec, 2, max_progress);
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

void update_progress(MSIHANDLE h_install, const wstring& file_name) {
  PMSIHANDLE h_action_rec = MsiCreateRecord(1);
  MsiRecordSetStringW(h_action_rec, 1, file_name.c_str());
  MsiProcessMessage(h_install, INSTALLMESSAGE_ACTIONDATA, h_action_rec);
}

wstring get_error_message(const Error& e) {
  wostringstream st;
  st << L"Error at " << widen(e.file) << L":" << e.line << L": ";
  if (e.code != NO_ERROR)
    st << get_system_message(e.code);
  if (!e.message.empty()) {
    if (e.code != NO_ERROR)
      st << L": ";
    st << e.message;
  }
  return st.str();
}

wstring get_error_message(const exception& e) {
  wostringstream st;
  st << widen(typeid(e).name()) << L": " << widen(e.what());
  return st.str();
}

wstring get_error_message(const _com_error& e) {
  wostringstream st;
  st << L"_com_error: " << get_system_message(e.Error());
  return st.str();
}

void log_message(MSIHANDLE h_install, const wstring& message) {
  PMSIHANDLE h_rec = MsiCreateRecord(0);
  MsiRecordSetStringW(h_rec, 0, message.c_str());
  MsiProcessMessage(h_install, INSTALLMESSAGE_INFO, h_rec);
}

#define BEGIN_ERROR_HANDLER try {
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
    log_message(h_install, L"unknwon error"); \
  }

void save_shortcut_props(MSIHANDLE h_install) {
  BEGIN_ERROR_HANDLER
  HRESULT com_hr = CoInitialize(NULL);
  CHECK_COM(com_hr);
  CLEAN(HRESULT, com_hr, CoUninitialize());

  wstring data;
  list<wstring> shortcut_list = get_shortcut_list(h_install);
  init_progress(h_install, L"SaveShortcutProps", L"Saving shortcut properties", shortcut_list.size());
  for (list<wstring>::const_iterator file_path = shortcut_list.begin(); file_path != shortcut_list.end(); file_path++) {
    BEGIN_ERROR_HANDLER
    update_progress(h_install, *file_path);
    wstring props = get_shortcut_props(*file_path);
    data.append(*file_path).append(L"\n").append(props).append(L"\n");
    END_ERROR_HANDLER
  }
  CHECK_ADVSYS(MsiSetPropertyW(h_install, L"RestoreShortcutProps", data.c_str()));
  END_ERROR_HANDLER
}

list<wstring> split(const wstring& str, wchar_t sep) {
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

void restore_shortcut_props(MSIHANDLE h_install) {
  BEGIN_ERROR_HANDLER
  HRESULT com_hr = CoInitialize(NULL);
  CHECK_COM(com_hr);
  CLEAN(HRESULT, com_hr, CoUninitialize());

  wstring data = get_property(h_install, L"CustomActionData");
  list<wstring> str_list = split(data, L'\n');
  CHECK(str_list.size() % 2 == 0);
  init_progress(h_install, L"RestoreShortcutProps", L"Restoring shortcut properties", str_list.size());
  for (list<wstring>::const_iterator str = str_list.begin(); str != str_list.end(); str++) {
    BEGIN_ERROR_HANDLER
    const wstring& file_path = *str;
    str++;
    const wstring& props = *str;
    update_progress(h_install, file_path);
    set_shortcut_props(file_path, props);
    END_ERROR_HANDLER
  }
  END_ERROR_HANDLER
}

bool is_inst(MSIHANDLE h_install, const wstring& feature) {
  INSTALLSTATE st_inst, st_action;
  CHECK_ADVSYS(MsiGetFeatureStateW(h_install, feature.c_str(), &st_inst, &st_action));
  INSTALLSTATE st = st_action;
  if (st <= 0) st = st_inst;
  if (st <= 0) return false;
  if ((st == INSTALLSTATE_REMOVED) || (st == INSTALLSTATE_ABSENT)) return false;
  return true;
}

void update_feature_state(MSIHANDLE h_install) {
  PMSIHANDLE h_db = MsiGetActiveDatabase(h_install);
  CHECK(h_db);
  PMSIHANDLE h_view;
  CHECK_ADVSYS(MsiDatabaseOpenView(h_db, "SELECT Feature FROM Feature WHERE Display = 0", &h_view));
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
    for (list<wstring>::const_iterator feature = sub_features.begin(); feature != sub_features.end(); feature++) {
      if (!is_inst(h_install, *feature)) {
        inst = false;
        break;
      }
    }
    CHECK_ADVSYS(MsiSetFeatureStateW(h_install, feature_id.c_str(), inst ? INSTALLSTATE_LOCAL : INSTALLSTATE_ABSENT));
  }
}

// Find all hidden features with names like F1.F2...FN and set their state to installed
// when all the features F1, F2, ..., FN are installed.
// Example: feature Docs.Russian (Russian documentation) is installed only when feature Docs (Documentation)
// and feature Russian (Russian language support) are installed by user.
UINT __stdcall UpdateFeatureState(MSIHANDLE h_install) {
  BEGIN_ERROR_HANDLER
  update_feature_state(h_install);
  return ERROR_SUCCESS;
  END_ERROR_HANDLER
  return ERROR_INSTALL_FAILURE;
}

// Read console properties for all existing shortcuts and save them for later use by RestoreShortcutProps.
UINT __stdcall SaveShortcutProps(MSIHANDLE h_install) {
  save_shortcut_props(h_install);
  return ERROR_SUCCESS;
}

// Restore console properties saved by SaveShortcutProps.
// This action is run after shortcuts are recreated by upgrade/repair.
UINT __stdcall RestoreShortcutProps(MSIHANDLE h_install) {
  restore_shortcut_props(h_install);
  return ERROR_SUCCESS;
}
