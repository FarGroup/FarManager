#include "msg.hpp"
#include "guids.hpp"
#include "utils.hpp"
#include "farutils.hpp"
#include "sysutils.hpp"
#include "common.hpp"
#include "ui.hpp"
#include "archive.hpp"
#include "options.hpp"
#include <algorithm>

std::wstring get_error_dlg_title() {
  return Far::get_msg(MSG_PLUGIN_NAME);
}

ProgressMonitor::ProgressMonitor(const std::wstring& progress_title, bool progress_known, bool lazy) :
  h_scr(nullptr),
  paused(false),
  low_priority(false),
  confirm_esc(false),
  progress_title(progress_title),
  progress_known(progress_known),
  percent_done(0)
{
  QueryPerformanceCounter(reinterpret_cast<PLARGE_INTEGER>(&time_cnt));
  QueryPerformanceFrequency(reinterpret_cast<PLARGE_INTEGER>(&time_freq));
  time_total = 0;
  if (lazy)
    time_update = time_total + time_freq / c_first_delay_div;
  else
    time_update = time_total;
  CHECK(get_app_option(FSSF_CONFIRMATIONS, c_esc_confirmation_option, confirm_esc));
  initial_priority = GetPriorityClass(GetCurrentProcess());
}

ProgressMonitor::~ProgressMonitor() {
  clean();
}

void ProgressMonitor::display() {
  do_update_ui();
  std::wstring title;
  if (progress_known) {
    title += L"{" + int_to_str(percent_done) + L"%} ";
  }
  if (paused) {
    title += L"[" + Far::get_msg(MSG_PROGRESS_PAUSED) + L"] ";
  }
  else if (low_priority) {
    title += L"[" + Far::get_msg(MSG_PROGRESS_LOW_PRIORITY) + L"] ";
  }
  title += progress_title;
  if (progress_known) {
    Far::set_progress_state(TBPF_NORMAL);
    Far::set_progress_value(percent_done, 100);
  }
  else {
    Far::set_progress_state(TBPF_INDETERMINATE);
  }
  Far::message(c_progress_dialog_guid, title + L'\n' + progress_text, 0, FMSG_LEFTALIGN);
  SetConsoleTitleW(title.c_str());
}

void ProgressMonitor::update_ui(bool force) {
  update_time();
  if ((time_total >= time_update) || force) {
    time_update = time_total + time_freq / c_update_delay_div;
    if (h_scr == nullptr) {
      h_scr = Far::save_screen();
      con_title = get_console_title();
    }

    HANDLE h_con = GetStdHandle(STD_INPUT_HANDLE);
    INPUT_RECORD rec;
    DWORD read_cnt;
    while (true) {
      if (!paused) {
        PeekConsoleInputW(h_con, &rec, 1, &read_cnt);
        if (read_cnt == 0) break;
      }
      ReadConsoleInputW(h_con, &rec, 1, &read_cnt);
      if (rec.EventType == KEY_EVENT) {
        const KEY_EVENT_RECORD& key_event = rec.Event.KeyEvent;
        const WORD c_vk_b = 0x42;
        const WORD c_vk_p = 0x50;
        if (is_single_key(key_event)) {
          if (key_event.wVirtualKeyCode == VK_ESCAPE) {
            handle_esc();
          }
          else if (key_event.wVirtualKeyCode == c_vk_b) {
            low_priority = !low_priority;
            SetPriorityClass(GetCurrentProcess(), low_priority ? IDLE_PRIORITY_CLASS : initial_priority);
            if (paused)
              display();
          }
          else if (key_event.wVirtualKeyCode == c_vk_p) {
            paused = !paused;
            if (paused) {
              update_time();
              display();
            }
            else
              discard_time();
          }
        }
      }
    }
    display();
  }
}

bool ProgressMonitor::is_single_key(const KEY_EVENT_RECORD& key_event) {
  return key_event.bKeyDown && (key_event.dwControlKeyState & (LEFT_ALT_PRESSED | LEFT_CTRL_PRESSED | RIGHT_ALT_PRESSED | RIGHT_CTRL_PRESSED | SHIFT_PRESSED)) == 0;
}

void ProgressMonitor::handle_esc() {
  if (!confirm_esc)
    FAIL(E_ABORT);
  ProgressSuspend ps(*this);
  if (Far::message(c_interrupt_dialog_guid, Far::get_msg(MSG_PLUGIN_NAME) + L"\n" + Far::get_msg(MSG_PROGRESS_INTERRUPT), 0, FMSG_MB_YESNO) == 0)
    FAIL(E_ABORT);
}

void ProgressMonitor::clean() {
  if (h_scr) {
    Far::restore_screen(h_scr);
    SetConsoleTitleW(con_title.c_str());
    Far::set_progress_state(TBPF_NOPROGRESS);
    h_scr = nullptr;
  }
  SetPriorityClass(GetCurrentProcess(), initial_priority);
}

void ProgressMonitor::update_time() {
  UInt64 time_curr;
  QueryPerformanceCounter(reinterpret_cast<PLARGE_INTEGER>(&time_curr));
  time_total += time_curr - time_cnt;
  time_cnt = time_curr;
}

void ProgressMonitor::discard_time() {
  QueryPerformanceCounter(reinterpret_cast<PLARGE_INTEGER>(&time_cnt));
}

UInt64 ProgressMonitor::time_elapsed() {
  update_time();
  return time_total;
}

UInt64 ProgressMonitor::ticks_per_sec() {
  return time_freq;
}

static const wchar_t** get_suffixes(int start) {
  static const int msg_ids[1 + 5 + 5] =
  { MSG_LANG
  , -1,                 MSG_SUFFIX_SIZE_KB,  MSG_SUFFIX_SIZE_MB,  MSG_SUFFIX_SIZE_GB,  MSG_SUFFIX_SIZE_TB
  , MSG_SUFFIX_SPEED_B, MSG_SUFFIX_SPEED_KB, MSG_SUFFIX_SPEED_MB, MSG_SUFFIX_SPEED_GB, MSG_SUFFIX_SPEED_TB
  };
  static std::wstring  msg_texts[1 + 5 + 5];
  static const wchar_t* suffixes[1 + 5 + 5];

  if (Far::get_msg(msg_ids[0]) != msg_texts[0]) {
    for (int i = 0; i < 1 + 5 + 5; ++i) {
      auto msg_id = msg_ids[i];
      suffixes[i] = (msg_texts[i] = msg_id >= 0 ? Far::get_msg(msg_id) : std::wstring()).c_str();
    }
  }

  return suffixes + start;
}

const wchar_t** get_size_suffixes()  { return get_suffixes(1+0); }
const wchar_t** get_speed_suffixes() { return get_suffixes(1+5); }

class PasswordDialog: public Far::Dialog {
private:
  enum {
    c_client_xs = 60
  };

  std::wstring arc_path;
  std::wstring& password;

  int password_ctrl_id{};
  int ok_ctrl_id{};
  int cancel_ctrl_id{};

  intptr_t dialog_proc(intptr_t msg, intptr_t param1, void* param2) override {
    if ((msg == DN_CLOSE) && (param1 >= 0) && (param1 != cancel_ctrl_id)) {
      password = get_text(password_ctrl_id);
    }
    return default_dialog_proc(msg, param1, param2);
  }

public:
  PasswordDialog(std::wstring& password, const std::wstring& arc_path) :
    Far::Dialog(Far::get_msg(MSG_PASSWORD_TITLE), &c_password_dialog_guid, c_client_xs),
    arc_path(arc_path),
    password(password)
  {}

  bool show() {
    label(fit_str(arc_path, c_client_xs), c_client_xs, DIF_SHOWAMPERSAND);
    new_line();
    label(Far::get_msg(MSG_PASSWORD_PASSWORD));
    password_ctrl_id = pwd_edit_box(password);
    new_line();
    separator();
    new_line();

    ok_ctrl_id = def_button(Far::get_msg(MSG_BUTTON_OK), DIF_CENTERGROUP);
    cancel_ctrl_id = button(Far::get_msg(MSG_BUTTON_CANCEL), DIF_CENTERGROUP);
    new_line();

    intptr_t item = Far::Dialog::show();

    return (item != -1) && (item != cancel_ctrl_id);
  }
};

bool password_dialog(std::wstring& password, const std::wstring& arc_path) {
  return PasswordDialog(password, arc_path).show();
}


class OverwriteDialog: public Far::Dialog {
private:
  enum {
    c_client_xs = 60
  };

  std::wstring file_path;
  OverwriteFileInfo src_file_info;
  OverwriteFileInfo dst_file_info;
  OverwriteDialogKind kind;
  OverwriteOptions& options;

  int all_ctrl_id{};
  int overwrite_ctrl_id{};
  int skip_ctrl_id{};
  int rename_ctrl_id{};
  int append_ctrl_id{};
  int cancel_ctrl_id{};

  intptr_t dialog_proc(intptr_t msg, intptr_t param1, void* param2) override {
    if (msg == DN_CLOSE && param1 >= 0 && param1 != cancel_ctrl_id) {
      options.all = get_check(all_ctrl_id);
      if (param1 == overwrite_ctrl_id)
        options.action = oaOverwrite;
      else if (param1 == skip_ctrl_id)
        options.action = oaSkip;
      else if (kind == odkExtract && param1 == rename_ctrl_id)
        options.action = oaRename;
      else if (kind == odkExtract && param1 == append_ctrl_id)
        options.action = oaAppend;
      else
        FAIL(E_ABORT);
    }
    return default_dialog_proc(msg, param1, param2);
  }

public:
  OverwriteDialog(const std::wstring& file_path, const OverwriteFileInfo& src_file_info, const OverwriteFileInfo& dst_file_info, OverwriteDialogKind kind, OverwriteOptions& options):
    Far::Dialog(Far::get_msg(MSG_OVERWRITE_DLG_TITLE), &c_overwrite_dialog_guid, c_client_xs, nullptr, FDLG_WARNING),
    file_path(file_path),
    src_file_info(src_file_info),
    dst_file_info(dst_file_info),
    kind(kind),
    options(options) {
  }

  bool show() {
    label(fit_str(file_path, c_client_xs), c_client_xs, DIF_SHOWAMPERSAND);
    new_line();
    label(Far::get_msg(MSG_OVERWRITE_DLG_QUESTION));
    new_line();
    separator();
    new_line();

    std::wstring src_label = Far::get_msg(MSG_OVERWRITE_DLG_SOURCE);
    std::wstring dst_label = Far::get_msg(MSG_OVERWRITE_DLG_DESTINATION);
    uintptr_t label_pad = std::max(src_label.size(), dst_label.size()) + 1;
    std::wstring src_size = uint_to_str(src_file_info.size);
    std::wstring dst_size = uint_to_str(dst_file_info.size);
    uintptr_t size_pad = label_pad + std::max(src_size.size(), dst_size.size()) + 1;

    label(src_label);
    pad(label_pad);
    if (!src_file_info.is_dir) {
      label(src_size);
      pad(size_pad);
    }
    label(format_file_time(src_file_info.mtime));
    if (CompareFileTime(&src_file_info.mtime, &dst_file_info.mtime) > 0) {
      spacer(1);
      label(Far::get_msg(MSG_OVERWRITE_DLG_NEWER));
    }
    new_line();

    label(dst_label);
    pad(label_pad);
    if (!dst_file_info.is_dir) {
      label(dst_size);
      pad(size_pad);
    }
    label(format_file_time(dst_file_info.mtime));
    if (CompareFileTime(&src_file_info.mtime, &dst_file_info.mtime) < 0) {
      spacer(1);
      label(Far::get_msg(MSG_OVERWRITE_DLG_NEWER));
    }
    new_line();

    separator();
    new_line();
    all_ctrl_id = check_box(Far::get_msg(MSG_OVERWRITE_DLG_ALL), false);
    new_line();
    separator();
    new_line();
    overwrite_ctrl_id = def_button(Far::get_msg(MSG_OVERWRITE_DLG_OVERWRITE), DIF_CENTERGROUP);
    spacer(1);
    skip_ctrl_id = button(Far::get_msg(MSG_OVERWRITE_DLG_SKIP), DIF_CENTERGROUP);
    spacer(1);
    if (kind == odkExtract) {
      rename_ctrl_id = button(Far::get_msg(MSG_OVERWRITE_DLG_RENAME), DIF_CENTERGROUP);
      spacer(1);
      append_ctrl_id = button(Far::get_msg(MSG_OVERWRITE_DLG_APPEND), DIF_CENTERGROUP);
      spacer(1);
    }
    cancel_ctrl_id = button(Far::get_msg(MSG_OVERWRITE_DLG_CANCEL), DIF_CENTERGROUP);
    new_line();

    intptr_t item = Far::Dialog::show();

    return item >= 0 && item != cancel_ctrl_id;
  }
};

