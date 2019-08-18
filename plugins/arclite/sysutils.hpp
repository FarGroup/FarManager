#pragma once

#ifdef DEBUG
  #define DEBUG_OUTPUT(msg) OutputDebugStringW(((msg) + L"\n").c_str())
#else
  #define DEBUG_OUTPUT(msg)
#endif

#if defined(__GNUC__) || (defined(_MSC_VER) && _MSC_VER >= 1900)
# define NOEXCEPT noexcept
#else
# define NOEXCEPT throw()
#endif

extern HINSTANCE g_h_instance;

wstring get_system_message(HRESULT hr, DWORD lang_id = 0);
wstring get_console_title();
bool wait_for_single_object(HANDLE handle, DWORD timeout);
wstring ansi_to_unicode(const string& str, unsigned code_page);
string unicode_to_ansi(const wstring& str, unsigned code_page);
wstring expand_env_vars(const wstring& str);
wstring get_full_path_name(const wstring& path);
wstring get_current_directory();

class CriticalSection: private NonCopyable, private CRITICAL_SECTION {
public:
  CriticalSection() {
    InitializeCriticalSection(this);
  }
  virtual ~CriticalSection() {
    DeleteCriticalSection(this);
  }
  friend class CriticalSectionLock;
};

CriticalSection& GetSync();
CriticalSection& GetExportSync();

class CriticalSectionLock: private NonCopyable {
private:
  CriticalSection& m_cs;
public:
  CriticalSectionLock(CriticalSection& cs): m_cs(cs) {
    EnterCriticalSection(&m_cs);
  }
  ~CriticalSectionLock() {
    LeaveCriticalSection(&m_cs);
  }
};

