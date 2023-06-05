#include "utils.hpp"
#include "farutils.hpp"
#include "sysutils.hpp"

#ifdef _MSC_VER
# pragma warning (disable : 4996)
#endif

HINSTANCE g_h_instance = nullptr;

CriticalSection& GetSync()
{
  static CriticalSection sync;
  return sync;
}

CriticalSection& GetExportSync()
{
  static CriticalSection sync;
  return sync;
}

std::wstring get_system_message(HRESULT hr, DWORD lang_id) {
  std::wostringstream st;
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
    std::wstring message;
    try {
      message = sys_msg;
    }
    catch (...) {
      LocalFree(static_cast<HLOCAL>(sys_msg));
      throw;
    }
    LocalFree(static_cast<HLOCAL>(sys_msg));
    st << strip(message) << L" (0x" << std::hex << std::uppercase << std::setw(8) << std::setfill(L'0') << hr << L")";
  }
  else {
    st << L"HRESULT: 0x" << std::hex << std::uppercase << std::setw(8) << std::setfill(L'0') << hr;
  }
  return st.str();
}

std::wstring get_console_title() {
  Buffer<wchar_t> buf(10000);
  DWORD size = GetConsoleTitleW(buf.data(), static_cast<DWORD>(buf.size()));
  return std::wstring(buf.data(), size);
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

std::wstring ansi_to_unicode(const std::string& str, unsigned code_page) {
  unsigned str_size = static_cast<unsigned>(str.size());
  if (str_size == 0)
    return std::wstring();
  int size = MultiByteToWideChar(code_page, 0, str.data(), str_size, nullptr, 0);
  Buffer<wchar_t> out(size);
  size = MultiByteToWideChar(code_page, 0, str.data(), str_size, out.data(), size);
  CHECK_SYS(size);
  return std::wstring(out.data(), size);
}

std::string unicode_to_ansi(const std::wstring& str, unsigned code_page) {
  unsigned str_size = static_cast<unsigned>(str.size());
  if (str_size == 0)
    return std::string();
  int size = WideCharToMultiByte(code_page, 0, str.data(), str_size, nullptr, 0, nullptr, nullptr);
  Buffer<char> out(size);
  size = WideCharToMultiByte(code_page, 0, str.data(), str_size, out.data(), size, nullptr, nullptr);
  CHECK_SYS(size);
  return std::string(out.data(), size);
}

std::wstring expand_env_vars(const std::wstring& str) {
  Buffer<wchar_t> buf(MAX_PATH);
  unsigned size = ExpandEnvironmentStringsW(str.c_str(), buf.data(), static_cast<DWORD>(buf.size()));
  if (size > buf.size()) {
    buf.resize(size);
    size = ExpandEnvironmentStringsW(str.c_str(), buf.data(), static_cast<DWORD>(buf.size()));
  }
  CHECK_SYS(size);
  return std::wstring(buf.data(), size - 1);
}

std::wstring get_full_path_name(const std::wstring& path) {
  Buffer<wchar_t> buf(MAX_PATH);
  DWORD size = GetFullPathNameW(path.c_str(), static_cast<DWORD>(buf.size()), buf.data(), nullptr);
  if (size > buf.size()) {
    buf.resize(size);
    size = GetFullPathNameW(path.c_str(), static_cast<DWORD>(buf.size()), buf.data(), nullptr);
  }
  CHECK_SYS(size);
  return std::wstring(buf.data(), size);
}

std::wstring get_current_directory() {
  Buffer<wchar_t> buf(MAX_PATH);
  DWORD size = GetCurrentDirectoryW(static_cast<DWORD>(buf.size()), buf.data());
  if (size > buf.size()) {
    buf.resize(size);
    size = GetCurrentDirectoryW(static_cast<DWORD>(buf.size()), buf.data());
  }
  CHECK_SYS(size);
  return std::wstring(buf.data(), size);
}


#define CHECK_FILE(code, path) do { if (!(code)) throw Error(HRESULT_FROM_WIN32(GetLastError()), path, __FILE__, __LINE__); } while(false)

File::File() noexcept : h_file(INVALID_HANDLE_VALUE) {
}

File::~File() noexcept {
  close();
}

File::File(const std::wstring& file_path, DWORD desired_access, DWORD share_mode, DWORD creation_disposition, DWORD flags_and_attributes): h_file(INVALID_HANDLE_VALUE) {
  open(file_path, desired_access, share_mode, creation_disposition, flags_and_attributes);
}

void File::open(const std::wstring& file_path, DWORD desired_access, DWORD share_mode, DWORD creation_disposition, DWORD flags_and_attributes) {
  CHECK_FILE(open_nt(file_path, desired_access, share_mode, creation_disposition, flags_and_attributes), file_path);
}

bool File::open_nt(const std::wstring& file_path, DWORD desired_access, DWORD share_mode, DWORD creation_disposition, DWORD flags_and_attributes) noexcept {
  close();
  m_file_path = file_path;
  const auto system_functions = Far::get_system_functions();
  if (system_functions) {
    h_file = system_functions->CreateFile(long_path_norm(file_path).c_str(), desired_access, share_mode, nullptr, creation_disposition, flags_and_attributes, nullptr);
    if (h_file == nullptr)
      h_file = INVALID_HANDLE_VALUE;
  }
  else
    h_file = CreateFileW(long_path_norm(file_path).c_str(), desired_access, share_mode, nullptr, creation_disposition, flags_and_attributes, nullptr);
  return h_file != INVALID_HANDLE_VALUE;
}

void File::close() noexcept {
  if (h_file != INVALID_HANDLE_VALUE) {
    CloseHandle(h_file);
    h_file = INVALID_HANDLE_VALUE;
  }
}

uint64_t File::size() {
  uint64_t file_size;
  CHECK_FILE(size_nt(file_size), m_file_path);
  return file_size;
}

bool File::size_nt(uint64_t& file_size) noexcept {
  LARGE_INTEGER fs;
  if (GetFileSizeEx(h_file, &fs)) {
    file_size = fs.QuadPart;
    return true;
  }
  else
    return false;
}

size_t File::read(void* data, size_t size) {
  size_t size_read;
  CHECK_FILE(read_nt(data, size, size_read), m_file_path);
  return size_read;
}

bool File::read_nt(void* data, size_t size, size_t& size_read) noexcept {
  DWORD sz;
  if (ReadFile(h_file, data, static_cast<DWORD>(size), &sz, nullptr)) {
    size_read = sz;
    return true;
  }
  else
    return false;
}

size_t File::write(const void* data, size_t size) {
  size_t size_written;
  CHECK_FILE(write_nt(data, size, size_written), m_file_path);
  return size_written;
}

bool File::write_nt(const void* data, size_t size, size_t& size_written) noexcept {
  DWORD sz;
  if (WriteFile(h_file, data, static_cast<DWORD>(size), &sz, nullptr)) {
    size_written = sz;
    return true;
  }
  else
    return false;
}

void File::set_time(const FILETIME& ctime, const FILETIME& atime, const FILETIME& mtime) {
  CHECK_FILE(set_time_nt(ctime, atime, mtime), m_file_path);
};

bool File::set_time_nt(const FILETIME& ctime, const FILETIME& atime, const FILETIME& mtime) noexcept {
  return SetFileTime(h_file, &ctime, &atime, &mtime) != 0;
};

bool File::copy_ctime_from(const std::wstring& source_file) noexcept
{
  WIN32_FILE_ATTRIBUTE_DATA fa;
  if (!attributes_ex(source_file, &fa))
    return false;
  FILETIME dummy{};
  return set_time_nt(fa.ftCreationTime, dummy, dummy);
}

uint64_t File::set_pos(int64_t offset, DWORD method) {
  uint64_t new_pos;
  CHECK_FILE(set_pos_nt(offset, method, &new_pos), m_file_path);
  return new_pos;
}

bool File::set_pos_nt(int64_t offset, DWORD method, uint64_t* new_pos) noexcept {
  LARGE_INTEGER distance_to_move, new_file_pointer;
  distance_to_move.QuadPart = offset;
  if (!SetFilePointerEx(h_file, distance_to_move, &new_file_pointer, method))
    return false;
  if (new_pos)
    *new_pos = new_file_pointer.QuadPart;
  return true;
}

void File::set_end() {
  CHECK_FILE(set_end_nt(), m_file_path);
}

bool File::set_end_nt() noexcept {
  return SetEndOfFile(h_file) != 0;
}

BY_HANDLE_FILE_INFORMATION File::get_info() {
  BY_HANDLE_FILE_INFORMATION info;
  CHECK_FILE(get_info_nt(info), m_file_path);
  return info;
}

bool File::get_info_nt(BY_HANDLE_FILE_INFORMATION& info) noexcept {
  return GetFileInformationByHandle(h_file, &info) != 0;
}

bool File::exists(const std::wstring& file_path) noexcept {
  return attributes(file_path) != INVALID_FILE_ATTRIBUTES;
}

DWORD File::attributes(const std::wstring& file_path) noexcept {
  const auto system_functions = Far::get_system_functions();
  if (system_functions)
    return system_functions->GetFileAttributes(long_path_norm(file_path).c_str());
  else
    return GetFileAttributesW(long_path_norm(file_path).c_str());
}

bool File::attributes_ex(const std::wstring& file_path, WIN32_FILE_ATTRIBUTE_DATA* ex_attrs) noexcept
{
  static int have_attributes_ex = 0;
  static BOOL (WINAPI *pfGetFileAttributesExW)(LPCWSTR pname, GET_FILEEX_INFO_LEVELS level, LPVOID pinfo) = nullptr;
  if (have_attributes_ex == 0) {
    auto pf = GetProcAddress(GetModuleHandleW(L"kernel32"), "GetFileAttributesExW");
    if (pf == nullptr)
      have_attributes_ex = -1;
    else {
      pfGetFileAttributesExW = reinterpret_cast<decltype(pfGetFileAttributesExW)>(reinterpret_cast<void*>(pf));
      have_attributes_ex = +1;
    }
  }

  auto norm_path = long_path_norm(file_path);
  if (have_attributes_ex > 0) {
    return pfGetFileAttributesExW(norm_path.c_str(), GetFileExInfoStandard, ex_attrs) != FALSE;
  }
  else {
    WIN32_FIND_DATAW ff;
    auto hfind = FindFirstFileW(norm_path.c_str(), &ff);
    if (hfind == INVALID_HANDLE_VALUE)
      return false;
    FindClose(hfind);
    ex_attrs->dwFileAttributes = ff.dwFileAttributes;
    ex_attrs->ftCreationTime   = ff.ftCreationTime;
    ex_attrs->ftLastWriteTime  = ff.ftLastWriteTime;
    ex_attrs->ftLastAccessTime = ff.ftLastAccessTime;
    ex_attrs->nFileSizeLow     = ff.nFileSizeLow;
    ex_attrs->nFileSizeHigh    = ff.nFileSizeHigh;
	 return true;
  }
}

void File::set_attr(const std::wstring& file_path, DWORD attr) {
  CHECK_FILE(set_attr_nt(file_path, attr), file_path);
}

bool File::set_attr_nt(const std::wstring& file_path, DWORD attr) noexcept {
  const auto system_functions = Far::get_system_functions();
  if (system_functions)
    return system_functions->SetFileAttributes(long_path_norm(file_path).c_str(), attr) != 0;
  else
    return SetFileAttributesW(long_path_norm(file_path).c_str(), attr) != 0;
}

void File::delete_file(const std::wstring& file_path) {
  CHECK_FILE(delete_file_nt(file_path), file_path);
}

bool File::delete_file_nt(const std::wstring& file_path) noexcept {
  const auto system_functions = Far::get_system_functions();
  if (system_functions)
    return system_functions->DeleteFile(long_path_norm(file_path).c_str()) != 0;
  else
    return DeleteFileW(long_path_norm(file_path).c_str()) != 0;
}

void File::create_dir(const std::wstring& file_path) {
  CHECK_FILE(create_dir_nt(file_path), file_path);
}

bool File::create_dir_nt(const std::wstring& file_path) noexcept {
  const auto system_functions = Far::get_system_functions();
  if (system_functions)
    return system_functions->CreateDirectory(long_path_norm(file_path).c_str(), nullptr) != 0;
  else
    return CreateDirectoryW(long_path_norm(file_path).c_str(), nullptr) != 0;
}

void File::remove_dir(const std::wstring& file_path) {
  CHECK_FILE(remove_dir_nt(file_path), file_path);
}

bool File::remove_dir_nt(const std::wstring& file_path) noexcept {
  const auto system_functions = Far::get_system_functions();
  if (system_functions)
    return system_functions->RemoveDirectory(long_path_norm(file_path).c_str()) != 0;
  else
    return RemoveDirectoryW(long_path_norm(file_path).c_str()) != 0;
}

void File::move_file(const std::wstring& file_path, const std::wstring& new_path, DWORD flags) {
  CHECK_FILE(move_file_nt(file_path, new_path, flags), file_path);
}

bool File::move_file_nt(const std::wstring& file_path, const std::wstring& new_path, DWORD flags) noexcept {
  const auto system_functions = Far::get_system_functions();
  if (system_functions)
    return system_functions->MoveFileEx(long_path_norm(file_path).c_str(), long_path_norm(new_path).c_str(), flags) != 0;
  else
    return MoveFileExW(long_path_norm(file_path).c_str(), long_path_norm(new_path).c_str(), flags) != 0;
}

FindData File::get_find_data(const std::wstring& file_path) {
  FindData find_data;
  CHECK_FILE(get_find_data_nt(file_path, find_data), file_path);
  return find_data;
}

bool File::get_find_data_nt(const std::wstring& file_path, FindData& find_data) noexcept {
  HANDLE h_find = FindFirstFileW(long_path_norm(file_path).c_str(), &find_data);
  if (h_find != INVALID_HANDLE_VALUE) {
    FindClose(h_find);
    return true;
  }
  else
    return false;
}

#undef CHECK_FILE


void Key::close() noexcept {
  if (h_key) {
    RegCloseKey(h_key);
    h_key = nullptr;
  }
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

bool Key::open_nt(HKEY h_parent, LPCWSTR sub_key, REGSAM sam_desired, bool create) noexcept {
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

bool Key::query_bool_nt(bool& value, const wchar_t* name) noexcept {
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

bool Key::query_int_nt(unsigned& value, const wchar_t* name) noexcept {
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

std::wstring Key::query_str(const wchar_t* name) {
  std::wstring value;
  CHECK_SYS(query_str_nt(value, name));
  return value;
}

bool Key::query_str_nt(std::wstring& value, const wchar_t* name) noexcept {
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

bool Key::query_binary_nt(ByteVector& value, const wchar_t* name) noexcept {
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

bool Key::set_bool_nt(const wchar_t* name, bool value) noexcept {
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

bool Key::set_int_nt(const wchar_t* name, unsigned value) noexcept {
  DWORD data = value;
  LONG res = RegSetValueExW(h_key, name, 0, REG_DWORD, reinterpret_cast<LPBYTE>(&data), sizeof(data));
  if (res != ERROR_SUCCESS) {
    SetLastError(res);
    return false;
  }
  return true;
}

void Key::set_str(const wchar_t* name, const std::wstring& value) {
  CHECK_SYS(set_str_nt(name, value));
}

bool Key::set_str_nt(const wchar_t* name, const std::wstring& value) noexcept {
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

bool Key::set_binary_nt(const wchar_t* name, const unsigned char* value, unsigned size) noexcept {
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

bool Key::delete_value_nt(const wchar_t* name) noexcept {
  LONG res = RegDeleteValueW(h_key, name);
  if (res != ERROR_SUCCESS) {
    SetLastError(res);
    return false;
  }
  return true;
}

std::vector<std::wstring> Key::enum_sub_keys() {
  std::vector<std::wstring> names;
  CHECK_SYS(enum_sub_keys_nt(names));
  return names;
}

bool Key::enum_sub_keys_nt(std::vector<std::wstring>& names) noexcept {
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
    names.emplace_back(name.data(), name_size);
    index++;
  }
  return true;
}

void Key::delete_sub_key(const wchar_t* name) {
  CHECK_SYS(delete_sub_key_nt(name));
}

bool Key::delete_sub_key_nt(const wchar_t* name) noexcept {
  LONG res = RegDeleteKeyW(h_key, name);
  if (res != ERROR_SUCCESS) {
    SetLastError(res);
    return false;
  }
  return true;
}

FileEnum::FileEnum(const std::wstring& file_mask) noexcept : file_mask(file_mask), h_find(INVALID_HANDLE_VALUE) {
  n_far_items = -1;
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

static int WINAPI find_cb(const struct PluginPanelItem *FData, const wchar_t *FullName, void *Param)
{
  (void)FullName;
  return ((FileEnum *)Param)->far_emum_cb(*FData);
}

#ifndef TOOLS_TOOL
int FileEnum::far_emum_cb(const PluginPanelItem& item)
{
  far_items.emplace_back();
  auto& fdata = far_items.back();

  fdata.dwFileAttributes = static_cast<DWORD>(item.FileAttributes & 0xFFFFFFFF);
  fdata.ftCreationTime = item.CreationTime;
  fdata.ftLastAccessTime = item.LastAccessTime;
  fdata.ftLastWriteTime = item.LastWriteTime;
  fdata.nFileSizeHigh = static_cast<DWORD>((item.FileSize >> 32) & 0xFFFFFFFF);
  fdata.nFileSizeLow = static_cast<DWORD>(item.FileSize & 0xFFFFFFFF);
  fdata.dwReserved0 = static_cast<DWORD>(item.Reserved[0]);
  fdata.dwReserved1 = static_cast<DWORD>(item.Reserved[1]);
  std::wcscpy(fdata.cAlternateFileName, null_to_empty(item.AlternateFileName));
  std::wcsncpy(fdata.cFileName, null_to_empty(item.FileName), sizeof(fdata.cFileName)/sizeof(fdata.cFileName[0]));

  ++n_far_items;
  return TRUE;
}
#endif

bool FileEnum::next_nt(bool& more) noexcept {
  for (;;) {
    if (h_find == INVALID_HANDLE_VALUE) {
      if (n_far_items >= 0) {
        more = (n_far_items > 0);
        if (!more)
          return true;
        find_data = far_items.front();
        far_items.pop_front();
        --n_far_items;
      }
      else {
        if ((h_find = FindFirstFileW(long_path(file_mask).c_str(), &find_data)) == INVALID_HANDLE_VALUE) {
          if (GetLastError() == ERROR_ACCESS_DENIED) { //
            auto dir = extract_file_path(file_mask);   // M$ FindFirst/NextFile doesn't work for junction/symlink folder.
            auto msk = extract_file_name(file_mask);   // Try to use FarRecursiveSearch in such case.
            if (!dir.empty() && !msk.empty()) {        //
              auto attr = File::attributes(dir);       //
#ifndef TOOLS_TOOL
              if (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY) && (attr & FILE_ATTRIBUTE_REPARSE_POINT)) {
                n_far_items = 0;
                Far::g_fsf.FarRecursiveSearch(long_path(dir).c_str(), msk.c_str(), find_cb, FRS_NONE, this);
                continue;
              }
#endif
            }
          }
          more = false;
          return false;
        }
      }
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
    auto mask_dot_pos = file_mask.find_last_of(L'.'); // avoid found "name.ext_" using mask "*.ext"
    if (mask_dot_pos != std::wstring::npos && file_mask.find_first_of(L'*', mask_dot_pos) == std::wstring::npos) {
      const auto last_dot_in_fname = std::wcsrchr(find_data.cFileName, L'.');
      if (nullptr != last_dot_in_fname && std::wcslen(last_dot_in_fname) > file_mask.size() - mask_dot_pos)
        continue;
    }
    more = true;
    return true;
  }
}

DirList::DirList(const std::wstring& dir_path) noexcept : FileEnum(add_trailing_slash(dir_path) + L'*') {
}

std::wstring get_temp_path() {
  Buffer<wchar_t> buf(MAX_PATH);
  DWORD len = GetTempPathW(static_cast<DWORD>(buf.size()), buf.data());
  CHECK(len <= buf.size());
  CHECK_SYS(len);
  return std::wstring(buf.data(), len);
}

TempFile::TempFile() {
  Buffer<wchar_t> buf(MAX_PATH);
  std::wstring temp_path = get_temp_path();
  CHECK_SYS(GetTempFileNameW(temp_path.c_str(), L"", 0, buf.data()));
  path.assign(buf.data());
}

TempFile::~TempFile() {
  DeleteFileW(path.c_str());
}

std::wstring format_file_time(const FILETIME& file_time) {
  FILETIME local_ft;
  CHECK_SYS(FileTimeToLocalFileTime(&file_time, &local_ft));
  SYSTEMTIME st;
  CHECK_SYS(FileTimeToSystemTime(&local_ft, &st));
  Buffer<wchar_t> buf(1024);
  CHECK_SYS(GetDateFormatW(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &st, nullptr, buf.data(), static_cast<int>(buf.size())));
  std::wstring date_str = buf.data();
  CHECK_SYS(GetTimeFormatW(LOCALE_USER_DEFAULT, 0, &st, nullptr, buf.data(), static_cast<int>(buf.size())));
  std::wstring time_str = buf.data();
  return date_str + L' ' + time_str;
}

std::wstring upcase(const std::wstring& str) {
  Buffer<wchar_t> up_str(str.size());
  std::wmemcpy(up_str.data(), str.data(), str.size());
  CharUpperBuffW(up_str.data(), static_cast<DWORD>(up_str.size()));
  return std::wstring(up_str.data(), up_str.size());
}

std::wstring create_guid() {
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

std::wstring search_path(const std::wstring& file_name) {
  Buffer<wchar_t> path(MAX_PATH);
  wchar_t* name_ptr;
  DWORD size = SearchPathW(nullptr, file_name.c_str(), nullptr, static_cast<DWORD>(path.size()), path.data(), &name_ptr);
  if (size > path.size()) {
    path.resize(size);
    size = SearchPathW(nullptr, file_name.c_str(), nullptr, static_cast<DWORD>(path.size()), path.data(), &name_ptr);
  }
  CHECK_SYS(size);
  CHECK(size < path.size());
  return std::wstring(path.data(), size);
}

std::pair<DWORD, DWORD> get_posix_and_nt_attributes(DWORD const RawAttributes)
{
	// some programs store posix attributes in high 16 bits.
	// p7zip - stores additional 0x8000 flag marker.
	// macos - stores additional 0x4000 flag marker.
	// info-zip - no additional marker.

	if (RawAttributes & 0xF0000000)
		return { RawAttributes >> 16, RawAttributes & 0x3FFF };

	return { 0, RawAttributes };
}