bool overwrite_dialog(const std::wstring& file_path, const OverwriteFileInfo& src_file_info, const OverwriteFileInfo& dst_file_info, OverwriteDialogKind kind, OverwriteOptions& options) {
  return OverwriteDialog(file_path, src_file_info, dst_file_info, kind, options).show();
}


class ExtractDialog: public Far::Dialog {
private:
  enum {
    c_client_xs = 60
  };

  ExtractOptions& m_options;

  int dst_dir_ctrl_id{};
  int ignore_errors_ctrl_id{};
  int oa_ask_ctrl_id{};
  int oa_overwrite_ctrl_id{};
  int oa_skip_ctrl_id{};
  int oa_rename_ctrl_id{};
  int oa_append_ctrl_id{};
  int move_files_ctrl_id{};
  int password_ctrl_id{};
  int separate_dir_ctrl_id{};
  int delete_archive_ctrl_id{};
  int open_dir_ctrl_id{};
  int ok_ctrl_id{};
  int cancel_ctrl_id{};
  int save_params_ctrl_id{};

  void read_controls(ExtractOptions& options) {
    auto dst = search_and_replace(strip(get_text(dst_dir_ctrl_id)), L"\"", std::wstring());
    bool expand_env = add_trailing_slash(options.dst_dir) != add_trailing_slash(dst);
    options.dst_dir = expand_env ? expand_env_vars(dst) : dst;
    options.ignore_errors = get_check(ignore_errors_ctrl_id);
    if (get_check(oa_ask_ctrl_id)) options.overwrite = oaAsk;
    else if (get_check(oa_overwrite_ctrl_id)) options.overwrite = oaOverwrite;
    else if (get_check(oa_skip_ctrl_id)) options.overwrite = oaSkip;
    else if (get_check(oa_rename_ctrl_id)) options.overwrite = oaRename;
    else if (get_check(oa_append_ctrl_id)) options.overwrite = oaAppend;
    else options.overwrite = oaAsk;
    if (options.move_files != triUndef)
      options.move_files = get_check3(move_files_ctrl_id);
    options.password = get_text(password_ctrl_id);
    options.separate_dir = get_check3(separate_dir_ctrl_id);
    options.delete_archive = get_check(delete_archive_ctrl_id);
    if (options.open_dir != triUndef)
      options.open_dir = get_check3(open_dir_ctrl_id);
  }

  intptr_t dialog_proc(intptr_t msg, intptr_t param1, void* param2) override {
    if ((msg == DN_CLOSE) && (param1 >= 0) && (param1 != cancel_ctrl_id)) {
      read_controls(m_options);
    }
    else if (msg == DN_BTNCLICK && param1 == delete_archive_ctrl_id) {
      enable(move_files_ctrl_id, m_options.move_files != triUndef && !get_check(delete_archive_ctrl_id));
    }
    else if (msg == DN_BTNCLICK && param1 == save_params_ctrl_id) {
      ExtractOptions options;
      read_controls(options);
      g_options.extract_ignore_errors = options.ignore_errors;
      g_options.extract_overwrite = options.overwrite;
      g_options.extract_separate_dir = options.separate_dir;
      if (options.open_dir != triUndef)
        g_options.extract_open_dir = options.open_dir == triTrue;
      g_options.save();
      Far::info_dlg(c_extract_params_saved_dialog_guid, Far::get_msg(MSG_EXTRACT_DLG_TITLE), Far::get_msg(MSG_EXTRACT_DLG_PARAMS_SAVED));
      set_focus(ok_ctrl_id);
    }
    return default_dialog_proc(msg, param1, param2);
  }

public:
  ExtractDialog(ExtractOptions& options): Far::Dialog(Far::get_msg(MSG_EXTRACT_DLG_TITLE), &c_extract_dialog_guid, c_client_xs, L"Extract"), m_options(options) {
  }

  bool show() {
    label(Far::get_msg(MSG_EXTRACT_DLG_DST_DIR));
    new_line();
    dst_dir_ctrl_id = history_edit_box(add_trailing_slash(m_options.dst_dir), L"arclite.extract_dir", c_client_xs, DIF_EDITPATH);
    new_line();
    separator();
    new_line();

    ignore_errors_ctrl_id = check_box(Far::get_msg(MSG_EXTRACT_DLG_IGNORE_ERRORS), m_options.ignore_errors);
    new_line();

    label(Far::get_msg(MSG_EXTRACT_DLG_OA));
    new_line();
    spacer(2);
    oa_ask_ctrl_id = radio_button(Far::get_msg(MSG_EXTRACT_DLG_OA_ASK), m_options.overwrite == oaAsk);
    spacer(2);
    oa_overwrite_ctrl_id = radio_button(Far::get_msg(MSG_EXTRACT_DLG_OA_OVERWRITE), m_options.overwrite == oaOverwrite || m_options.overwrite == oaOverwriteCase);
    spacer(2);
    oa_skip_ctrl_id = radio_button(Far::get_msg(MSG_EXTRACT_DLG_OA_SKIP), m_options.overwrite == oaSkip);
    new_line();
    spacer(2);
    oa_rename_ctrl_id = radio_button(Far::get_msg(MSG_EXTRACT_DLG_OA_RENAME), m_options.overwrite == oaRename);
    spacer(2);
    oa_append_ctrl_id = radio_button(Far::get_msg(MSG_EXTRACT_DLG_OA_APPEND), m_options.overwrite == oaAppend);
    new_line();

    move_files_ctrl_id = check_box3(Far::get_msg(MSG_EXTRACT_DLG_MOVE_FILES), m_options.move_files, m_options.move_files == triUndef ? DIF_DISABLE : 0);
    new_line();
    delete_archive_ctrl_id = check_box(Far::get_msg(MSG_EXTRACT_DLG_DELETE_ARCHIVE), m_options.delete_archive, m_options.disable_delete_archive ? DIF_DISABLE : 0);
    new_line();
    separate_dir_ctrl_id = check_box3(Far::get_msg(MSG_EXTRACT_DLG_SEPARATE_DIR), m_options.separate_dir);
    new_line();
    open_dir_ctrl_id = check_box3(Far::get_msg(MSG_EXTRACT_DLG_OPEN_DIR), m_options.open_dir, m_options.open_dir == triUndef ? DIF_DISABLE : 0);
    new_line();

    label(Far::get_msg(MSG_EXTRACT_DLG_PASSWORD));
    password_ctrl_id = pwd_edit_box(m_options.password, 20);
    new_line();

    separator();
    new_line();
    ok_ctrl_id = def_button(Far::get_msg(MSG_BUTTON_OK), DIF_CENTERGROUP);
    cancel_ctrl_id = button(Far::get_msg(MSG_BUTTON_CANCEL), DIF_CENTERGROUP);
    save_params_ctrl_id = button(Far::get_msg(MSG_EXTRACT_DLG_SAVE_PARAMS), DIF_CENTERGROUP | DIF_BTNNOCLOSE);
    new_line();

    intptr_t item = Far::Dialog::show();

    return (item != -1) && (item != cancel_ctrl_id);
  }
};

bool extract_dialog(ExtractOptions& options) {
  return ExtractDialog(options).show();
}

void retry_or_ignore_error(const Error& error, bool& ignore, bool& ignore_errors, ErrorLog& error_log, ProgressMonitor& progress, bool can_retry, bool can_ignore) {
  if (error.code == E_ABORT)
    throw error;
  ignore = ignore_errors;
  if (!ignore) {
    std::wostringstream st;
    st << Far::get_msg(MSG_PLUGIN_NAME) << L'\n';
    if (error.code != E_MESSAGE) {
      std::wstring sys_msg = get_system_message(error.code, Far::get_lang_id());
      if (!sys_msg.empty())
        st << word_wrap(sys_msg, Far::get_optimal_msg_width()) << L'\n';
    }
    for (const auto& msg: error.messages) {
      st << word_wrap(msg, Far::get_optimal_msg_width()) << L'\n';
    }
    st << extract_file_name(widen(error.file)) << L':' << error.line << L'\n';
    unsigned button_cnt = 0;
    unsigned retry_id=0, ignore_id=0, ignore_all_id=0;
    if (can_retry) {
      st << Far::get_msg(MSG_BUTTON_RETRY) << L'\n';
      retry_id = button_cnt;
      button_cnt++;
    }
    if (can_ignore) {
      st << Far::get_msg(MSG_BUTTON_IGNORE) << L'\n';
      ignore_id = button_cnt;
      button_cnt++;
      st << Far::get_msg(MSG_BUTTON_IGNORE_ALL) << L'\n';
      ignore_all_id = button_cnt;
      button_cnt++;
    }
    st << Far::get_msg(MSG_BUTTON_CANCEL);
    button_cnt++;
    ProgressSuspend ps(progress);
    auto id = Far::message(c_retry_ignore_dialog_guid, st.str(), button_cnt, FMSG_WARNING);
    if (can_retry && (unsigned)id == retry_id)
      return;
    if (can_ignore && (unsigned)id == ignore_id) {
      ignore = true;
    }
    else if (can_ignore && (unsigned)id == ignore_all_id) {
      ignore = true;
      ignore_errors = true;
    }
    else
      FAIL(E_ABORT);
  }
  if (ignore)
    error_log.push_back(error);
}

