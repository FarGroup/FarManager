#include "utils.hpp"
#include "farutils.hpp"
#include "sysutils.hpp"

HINSTANCE g_h_instance = nullptr;

wstring get_system_message(HRESULT hr, DWORD lang_id) {
  wostringstream st;
  wchar_t* sys_msg;
  DWORD len = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, hr, lang_id, reinterpret_cast<LPWSTR>(&sys_msg), 0, nullptr);
  if (!len && lang_id && GetLastError() == ERROR_RESOURCE_LANG_NOT_FOUND)
    len = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, hr, 0, reinterpret_cast<LPWSTR>(&sys_msg), 0, nullptr);
  if (!len) {
    if (HRESULT_FACILITY(hr) == FACILITY_WIN32) {
      HMODULE h_winhttp = GetModuleHandle(L"winhttp");
      if (h_winhttp) {
        len = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS, h_winhttp, HRESULT_CODE(hr), lang_id, reinterpret_cast<LPWSTR>(&sys_msg), 0, nullptr);
        if (!len && lang_id && GetLastError() == ERROR_RESOURCE_LANG_NOT_FOUND)
          len = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS, h_winhttp, HRESULT_CODE(hr), 0, reinterpret_cast<LPWSTR>(&sys_msg), 0, nullptr);
      }
    }
  }
  if (len) {
    wstring message;
    try {
      message = sys_msg;
    }
    catch (...) {
      LocalFree(static_cast<HLOCAL>(sys_msg));
      throw;
    }
    LocalFree(static_cast<HLOCAL>(sys_msg));
    st << strip(message) << L" (0x" << hex << uppercase << setw(8) << setfill(L'0') << hr << L")";
  }
  else {
    st << L"HRESULT: 0x" << hex << uppercase << setw(8) << setfill(L'0') << hr;
  }
  return st.str();
}

wstring get_console_title() {
  Buffer<wchar_t> buf(10000);
  DWORD size = GetConsoleTitleW(buf.data(), static_cast<DWORD>(buf.size()));
  return wstring(buf.data(), size);
}

bool wait_for_single_object(HANDLE handle, DWORD timeout) {
  DWORD res = WaitForSingleObject(handle, timeout);
  CHECK_SYS(res != WAIT_FAILED);
  if (res == WAIT_OBJECT_0)
    return true;
  else if (res == WAIT_TIMEOUT)
    return false;
  else
    FAIL(E_FAIL);
}

wstring ansi_to_unicode(const string& str, unsigned code_page) {
  unsigned str_size = static_cast<unsigned>(str.size());
  if (str_size == 0)
    return wstring();
  int size = MultiByteToWideChar(code_page, 0, str.data(), str_size, nullptr, 0);
  Buffer<wchar_t> out(size);
  size = MultiByteToWideChar(code_page, 0, str.data(), str_size, out.data(), size);
  CHECK_SYS(size);
  return wstring(out.data(), size);
}

string unicode_to_ansi(const wstring& str, unsigned code_page) {
  unsigned str_size = static_cast<unsigned>(str.size());
  if (str_size == 0)
    return string();
  int size = WideCharToMultiByte(code_page, 0, str.data(), str_size, nullptr, 0, nullptr, nullptr);
  Buffer<char> out(size);
  size = WideCharToMultiByte(code_page, 0, str.data(), str_size, out.data(), size, nullptr, nullptr);
  CHECK_SYS(size);
  return string(out.data(), size);
}

wstring expand_env_vars(const wstring& str) {
  Buffer<wchar_t> buf(MAX_PATH);
  unsigned size = ExpandEnvironmentStringsW(str.c_str(), buf.data(), static_cast<DWORD>(buf.size()));
  if (size > buf.size()) {
    buf.resize(size);
    size = ExpandEnvironmentStringsW(str.c_str(), buf.data(), static_cast<DWORD>(buf.size()));
  }
  CHECK_SYS(size);
  return wstring(buf.data(), size - 1);
}

