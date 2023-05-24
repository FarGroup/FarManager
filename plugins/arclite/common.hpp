#pragma once

typedef ByteVector ArcType;
typedef std::list<ArcType> ArcTypes;

typedef std::list<Error> ErrorLog;

#define RETRY_OR_IGNORE_BEGIN \
  bool error_ignored = false; \
  while (true) { \
    try {

#define RETRY_OR_IGNORE_END(ignore_errors, error_log, progress) \
      break; \
    } \
    catch (const Error& error) { \
      retry_or_ignore_error(error, error_ignored, ignore_errors, error_log, progress, true, true); \
      if (error_ignored) break; \
    } \
  }

#define RETRY_END(progress) \
      break; \
    } \
    catch (const Error& error) { \
      bool ignore_errors = false; \
      ErrorLog error_log; \
      retry_or_ignore_error(error, error_ignored, ignore_errors, error_log, progress, true, false); \
    } \
  }

#define IGNORE_END(ignore_errors, error_log, progress) \
      break; \
    } \
    catch (const Error& error) { \
      retry_or_ignore_error(error, error_ignored, ignore_errors, error_log, progress, false, true); \
      if (error_ignored) break; \
    } \
  }

class ProgressMonitor: private NonCopyable {
private:
  HANDLE h_scr;
  std::wstring con_title;
  static const unsigned c_first_delay_div = 2;
  static const unsigned c_update_delay_div = 2;
  UInt64 time_cnt;
  UInt64 time_freq;
  UInt64 time_total;
  UInt64 time_update;
  bool paused;
  bool low_priority;
  DWORD initial_priority;
  bool confirm_esc;
  void update_time();
  void discard_time();
  void display();
protected:
  std::wstring progress_title;
  std::wstring progress_text;
  bool progress_known;
  unsigned percent_done;
  virtual void do_update_ui() = 0;
protected:
  bool is_single_key(const KEY_EVENT_RECORD& key_event);
  void handle_esc();
public:
  ProgressMonitor(const std::wstring& progress_title, bool progress_known = true, bool lazy = true);
  virtual ~ProgressMonitor();
  void update_ui(bool force = false);
  void clean();
  UInt64 time_elapsed();
  UInt64 ticks_per_sec();
  friend class ProgressSuspend;
};

class ProgressSuspend: private NonCopyable {
private:
  ProgressMonitor& progress;
public:
  ProgressSuspend(ProgressMonitor& progress): progress(progress) {
    progress.update_time();
  }
  ~ProgressSuspend() {
    progress.discard_time();
  }
};

void retry_or_ignore_error(const Error& error, bool& ignore, bool& ignore_errors, ErrorLog& error_log, ProgressMonitor& progress, bool can_retry, bool can_ignore);

enum OverwriteAction { oaAsk, oaOverwrite, oaSkip, oaRename, oaAppend, oaOverwriteCase };

struct OpenOptions {
  std::wstring arc_path;
  bool detect;
  ArcTypes arc_types;
  std::wstring password;
  int *open_password_len;
  bool recursive_panel;
  char delete_on_close;
  OpenOptions();
};

struct ExtractOptions {
  std::wstring dst_dir;
  bool ignore_errors;
  OverwriteAction overwrite;
  TriState move_files;
  std::wstring password;
  TriState separate_dir;
  bool delete_archive;
  bool disable_delete_archive;
  TriState open_dir;
  ExtractOptions();
};

struct SfxVersionInfo {
  std::wstring version;
  std::wstring comments;
  std::wstring company_name;
  std::wstring file_description;
  std::wstring legal_copyright;
  std::wstring product_name;
};

struct SfxInstallConfig {
  std::wstring title;
  std::wstring begin_prompt;
  std::wstring progress;
  std::wstring run_program;
  std::wstring directory;
  std::wstring execute_file;
  std::wstring execute_parameters;
};

struct SfxOptions {
  std::wstring name;
  bool replace_icon;
  std::wstring icon_path;
  bool replace_version;
  SfxVersionInfo ver_info;
  bool append_install_config;
  SfxInstallConfig install_config;
  SfxOptions();
};

struct ProfileOptions {
  ArcType arc_type;
  unsigned level;
  std::wstring levels; // format=level;...
  std::wstring method;
  bool solid;
  std::wstring password;
  bool encrypt;
  TriState encrypt_header;
  bool create_sfx;
  SfxOptions sfx_options;
  bool enable_volumes;
  std::wstring volume_size;
  bool move_files;
  bool ignore_errors;
  std::wstring advanced;
  ProfileOptions();
};

struct UpdateOptions: public ProfileOptions {
  std::wstring arc_path;
  bool show_password;
  bool open_shared;
  OverwriteAction overwrite;
  std::shared_ptr<Far::FileFilter> filter;
  bool append_ext;
  UpdateOptions();
};

bool operator==(const ProfileOptions& o1, const ProfileOptions& o2);
bool operator==(const SfxOptions& o1, const SfxOptions& o2);

struct UpdateProfile {
  std::wstring name;
  ProfileOptions options;
};
struct UpdateProfiles: public std::vector<UpdateProfile> {
  void load();
  void save() const;
  unsigned find_by_name(const std::wstring& name);
  unsigned find_by_options(const UpdateOptions& options);
  void sort_by_name();
  void update(const std::wstring& name, const UpdateOptions& options);
};

struct Attr {
  std::wstring name;
  std::wstring value;
};
typedef std::list<Attr> AttrList;

unsigned calc_percent(UInt64 completed, UInt64 total);
UInt64 get_module_version(const std::wstring& file_path);
UInt64 parse_size_string(const std::wstring& str);
DWORD translate_seek_method(UInt32 seek_origin);
std::wstring expand_macros(const std::wstring& text);
std::wstring load_file(const std::wstring& file_name, unsigned* code_page = nullptr);
std::wstring auto_rename(const std::wstring& file_path);