void show_error_log(const ErrorLog& error_log) {
  std::wstring msg;
  msg += Far::get_msg(MSG_LOG_TITLE) + L'\n';
  msg += Far::get_msg(MSG_LOG_INFO) + L'\n';
  msg += Far::get_msg(MSG_LOG_CLOSE) + L'\n';
  msg += Far::get_msg(MSG_LOG_SHOW) + L'\n';
  if (Far::message(c_error_log_dialog_guid, msg, 2, FMSG_WARNING) != 1) return;

  TempFile temp_file;
  File file(temp_file.get_path(), GENERIC_WRITE, FILE_SHARE_READ, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL);

  const wchar_t sig = 0xFEFF;
  file.write(&sig, sizeof(sig));
  std::wstring line;
  for (const auto& error: error_log) {
    line.clear();
    if (error.code != E_MESSAGE && error.code != S_FALSE) {
      std::wstring sys_msg = get_system_message(error.code, Far::get_lang_id());
      if (!sys_msg.empty())
        line.append(sys_msg).append(1, L'\n');
    }
    for (const auto& o : error.objects)
       line.append(o).append(1, L'\n');
    std::wstring indent;
    if (!error.messages.empty() && (!error.objects.empty() || !error.warnings.empty())) {
       line.append(Far::get_msg(MSG_KPID_ERRORFLAGS)).append(L":\n");
       indent = L"  ";
    }
    for (const auto& e : error.messages)
       line.append(indent).append(e).append(1, L'\n');
    if (!error.warnings.empty())
       line.append(Far::get_msg(MSG_KPID_WARNINGFLAGS)).append(L":\n");
    for (const auto& w : error.warnings)
       line.append(L"  ").append(w).append(1, L'\n');
    line.append(1, L'\n');
    file.write(line.data(), static_cast<unsigned>(line.size()) * sizeof(wchar_t));
  }
  file.close();

  Far::viewer(temp_file.get_path(), Far::get_msg(MSG_LOG_TITLE), VF_DISABLEHISTORY | VF_ENABLE_F6);
}

bool operator==(const SfxOptions& o1, const SfxOptions& o2) {
  if (o1.name != o2.name || o1.replace_icon != o2.replace_icon ||
    o1.replace_version != o2.replace_version || o1.append_install_config != o2.append_install_config)
    return false;
  if (o1.replace_icon) {
    if (o1.icon_path != o2.icon_path)
      return false;
  }
  if (o1.replace_version) {
    if (o1.ver_info.version != o2.ver_info.version || o1.ver_info.comments != o2.ver_info.comments ||
      o1.ver_info.company_name != o2.ver_info.company_name || o1.ver_info.file_description != o2.ver_info.file_description ||
      o1.ver_info.legal_copyright != o2.ver_info.legal_copyright || o1.ver_info.product_name != o2.ver_info.product_name)
      return false;
  }
  if (o1.append_install_config) {
    if (o1.install_config.title != o2.install_config.title || o1.install_config.begin_prompt != o2.install_config.begin_prompt ||
      o1.install_config.progress != o2.install_config.progress || o1.install_config.run_program != o2.install_config.run_program ||
      o1.install_config.directory != o2.install_config.directory || o1.install_config.execute_file != o2.install_config.execute_file ||
      o1.install_config.execute_parameters != o2.install_config.execute_parameters)
      return false;
  }
  return true;
}

bool operator==(const ProfileOptions& o1, const ProfileOptions& o2) {
  if (o1.arc_type != o2.arc_type || o1.level != o2.level)
    return false;
  bool is_7z = o1.arc_type == c_7z;
  bool is_zip = o1.arc_type == c_zip;
  bool is_compressed = o1.level != 0;

  if (is_7z || is_zip) {
    if (is_compressed) {
      if (o1.method != o2.method)
        return false;
    }
    if (o1.solid != o2.solid)
      return false;
  }
  if (o1.advanced != o2.advanced)
    return false;
  if (is_7z || is_zip) {
    if (o1.encrypt != o2.encrypt)
      return false;
    bool is_encrypted = o1.encrypt;
    if (is_encrypted) {
      if (o1.password != o2.password)
        return false;
      if (is_7z) {
        if (o1.encrypt_header != o2.encrypt_header)
          return false;
      }
    }
  }
  bool is_sfx = o1.create_sfx;
  if (is_7z) {
    if (o1.create_sfx != o2.create_sfx)
      return false;
    if (is_sfx) {
      if (!(o1.sfx_options == o2.sfx_options))
        return false;
    }
  }
  if (!is_7z || !is_sfx) {
    if (o1.enable_volumes != o2.enable_volumes)
      return false;
    bool is_multi_volume = o1.enable_volumes;
    if (is_multi_volume) {
      if (o1.volume_size != o2.volume_size)
        return false;
    }
  }
  if (o1.move_files != o2.move_files || o1.ignore_errors != o2.ignore_errors)
    return false;
  return true;
}

struct ArchiveType {
  unsigned name_id;
  const ArcType& value;
};

const ArchiveType c_archive_types[] = {
  { MSG_COMPRESSION_ARCHIVE_7Z, c_7z },
  { MSG_COMPRESSION_ARCHIVE_ZIP, c_zip },
};

struct CompressionLevel {
  unsigned name_id;
  unsigned value;
};

const CompressionLevel c_levels[] = {
  { MSG_COMPRESSION_LEVEL_STORE, 0 },
  { MSG_COMPRESSION_LEVEL_FASTEST, 1 },
  { MSG_COMPRESSION_LEVEL_FAST, 3 },
  { MSG_COMPRESSION_LEVEL_NORMAL, 5 },
  { MSG_COMPRESSION_LEVEL_MAXIMUM, 7 },
  { MSG_COMPRESSION_LEVEL_ULTRA, 9 },
};

struct CompressionMethod {
  unsigned name_id;
  const wchar_t* value;
};

const CompressionMethod c_methods[] = {
  { MSG_COMPRESSION_METHOD_LZMA, c_method_lzma },
  { MSG_COMPRESSION_METHOD_LZMA2, c_method_lzma2 },
  { MSG_COMPRESSION_METHOD_PPMD, c_method_ppmd },
  { MSG_COMPRESSION_METHOD_DEFLATE, c_method_deflate },
  { MSG_COMPRESSION_METHOD_DEFLATE64, c_method_deflate64 },
};

static bool is_SWFu(const std::wstring& fname)
{
  bool unpacked_swf = false;
  File file;
  file.open_nt(Far::get_absolute_path(fname),
    FILE_READ_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN
  );
  if (file.is_open())
  {
    char bf[8];
    size_t nr = 0;
    file.read_nt(bf, sizeof(bf), nr);
    unpacked_swf = nr==sizeof(bf) && bf[2]=='S' && bf[1]=='W' && bf[0]=='F' && (unsigned)bf[3] < 20;
  }
  return unpacked_swf;
}

class UpdateDialog: public Far::Dialog {
private:
  enum {
    c_client_xs = 68
  };

  bool new_arc;
  bool multifile;
  std::wstring default_arc_name;
  std::vector<ArcType> main_formats;
  std::vector<ArcType> other_formats;
  UpdateOptions& m_options;
  UpdateProfiles& profiles;

  int profile_ctrl_id{};
  int save_profile_ctrl_id{};
  int delete_profile_ctrl_id{};
  int arc_path_ctrl_id{};
  int arc_path_eval_ctrl_id{};
  int append_ext_ctrl_id{};
  int main_formats_ctrl_id{};
  int other_formats_ctrl_id{};
  int level_ctrl_id{};
  int method_ctrl_id{};
  int solid_ctrl_id{};
  int advanced_ctrl_id{};
  int encrypt_ctrl_id{};
  int encrypt_header_ctrl_id{};
  int show_password_ctrl_id{};
  int password_ctrl_id{};
  int password_verify_ctrl_id{};
  int password_visible_ctrl_id{};
  int create_sfx_ctrl_id{};
  int sfx_options_ctrl_id{};
  int move_files_ctrl_id{};
  int open_shared_ctrl_id{};
  int ignore_errors_ctrl_id{};
  int enable_volumes_ctrl_id{};
  int volume_size_ctrl_id{};
  int oa_ask_ctrl_id{};
  int oa_overwrite_ctrl_id{};
  int oa_skip_ctrl_id{};
  int enable_filter_ctrl_id{};
  int ok_ctrl_id{};
  int cancel_ctrl_id{};
  int save_params_ctrl_id{};

  std::wstring old_ext;
  ArcType arc_type;
  unsigned level;

  std::wstring get_default_ext() const {
    std::wstring ext;
    bool create_sfx = get_check(create_sfx_ctrl_id);
    bool enable_volumes = get_check(enable_volumes_ctrl_id);
    if (ArcAPI::formats().count(arc_type))
      ext = ArcAPI::formats().at(arc_type).default_extension();
    if (create_sfx && arc_type == c_7z)
      ext += c_sfx_ext;
    else if (enable_volumes)
      ext += c_volume_ext;
    return ext;
  }

  bool change_extension() {
    assert(new_arc);

    std::wstring new_ext = get_default_ext();

    if (old_ext.empty() || new_ext.empty())
      return false;

    std::wstring arc_path = get_text(arc_path_ctrl_id);
    std::wstring file_name = extract_file_name(strip(unquote(strip(arc_path))));
    size_t pos = file_name.find_last_of(L'.');
    if (pos == std::wstring::npos || pos == 0)
      return false;
    std::wstring ext = file_name.substr(pos);
    if (_wcsicmp(ext.c_str(), c_sfx_ext) == 0 || _wcsicmp(ext.c_str(), c_volume_ext) == 0) {
      pos = file_name.find_last_of(L'.', pos - 1);
      if (pos != std::wstring::npos && pos != 0)
        ext = file_name.substr(pos);
    }
    if (_wcsicmp(old_ext.c_str(), ext.c_str()) != 0)
      return false;
    pos = arc_path.find_last_of(ext) - (ext.size() - 1);
    arc_path.replace(pos, ext.size(), new_ext);
    set_text(arc_path_ctrl_id, arc_path);

    old_ext = std::move(new_ext);
    return true;
  }

  bool arc_levels(char mode)
  {
    if (!ArcAPI::formats().count(arc_type))
      return false;
    auto search = ArcAPI::formats().at(arc_type).name;
    if (search.empty())
      return false;
    search += L"=";
    size_t pos = 0;
    while (pos+search.size() <= m_options.levels.size()) {
      if (m_options.levels.substr(pos, search.size()) == search) {
        if (mode == 'g') {
          level = (unsigned)str_to_int(m_options.levels.substr(pos+search.size()));
          return true;
        }
        auto ndel = search.size();
        while (pos+ndel < m_options.levels.size() && std::wcschr(L"0123456789", m_options.levels[pos+ndel])) ++ndel;
        if (pos+ndel < m_options.levels.size() && m_options.levels[pos+ndel] == L';') ++ndel;
        if (pos > 0 && pos + ndel >= m_options.levels.size() && m_options.levels[pos-1] == L';') { --pos; ++ndel; }
         m_options.levels.erase(pos, ndel);
        break;
      }
      pos = m_options.levels.find(L';', pos+1);
      if (pos == std::wstring::npos)
        break;
      ++pos;
    }
    if (mode == 'g')
      return false;
    m_options.levels = search + int_to_str((int)level) + (m_options.levels.empty() ? L"" : L";") + m_options.levels;
    return true;
  }

  void update_level_list() {
    unsigned level_sel = get_list_pos(level_ctrl_id);
    unsigned new_level_sel = -1, def_level_sel = -1;
    for (unsigned i = 0; i < ARRAYSIZE(c_levels); ++i) {
      bool skip = c_levels[i].value == 0 && arc_type == c_bzip2;
      skip = skip || (c_levels[i].value != 0 && (arc_type == c_wim || arc_type == c_tar));
      FarListGetItem flgi{};
      flgi.StructSize = sizeof(FarListGetItem);
      flgi.ItemIndex = i;
      CHECK(send_message(DM_LISTGETITEM, level_ctrl_id, &flgi));
      if ((skip && (flgi.Item.Flags & LIF_DISABLE) == 0) || (!skip && (flgi.Item.Flags & LIF_DISABLE) != 0)) {
        FarListUpdate flu{};
        flu.StructSize = sizeof(FarListUpdate);
        flu.Index = i;
        flu.Item.Flags = skip ? LIF_DISABLE : 0;
        flu.Item.Text = Far::msg_ptr(c_levels[i].name_id);
        send_message(DM_LISTUPDATE, level_ctrl_id, &flu);
      }
      if (!skip && (def_level_sel == (unsigned)-1 || c_levels[i].value == 5)) // normal or first enabled
          def_level_sel = i;
      if (c_levels[i].value == level && !skip) // if current level enabled
        new_level_sel = i;
    }
    if (new_level_sel == (unsigned)-1)
      new_level_sel = def_level_sel != (unsigned)-1 ? def_level_sel : level_sel;
    if (new_level_sel != level_sel)
      set_list_pos(level_ctrl_id, new_level_sel);
    level = c_levels[new_level_sel].value;
    arc_levels('s');
  }