wstring get_full_path_name(const wstring& path) {
  Buffer<wchar_t> buf(MAX_PATH);
  DWORD size = GetFullPathNameW(path.c_str(), static_cast<DWORD>(buf.size()), buf.data(), nullptr);
  if (size > buf.size()) {
    buf.resize(size);
    size = GetFullPathNameW(path.c_str(), static_cast<DWORD>(buf.size()), buf.data(), nullptr);
  }
  CHECK_SYS(size);
  return wstring(buf.data(), size);
}

wstring get_current_directory() {
  Buffer<wchar_t> buf(MAX_PATH);
  DWORD size = GetCurrentDirectoryW(static_cast<DWORD>(buf.size()), buf.data());
  if (size > buf.size()) {
    buf.resize(size);
    size = GetCurrentDirectoryW(static_cast<DWORD>(buf.size()), buf.data());
  }
  CHECK_SYS(size);
  return wstring(buf.data(), size);
}


#define CHECK_FILE(code) { if (!(code)) throw Error(HRESULT_FROM_WIN32(GetLastError()), file_path, __FILE__, __LINE__); }

File::File(): h_file(INVALID_HANDLE_VALUE) {
}

File::~File() {
  close();
}

File::File(const wstring& file_path, DWORD desired_access, DWORD share_mode, DWORD creation_disposition, DWORD dlags_and_attributes): h_file(INVALID_HANDLE_VALUE) {
  open(file_path, desired_access, share_mode, creation_disposition, dlags_and_attributes);
}

void File::open(const wstring& file_path, DWORD desired_access, DWORD share_mode, DWORD creation_disposition, DWORD dlags_and_attributes) {
  CHECK_FILE(open_nt(file_path, desired_access, share_mode, creation_disposition, dlags_and_attributes));
}

bool File::open_nt(const wstring& file_path, DWORD desired_access, DWORD share_mode, DWORD creation_disposition, DWORD flags_and_attributes) {
  close();
  this->file_path = file_path;
  ArclitePrivateInfo* system_functions = Far::get_system_functions();
  if (system_functions)
    h_file = system_functions->CreateFile(long_path(file_path).c_str(), desired_access, share_mode, nullptr, creation_disposition, flags_and_attributes, nullptr);
  else
    h_file = CreateFileW(long_path(file_path).c_str(), desired_access, share_mode, nullptr, creation_disposition, flags_and_attributes, nullptr);
  return h_file != INVALID_HANDLE_VALUE;
}

void File::close() {
  if (h_file != INVALID_HANDLE_VALUE) {
    CloseHandle(h_file);
    h_file = INVALID_HANDLE_VALUE;
  }
}

unsigned __int64 File::size() {
  unsigned __int64 file_size;
  CHECK_FILE(size_nt(file_size));
  return file_size;
}

bool File::size_nt(unsigned __int64& file_size) {
  LARGE_INTEGER fs;
  if (GetFileSizeEx(h_file, &fs)) {
    file_size = fs.QuadPart;
    return true;
  }
  else
    return false;
}

unsigned File::read(void* data, unsigned size) {
  unsigned size_read;
  CHECK_FILE(read_nt(data, size, size_read));
  return size_read;
}

bool File::read_nt(void* data, unsigned size, unsigned& size_read) {
  DWORD sz;
  if (ReadFile(h_file, data, size, &sz, nullptr)) {
    size_read = sz;
    return true;
  }
  else
    return false;
}

unsigned File::write(const void* data, unsigned size) {
  unsigned size_written;
  CHECK_FILE(write_nt(data, size, size_written));
  return size_written;
}

bool File::write_nt(const void* data, unsigned size, unsigned& size_written) {
  DWORD sz;
  if (WriteFile(h_file, data, size, &sz, nullptr)) {
    size_written = sz;
    return true;
  }
  else
    return false;
}

void File::set_time(const FILETIME& ctime, const FILETIME& atime, const FILETIME& mtime) {
  CHECK_FILE(set_time_nt(ctime, atime, mtime));
};

bool File::set_time_nt(const FILETIME& ctime, const FILETIME& atime, const FILETIME& mtime) {
  return SetFileTime(h_file, &ctime, &atime, &mtime) != 0;
};