struct FindData: public WIN32_FIND_DATAW {
  bool is_dir() const {
    return (dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
  }
  UInt64 size() const {
    return (static_cast<UInt64>(nFileSizeHigh) << 32) | nFileSizeLow;
  }
  void set_size(UInt64 size) {
    nFileSizeLow = static_cast<DWORD>(size & 0xFFFFFFFF);
    nFileSizeHigh = static_cast<DWORD>(size >> 32);
  }
};

class File: private NonCopyable {
protected:
  HANDLE h_file;
  wstring file_path;
public:
  File() NOEXCEPT;
  ~File() NOEXCEPT;
  File(const wstring& file_path, DWORD desired_access, DWORD share_mode, DWORD creation_disposition, DWORD flags_and_attributes);
  void open(const wstring& file_path, DWORD desired_access, DWORD share_mode, DWORD creation_disposition, DWORD flags_and_attributes);
  bool open_nt(const wstring& file_path, DWORD desired_access, DWORD share_mode, DWORD creation_disposition, DWORD flags_and_attributes) NOEXCEPT;
  void close() NOEXCEPT;
  bool is_open() const NOEXCEPT {
    return h_file != INVALID_HANDLE_VALUE;
  }
  HANDLE handle() const NOEXCEPT {
    return h_file;
  }
  const wstring& path() const NOEXCEPT {
    return file_path;
  }
  UInt64 size();
  bool size_nt(UInt64& file_size) NOEXCEPT;
  size_t read(void* data, size_t size);
  bool read_nt(void* data, size_t size, size_t& size_read) NOEXCEPT;
  size_t write(const void* data, size_t size);
  bool write_nt(const void* data, size_t size, size_t& size_written) NOEXCEPT;
  void set_time(const FILETIME& ctime, const FILETIME& atime, const FILETIME& mtime);
  bool set_time_nt(const FILETIME& ctime, const FILETIME& atime, const FILETIME& mtime) NOEXCEPT;
  UInt64 set_pos(Int64 offset, DWORD method = FILE_BEGIN);
  bool set_pos_nt(Int64 offset, DWORD method = FILE_BEGIN, UInt64* new_pos = nullptr) NOEXCEPT;
  void set_end();
  bool set_end_nt() NOEXCEPT;
  BY_HANDLE_FILE_INFORMATION get_info();
  bool get_info_nt(BY_HANDLE_FILE_INFORMATION& info) NOEXCEPT;
  template<typename Type> bool io_control_out_nt(DWORD code, Type& data) NOEXCEPT {
    DWORD size_ret;
    return DeviceIoControl(h_file, code, nullptr, 0, &data, sizeof(Type), &size_ret, nullptr) != 0;
  }
  static DWORD attributes(const wstring& file_path) NOEXCEPT;
  static bool exists(const wstring& file_path) NOEXCEPT;
  static void set_attr(const wstring& file_path, DWORD attr);
  static bool set_attr_nt(const wstring& file_path, DWORD attr) NOEXCEPT;
  static void delete_file(const wstring& file_path);
  static bool delete_file_nt(const wstring& file_path) NOEXCEPT;
  static void create_dir(const wstring& dir_path);
  static bool create_dir_nt(const wstring& dir_path) NOEXCEPT;
  static void remove_dir(const wstring& file_path);
  static bool remove_dir_nt(const wstring& file_path) NOEXCEPT;
  static void move_file(const wstring& file_path, const wstring& new_path, DWORD flags);
  static bool move_file_nt(const wstring& file_path, const wstring& new_path, DWORD flags) NOEXCEPT;
  static FindData get_find_data(const wstring& file_path);
  static bool get_find_data_nt(const wstring& file_path, FindData& find_data) NOEXCEPT;
};

class Key: private NonCopyable {
protected:
  HKEY h_key;
public:
  Key() NOEXCEPT;
  ~Key() NOEXCEPT;
  Key(HKEY h_parent, LPCWSTR sub_key, REGSAM sam_desired, bool create);
  Key& open(HKEY h_parent, LPCWSTR sub_key, REGSAM sam_desired, bool create);
  bool open_nt(HKEY h_parent, LPCWSTR sub_key, REGSAM sam_desired, bool create) NOEXCEPT;
  void close() NOEXCEPT;
  HKEY handle() const NOEXCEPT;
  bool query_bool(const wchar_t* name);
  bool query_bool_nt(bool& value, const wchar_t* name) NOEXCEPT;
  unsigned query_int(const wchar_t* name);
  bool query_int_nt(unsigned& value, const wchar_t* name) NOEXCEPT;
  wstring query_str(const wchar_t* name);
  bool query_str_nt(wstring& value, const wchar_t* name) NOEXCEPT;
  ByteVector query_binary(const wchar_t* name);
  bool query_binary_nt(ByteVector& value, const wchar_t* name) NOEXCEPT;
  void set_bool(const wchar_t* name, bool value);
  bool set_bool_nt(const wchar_t* name, bool value) NOEXCEPT;
  void set_int(const wchar_t* name, unsigned value);
  bool set_int_nt(const wchar_t* name, unsigned value) NOEXCEPT;
  void set_str(const wchar_t* name, const wstring& value);
  bool set_str_nt(const wchar_t* name, const wstring& value) NOEXCEPT;
  void set_binary(const wchar_t* name, const unsigned char* value, unsigned size);
  bool set_binary_nt(const wchar_t* name, const unsigned char* value, unsigned size) NOEXCEPT;
  void delete_value(const wchar_t* name);
  bool delete_value_nt(const wchar_t* name) NOEXCEPT;
  vector<wstring> enum_sub_keys();
  bool enum_sub_keys_nt(vector<wstring>& names) NOEXCEPT;
  void delete_sub_key(const wchar_t* name);
  bool delete_sub_key_nt(const wchar_t* name) NOEXCEPT;
};

class FileEnum: private NonCopyable {
protected:
  wstring file_mask;
  HANDLE h_find;
  FindData find_data;
  int n_far_items;
  std::list<FindData> far_items;
public:
  FileEnum(const wstring& file_mask) NOEXCEPT;
  ~FileEnum() NOEXCEPT;
  bool next();
  bool next_nt(bool& more) NOEXCEPT;
  const FindData& data() const NOEXCEPT {
    return find_data;
  }
public:
  int far_emum_cb(const PluginPanelItem& item);
};

class DirList: public FileEnum {
public:
  DirList(const wstring& dir_path) NOEXCEPT;
};

wstring get_temp_path();

class TempFile: private NonCopyable {
private:
  wstring path;
public:
  TempFile();
  ~TempFile();
  wstring get_path() const {
    return path;
  }
};

wstring format_file_time(const FILETIME& file_time);
wstring upcase(const wstring& str);
wstring create_guid();
void enable_lfh();
wstring search_path(const wstring& file_name);

class DisableSleepMode {
private:
  EXECUTION_STATE saved_state;
public:
  DisableSleepMode() {
    saved_state = SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED);
  }
  ~DisableSleepMode() {
    if (saved_state) {
      SetThreadExecutionState(saved_state);
    }
  }
};

class Patch7zCP
{
public:
  static void SetCP(UINT oemCP, UINT ansiCP);
};

std::pair<DWORD, DWORD> get_posix_and_nt_attributes(DWORD RawAttributes);