  void set_control_state() {
    DisableEvents de(*this);
    bool is_7z = arc_type == c_7z;
    bool is_zip = arc_type == c_zip;
    update_level_list();
    bool is_compressed = get_list_pos(level_ctrl_id) != 0;
    for (int i = method_ctrl_id - 1; i <= method_ctrl_id; i++) {
      enable(i, (is_7z || is_zip) && is_compressed);
    }
    enable(solid_ctrl_id, is_7z && is_compressed);
    enable(encrypt_ctrl_id, is_7z || is_zip);
    bool encrypt = get_check(encrypt_ctrl_id);
    for (int i = encrypt_ctrl_id + 1; i <= password_visible_ctrl_id; i++) {
      enable(i, encrypt && (is_7z || is_zip));
    }
    enable(encrypt_header_ctrl_id, is_7z && encrypt);
    bool show_password = get_check(show_password_ctrl_id);
    for (int i = password_ctrl_id - 1; i <= password_verify_ctrl_id; i++) {
      set_visible(i, !show_password);
    }
    for (int i = password_visible_ctrl_id - 1; i <= password_visible_ctrl_id; i++) {
      set_visible(i, show_password);
    }
    if (new_arc) {
      change_extension();
      bool create_sfx = get_check(create_sfx_ctrl_id);
      bool enable_volumes = get_check(enable_volumes_ctrl_id);
      if (create_sfx && enable_volumes)
        enable_volumes = false;
      enable(create_sfx_ctrl_id, is_7z && !enable_volumes);
      for (int i = create_sfx_ctrl_id + 1; i <= sfx_options_ctrl_id; i++) {
        enable(i, is_7z && create_sfx && !enable_volumes);
      }
      enable(enable_volumes_ctrl_id, !is_7z || !create_sfx);
      for (int i = enable_volumes_ctrl_id + 1; i <= volume_size_ctrl_id; i++) {
        enable(i, enable_volumes && (!is_7z || !create_sfx));
      }
      bool other_format = get_check(other_formats_ctrl_id);
      enable(other_formats_ctrl_id + 1, other_format);
      unsigned profile_idx = get_list_pos(profile_ctrl_id);
      enable(delete_profile_ctrl_id, profile_idx != (unsigned)-1 && profile_idx < profiles.size());
    }
    enable(move_files_ctrl_id, !get_check(enable_filter_ctrl_id));
  }

  std::wstring eval_arc_path() {
    std::wstring arc_path = expand_env_vars(search_and_replace(expand_macros(strip(get_text(arc_path_ctrl_id))), L"\"", std::wstring()));
    if (arc_path.empty() || arc_path.back() == L'\\')
      arc_path += default_arc_name + get_default_ext();
    if (get_check(append_ext_ctrl_id)) {
      std::wstring ext = get_default_ext();
      if (ext.size() > arc_path.size() || upcase(arc_path.substr(arc_path.size() - ext.size())) != upcase(ext))
        arc_path += ext;
    }
    return Far::get_absolute_path(arc_path);
  }

  void read_controls(UpdateOptions& options) {
    if (new_arc) {
      options.levels = m_options.levels;
      options.append_ext = get_check(append_ext_ctrl_id);

      for (unsigned i = 0; i < main_formats.size(); i++) {
        if (get_check(main_formats_ctrl_id + i)) {
          options.arc_type = c_archive_types[i].value;
          break;
        }
      }
      if (!other_formats.empty() && get_check(other_formats_ctrl_id)) {
        options.arc_type = other_formats[get_list_pos(other_formats_ctrl_id + 1)];
      }
      if (options.arc_type.empty()) {
        FAIL_MSG(Far::get_msg(MSG_UPDATE_DLG_WRONG_ARC_TYPE));
      }
    }
    else {
      options.arc_type = arc_type;
    }
    bool is_7z = options.arc_type == c_7z;
    bool is_zip = options.arc_type == c_zip;

	options.level = (unsigned)-1;
    unsigned level_sel = get_list_pos(level_ctrl_id);
    if (level_sel < ARRAYSIZE(c_levels))
      options.level = c_levels[level_sel].value;
    if (options.level == (unsigned)-1) {
      FAIL_MSG(Far::get_msg(MSG_UPDATE_DLG_WRONG_LEVEL));
    }
    bool is_compressed = options.level != 0;

    if (is_compressed) {
      if (is_7z || is_zip) {
        options.method.clear();
        unsigned method_sel = get_list_pos(method_ctrl_id);
        const auto& codecs = ArcAPI::codecs();
        if (method_sel < ARRAYSIZE(c_methods) + codecs.size())
          options.method = method_sel < ARRAYSIZE(c_methods) ? c_methods[method_sel].value : codecs[method_sel-ARRAYSIZE(c_methods)].Name;
        if (options.method.empty()) {
          FAIL_MSG(Far::get_msg(MSG_UPDATE_DLG_WRONG_METHOD));
        }
      }
    }
    else
      options.method = c_method_copy;

    if (is_7z && is_compressed)
      options.solid = get_check(solid_ctrl_id);

    options.advanced = get_text(advanced_ctrl_id);

    if (is_7z || is_zip)
      options.encrypt = get_check(encrypt_ctrl_id);
    if (options.encrypt) {
      options.show_password = get_check(show_password_ctrl_id);
      if (options.show_password) {
        options.password = get_text(password_visible_ctrl_id);
      }
      else {
        options.password = get_text(password_ctrl_id);
        std::wstring password_verify = get_text(password_verify_ctrl_id);
        if (options.password != password_verify) {
          FAIL_MSG(Far::get_msg(MSG_UPDATE_DLG_PASSWORDS_DONT_MATCH));
        }
      }
      if (options.password.empty()) {
        FAIL_MSG(Far::get_msg(MSG_UPDATE_DLG_PASSWORD_IS_EMPTY));
      }
      if (is_7z)
        options.encrypt_header = get_check3(encrypt_header_ctrl_id);
    }

    if (new_arc) {
      options.create_sfx = is_7z && get_check(create_sfx_ctrl_id);
      if (options.create_sfx) {
        options.sfx_options = m_options.sfx_options;
        uintptr_t sfx_id = ArcAPI::sfx().find_by_name(options.sfx_options.name);
        if (sfx_id >= ArcAPI::sfx().size())
          FAIL_MSG(Far::get_msg(MSG_SFX_OPTIONS_DLG_WRONG_MODULE));
        if (options.method == c_method_ppmd && !ArcAPI::sfx()[sfx_id].all_codecs())
          FAIL_MSG(Far::get_msg(MSG_UPDATE_DLG_SFX_NO_PPMD));
        if (options.encrypt && !ArcAPI::sfx()[sfx_id].all_codecs())
          FAIL_MSG(Far::get_msg(MSG_UPDATE_DLG_SFX_NO_ENCRYPT));
      }

      options.enable_volumes = get_check(enable_volumes_ctrl_id);
      if (options.enable_volumes) {
        options.volume_size = get_text(volume_size_ctrl_id);
        if (parse_size_string(options.volume_size) < c_min_volume_size) {
          FAIL_MSG(Far::get_msg(MSG_UPDATE_DLG_WRONG_VOLUME_SIZE));
        }
      }
    }

    options.move_files = get_check(enable_filter_ctrl_id) ? false : get_check(move_files_ctrl_id);
    options.open_shared = get_check(open_shared_ctrl_id);
    options.ignore_errors = get_check(ignore_errors_ctrl_id);

    if (!new_arc) {
      if (get_check(oa_ask_ctrl_id)) options.overwrite = oaAsk;
      else if (get_check(oa_overwrite_ctrl_id)) options.overwrite = oaOverwrite;
      else if (get_check(oa_skip_ctrl_id)) options.overwrite = oaSkip;
      else options.overwrite = oaAsk;
    }
  }

  void write_controls(const ProfileOptions& options) {
    DisableEvents de(*this);
    if (new_arc) {
      arc_type = options.arc_type;
      for (unsigned i = 0; i < main_formats.size(); i++) {
        if (options.arc_type == main_formats[i]) {
          set_check(main_formats_ctrl_id + i);
          break;
        }
      }
      for (unsigned i = 0; i < other_formats.size(); i++) {
        if (options.arc_type == other_formats[i]) {
          set_check(other_formats_ctrl_id);
          set_list_pos(other_formats_ctrl_id + 1, i);
          break;
        }
      }
    }

    level = options.level;
    unsigned level_sel = 0;
    for (unsigned i = 0; i < ARRAYSIZE(c_levels); i++) {
      if (options.level == c_levels[i].value) {
        level_sel = i;
        break;
      }
    }
    set_list_pos(level_ctrl_id, level_sel);

    std::wstring method = options.method.empty() && options.arc_type == c_7z ? c_methods[0].value : options.method;
    unsigned method_sel = 0;
    const auto& codecs = ArcAPI::codecs();
    for (unsigned i = 0; i < ARRAYSIZE(c_methods)+codecs.size(); ++i) {
      std::wstring method_name = i < ARRAYSIZE(c_methods) ? c_methods[i].value : codecs[i-ARRAYSIZE(c_methods)].Name;
      if (method == method_name) {
        method_sel = i;
        break;
      }
    }
    set_list_pos(method_ctrl_id, method_sel);

    set_check(solid_ctrl_id, options.solid);

    set_text(advanced_ctrl_id, options.advanced);

    set_check(encrypt_ctrl_id, options.encrypt);
    set_check3(encrypt_header_ctrl_id, options.encrypt_header);
    set_text(password_ctrl_id, options.password);
    set_text(password_verify_ctrl_id, options.password);
    set_text(password_visible_ctrl_id, options.password);

    if (new_arc) {
      set_check(create_sfx_ctrl_id, options.create_sfx);
      m_options.sfx_options = options.sfx_options;

      set_check(enable_volumes_ctrl_id, options.enable_volumes);
      set_text(volume_size_ctrl_id, options.volume_size);
    }

    set_check(move_files_ctrl_id, options.move_files);
    set_check(ignore_errors_ctrl_id, options.ignore_errors);
  }

  void populate_profile_list() {
    DisableEvents de(*this);
    std::vector<FarListItem> fl_items;
    fl_items.reserve(profiles.size() + 1);
    FarListItem fl_item{};
    for (unsigned i = 0; i < profiles.size(); i++) {
      fl_item.Text = profiles[i].name.c_str();
      fl_items.push_back(fl_item);
    }
    fl_item.Text = L"";
    fl_items.push_back(fl_item);
    FarList fl;
    fl.StructSize = sizeof(FarList);
    fl.ItemsNumber = profiles.size() + 1;
    fl.Items = fl_items.data();
    send_message(DM_LISTSET, profile_ctrl_id, &fl);
  }