unsigned __int64 File::set_pos(__int64 offset, DWORD method) {
  unsigned __int64 new_pos;
  CHECK_FILE(set_pos_nt(offset, method, &new_pos));
  return new_pos;
}

bool File::set_pos_nt(__int64 offset, DWORD method, unsigned __int64* new_pos) {
  LARGE_INTEGER distance_to_move, new_file_pointer;
  distance_to_move.QuadPart = offset;
  if (!SetFilePointerEx(h_file, distance_to_move, &new_file_pointer, method))
    return false;
  if (new_pos)
    *new_pos = new_file_pointer.QuadPart;
  return true;
}

void File::set_end() {
  CHECK_FILE(set_end_nt());
}

bool File::set_end_nt() {
  return SetEndOfFile(h_file) != 0;
}

BY_HANDLE_FILE_INFORMATION File::get_info() {
  BY_HANDLE_FILE_INFORMATION info;
  CHECK_FILE(get_info_nt(info));
  return info;
}

bool File::get_info_nt(BY_HANDLE_FILE_INFORMATION& info) {
  return GetFileInformationByHandle(h_file, &info) != 0;
}

bool File::exists(const wstring& file_path) {
  ArclitePrivateInfo* system_functions = Far::get_system_functions();
  if (system_functions)
    return system_functions->GetFileAttributes(long_path(file_path).c_str()) != INVALID_FILE_ATTRIBUTES;
  else
    return GetFileAttributesW(long_path(file_path).c_str()) != INVALID_FILE_ATTRIBUTES;
}

void File::set_attr(const wstring& file_path, DWORD attr) {
  CHECK_FILE(set_attr_nt(file_path, attr));
}

bool File::set_attr_nt(const wstring& file_path, DWORD attr) {
  ArclitePrivateInfo* system_functions = Far::get_system_functions();
  if (system_functions)
    return system_functions->SetFileAttributes(long_path(file_path).c_str(), attr) != 0;
  else
    return SetFileAttributesW(long_path(file_path).c_str(), attr) != 0;
}

void File::delete_file(const wstring& file_path) {
  CHECK_FILE(delete_file_nt(file_path));
}

bool File::delete_file_nt(const wstring& file_path) {
  ArclitePrivateInfo* system_functions = Far::get_system_functions();
  if (system_functions)
    return system_functions->DeleteFile(long_path(file_path).c_str()) != 0;
  else
    return DeleteFileW(long_path(file_path).c_str()) != 0;
}

void File::create_dir(const wstring& file_path) {
  CHECK_FILE(create_dir_nt(file_path));
}

bool File::create_dir_nt(const wstring& file_path) {
  ArclitePrivateInfo* system_functions = Far::get_system_functions();
  if (system_functions)
    return system_functions->CreateDirectory(long_path(file_path).c_str(), nullptr) != 0;
  else
    return CreateDirectoryW(long_path(file_path).c_str(), nullptr) != 0;
}

void File::remove_dir(const wstring& file_path) {
  CHECK_FILE(remove_dir_nt(file_path));
}

bool File::remove_dir_nt(const wstring& file_path) {
  ArclitePrivateInfo* system_functions = Far::get_system_functions();
  if (system_functions)
    return system_functions->RemoveDirectory(long_path(file_path).c_str()) != 0;
  else
    return RemoveDirectoryW(long_path(file_path).c_str()) != 0;
}

void File::move_file(const wstring& file_path, const wstring& new_path, DWORD flags) {
  CHECK_FILE(move_file_nt(file_path, new_path, flags));
}

bool File::move_file_nt(const wstring& file_path, const wstring& new_path, DWORD flags) {
  ArclitePrivateInfo* system_functions = Far::get_system_functions();
  if (system_functions)
    return system_functions->MoveFileEx(long_path(file_path).c_str(), long_path(new_path).c_str(), flags) != 0;
  else
    return MoveFileExW(long_path(file_path).c_str(), long_path(new_path).c_str(), flags) != 0;
}

FindData File::get_find_data(const wstring& file_path) {
  FindData find_data;
  CHECK_FILE(get_find_data_nt(file_path, find_data));
  return find_data;
}

