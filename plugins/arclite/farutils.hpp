#pragma once

extern wstring get_error_dlg_title();

#define FAR_ERROR_HANDLER_BEGIN \
  try { \
    try {

#define FAR_ERROR_HANDLER_END(return_error, return_cancel, silent) \
    } \
    catch (const Error& e) { \
      if (e.code == E_ABORT) { \
        return_cancel; \
      } \
      else { \
        if (!silent) \
          Far::error_dlg(get_error_dlg_title(), e); \
        return_error; \
      } \
    } \
    catch (const std::exception& e) { \
      if (!silent) \
        Far::error_dlg(get_error_dlg_title(), e); \
      return_error; \
    } \
  } \
  catch (...) { \
    return_error; \
  }

namespace Far {

void init(const PluginStartupInfo* psi);
wstring get_plugin_module_path();
wstring get_root_key_name();

#define MAKE_VERSION(major, minor, build) (((major) << 24) | ((minor) << 16) | (build))
#define VER_MAJOR(version) HIBYTE(HIWORD(version))
#define VER_MINOR(version) LOBYTE(HIWORD(version))
#define VER_BUILD(version) LOWORD(version)
unsigned get_version();

const wchar_t* msg_ptr(int id);
wstring get_msg(int id);

unsigned get_optimal_msg_width();
int message(const wstring& msg, int button_cnt = 0, DWORD flags = 0);

class MenuItems: public vector<wstring> {
public:
  unsigned add(const wstring& item);
};
int menu(const wstring& title, const MenuItems& items, const wchar_t* help = NULL);

wstring get_progress_bar_str(unsigned width, unsigned __int64 completed, unsigned __int64 total);
void set_progress_state(TBPFLAG state);
void set_progress_value(unsigned __int64 completed, unsigned __int64 total);
void progress_notify();

void call_user_apc(void* param);
void post_keys(const vector<DWORD>& keys, DWORD flags = 0);
void quit();

HANDLE save_screen();
void restore_screen(HANDLE h_scr);
void flush_screen();

int viewer(const wstring& file_name, const wstring& title, DWORD flags = 0);
int editor(const wstring& file_name, const wstring& title, DWORD flags = 0);

void update_panel(HANDLE h_panel, bool keep_selection);
void set_view_mode(HANDLE h_panel, unsigned view_mode);
void set_sort_mode(HANDLE h_panel, unsigned sort_mode);
void set_reverse_sort(HANDLE h_panel, bool reverse_sort);
void set_numeric_sort(HANDLE h_panel, bool numeric_sort);
void set_directories_first(HANDLE h_panel, bool first);

bool get_panel_info(HANDLE h_panel, PanelInfo& panel_info);
bool is_real_file_panel(const PanelInfo& panel_info);
wstring get_panel_dir(HANDLE h_panel);

struct PanelItem {
  DWORD file_attributes;
  FILETIME creation_time;
  FILETIME last_access_time;
  FILETIME last_write_time;
  unsigned __int64 file_size;
  unsigned __int64 pack_size;
  wstring file_name;
  wstring alt_file_name;
  DWORD_PTR user_data;
};
PanelItem get_current_panel_item(HANDLE h_panel);
PanelItem get_panel_item(HANDLE h_panel, unsigned index);
PanelItem get_selected_panel_item(HANDLE h_panel, unsigned index);

void error_dlg(const wstring& title, const Error& e);
void info_dlg(const wstring& title, const wstring& msg);
bool input_dlg(const wstring& title, const wstring& msg, wstring& text, DWORD flags = 0);

#define AUTO_SIZE (-1)
const unsigned c_x_frame = 5;
const unsigned c_y_frame = 2;

struct DialogItem {
  DialogItemTypes type;
  unsigned x1;
  unsigned y1;
  unsigned x2;
  unsigned y2;
  DWORD flags;
  bool focus;
  bool default_button;
  int selected;
  unsigned history_idx;
  unsigned mask_idx;
  unsigned text_idx;
  unsigned list_idx;
  unsigned list_size;
  unsigned list_pos;
  DialogItem() {
    memzero(*this);
  }
};

class Dialog {
private:
  unsigned client_xs;
  unsigned client_ys;
  unsigned x;
  unsigned y;
  const wchar_t* help;
  vector<wstring> values;
  vector<DialogItem> items;
  HANDLE h_dlg;
  const GUID* guid;
  unsigned new_value(const wstring& text);
  const wchar_t* get_value(unsigned idx) const;
  void frame(const wstring& text);
  void calc_frame_size();
  unsigned new_item(const DialogItem& di);
  static LONG_PTR WINAPI internal_dialog_proc(HANDLE h_dlg, int msg, int param1, LONG_PTR param2);
  bool events_enabled;
protected:
  class DisableEvents {
  private:
    Dialog& dlg;
    bool events_enabled;
  public:
    DisableEvents(Dialog& dlg): dlg(dlg), events_enabled(dlg.events_enabled) {
      dlg.events_enabled = false;
      dlg.send_message(DM_ENABLEREDRAW, FALSE);
    }
    ~DisableEvents() {
      dlg.send_message(DM_ENABLEREDRAW, TRUE);
      dlg.events_enabled = events_enabled;
    }
  };
  unsigned get_label_len(const wstring& str);
  LONG_PTR default_dialog_proc(int msg, int param1, LONG_PTR param2);
  virtual LONG_PTR dialog_proc(int msg, int param1, LONG_PTR param2) {
    return default_dialog_proc(msg, param1, param2);
  }
  void set_width(unsigned width) {
    client_xs = width;
  }
  LONG_PTR send_message(int msg, int param1, const void* param2 = nullptr);
public:
  Dialog(const wstring& title, const GUID* guid = nullptr, unsigned width = 60, const wchar_t* help = nullptr);
  // create different controls
  void new_line();
  void reset_line();
  void spacer(unsigned size);
  void pad(unsigned pos);
  unsigned separator();
  unsigned separator(const wstring& text);
  unsigned label(const wstring& text, unsigned boxsize = AUTO_SIZE, DWORD flags = 0);
  unsigned edit_box(const wstring& text, unsigned boxsize = AUTO_SIZE, DWORD flags = 0);
  unsigned mask_edit_box(const wstring& text, const wstring& mask, unsigned boxsize = AUTO_SIZE, DWORD flags = 0);
  unsigned history_edit_box(const wstring& text, const wstring& history_name, unsigned boxsize = AUTO_SIZE, DWORD flags = 0);
  unsigned fix_edit_box(const wstring& text, unsigned boxsize = AUTO_SIZE, DWORD flags = 0);
  unsigned pwd_edit_box(const wstring& text, unsigned boxsize = AUTO_SIZE, DWORD flags = 0);
  unsigned button(const wstring& text, DWORD flags = 0, bool def = false);
  unsigned def_button(const wstring& text, DWORD flags = 0) {
    return button(text, flags, true);
  }
  unsigned check_box(const wstring& text, int value, DWORD flags = 0);
  unsigned check_box(const wstring& text, bool value, DWORD flags = 0) {
    return check_box(text, value ? BSTATE_CHECKED : BSTATE_UNCHECKED, flags);
  }
  unsigned check_box3(const wstring& text, bool value, bool value_defined, DWORD flags = 0) {
    return check_box(text, value_defined ? (value ? BSTATE_CHECKED : BSTATE_UNCHECKED) : BSTATE_3STATE, flags | DIF_3STATE);
  }
  unsigned check_box3(const wstring& text, TriState value, DWORD flags = 0) {
    return check_box(text, value == triUndef ? BSTATE_3STATE : value == triTrue ? BSTATE_CHECKED : BSTATE_UNCHECKED, flags | DIF_3STATE);
  }
  unsigned radio_button(const wstring& text, bool value, DWORD flags = 0);
  unsigned combo_box(const vector<wstring>& items, unsigned sel_idx, unsigned boxsize = AUTO_SIZE, DWORD flags = 0);
  // display dialog
  int show();
  // utilities to set/get control values
  wstring get_text(unsigned ctrl_id) const;
  void set_text(unsigned ctrl_id, const wstring& text);
  bool get_check(unsigned ctrl_id) const;
  void set_check(unsigned ctrl_id, bool check = true);
  TriState get_check3(unsigned ctrl_id) const;
  void set_check3(unsigned ctrl_id, TriState check);
  unsigned get_list_pos(unsigned ctrl_id) const;
  void set_list_pos(unsigned ctrl_id, unsigned pos);
  void set_color(unsigned ctrl_id, unsigned char color);
  void set_focus(unsigned ctrl_id);
  void enable(unsigned ctrl_id, bool enable);
  void set_visible(unsigned ctrl_id, bool visible);
};

class Regex: private NonCopyable {
private:
  HANDLE h_regex;
public:
  Regex();
  ~Regex();
  size_t search(const wstring& expr, const wstring& text);
};

class Selection {
private:  
  HANDLE h_plugin;
public:  
  Selection(HANDLE h_plugin);
  ~Selection();
  void select(unsigned idx, bool value);
};

class FileFilter: private NonCopyable {
private:
  HANDLE h_filter;
  void clean();
public:
  FileFilter();
  ~FileFilter();
  bool create(HANDLE h_panel, int type);
  bool menu();
  void start();
  bool match(const FAR_FIND_DATA& find_data);
};

wstring get_absolute_path(const wstring& rel_path);
INT_PTR adv_control(int command, void* param = nullptr);
bool match_masks(const wstring& file_name, const wstring& masks);
unsigned char get_colors(PaletteColors color_id);
bool panel_go_to_dir(HANDLE h_panel, const wstring& dir);
bool panel_go_to_file(HANDLE h_panel, const wstring& file_path);
DWORD get_lang_id();
void close_plugin(HANDLE h_plugin, const wstring& dir);
void open_help(const wstring& topic);

};