  intptr_t dialog_proc(intptr_t msg, intptr_t param1, void* param2) override {
    if (msg == DN_CLOSE && param1 >= 0 && param1 != cancel_ctrl_id) {
      read_controls(m_options);
      if (new_arc)
        m_options.arc_path = eval_arc_path();
    }
    else if (msg == DN_INITDIALOG) {
      set_control_state();
      set_focus(arc_path_ctrl_id);
    }
    else if (msg == DN_EDITCHANGE && param1 == profile_ctrl_id) {
      unsigned profile_idx = get_list_pos(profile_ctrl_id);
      if (profile_idx != (unsigned)-1 && profile_idx < profiles.size()) {
        write_controls(profiles[profile_idx].options);
        set_control_state();
      }
    }
    else if (new_arc && msg == DN_BTNCLICK && !main_formats.empty() && param1 >= main_formats_ctrl_id && param1 < main_formats_ctrl_id + static_cast<int>(main_formats.size())) {
      if (param2) {
        ArcType prev_arc_type = arc_type;
        arc_type = main_formats[param1 - main_formats_ctrl_id];
        arc_levels('g');
        if (arc_type != prev_arc_type) {
          unsigned method_sel = get_list_pos(method_ctrl_id);
          if (method_sel == 3 && arc_type == c_7z) {
            set_list_pos(method_ctrl_id, 0); // Deflate -> LZMA
          }
          else if (method_sel == 0 && arc_type == c_zip) {
            set_list_pos(method_ctrl_id, 3); // LZMA -> Deflate
          }
        }
        set_control_state();
      }
    }
    else if (new_arc && msg == DN_BTNCLICK && !other_formats.empty() && param1 == other_formats_ctrl_id) {
      if (param2) {
        arc_type = other_formats[get_list_pos(other_formats_ctrl_id + 1)];
        arc_levels('g');
        set_control_state();
      }
    }
    else if (new_arc && msg == DN_EDITCHANGE && !other_formats.empty() && param1 == other_formats_ctrl_id + 1) {
      arc_type = other_formats[get_list_pos(other_formats_ctrl_id + 1)];
      arc_levels('g');
      set_control_state();
    }
    else if (msg == DN_EDITCHANGE && param1 == level_ctrl_id) {
      unsigned level_sel = get_list_pos(level_ctrl_id);
      if (level_sel < ARRAYSIZE(c_levels))
        level = c_levels[level_sel].value;
      set_control_state();
    }
    else if (msg == DN_BTNCLICK && param1 == encrypt_ctrl_id) {
      set_control_state();
    }
    else if (new_arc && msg == DN_BTNCLICK && param1 == create_sfx_ctrl_id) {
      set_control_state();
    }
    else if (new_arc && msg == DN_BTNCLICK && param1 == enable_volumes_ctrl_id) {
      set_control_state();
    }
    else if (msg == DN_BTNCLICK && param1 == show_password_ctrl_id) {
      set_control_state();
      if (!param2) {
        set_text(password_ctrl_id, get_text(password_visible_ctrl_id));
        set_text(password_verify_ctrl_id, get_text(password_visible_ctrl_id));
      }
      else {
        set_text(password_visible_ctrl_id, get_text(password_ctrl_id));
      }
    }
    else if (new_arc && msg == DN_BTNCLICK && param1 == save_profile_ctrl_id) {
      std::wstring name;
      UpdateOptions options;
      read_controls(options);
      unsigned profile_idx = profiles.find_by_options(options);
      if (profile_idx < profiles.size())
        name = profiles[profile_idx].name;
      if (Far::input_dlg(c_save_profile_dialog_guid, Far::get_msg(MSG_PLUGIN_NAME), Far::get_msg(MSG_UPDATE_DLG_INPUT_PROFILE_NAME), name)) {
        DisableEvents de(*this);
        profiles.update(name, options);
        profiles.save();
        populate_profile_list();
        set_list_pos(profile_ctrl_id, profiles.find_by_name(name));
        set_control_state();
      }
    }
    else if (new_arc && msg == DN_BTNCLICK && param1 == delete_profile_ctrl_id) {
      unsigned profile_idx = get_list_pos(profile_ctrl_id);
      if (profile_idx != (unsigned)-1 && profile_idx < profiles.size()) {
        if (Far::message(c_delete_profile_dialog_guid, Far::get_msg(MSG_PLUGIN_NAME) + L'\n' + Far::get_msg(MSG_UPDATE_DLG_CONFIRM_PROFILE_DELETE), 0, FMSG_MB_YESNO) == 0) {
          DisableEvents de(*this);
          profiles.erase(profiles.begin() + profile_idx);
          profiles.save();
          populate_profile_list();
          set_list_pos(profile_ctrl_id, static_cast<unsigned>(profiles.size()));
          set_control_state();
        }
      }
    }
    else if (new_arc && msg == DN_BTNCLICK && param1 == arc_path_eval_ctrl_id) {
      Far::info_dlg(c_arc_path_eval_dialog_guid, std::wstring(), word_wrap(eval_arc_path(), Far::get_optimal_msg_width()));
      set_focus(arc_path_ctrl_id);
    }
    else if (msg == DN_BTNCLICK && param1 == enable_filter_ctrl_id) {
      if (param2) {
        m_options.filter.reset(new Far::FileFilter());
        if (!m_options.filter->create(PANEL_NONE, FFT_CUSTOM) || !m_options.filter->menu()) {
          m_options.filter.reset();
          DisableEvents de(*this);
          set_check(enable_filter_ctrl_id, false);
        }
      }
      else
        m_options.filter.reset();
      set_control_state();
    }
    else if (msg == DN_BTNCLICK && param1 == sfx_options_ctrl_id) {
      sfx_options_dialog(m_options.sfx_options, profiles);
      set_control_state();
    }
    else if (msg == DN_BTNCLICK && param1 == save_params_ctrl_id) {
      UpdateOptions options;
      read_controls(options);
      if (new_arc) {
        g_options.update_arc_format_name = ArcAPI::formats().at(options.arc_type).name;
        g_options.update_create_sfx = options.create_sfx;
        g_options.update_sfx_options = options.sfx_options;
        g_options.update_enable_volumes = options.enable_volumes;
        g_options.update_volume_size = options.volume_size;
        g_options.update_encrypt_header = options.encrypt_header;
        g_options.update_password = options.password;
        g_options.update_append_ext = options.append_ext;
        g_options.update_move_files = options.move_files;
      }
      else {
        g_options.update_overwrite = options.overwrite;
      }
      g_options.update_level = options.level;
      g_options.update_levels = options.levels;
      g_options.update_method = options.method;
      g_options.update_solid = options.solid;
      g_options.update_advanced = options.advanced;
      g_options.update_encrypt = options.encrypt;

      g_options.update_show_password = options.show_password;
      g_options.update_ignore_errors = options.ignore_errors;

      g_options.save();
      Far::info_dlg(c_update_params_saved_dialog_guid, Far::get_msg(MSG_UPDATE_DLG_TITLE), Far::get_msg(MSG_UPDATE_DLG_PARAMS_SAVED));
      set_focus(ok_ctrl_id);
    }

    if (new_arc && (msg == DN_EDITCHANGE || msg == DN_BTNCLICK)) {
      unsigned profile_idx = static_cast<unsigned>(profiles.size());
      UpdateOptions options;
      bool valid_options = true;
      try {
        read_controls(options);
      }
      catch (const Error&) {
        valid_options = false;
      }
      if (valid_options) {
        for (unsigned i = 0; i < profiles.size(); i++) {
          if (options == profiles[i].options) {
            profile_idx = i;
            break;
          }
        }
      }
      if (profile_idx != get_list_pos(profile_ctrl_id)) {
        DisableEvents de(*this);
        set_list_pos(profile_ctrl_id, profile_idx);
        set_control_state();
      }
    }

    return default_dialog_proc(msg, param1, param2);
  }

public:
  UpdateDialog(bool new_arc, bool multifile, UpdateOptions& options, UpdateProfiles& profiles):
    Far::Dialog(Far::get_msg(new_arc ? MSG_UPDATE_DLG_TITLE_CREATE : MSG_UPDATE_DLG_TITLE), &c_update_dialog_guid, c_client_xs, L"Update"),
    new_arc(new_arc),
    multifile(multifile),
    default_arc_name(options.arc_path),
    m_options(options),
    profiles(profiles),
    arc_type(options.arc_type),
    level(options.level) {
  }