bool File::get_find_data_nt(const wstring& file_path, FindData& find_data) {
  HANDLE h_find = FindFirstFileW(long_path(file_path).c_str(), &find_data);
  if (h_find != INVALID_HANDLE_VALUE) {
    FindClose(h_find);
    return true;
  }
  else
    return false;
}

#undef CHECK_FILE


void Key::close() {
  if (h_key) {
    RegCloseKey(h_key);
    h_key = nullptr;
  }
}

Key::Key(): h_key(nullptr) {
}

Key::~Key() {
  close();
}

Key::Key(HKEY h_parent, LPCWSTR sub_key, REGSAM sam_desired, bool create) {
  open(h_parent, sub_key, sam_desired, create);
}

Key& Key::open(HKEY h_parent, LPCWSTR sub_key, REGSAM sam_desired, bool create) {
  close();
  CHECK_SYS(open_nt(h_parent, sub_key, sam_desired, create));
  return *this;
}

bool Key::open_nt(HKEY h_parent, LPCWSTR sub_key, REGSAM sam_desired, bool create) {
  close();
  LONG res;
  if (create)
    res = RegCreateKeyExW(h_parent, sub_key, 0, nullptr, REG_OPTION_NON_VOLATILE, sam_desired, nullptr, &h_key, nullptr);
  else
    res = RegOpenKeyExW(h_parent, sub_key, 0, sam_desired, &h_key);
  if (res != ERROR_SUCCESS) {
    SetLastError(res);
    return false;
  }
  return true;
}

bool Key::query_bool(const wchar_t* name) {
  bool value;
  CHECK_SYS(query_bool_nt(value, name));
  return value;
}

bool Key::query_bool_nt(bool& value, const wchar_t* name) {
  DWORD type = REG_DWORD;
  DWORD data;
  DWORD data_size = sizeof(data);
  LONG res = RegQueryValueExW(h_key, name, nullptr, &type, reinterpret_cast<LPBYTE>(&data), &data_size);
  if (res != ERROR_SUCCESS) {
    SetLastError(res);
    return false;
  }
  value = data != 0;
  return true;
}

unsigned Key::query_int(const wchar_t* name) {
  unsigned value;
  CHECK_SYS(query_int_nt(value, name));
  return value;
}

bool Key::query_int_nt(unsigned& value, const wchar_t* name) {
  DWORD type = REG_DWORD;
  DWORD data;
  DWORD data_size = sizeof(data);
  LONG res = RegQueryValueExW(h_key, name, nullptr, &type, reinterpret_cast<LPBYTE>(&data), &data_size);
  if (res != ERROR_SUCCESS) {
    SetLastError(res);
    return false;
  }
  value = data;
  return true;
}

wstring Key::query_str(const wchar_t* name) {
  wstring value;
  CHECK_SYS(query_str_nt(value, name));
  return value;
}

bool Key::query_str_nt(wstring& value, const wchar_t* name) {
  DWORD type = REG_SZ;
  DWORD data_size;
  LONG res = RegQueryValueExW(h_key, name, nullptr, &type, nullptr, &data_size);
  if (res != ERROR_SUCCESS) {
    SetLastError(res);
    return false;
  }
  Buffer<wchar_t> buf(data_size / sizeof(wchar_t));
  res = RegQueryValueExW(h_key, name, nullptr, &type, reinterpret_cast<LPBYTE>(buf.data()), &data_size);
  if (res != ERROR_SUCCESS) {
    SetLastError(res);
    return false;
  }
  value.assign(buf.data(), buf.size() - 1);
  return true;
}

ByteVector Key::query_binary(const wchar_t* name) {
  ByteVector value;
  CHECK_SYS(query_binary_nt(value, name));
  return value;
}

