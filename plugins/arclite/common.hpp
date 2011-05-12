#pragma once

typedef ByteVector ArcType;
typedef list<ArcType> ArcTypes;

typedef list<Error> ErrorLog;

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
  wstring con_title;
  static const unsigned c_first_delay_div = 2;
  static const unsigned c_update_delay_div = 2;
  unsigned __int64 time_cnt;
  unsigned __int64 time_freq;
  unsigned __int64 time_total;
  unsigned __int64 time_update;
  bool paused;
  bool low_priority;
  DWORD initial_priority;
  bool confirm_esc;
  void update_time();
  void discard_time();
  void display();
protected:
  wstring progress_title;
  wstring progress_text;
  bool progress_known;
  unsigned percent_done;
  virtual void do_update_ui() = 0;
protected:
  bool is_single_key(const KEY_EVENT_RECORD& key_event);
  void handle_esc();
public:
  ProgressMonitor(const wstring& progress_title, bool progress_known = true, bool lazy = true);
  virtual ~ProgressMonitor();
  void update_ui(bool force = false);
  void clean();
  unsigned __int64 time_elapsed();
  unsigned __int64 ticks_per_sec();
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

enum OverwriteAction { oaAsk, oaOverwrite, oaSkip, oaRename, oaAppend };

struct OpenOptions {
  wstring arc_path;
  bool detect;
  ArcTypes arc_types;
  wstring password;
  OpenOptions();
};

struct ExtractOptions {
  wstring dst_dir;
  bool ignore_errors;
  OverwriteAction overwrite;
  TriState move_files;
  wstring password;
  TriState separate_dir;
  bool delete_archive;
  TriState open_dir;
  ExtractOptions();
};

struct SfxVersionInfo {
  wstring version;
  wstring comments;
  wstring company_name;
  wstring file_description;
  wstring legal_copyright;
  wstring product_name;
};

struct SfxInstallConfig {
  wstring title;
  wstring begin_prompt;
  wstring progress;
  wstring run_program;
  wstring directory;
  wstring execute_file;
  wstring execute_parameters;
};

struct SfxOptions {
  wstring name;
  bool replace_icon;
  wstring icon_path;
  bool replace_version;
  SfxVersionInfo ver_info;
  bool append_install_config;
  SfxInstallConfig install_config;
  SfxOptions();
};

struct ProfileOptions {
  ArcType arc_type;
  unsigned level;
  wstring method;
  bool solid;
  wstring password;
  bool encrypt;
  TriState encrypt_header;
  bool create_sfx;
  SfxOptions sfx_options;
  bool enable_volumes;
  wstring volume_size;
  bool move_files;
  bool ignore_errors;
  wstring advanced;
  ProfileOptions();
};

struct UpdateOptions: public ProfileOptions {
  wstring arc_path;
  bool show_password;
  bool open_shared;
  OverwriteAction overwrite;
  shared_ptr<Far::FileFilter> filter;
  bool append_ext;
  UpdateOptions();
};

bool operator==(const ProfileOptions& o1, const ProfileOptions& o2);
bool operator==(const SfxOptions& o1, const SfxOptions& o2);

struct UpdateProfile {
  wstring name;
  ProfileOptions options;
};
struct UpdateProfiles: public vector<UpdateProfile> {
  void load();
  void save() const;
  unsigned find_by_name(const wstring& name);
  unsigned find_by_options(const UpdateOptions& options);
  void sort_by_name();
  void update(const wstring& name, const UpdateOptions& options);
};

struct Attr {
  wstring name;
  wstring value;
};
typedef list<Attr> AttrList;

int round(double d);
unsigned calc_percent(unsigned __int64 completed, unsigned __int64 total);
unsigned __int64 get_module_version(const wstring& file_path);
unsigned __int64 parse_size_string(const wstring& str);
DWORD translate_seek_method(UInt32 seek_origin);
wstring expand_macros(const wstring& text);
wstring load_file(const wstring& file_name, unsigned* code_page = nullptr);
wstring auto_rename(const wstring& file_path);