  bool show() {
    if (new_arc) {
      if (ArcAPI::formats().count(m_options.arc_type))
        old_ext = ArcAPI::formats().at(m_options.arc_type).default_extension();

      std::vector<std::wstring> profile_names;
      profile_names.reserve(profiles.size() + 1);
      unsigned profile_idx = static_cast<unsigned>(profiles.size());
      std::for_each(profiles.begin(), profiles.end(), [&] (const UpdateProfile& profile) {
        profile_names.push_back(profile.name);
        if (profile.options == m_options)
          profile_idx = static_cast<unsigned>(profile_names.size()) - 1;
      });
      profile_names.emplace_back();
      label(Far::get_msg(MSG_UPDATE_DLG_PROFILE));
      profile_ctrl_id = combo_box(profile_names, profile_idx, 30, DIF_DROPDOWNLIST);
      spacer(1);
      save_profile_ctrl_id = button(Far::get_msg(MSG_UPDATE_DLG_SAVE_PROFILE), DIF_BTNNOCLOSE);
      spacer(1);
      delete_profile_ctrl_id = button(Far::get_msg(MSG_UPDATE_DLG_DELETE_PROFILE), DIF_BTNNOCLOSE);
      new_line();
      separator();
      new_line();

      const auto label_text = Far::get_msg(MSG_UPDATE_DLG_ARC_PATH);
      spacer(get_label_len(label_text, 0) + 1);
      arc_path_eval_ctrl_id = button(Far::get_msg(MSG_UPDATE_DLG_ARC_PATH_EVAL), DIF_BTNNOCLOSE);
      spacer(1);
      append_ext_ctrl_id = check_box(Far::get_msg(MSG_UPDATE_DLG_APPEND_EXT), m_options.append_ext);
      reset_line();
      label(label_text);
      new_line();
      arc_path_ctrl_id = history_edit_box(m_options.arc_path + old_ext, L"arclite.arc_path", c_client_xs, DIF_EDITPATH);
      new_line();
      separator();
      new_line();

      label(Far::get_msg(MSG_UPDATE_DLG_ARC_TYPE));
      spacer(1);
      const ArcFormats& arc_formats = ArcAPI::formats();
      for (unsigned i = 0; i < ARRAYSIZE(c_archive_types); i++) {
        const auto arc_iter = arc_formats.find(c_archive_types[i].value);
        if (arc_iter != arc_formats.end() && arc_iter->second.updatable) {
          bool first = main_formats.size() == 0;
          if (!first)
            spacer(1);
          int ctrl_id = radio_button(Far::get_msg(c_archive_types[i].name_id), m_options.arc_type == c_archive_types[i].value, first ? DIF_GROUP : 0);
          if (first)
            main_formats_ctrl_id = ctrl_id;
          main_formats.push_back(c_archive_types[i].value);
        }
      }

      for (const auto& [key, value]: arc_formats) {
        if (value.updatable && (!multifile || !ArcAPI::is_single_file_format(key))) {
          if (!multifile && key == c_SWFc && !is_SWFu(m_options.arc_path))
            continue;
          if (std::find(main_formats.begin(), main_formats.end(), key) == main_formats.end()) {
            other_formats.push_back(key);
          }
        }
      }
      std::sort(other_formats.begin(), other_formats.end(), [&](const auto& a, const auto& b) {
        const auto& a_format = arc_formats.at(a);
        const auto& b_format = arc_formats.at(b);
        if (a_format.lib_index != b_format.lib_index)
          return a_format.lib_index < b_format.lib_index;
        else
          return _wcsicmp(a_format.name.c_str(), b_format.name.c_str()) < 0;
      });
      std::vector<std::wstring> other_format_names;
      other_format_names.reserve(other_formats.size());
      unsigned other_format_index = 0;
      bool found = false;
      for (const auto& t : other_formats) {
        if (!found && t == m_options.arc_type) {
          other_format_index = static_cast<unsigned>(other_format_names.size());
          found = true;
        }
        other_format_names.push_back(arc_formats.at(t).name);
      }
      if (!other_formats.empty()) {
        if (!main_formats.empty())
          spacer(1);
        other_formats_ctrl_id = radio_button(Far::get_msg(MSG_UPDATE_DLG_ARC_TYPE_OTHER), found);
        combo_box(other_format_names, other_format_index, AUTO_SIZE, DIF_DROPDOWNLIST);
      }

      new_line();
    }

    label(Far::get_msg(MSG_UPDATE_DLG_LEVEL));
    std::vector<std::wstring> level_names;
    unsigned level_sel = 0;
    for (unsigned i = 0; i < ARRAYSIZE(c_levels); i++) {
      level_names.emplace_back(Far::get_msg(c_levels[i].name_id));
      if (m_options.level == c_levels[i].value)
        level_sel = i;
    }
    level_ctrl_id = combo_box(level_names, level_sel, AUTO_SIZE, DIF_DROPDOWNLIST);
    spacer(2);

    label(Far::get_msg(MSG_UPDATE_DLG_METHOD));
    std::wstring method = m_options.method.empty() && m_options.arc_type == c_7z? c_methods[0].value : m_options.method;
    std::vector<std::wstring> method_names;
    const auto sizeMethods = ARRAYSIZE(c_methods) + ArcAPI::Count7zCodecs();
    method_names.reserve(sizeMethods);
    const auto& codecs = ArcAPI::codecs();
    unsigned method_sel = 0;
    for (unsigned i = 0; i < sizeMethods; ++i) {
      std::wstring method_name = i < ARRAYSIZE(c_methods) ? c_methods[i].value : codecs[i - ARRAYSIZE(c_methods)].Name;
      if (method == method_name)
        method_sel = i;
      if (i < ARRAYSIZE(c_methods))
        method_name = Far::get_msg(c_methods[i].name_id);
      method_names.push_back(method_name);
    }
    method_ctrl_id = combo_box(method_names, method_sel, AUTO_SIZE, DIF_DROPDOWNLIST);
    spacer(2);

    solid_ctrl_id = check_box(Far::get_msg(MSG_UPDATE_DLG_SOLID), m_options.solid);
    new_line();

    label(Far::get_msg(MSG_UPDATE_DLG_ADVANCED));
    advanced_ctrl_id = history_edit_box(m_options.advanced, L"arclite.advanced");
    new_line();
    separator();
    new_line();

    encrypt_ctrl_id = check_box(Far::get_msg(MSG_UPDATE_DLG_ENCRYPT), m_options.encrypt);
    spacer(2);
    encrypt_header_ctrl_id = check_box3(Far::get_msg(MSG_UPDATE_DLG_ENCRYPT_HEADER), m_options.encrypt_header);
    spacer(2);
    show_password_ctrl_id = check_box(Far::get_msg(MSG_UPDATE_DLG_SHOW_PASSWORD), m_options.show_password);
    new_line();
    label(Far::get_msg(MSG_UPDATE_DLG_PASSWORD));
    password_ctrl_id = pwd_edit_box(m_options.password, 20);
    spacer(2);
    label(Far::get_msg(MSG_UPDATE_DLG_PASSWORD2));
    password_verify_ctrl_id = pwd_edit_box(m_options.password, 20);
    reset_line();
    label(Far::get_msg(MSG_UPDATE_DLG_PASSWORD));
    password_visible_ctrl_id = edit_box(m_options.password, 20);
    new_line();
    separator();
    new_line();

    if (new_arc) {
      create_sfx_ctrl_id = check_box(Far::get_msg(MSG_UPDATE_DLG_CREATE_SFX), m_options.create_sfx);
      spacer(2);
      sfx_options_ctrl_id = button(Far::get_msg(MSG_UPDATE_DLG_SFX_OPTIONS), DIF_BTNNOCLOSE);
      new_line();
      separator();
      new_line();

      enable_volumes_ctrl_id = check_box(Far::get_msg(MSG_UPDATE_DLG_ENABLE_VOLUMES), m_options.enable_volumes);
      spacer(2);
      label(Far::get_msg(MSG_UPDATE_DLG_VOLUME_SIZE));
      volume_size_ctrl_id = history_edit_box(m_options.volume_size, L"arclite.volume_size", 20);
      new_line();
      separator();
      new_line();
    }

    if (!new_arc) {
      label(Far::get_msg(MSG_UPDATE_DLG_OA));
      new_line();
      spacer(2);
      oa_ask_ctrl_id = radio_button(Far::get_msg(MSG_UPDATE_DLG_OA_ASK), m_options.overwrite == oaAsk);
      spacer(2);
      oa_overwrite_ctrl_id = radio_button(Far::get_msg(MSG_UPDATE_DLG_OA_OVERWRITE), m_options.overwrite == oaOverwrite || m_options.overwrite == oaOverwriteCase);
      spacer(2);
      oa_skip_ctrl_id = radio_button(Far::get_msg(MSG_UPDATE_DLG_OA_SKIP), m_options.overwrite == oaSkip);
      new_line();
      separator();
      new_line();
    }

    move_files_ctrl_id = check_box(Far::get_msg(MSG_UPDATE_DLG_MOVE_FILES), m_options.move_files);
    spacer(2);
    ignore_errors_ctrl_id = check_box(Far::get_msg(MSG_UPDATE_DLG_IGNORE_ERRORS), m_options.ignore_errors);
    new_line();
    open_shared_ctrl_id = check_box(Far::get_msg(MSG_UPDATE_DLG_OPEN_SHARED), m_options.open_shared);
    spacer(2);
    enable_filter_ctrl_id = check_box(Far::get_msg(MSG_UPDATE_DLG_ENABLE_FILTER), m_options.filter != nullptr);
    new_line();

    separator();
    new_line();
    ok_ctrl_id = def_button(Far::get_msg(MSG_BUTTON_OK), DIF_CENTERGROUP);
    cancel_ctrl_id = button(Far::get_msg(MSG_BUTTON_CANCEL), DIF_CENTERGROUP);
    save_params_ctrl_id = button(Far::get_msg(MSG_UPDATE_DLG_SAVE_PARAMS), DIF_CENTERGROUP | DIF_BTNNOCLOSE);
    new_line();

    intptr_t item = Far::Dialog::show();

    return (item != -1) && (item != cancel_ctrl_id);
  }
};

bool update_dialog(bool new_arc, bool multifile, UpdateOptions& options, UpdateProfiles& profiles) {
  return UpdateDialog(new_arc, multifile, options, profiles).show();
}


class MultiSelectDialog: public Far::Dialog {
private:
  bool read_only{};
  std::wstring items_str;
  std::wstring& selected_str;

  std::vector<std::wstring> m_items;
  int first_item_ctrl_id{};

  int ok_ctrl_id{};
  int cancel_ctrl_id{};

  std::vector<size_t> estimate_column_widths(const std::vector<std::wstring>& dialog_items) {
    SMALL_RECT console_rect;
    double window_ratio;
    if (Far::adv_control(ACTL_GETFARRECT, 0, &console_rect)) {
      window_ratio = static_cast<double>(console_rect.Right - console_rect.Left + 1) / (console_rect.Bottom - console_rect.Top + 1);
    }
    else {
      window_ratio = 80 / 25;
    }
    double window_ratio_diff = std::numeric_limits<double>::max();
    std::vector<size_t> prev_col_widths;
    for (unsigned num_cols = 1; num_cols <= dialog_items.size(); ++num_cols) {
      std::vector<size_t> col_widths(num_cols, 0);
      for (size_t i = 0; i < dialog_items.size(); ++i) {
        size_t col_index = i % num_cols;
        if (col_widths[col_index] < dialog_items[i].size())
          col_widths[col_index] = dialog_items[i].size();
      }
      size_t width = accumulate(col_widths.cbegin(), col_widths.cend(), size_t(0));
      width += num_cols * 4 + (num_cols - 1);
      size_t height = dialog_items.size() / num_cols + (dialog_items.size() % num_cols ? 1 : 0);
      double ratio = static_cast<double>(width) / static_cast<double>(height);
      double diff = fabs(ratio - window_ratio);
      if (diff > window_ratio_diff)
        break;
      window_ratio_diff = diff;
      prev_col_widths = std::move(col_widths);
    }
    return prev_col_widths;
  }

  intptr_t dialog_proc(intptr_t msg, intptr_t param1, void* param2) override {
    if (!read_only && (msg == DN_CLOSE) && (param1 >= 0) && (param1 != cancel_ctrl_id)) {
      selected_str.clear();
      for (unsigned i = 0; i < m_items.size(); ++i) {
        if (get_check(first_item_ctrl_id + i)) {
          if (!selected_str.empty())
            selected_str += L',';
          selected_str += m_items[i];
        }
      }
    }
    else if (read_only && msg == DN_CTLCOLORDLGITEM) {
      FarDialogItem dlg_item;
      if (send_message(DM_GETDLGITEMSHORT, param1, &dlg_item) && dlg_item.Type == DI_CHECKBOX) {
        FarColor color;
        if (Far::get_color(COL_DIALOGTEXT, color)) {
          FarDialogItemColors* item_colors = static_cast<FarDialogItemColors*>(param2);
          CHECK(item_colors->ColorsCount == 4);
          item_colors->Colors[0] = color;
        }
      }
    }
    return default_dialog_proc(msg, param1, param2);
  }

public:
  MultiSelectDialog(const std::wstring& title, const std::wstring& items_str, std::wstring& selected_str) :
    Far::Dialog(title, &c_multi_select_dialog_guid, 1),
    items_str(items_str),
    selected_str(selected_str)
  {}

  bool show() {
    struct ItemCompare {
      bool operator()(const std::wstring& a, const std::wstring& b) const {
        return upcase(a) < upcase(b);
      }
    };

    std::set<std::wstring, ItemCompare> selected_items;
    if (!read_only) {
      std::list<std::wstring> split_selected_str = split(selected_str, L',');
      selected_items.insert(split_selected_str.cbegin(), split_selected_str.cend());
    }

    std::list<std::wstring> split_items_str = split(items_str, L',');
    m_items.assign(split_items_str.cbegin(), split_items_str.cend());
    std::sort(m_items.begin(), m_items.end(), ItemCompare());
    if (m_items.empty())
      return false;
    std::vector<size_t> col_widths = estimate_column_widths(m_items);
    first_item_ctrl_id = -1;
    for (unsigned i = 0; i < m_items.size(); ++i) {
      unsigned col_index = i % col_widths.size();
      unsigned ctrl_id = check_box(m_items[i], read_only ? true : selected_items.count(m_items[i]) != 0, read_only ? DIF_DISABLE : 0);
      if (first_item_ctrl_id == -1)
        first_item_ctrl_id = ctrl_id;
      if (col_index != col_widths.size() - 1) {
        spacer(col_widths[col_index] - m_items[i].size() + 1);
      }
      else {
        new_line();
      }
    }
    if (m_items.size() % col_widths.size())
      new_line();

    if (read_only) {
      Far::Dialog::show();
      return true;
    }
    else {
      separator();
      new_line();
      ok_ctrl_id = def_button(Far::get_msg(MSG_BUTTON_OK), DIF_CENTERGROUP);
      cancel_ctrl_id = button(Far::get_msg(MSG_BUTTON_CANCEL), DIF_CENTERGROUP);
      new_line();

      intptr_t item = Far::Dialog::show();
      return (item != -1) && (item != cancel_ctrl_id);
    }

  }
};