bool Key::query_binary_nt(ByteVector& value, const wchar_t* name) {
  DWORD type = REG_BINARY;
  DWORD data_size;
  LONG res = RegQueryValueExW(h_key, name, nullptr, &type, nullptr, &data_size);
  if (res != ERROR_SUCCESS) {
    SetLastError(res);
    return false;
  }
  Buffer<unsigned char> buf(data_size);
  res = RegQueryValueExW(h_key, name, nullptr, &type, buf.data(), &data_size);
  if (res != ERROR_SUCCESS) {
    SetLastError(res);
    return false;
  }
  value.assign(buf.data(), buf.data() + data_size);
  return true;
}

void Key::set_bool(const wchar_t* name, bool value) {
  CHECK_SYS(set_bool_nt(name, value));
}

bool Key::set_bool_nt(const wchar_t* name, bool value) {
  DWORD data = value ? 1 : 0;
  LONG res = RegSetValueExW(h_key, name, 0, REG_DWORD, reinterpret_cast<LPBYTE>(&data), sizeof(data));
  if (res != ERROR_SUCCESS) {
    SetLastError(res);
    return false;
  }
  return true;
}

void Key::set_int(const wchar_t* name, unsigned value) {
  CHECK_SYS(set_int_nt(name, value));
}

bool Key::set_int_nt(const wchar_t* name, unsigned value) {
  DWORD data = value;
  LONG res = RegSetValueExW(h_key, name, 0, REG_DWORD, reinterpret_cast<LPBYTE>(&data), sizeof(data));
  if (res != ERROR_SUCCESS) {
    SetLastError(res);
    return false;
  }
  return true;
}

void Key::set_str(const wchar_t* name, const wstring& value) {
  CHECK_SYS(set_str_nt(name, value));
}

bool Key::set_str_nt(const wchar_t* name, const wstring& value) {
  LONG res = RegSetValueExW(h_key, name, 0, REG_SZ, reinterpret_cast<LPBYTE>(const_cast<wchar_t*>(value.c_str())), (static_cast<DWORD>(value.size()) + 1) * sizeof(wchar_t));
  if (res != ERROR_SUCCESS) {
    SetLastError(res);
    return false;
  }
  return true;
}

void Key::set_binary(const wchar_t* name, const unsigned char* value, unsigned size) {
  CHECK_SYS(set_binary_nt(name, value, size));
}

bool Key::set_binary_nt(const wchar_t* name, const unsigned char* value, unsigned size) {
  LONG res = RegSetValueExW(h_key, name, 0, REG_BINARY, value, size);
  if (res != ERROR_SUCCESS) {
    SetLastError(res);
    return false;
  }
  return true;
}

void Key::delete_value(const wchar_t* name) {
  CHECK_SYS(delete_value_nt(name));
}

bool Key::delete_value_nt(const wchar_t* name) {
  LONG res = RegDeleteValueW(h_key, name);
  if (res != ERROR_SUCCESS) {
    SetLastError(res);
    return false;
  }
  return true;
}

vector<wstring> Key::enum_sub_keys() {
  vector<wstring> names;
  CHECK_SYS(enum_sub_keys_nt(names));
  return names;
}

bool Key::enum_sub_keys_nt(vector<wstring>& names) {
  DWORD index = 0;
  const unsigned c_key_name_size = 256;
  Buffer<wchar_t> name(c_key_name_size);
  while (true) {
    DWORD name_size = static_cast<DWORD>(name.size());
    LONG res = RegEnumKeyExW(h_key, index, name.data(), &name_size, nullptr, nullptr, nullptr, nullptr);
    if (res == ERROR_NO_MORE_ITEMS)
      break;
    if (res == ERROR_MORE_DATA) {
      name.resize(name.size() * 2);
      continue;
    }
    if (res != ERROR_SUCCESS) {
      SetLastError(res);
      return false;
    }
    names.push_back(wstring(name.data(), name_size));
    index++;
  }
  return true;
}

void Key::delete_sub_key(const wchar_t* name) {
  CHECK_SYS(delete_sub_key_nt(name));
}

bool Key::delete_sub_key_nt(const wchar_t* name) {
  LONG res = RegDeleteKeyW(h_key, name);
  if (res != ERROR_SUCCESS) {
    SetLastError(res);
    return false;
  }
  return true;
}