class FormatLibraryInfoDialog: public Far::Dialog {
private:

  std::map<intptr_t, size_t> format_btn_map;
  std::map<intptr_t, size_t> mask_btn_map;

  std::wstring get_masks(size_t lib_index) {
    std::wstring masks;
    for (auto format_iter = ArcAPI::formats().cbegin(); format_iter != ArcAPI::formats().cend(); ++format_iter) {
      const ArcFormat& format = format_iter->second;
      if ((size_t)format.lib_index == lib_index) {
        std::for_each(format.extension_list.cbegin(), format.extension_list.cend(), [&] (const std::wstring& ext) {
          masks += L"*" + ext + L",";
        });
      }
    }
    if (!masks.empty())
      masks.erase(masks.size() - 1);
    return masks;
  }

  std::wstring get_formats(size_t lib_index) {
    std::wstring formats;
    for (auto format_iter = ArcAPI::formats().cbegin(); format_iter != ArcAPI::formats().cend(); ++format_iter) {
      const ArcFormat& format = format_iter->second;
      if ((size_t)format.lib_index == lib_index) {
        if (!formats.empty())
          formats += L',';
        formats += format.name;
      }
    }
    return formats;
  }

  intptr_t dialog_proc(intptr_t msg, intptr_t param1, void* param2) override {
    if (msg == DN_INITDIALOG) {
      FarDialogItem dlg_item;
      for (unsigned ctrl_id = 0; send_message(DM_GETDLGITEMSHORT, ctrl_id, &dlg_item); ctrl_id++) {
        if (dlg_item.Type == DI_EDIT) {
          EditorSetPosition esp = { sizeof(EditorSetPosition) };
          send_message(DM_SETEDITPOSITION, ctrl_id, &esp);
        }
      }
    }
    else if (msg == DN_CTLCOLORDLGITEM) {
      FarDialogItem dlg_item;
      if (send_message(DM_GETDLGITEMSHORT, param1, &dlg_item) && dlg_item.Type == DI_EDIT) {
        FarColor color;
        if (Far::get_color(COL_DIALOGTEXT, color)) {
          FarDialogItemColors* item_colors = static_cast<FarDialogItemColors*>(param2);
          CHECK(item_colors->ColorsCount == 4);
          item_colors->Colors[0] = color;
          item_colors->Colors[2] = color;
        }
      }
    }
    else if (msg == DN_BTNCLICK) {
      std::wstring unused;
      auto lib_iter = format_btn_map.find(param1);
      if (lib_iter != format_btn_map.end()) {
        MultiSelectDialog(Far::get_msg(MSG_SETTINGS_DLG_LIB_FORMATS), get_formats(lib_iter->second),unused).show();
      }
      else {
        lib_iter = mask_btn_map.find(param1);
        if (lib_iter != mask_btn_map.end()) {
          MultiSelectDialog(Far::get_msg(MSG_SETTINGS_DLG_LIB_MASKS), get_masks(lib_iter->second),unused).show();
        }
      }
    }
    return default_dialog_proc(msg, param1, param2);
  }

public:
  FormatLibraryInfoDialog(): Far::Dialog(Far::get_msg(MSG_SETTINGS_DLG_LIB_INFO), &c_format_library_info_dialog_guid) {
  }

  void show() {
    const ArcLibs& libs = ArcAPI::libs();
    if (libs.empty()) {
      label(Far::get_msg(MSG_SETTINGS_DLG_LIB_NOT_FOUND));
      new_line();
    }
    else {
      size_t width = 0;
      for (size_t lib_index = 0; lib_index < libs.size(); ++lib_index) {
        if (width < libs[lib_index].module_path.size())
          width = libs[lib_index].module_path.size();
      }
      width += 1;
      if (width > Far::get_optimal_msg_width())
        width = Far::get_optimal_msg_width();
      for (size_t lib_index = 0; lib_index < libs.size(); ++lib_index) {
        edit_box(libs[lib_index].module_path, width, DIF_READONLY);
        new_line();
        label(Far::get_msg(MSG_SETTINGS_DLG_LIB_VERSION) + L' ' +
          int_to_str(HIWORD(libs[lib_index].version >> 32)) + L'.' + int_to_str(LOWORD(libs[lib_index].version >> 32)) + L'.' +
          int_to_str(HIWORD(libs[lib_index].version & 0xFFFFFFFF)) + L'.' + int_to_str(LOWORD(libs[lib_index].version & 0xFFFFFFFF)));
        spacer(1);
        format_btn_map[button(Far::get_msg(MSG_SETTINGS_DLG_LIB_FORMATS), DIF_BTNNOCLOSE)] = lib_index;
        spacer(1);
        mask_btn_map[button(Far::get_msg(MSG_SETTINGS_DLG_LIB_MASKS), DIF_BTNNOCLOSE)] = lib_index;
        new_line();
      }
    }

    Far::Dialog::show();
  }
};

static size_t llen(const std::wstring& label, int add)
{
  size_t len = label.length() + add;
  if (label.find(L'&') != std::wstring::npos)
    --len;
  return len;
}

class SettingsDialog: public Far::Dialog {
private:
  enum {
    c_client_xs = 60
  };

  PluginSettings& settings;

  int handle_create_ctrl_id{};
  int handle_commands_ctrl_id{};
  int own_panel_view_mode_ctrl_id{};
  int oemCP_ctrl_id{};
  int ansiCP_ctrl_id{};
  int saveCPs_ctrl_id{};
  int use_include_masks_ctrl_id{};
  int edit_include_masks_ctrl_id{};
  int include_masks_ctrl_id{};
  int use_exclude_masks_ctrl_id{};
  int edit_exclude_masks_ctrl_id{};
  int pgdn_masks_ctrl_id{};
  int exclude_masks_ctrl_id{};
  int generate_masks_ctrl_id{};
  int default_masks_ctrl_id{};
  int use_enabled_formats_ctrl_id{};
  int edit_enabled_formats_ctrl_id{};
  int enabled_formats_ctrl_id{};
  int use_disabled_formats_ctrl_id{};
  int edit_disabled_formats_ctrl_id{};
  int disabled_formats_ctrl_id{};
  int pgdn_formats_ctrl_id{};
  int lib_info_ctrl_id{};
  int ok_ctrl_id{};
  int cancel_ctrl_id{};

  intptr_t dialog_proc(intptr_t msg, intptr_t param1, void* param2) override {
    if ((msg == DN_CLOSE) && (param1 >= 0) && (param1 != cancel_ctrl_id)) {
      settings.handle_create = get_check(handle_create_ctrl_id);
      settings.handle_commands = get_check(handle_commands_ctrl_id);
      settings.own_panel_view_mode = get_check(own_panel_view_mode_ctrl_id);
      settings.oemCP = (UINT)str_to_uint(strip(get_text(oemCP_ctrl_id)));
      settings.ansiCP = (UINT)str_to_uint(strip(get_text(ansiCP_ctrl_id)));
      settings.saveCP = get_check(saveCPs_ctrl_id);
      settings.use_include_masks = get_check(use_include_masks_ctrl_id);
      settings.include_masks = get_text(include_masks_ctrl_id);
      settings.use_exclude_masks = get_check(use_exclude_masks_ctrl_id);
      settings.exclude_masks = get_text(exclude_masks_ctrl_id);
      settings.pgdn_masks = get_check(pgdn_masks_ctrl_id);
      settings.use_enabled_formats = get_check(use_enabled_formats_ctrl_id);
      settings.enabled_formats = get_text(enabled_formats_ctrl_id);
      settings.use_disabled_formats = get_check(use_disabled_formats_ctrl_id);
      settings.disabled_formats = get_text(disabled_formats_ctrl_id);
      settings.pgdn_formats = get_check(pgdn_formats_ctrl_id);
    }
    else if (msg == DN_INITDIALOG) {
      enable(include_masks_ctrl_id, settings.use_include_masks);
      enable(edit_include_masks_ctrl_id, settings.use_include_masks && !settings.include_masks.empty());
      enable(exclude_masks_ctrl_id, settings.use_exclude_masks);
      enable(edit_exclude_masks_ctrl_id, settings.use_exclude_masks && !settings.exclude_masks.empty());
      enable(enabled_formats_ctrl_id, settings.use_enabled_formats);
      enable(edit_enabled_formats_ctrl_id, settings.use_enabled_formats);
      enable(disabled_formats_ctrl_id, settings.use_disabled_formats);
      enable(edit_disabled_formats_ctrl_id, settings.use_disabled_formats);
    }
    else if (msg == DN_BTNCLICK && param1 == use_include_masks_ctrl_id) {
      enable(include_masks_ctrl_id, param2 != nullptr);
      enable(edit_include_masks_ctrl_id, param2 != nullptr);
    }
    else if (msg == DN_BTNCLICK && param1 == edit_include_masks_ctrl_id) {
      std::wstring include_masks = get_text(include_masks_ctrl_id);
      if (MultiSelectDialog(Far::get_msg(MSG_SETTINGS_DLG_USE_INCLUDE_MASKS), include_masks, include_masks).show()) {
        set_text(include_masks_ctrl_id, include_masks);
        set_focus(include_masks_ctrl_id);
      }
    }
    else if (msg == DN_EDITCHANGE && param1 == include_masks_ctrl_id) {
      enable(edit_include_masks_ctrl_id, get_check(use_include_masks_ctrl_id) && !get_text(include_masks_ctrl_id).empty());
    }
    else if (msg == DN_BTNCLICK && param1 == use_exclude_masks_ctrl_id) {
      enable(exclude_masks_ctrl_id, param2 != nullptr);
      enable(edit_exclude_masks_ctrl_id, param2 != nullptr);
    }
    else if (msg == DN_BTNCLICK && param1 == edit_exclude_masks_ctrl_id) {
      std::wstring exclude_masks = get_text(exclude_masks_ctrl_id);
      if (MultiSelectDialog(Far::get_msg(MSG_SETTINGS_DLG_USE_EXCLUDE_MASKS), exclude_masks, exclude_masks).show()) {
        set_text(exclude_masks_ctrl_id, exclude_masks);
        set_focus(exclude_masks_ctrl_id);
      }
    }
    else if (msg == DN_EDITCHANGE && param1 == exclude_masks_ctrl_id) {
      enable(edit_exclude_masks_ctrl_id, get_check(use_exclude_masks_ctrl_id) && !get_text(exclude_masks_ctrl_id).empty());
    }
    else if (msg == DN_BTNCLICK && param1 == generate_masks_ctrl_id) {
      generate_masks();
    }
    else if (msg == DN_BTNCLICK && param1 == default_masks_ctrl_id) {
      default_masks();
    }
    else if (msg == DN_BTNCLICK && param1 == use_enabled_formats_ctrl_id) {
      enable(enabled_formats_ctrl_id, param2 != nullptr);
      enable(edit_enabled_formats_ctrl_id, param2 != nullptr);
    }
    else if (msg == DN_BTNCLICK && param1 == edit_enabled_formats_ctrl_id) {
      std::wstring enabled_formats = get_text(enabled_formats_ctrl_id);
      if (MultiSelectDialog(Far::get_msg(MSG_SETTINGS_DLG_USE_ENABLED_FORMATS), get_available_formats(), enabled_formats).show()) {
        set_text(enabled_formats_ctrl_id, enabled_formats);
        set_focus(enabled_formats_ctrl_id);
      }
    }
    else if (msg == DN_BTNCLICK && param1 == use_disabled_formats_ctrl_id) {
      enable(disabled_formats_ctrl_id, param2 != nullptr);
      enable(edit_disabled_formats_ctrl_id, param2 != nullptr);
    }
    else if (msg == DN_BTNCLICK && param1 == edit_disabled_formats_ctrl_id) {
      std::wstring disabled_formats = get_text(disabled_formats_ctrl_id);
      if (MultiSelectDialog(Far::get_msg(MSG_SETTINGS_DLG_USE_DISABLED_FORMATS), get_available_formats(), disabled_formats).show()) {
        set_text(disabled_formats_ctrl_id, disabled_formats);
        set_focus(disabled_formats_ctrl_id);
      }
    }
    else if (msg == DN_BTNCLICK && param1 == lib_info_ctrl_id) {
      FormatLibraryInfoDialog().show();
    }
    return default_dialog_proc(msg, param1, param2);
  }

  void generate_masks() {
    const ArcFormats& arc_formats = ArcAPI::formats();
    std::wstring masks;
    std::for_each(arc_formats.begin(), arc_formats.end(), [&] (const std::pair<const ArcType, ArcFormat>& arc_type_format) {
      std::for_each(arc_type_format.second.extension_list.cbegin(), arc_type_format.second.extension_list.cend(), [&] (const std::wstring& ext) {
        masks += L"*" + ext + L",";
      });
    });
    if (!masks.empty())
      masks.erase(masks.size() - 1);
    set_text(include_masks_ctrl_id, masks);
  }

  void default_masks() {
    set_text(include_masks_ctrl_id, Options().include_masks);
  }

  std::wstring get_available_formats() {
    const ArcFormats& arc_formats = ArcAPI::formats();
    std::vector<std::wstring> format_list;
    format_list.reserve(arc_formats.size());
    std::for_each(arc_formats.begin(), arc_formats.end(), [&] (const std::pair<const ArcType, ArcFormat>& arc_type_format) {
      format_list.push_back(arc_type_format.second.name);
    });
    std::sort(format_list.begin(), format_list.end(), [] (const std::wstring& a, const std::wstring& b) -> bool {
      return upcase(a) < upcase(b);
    });
    std::wstring formats;
    std::for_each(format_list.begin(), format_list.end(), [&] (const std::wstring& format_name) {
      if (!formats.empty())
        formats += L',';
      formats += format_name;
    });
    return formats;
  }

public:
  SettingsDialog(PluginSettings& settings): Far::Dialog(Far::get_msg(MSG_PLUGIN_NAME), &c_settings_dialog_guid, c_client_xs, L"Config"), settings(settings) {
  }

  bool show() {
    std::wstring box1 = Far::get_msg(MSG_SETTINGS_DLG_HANDLE_CREATE);
    std::wstring box2 = Far::get_msg(MSG_SETTINGS_DLG_HANDLE_COMMANDS);
    std::wstring box3 = Far::get_msg(MSG_SETTINGS_DLG_OWN_PANEL_VIEW_MODE);
    handle_create_ctrl_id = check_box(box1, settings.handle_create);
    new_line();
    handle_commands_ctrl_id = check_box(box2, settings.handle_commands);
    new_line();
    own_panel_view_mode_ctrl_id = check_box(box3, settings.own_panel_view_mode);
    new_line();
    separator();
    new_line();
    std::wstring label1 = Far::get_msg(MSG_SETTINGS_DLG_OEM_CODEPAGE);
    std::wstring label2 = Far::get_msg(MSG_SETTINGS_DLG_ANSI_CODEPAGE);
    std::wstring label3 = Far::get_msg(MSG_SETTINGS_DLG_SAVE_CODEPAGE);
    label(label1);
    std::wstring tmp1 = settings.oemCP ? uint_to_str(settings.oemCP) : std::wstring();
    oemCP_ctrl_id = edit_box(tmp1, 5);
    auto total = llen(label1,5) + llen(label2,5) + llen(label3,4);
    auto width = std::max(std::max(std::max(std::max(llen(box1,4), llen(box2,4)), llen(box3,4)), total+2), size_t(c_client_xs));
    auto space = (width - total) / 2;
    pad(llen(label1,5) + space);
    label(label2);
    std::wstring tmp2 = settings.ansiCP ? uint_to_str(settings.ansiCP) : std::wstring();
    ansiCP_ctrl_id = edit_box(tmp2, 5);
    pad(width - llen(label3,4));
    saveCPs_ctrl_id = check_box(label3, settings.saveCP);
    new_line();
    separator();
    new_line();

    label(Far::get_msg(MSG_SETTINGS_DLG_USE_INCLUDE_MASKS));
    spacer(1);
    use_include_masks_ctrl_id = check_box(Far::get_msg(MSG_SETTINGS_DLG_ACTIVE), settings.use_include_masks);
    spacer(1);
    edit_include_masks_ctrl_id = button(Far::get_msg(MSG_SETTINGS_DLG_EDIT), DIF_BTNNOCLOSE);
    new_line();
    include_masks_ctrl_id = edit_box(settings.include_masks, width);
    new_line();
    label(Far::get_msg(MSG_SETTINGS_DLG_USE_EXCLUDE_MASKS));
    spacer(1);
    use_exclude_masks_ctrl_id = check_box(Far::get_msg(MSG_SETTINGS_DLG_ACTIVE), settings.use_exclude_masks);
    spacer(1);
    edit_exclude_masks_ctrl_id = button(Far::get_msg(MSG_SETTINGS_DLG_EDIT), DIF_BTNNOCLOSE);
    new_line();
    exclude_masks_ctrl_id = edit_box(settings.exclude_masks, width);
    new_line();
    pgdn_masks_ctrl_id = check_box(Far::get_msg(MSG_SETTINGS_DLG_PGDN_MASKS), settings.pgdn_masks);
    new_line();
    generate_masks_ctrl_id = button(Far::get_msg(MSG_SETTINGS_DLG_GENERATE_MASKS), DIF_BTNNOCLOSE);
    spacer(1);
    default_masks_ctrl_id = button(Far::get_msg(MSG_SETTINGS_DLG_DEFAULT_MASKS), DIF_BTNNOCLOSE);
    new_line();
    separator();
    new_line();

    label(Far::get_msg(MSG_SETTINGS_DLG_USE_ENABLED_FORMATS));
    spacer(1);
    use_enabled_formats_ctrl_id = check_box(Far::get_msg(MSG_SETTINGS_DLG_ACTIVE), settings.use_enabled_formats);
    spacer(1);
    edit_enabled_formats_ctrl_id = button(Far::get_msg(MSG_SETTINGS_DLG_EDIT), DIF_BTNNOCLOSE);
    new_line();
    enabled_formats_ctrl_id = edit_box(settings.enabled_formats, width);
    new_line();
    label(Far::get_msg(MSG_SETTINGS_DLG_USE_DISABLED_FORMATS));
    spacer(1);
    use_disabled_formats_ctrl_id = check_box(Far::get_msg(MSG_SETTINGS_DLG_ACTIVE), settings.use_disabled_formats);
    spacer(1);
    edit_disabled_formats_ctrl_id = button(Far::get_msg(MSG_SETTINGS_DLG_EDIT), DIF_BTNNOCLOSE);
    new_line();
    disabled_formats_ctrl_id = edit_box(settings.disabled_formats, width);
    new_line();
    pgdn_formats_ctrl_id = check_box(Far::get_msg(MSG_SETTINGS_DLG_PGDN_FORMATS), settings.pgdn_formats);
    new_line();
    separator();
    new_line();

    lib_info_ctrl_id = button(Far::get_msg(MSG_SETTINGS_DLG_LIB_INFO), DIF_BTNNOCLOSE);
    new_line();
    separator();
    new_line();

    ok_ctrl_id = def_button(Far::get_msg(MSG_BUTTON_OK), DIF_CENTERGROUP);
    cancel_ctrl_id = button(Far::get_msg(MSG_BUTTON_CANCEL), DIF_CENTERGROUP);
    new_line();

    intptr_t item = Far::Dialog::show();

    return (item != -1) && (item != cancel_ctrl_id);
  }
};

bool settings_dialog(PluginSettings& settings) {
  return SettingsDialog(settings).show();
}


class AttrDialog: public Far::Dialog {
private:
  const AttrList& attr_list;

  intptr_t dialog_proc(intptr_t msg, intptr_t param1, void* param2) override {
    if (msg == DN_INITDIALOG) {
      FarDialogItem dlg_item;
      for (unsigned ctrl_id = 0; send_message(DM_GETDLGITEMSHORT, ctrl_id, &dlg_item); ctrl_id++) {
        if (dlg_item.Type == DI_EDIT) {
          EditorSetPosition esp = { sizeof(EditorSetPosition) };
          send_message(DM_SETEDITPOSITION, ctrl_id, &esp);
        }
      }
    }
    else if (msg == DN_CTLCOLORDLGITEM) {
      FarDialogItem dlg_item;
      if (send_message(DM_GETDLGITEMSHORT, param1, &dlg_item) && dlg_item.Type == DI_EDIT) {
        FarColor color;
        if (Far::get_color(COL_DIALOGTEXT, color)) {
          FarDialogItemColors* item_colors = static_cast<FarDialogItemColors*>(param2);
          CHECK(item_colors->ColorsCount == 4);
          item_colors->Colors[0] = color;
          item_colors->Colors[2] = color;
        }
      }
    }
    return default_dialog_proc(msg, param1, param2);
  }

public:
  AttrDialog(const AttrList& attr_list): Far::Dialog(Far::get_msg(MSG_ATTR_DLG_TITLE), &c_attr_dialog_guid), attr_list(attr_list) {
  }

  void show() {
    unsigned max_name_len = 0;
    unsigned max_value_len = 0;
    std::for_each(attr_list.begin(), attr_list.end(), [&] (const Attr& attr) {
      if (attr.name.size() > max_name_len)
        max_name_len = static_cast<unsigned>(attr.name.size());
      if (attr.value.size() > max_value_len)
        max_value_len = static_cast<unsigned>(attr.value.size());
    });
    max_value_len += 1;

    unsigned max_width = Far::get_optimal_msg_width();
    if (max_name_len > max_width / 2)
      max_name_len = max_width / 2;
    if (max_name_len + 1 + max_value_len > max_width)
      max_value_len = max_width - max_name_len - 1;

    set_width(max_name_len + 1 + max_value_len);

    std::for_each(attr_list.begin(), attr_list.end(), [&] (const Attr& attr) {
      label(attr.name, max_name_len);
      spacer(1);
      edit_box(attr.value, max_value_len, DIF_READONLY);
      new_line();
    });

    Far::Dialog::show();
  }
};

void attr_dialog(const AttrList& attr_list) {
  AttrDialog(attr_list).show();
}