FileEnum::FileEnum(const wstring& file_mask): file_mask(file_mask), h_find(INVALID_HANDLE_VALUE) {
}

FileEnum::~FileEnum() {
  if (h_find != INVALID_HANDLE_VALUE)
    FindClose(h_find);
}

bool FileEnum::next() {
  bool more;
  if (!next_nt(more))
    throw Error(HRESULT_FROM_WIN32(GetLastError()), file_mask, __FILE__, __LINE__);
  return more;
}

bool FileEnum::next_nt(bool& more) {
  while (true) {
    if (h_find == INVALID_HANDLE_VALUE) {
      h_find = FindFirstFileW(long_path(file_mask).c_str(), &find_data);
      if (h_find == INVALID_HANDLE_VALUE)
        return false;
    }
    else {
      if (!FindNextFileW(h_find, &find_data)) {
        if (GetLastError() == ERROR_NO_MORE_FILES) {
          more = false;
          return true;
        }
        return false;
      }
    }
    if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
      if ((find_data.cFileName[0] == L'.') && ((find_data.cFileName[1] == 0) || ((find_data.cFileName[1] == L'.') && (find_data.cFileName[2] == 0))))
        continue;
    }
    more = true;
    return true;
  }
}

DirList::DirList(const wstring& dir_path): FileEnum(add_trailing_slash(dir_path) + L'*') {
}

wstring get_temp_path() {
  Buffer<wchar_t> buf(MAX_PATH);
  DWORD len = GetTempPathW(static_cast<DWORD>(buf.size()), buf.data());
  CHECK(len <= buf.size());
  CHECK_SYS(len);
  return wstring(buf.data(), len);
}

TempFile::TempFile() {
  Buffer<wchar_t> buf(MAX_PATH);
  wstring temp_path = get_temp_path();
  CHECK_SYS(GetTempFileNameW(temp_path.c_str(), L"", 0, buf.data()));
  path.assign(buf.data());
}

TempFile::~TempFile() {
  DeleteFileW(path.c_str());
}

wstring format_file_time(const FILETIME& file_time) {
  FILETIME local_ft;
  CHECK_SYS(FileTimeToLocalFileTime(&file_time, &local_ft));
  SYSTEMTIME st;
  CHECK_SYS(FileTimeToSystemTime(&local_ft, &st));
  Buffer<wchar_t> buf(1024);
  CHECK_SYS(GetDateFormatW(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &st, nullptr, buf.data(), static_cast<int>(buf.size())));
  wstring date_str = buf.data();
  CHECK_SYS(GetTimeFormatW(LOCALE_USER_DEFAULT, 0, &st, nullptr, buf.data(), static_cast<int>(buf.size())));
  wstring time_str = buf.data();
  return date_str + L' ' + time_str;
}

wstring upcase(const wstring& str) {
  Buffer<wchar_t> up_str(str.size());
  wmemcpy(up_str.data(), str.data(), str.size());
  CharUpperBuffW(up_str.data(), static_cast<DWORD>(up_str.size()));
  return wstring(up_str.data(), up_str.size());
}

wstring create_guid() {
  GUID guid;
  CHECK_COM(CoCreateGuid(&guid));
  wchar_t guid_str[50];
  CHECK(StringFromGUID2(guid, guid_str, ARRAYSIZE(guid_str)));
  return guid_str;
}

void enable_lfh() {
  ULONG heap_info = 2;
  HeapSetInformation(reinterpret_cast<HANDLE>(_get_heap_handle()), HeapCompatibilityInformation, &heap_info, sizeof(heap_info));
}

wstring search_path(const wstring& file_name) {
  Buffer<wchar_t> path(MAX_PATH);
  wchar_t* name_ptr;
  DWORD size = SearchPathW(nullptr, file_name.c_str(), nullptr, path.size(), path.data(), &name_ptr);
  if (size > path.size()) {
    path.resize(size);
    size = SearchPathW(nullptr, file_name.c_str(), nullptr, path.size(), path.data(), &name_ptr);
  }
  CHECK_SYS(size);
  CHECK(size < path.size());
  return wstring(path.data(), size);
}
