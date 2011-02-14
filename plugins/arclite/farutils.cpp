#include "utils.hpp"
#include "sysutils.hpp"
#include "farutils.hpp"
#include "guids.hpp"

namespace Far {

PluginStartupInfo g_far;
FarStandardFunctions g_fsf;

void init(const PluginStartupInfo* psi) {
  g_far = *psi;
  g_fsf = *psi->FSF;
}

wstring get_plugin_module_path() {
  return extract_file_path(g_far.ModuleName);
}

wstring get_root_key_name() {
  return g_far.RootKey;
}

unsigned get_version() {
  DWORD version;
  g_far.AdvControl(&c_plugin_guid, ACTL_GETFARVERSION, &version);
  return (LOWORD(version) << 16) | HIWORD(version);
}

const wchar_t* msg_ptr(int id) {
  return g_far.GetMsg(&c_plugin_guid, id);
}

wstring get_msg(int id) {
  return g_far.GetMsg(&c_plugin_guid, id);
}

unsigned get_optimal_msg_width() {
  HANDLE con = GetStdHandle(STD_OUTPUT_HANDLE);
  if (con != INVALID_HANDLE_VALUE) {
    CONSOLE_SCREEN_BUFFER_INFO con_info;
    if (GetConsoleScreenBufferInfo(con, &con_info)) {
      unsigned con_width = con_info.srWindow.Right - con_info.srWindow.Left + 1;
      if (con_width >= 80)
        return con_width - 20;
    }
  }
  return 60;
}

int message(const wstring& msg, int button_cnt, unsigned __int64 flags) {
  return g_far.Message(&c_plugin_guid, flags | FMSG_ALLINONE, NULL, reinterpret_cast<const wchar_t* const*>(msg.c_str()), 0, button_cnt);
}

unsigned MenuItems::add(const wstring& item) {
  push_back(item);
  return static_cast<unsigned>(size() - 1);
}

int menu(const wstring& title, const MenuItems& items, const wchar_t* help) {
  vector<FarMenuItem> menu_items;
  menu_items.reserve(items.size());
  FarMenuItem mi;
  for (unsigned i = 0; i < items.size(); i++) {
    memzero(mi);
    mi.Text = items[i].c_str();
    menu_items.push_back(mi);
  }
  return g_far.Menu(&c_plugin_guid, -1, -1, 0, FMENU_WRAPMODE, title.c_str(), NULL, help, NULL, NULL, menu_items.data(), static_cast<int>(menu_items.size()));
}

wstring get_progress_bar_str(unsigned width, unsigned __int64 completed, unsigned __int64 total) {
  const wchar_t c_pb_black = 9608;
  const wchar_t c_pb_white = 9617;
  unsigned len1;
  if (total == 0)
    len1 = 0;
  else
    len1 = static_cast<unsigned>(static_cast<double>(completed) * width / total);
  if (len1 > width)
    len1 = width;
  unsigned len2 = width - len1;
  wstring result;
  result.append(len1, c_pb_black);
  result.append(len2, c_pb_white);
  return result;
}

void set_progress_state(TBPFLAG state) {
  g_far.AdvControl(&c_plugin_guid, ACTL_SETPROGRESSSTATE, reinterpret_cast<void*>(state));
}

void set_progress_value(unsigned __int64 completed, unsigned __int64 total) {
  PROGRESSVALUE pv;
  pv.Completed = completed;
  pv.Total = total;
  g_far.AdvControl(&c_plugin_guid, ACTL_SETPROGRESSVALUE, &pv);
}

void progress_notify() {
  g_far.AdvControl(&c_plugin_guid, ACTL_PROGRESSNOTIFY, nullptr);
}

void call_user_apc(void* param) {
  g_far.AdvControl(&c_plugin_guid, ACTL_SYNCHRO, param);
}

bool post_macro(const wstring& macro) {
  MacroSendMacroText mcmd = { sizeof(MacroSendMacroText) };
  mcmd.SequenceText = macro.c_str();
  return g_far.MacroControl(0, MCTL_SENDSTRING, MSSC_POST, reinterpret_cast<INT_PTR>(&mcmd)) != 0;
}

void quit() {
  g_far.AdvControl(&c_plugin_guid, ACTL_QUIT, 0);
}

HANDLE save_screen() {
  return g_far.SaveScreen(0, 0, -1, -1);
}

void restore_screen(HANDLE h_scr) {
  g_far.RestoreScreen(h_scr);
}

void flush_screen() {
  g_far.Text(0, 0, 0, NULL); // flush buffer hack
  g_far.AdvControl(&c_plugin_guid, ACTL_REDRAWALL, 0);
}

int viewer(const wstring& file_name, const wstring& title, unsigned __int64 flags) {
  return g_far.Viewer(file_name.c_str(), title.c_str(), 0, 0, -1, -1, flags, CP_AUTODETECT);
}

int editor(const wstring& file_name, const wstring& title, unsigned __int64 flags) {
  return g_far.Editor(file_name.c_str(), title.c_str(), 0, 0, -1, -1, flags, 1, 1, CP_AUTODETECT);
}

void update_panel(HANDLE h_panel, bool keep_selection) {
  g_far.Control(h_panel, FCTL_UPDATEPANEL, keep_selection ? 1 : 0, 0);
  g_far.Control(h_panel, FCTL_REDRAWPANEL, 0, 0);
}

void set_view_mode(HANDLE h_panel, unsigned view_mode) {
  g_far.Control(h_panel, FCTL_SETVIEWMODE, view_mode, 0);
}

void set_sort_mode(HANDLE h_panel, unsigned sort_mode) {
  g_far.Control(h_panel, FCTL_SETSORTMODE, sort_mode, 0);
}

void set_reverse_sort(HANDLE h_panel, bool reverse_sort) {
  g_far.Control(h_panel, FCTL_SETSORTORDER, reverse_sort ? 1 : 0, 0);
}

void set_numeric_sort(HANDLE h_panel, bool numeric_sort) {
  g_far.Control(h_panel, FCTL_SETNUMERICSORT, numeric_sort ? 1 : 0, 0);
}

void set_directories_first(HANDLE h_panel, bool first) {
  g_far.Control(h_panel, FCTL_SETDIRECTORIESFIRST, first ? 1 : 0, 0);
}

bool get_panel_info(HANDLE h_panel, PanelInfo& panel_info) {
  return g_far.Control(h_panel, FCTL_GETPANELINFO, 0, reinterpret_cast<INT_PTR>(&panel_info)) == TRUE;
}

bool is_real_file_panel(const PanelInfo& panel_info) {
  return panel_info.PanelType == PTYPE_FILEPANEL && (panel_info.Flags & PFLAGS_REALNAMES);
}

wstring get_panel_dir(HANDLE h_panel) {
  Buffer<wchar_t> buf(MAX_PATH);
  unsigned size = g_far.Control(h_panel, FCTL_GETPANELDIR, static_cast<int>(buf.size()), reinterpret_cast<INT_PTR>(buf.data()));
  if (size > buf.size()) {
    buf.resize(size);
    size = g_far.Control(h_panel, FCTL_GETPANELDIR, static_cast<int>(buf.size()), reinterpret_cast<INT_PTR>(buf.data()));
  }
  CHECK(size)
  return wstring(buf.data(), size - 1);
}

PanelItem get_current_panel_item(HANDLE h_panel) {
  Buffer<unsigned char> buf(0x1000);
  unsigned size = g_far.Control(h_panel, FCTL_GETCURRENTPANELITEM, static_cast<int>(buf.size()), reinterpret_cast<INT_PTR>(buf.data()));
  if (size > buf.size()) {
    buf.resize(size);
    size = g_far.Control(h_panel, FCTL_GETCURRENTPANELITEM, static_cast<int>(buf.size()), reinterpret_cast<INT_PTR>(buf.data()));
  }
  CHECK(size)
  const PluginPanelItem* panel_item = reinterpret_cast<const PluginPanelItem*>(buf.data());
  PanelItem pi;
  pi.file_attributes = panel_item->FileAttributes;
  pi.creation_time = panel_item->CreationTime;
  pi.last_access_time = panel_item->LastAccessTime;
  pi.last_write_time = panel_item->LastWriteTime;
  pi.file_size = panel_item->FileSize;
  pi.pack_size = panel_item->PackSize;
  pi.file_name = panel_item->FileName;
  pi.alt_file_name = panel_item->AlternateFileName;
  pi.user_data = panel_item->UserData;
  return pi;
}

PanelItem get_panel_item(HANDLE h_panel, int command, int index) {
  unsigned size = g_far.Control(h_panel, command, index, 0);
  Buffer<unsigned char> buf(size);
  size = g_far.Control(h_panel, command, index, reinterpret_cast<INT_PTR>(buf.data()));
  CHECK(size)
  const PluginPanelItem* panel_item = reinterpret_cast<const PluginPanelItem*>(buf.data());
  PanelItem pi;
  pi.file_attributes = panel_item->FileAttributes;
  pi.creation_time = panel_item->CreationTime;
  pi.last_access_time = panel_item->LastAccessTime;
  pi.last_write_time = panel_item->LastWriteTime;
  pi.file_size = panel_item->FileSize;
  pi.pack_size = panel_item->PackSize;
  pi.file_name = panel_item->FileName;
  pi.alt_file_name = panel_item->AlternateFileName;
  pi.user_data = panel_item->UserData;
  return pi;
}

PanelItem get_panel_item(HANDLE h_panel, unsigned index) {
  return get_panel_item(h_panel, FCTL_GETPANELITEM, index);
}

PanelItem get_selected_panel_item(HANDLE h_panel, unsigned index) {
  return get_panel_item(h_panel, FCTL_GETSELECTEDPANELITEM, index);
}

void error_dlg(const wstring& title, const Error& e) {
  wostringstream st;
  st << title << L'\n';
  if (e.code != E_MESSAGE) {
    wstring sys_msg = get_system_message(e.code, get_lang_id());
    if (!sys_msg.empty())
      st << word_wrap(sys_msg, get_optimal_msg_width()) << L'\n';
  }
  for (list<wstring>::const_iterator msg = e.messages.begin(); msg != e.messages.end(); msg++) {
    st << word_wrap(*msg, get_optimal_msg_width()) << L'\n';
  }
  st << extract_file_name(widen(e.file)) << L':' << e.line;
  message(st.str(), 0, FMSG_WARNING | FMSG_MB_OK);
}

void info_dlg(const wstring& title, const wstring& msg) {
  message(title + L'\n' + msg, 0, FMSG_MB_OK);
}

bool input_dlg(const wstring& title, const wstring& msg, wstring& text, unsigned __int64 flags) {
  Buffer<wchar_t> buf(1024);
  if (g_far.InputBox(&c_plugin_guid, title.c_str(), msg.c_str(), nullptr, text.c_str(), buf.data(), static_cast<int>(buf.size()), nullptr, flags)) {
    text.assign(buf.data());
    return true;
  }
  return false;
}

unsigned Dialog::get_label_len(const wstring& str) {
  unsigned cnt = 0;
  for (unsigned i = 0; i < str.size(); i++) {
    if (str[i] != '&') cnt++;
  }
  return cnt;
}

unsigned Dialog::new_value(const wstring& text) {
  values.push_back(text);
  return static_cast<unsigned>(values.size());
}

const wchar_t* Dialog::get_value(unsigned idx) const {
  return values[idx - 1].c_str();
}

void Dialog::frame(const wstring& text) {
  DialogItem di;
  di.type = DI_DOUBLEBOX;
  di.x1 = c_x_frame - 2;
  di.y1 = c_y_frame - 1;
  di.x2 = c_x_frame + client_xs + 1;
  di.y2 = c_y_frame + client_ys;
  di.text_idx = new_value(text);
  new_item(di);
}

void Dialog::calc_frame_size() {
  client_ys = y - c_y_frame;
  DialogItem& di = items.front(); // dialog frame
  di.x2 = c_x_frame + client_xs + 1;
  di.y2 = c_y_frame + client_ys;
}

unsigned Dialog::new_item(const DialogItem& di) {
  items.push_back(di);
  return static_cast<unsigned>(items.size()) - 1;
}

INT_PTR WINAPI Dialog::internal_dialog_proc(HANDLE h_dlg, int msg, int param1, INT_PTR param2) {
  Dialog* dlg = reinterpret_cast<Dialog*>(g_far.SendDlgMessage(h_dlg, DM_GETDLGDATA, 0, 0));
  dlg->h_dlg = h_dlg;
  FAR_ERROR_HANDLER_BEGIN
  if (!dlg->events_enabled)
    return dlg->default_dialog_proc(msg, param1, param2);
  else
    return dlg->dialog_proc(msg, param1, param2);
  FAR_ERROR_HANDLER_END(return 0, return 0, false)
}

INT_PTR Dialog::default_dialog_proc(int msg, int param1, INT_PTR param2) {
  return g_far.DefDlgProc(h_dlg, msg, param1, param2);
}

INT_PTR Dialog::send_message(int msg, int param1, const void* param2) {
  return g_far.SendDlgMessage(h_dlg, msg, param1, reinterpret_cast<INT_PTR>(param2));
}

Dialog::Dialog(const wstring& title, const GUID* guid, unsigned width, const wchar_t* help): guid(guid), client_xs(width), x(c_x_frame), y(c_y_frame), help(help), events_enabled(true) {
  frame(title);
}

void Dialog::new_line() {
  x = c_x_frame;
  y++;
}

void Dialog::reset_line() {
  x = c_x_frame;
}

void Dialog::spacer(unsigned size) {
  x += size;
  if (x - c_x_frame > client_xs)
    client_xs = x - c_x_frame;
}

void Dialog::pad(unsigned pos) {
  if (pos > x - c_x_frame) spacer(pos - (x - c_x_frame));
}

unsigned Dialog::separator() {
  return separator(wstring());
}

unsigned Dialog::separator(const wstring& text) {
  DialogItem di;
  di.type = DI_TEXT;
  di.y1 = y;
  di.y2 = y;
  di.flags = DIF_SEPARATOR;
  if (!text.empty()) {
    di.flags |= DIF_CENTERTEXT;
    di.text_idx = new_value(text);
  }
  return new_item(di);
}

unsigned Dialog::label(const wstring& text, unsigned boxsize, FarDialogItemFlags flags) {
  DialogItem di;
  di.type = DI_TEXT;
  di.x1 = x;
  di.y1 = y;
  if (boxsize == AUTO_SIZE)
    x += get_label_len(text);
  else
    x += boxsize;
  if (x - c_x_frame > client_xs)
    client_xs = x - c_x_frame;
  di.x2 = x - 1;
  di.y2 = y;
  di.flags = flags;
  di.text_idx = new_value(text);
  return new_item(di);
}

unsigned Dialog::edit_box(const wstring& text, unsigned boxsize, FarDialogItemFlags flags) {
  DialogItem di;
  di.type = DI_EDIT;
  di.x1 = x;
  di.y1 = y;
  if (boxsize == AUTO_SIZE)
    x = c_x_frame + client_xs;
  else
    x += boxsize;
  if (x - c_x_frame > client_xs)
    client_xs = x - c_x_frame;
  di.x2 = x - 1 - (flags & DIF_HISTORY ? 1 : 0);
  di.y2 = y;
  di.flags = flags;
  di.text_idx = new_value(text);
  return new_item(di);
}

unsigned Dialog::history_edit_box(const wstring& text, const wstring& history_name, unsigned boxsize, FarDialogItemFlags flags) {
  unsigned idx = edit_box(text, boxsize, flags | DIF_HISTORY);
  items[idx].history_idx = new_value(history_name);
  return idx;
}

unsigned Dialog::mask_edit_box(const wstring& text, const wstring& mask, unsigned boxsize, FarDialogItemFlags flags) {
  unsigned idx = fix_edit_box(text, boxsize, flags | DIF_MASKEDIT);
  items[idx].mask_idx = new_value(mask);
  return idx;
}

unsigned Dialog::fix_edit_box(const wstring& text, unsigned boxsize, FarDialogItemFlags flags) {
  DialogItem di;
  di.type = DI_FIXEDIT;
  di.x1 = x;
  di.y1 = y;
  if (boxsize == AUTO_SIZE)
    x += static_cast<unsigned>(text.size());
  else
    x += boxsize;
  if (x - c_x_frame > client_xs)
    client_xs = x - c_x_frame;
  di.x2 = x - 1;
  di.y2 = y;
  di.flags = flags;
  di.text_idx = new_value(text);
  return new_item(di);
}

unsigned Dialog::pwd_edit_box(const wstring& text, unsigned boxsize, FarDialogItemFlags flags) {
  DialogItem di;
  di.type = DI_PSWEDIT;
  di.x1 = x;
  di.y1 = y;
  if (boxsize == AUTO_SIZE)
    x = c_x_frame + client_xs;
  else
    x += boxsize;
  if (x - c_x_frame > client_xs)
    client_xs = x - c_x_frame;
  di.x2 = x - 1;
  di.y2 = y;
  di.flags = flags;
  di.text_idx = new_value(text);
  return new_item(di);
}

unsigned Dialog::button(const wstring& text, FarDialogItemFlags flags) {
  DialogItem di;
  di.type = DI_BUTTON;
  di.x1 = x;
  di.y1 = y;
  x += get_label_len(text) + 4;
  if (x - c_x_frame > client_xs)
    client_xs = x - c_x_frame;
  di.y2 = y;
  di.flags = flags;
  di.text_idx = new_value(text);
  return new_item(di);
}

unsigned Dialog::check_box(const wstring& text, int value, FarDialogItemFlags flags) {
  DialogItem di;
  di.type = DI_CHECKBOX;
  di.x1 = x;
  di.y1 = y;
  x += get_label_len(text) + 4;
  if (x - c_x_frame > client_xs)
    client_xs = x - c_x_frame;
  di.y2 = y;
  di.flags = flags;
  di.selected = value;
  di.text_idx = new_value(text);
  return new_item(di);
}

unsigned Dialog::radio_button(const wstring& text, bool value, FarDialogItemFlags flags) {
  DialogItem di;
  di.type = DI_RADIOBUTTON;
  di.x1 = x;
  di.y1 = y;
  x += get_label_len(text) + 4;
  if (x - c_x_frame > client_xs)
    client_xs = x - c_x_frame;
  di.y2 = y;
  di.flags = flags;
  di.selected = value ? 1 : 0;
  di.text_idx = new_value(text);
  return new_item(di);
}

unsigned Dialog::combo_box(const vector<wstring>& list_items, unsigned sel_idx, unsigned boxsize, FarDialogItemFlags flags) {
  DialogItem di;
  di.type = DI_COMBOBOX;
  di.x1 = x;
  di.y1 = y;
  if (boxsize == AUTO_SIZE) {
    if (flags & DIF_DROPDOWNLIST) {
      unsigned max_len = 1;
      for (unsigned i = 0; i < list_items.size(); i++) {
        if (max_len < list_items[i].size())
          max_len = static_cast<unsigned>(list_items[i].size());
      }
      x += max_len + 5;
    }
    else
      x = c_x_frame + client_xs;
  }
  else
    x += boxsize;
  if (x - c_x_frame > client_xs)
    client_xs = x - c_x_frame;
  di.x2 = x - 1 - 1; // -1 for down arrow
  di.y2 = y;
  di.flags = flags;
  for (unsigned i = 0; i < list_items.size(); i++) {
    if (di.list_idx)
      new_value(list_items[i]);
    else
      di.list_idx = new_value(list_items[i]);
  }
  di.list_size = static_cast<unsigned>(list_items.size());
  di.list_pos = sel_idx;
  return new_item(di);
}

int Dialog::show() {
  calc_frame_size();

  unsigned list_cnt = 0;
  unsigned list_item_cnt = 0;
  for (unsigned i = 0; i < items.size(); i++) {
    if (items[i].list_idx) {
      list_cnt++;
      list_item_cnt += items[i].list_size;
    }
  }
  Buffer<FarList> far_lists(list_cnt);
  far_lists.clear();
  Buffer<FarListItem> far_list_items(list_item_cnt);
  far_list_items.clear();

  Buffer<FarDialogItem> dlg_items(items.size());
  dlg_items.clear();
  unsigned fl_idx = 0;
  unsigned fli_idx = 0;
  for (unsigned i = 0; i < items.size(); i++) {
    FarDialogItem* dlg_item = dlg_items.data() + i;
    dlg_item->Type = items[i].type;
    dlg_item->X1 = items[i].x1;
    dlg_item->Y1 = items[i].y1;
    dlg_item->X2 = items[i].x2;
    dlg_item->Y2 = items[i].y2;
    dlg_item->Flags = items[i].flags;
    dlg_item->Selected = items[i].selected;
    if (items[i].history_idx)
      dlg_item->History = get_value(items[i].history_idx);
    if (items[i].mask_idx)
      dlg_item->Mask = get_value(items[i].mask_idx);
    if (items[i].text_idx)
      dlg_item->PtrData = get_value(items[i].text_idx);
    if (items[i].list_idx) {
      FarList* fl = far_lists.data() + fl_idx;
      fl->Items = far_list_items.data() + fli_idx;
      fl->ItemsNumber = items[i].list_size;
      for (unsigned j = 0; j < items[i].list_size; j++) {
        FarListItem* fli = far_list_items.data() + fli_idx;
        if (j == items[i].list_pos)
          fli->Flags = LIF_SELECTED;
        fli->Text = get_value(items[i].list_idx + j);
        fli_idx++;
      }
      fl_idx++;
      dlg_item->ListItems = fl;
    }
  }

  int res = -1;
  HANDLE h_dlg = g_far.DialogInit(&c_plugin_guid, guid, -1, -1, client_xs + 2 * c_x_frame, client_ys + 2 * c_y_frame, help, dlg_items.data(), static_cast<unsigned>(dlg_items.size()), 0, 0, internal_dialog_proc, reinterpret_cast<INT_PTR>(this));
  if (h_dlg != INVALID_HANDLE_VALUE) {
    res = g_far.DialogRun(h_dlg);
    g_far.DialogFree(h_dlg);
  }
  return res;
}

wstring Dialog::get_text(unsigned ctrl_id) const {
  size_t len = g_far.SendDlgMessage(h_dlg, DM_GETTEXTLENGTH, ctrl_id, 0);
  Buffer<wchar_t> buf(len + 1);
  g_far.SendDlgMessage(h_dlg, DM_GETTEXTPTR, ctrl_id, reinterpret_cast<INT_PTR>(buf.data()));
  return wstring(buf.data(), len);
}

void Dialog::set_text(unsigned ctrl_id, const wstring& text) {
  g_far.SendDlgMessage(h_dlg, DM_SETTEXTPTR, ctrl_id, reinterpret_cast<INT_PTR>(text.c_str()));
}

bool Dialog::get_check(unsigned ctrl_id) const {
  return DlgItem_GetCheck(g_far, h_dlg, ctrl_id) == BSTATE_CHECKED;
}

void Dialog::set_check(unsigned ctrl_id, bool check) {
  g_far.SendDlgMessage(h_dlg, DM_SETCHECK, ctrl_id, check ? BSTATE_CHECKED : BSTATE_UNCHECKED);
}

TriState Dialog::get_check3(unsigned ctrl_id) const {
  INT_PTR value = DlgItem_GetCheck(g_far, h_dlg, ctrl_id);
  return value == BSTATE_3STATE ? triUndef : value == BSTATE_CHECKED ? triTrue : triFalse;
}

void Dialog::set_check3(unsigned ctrl_id, TriState check) {
  g_far.SendDlgMessage(h_dlg, DM_SETCHECK, ctrl_id, check == triUndef ? BSTATE_3STATE : check == triTrue ? BSTATE_CHECKED : BSTATE_UNCHECKED);
}

unsigned Dialog::get_list_pos(unsigned ctrl_id) const {
  return static_cast<unsigned>(g_far.SendDlgMessage(h_dlg, DM_LISTGETCURPOS, ctrl_id, 0));
}

void Dialog::set_list_pos(unsigned ctrl_id, unsigned pos) {
  FarListPos list_pos;
  list_pos.SelectPos = pos;
  list_pos.TopPos = -1;
  g_far.SendDlgMessage(h_dlg, DM_LISTSETCURPOS, ctrl_id, reinterpret_cast<INT_PTR>(&list_pos));
}

void Dialog::set_color(unsigned ctrl_id, unsigned char color) {
  size_t size = g_far.SendDlgMessage(h_dlg, DM_GETDLGITEM, ctrl_id, NULL);
  Buffer<unsigned char> buf(size);
  FarDialogItem* dlg_item = reinterpret_cast<FarDialogItem*>(buf.data());
  g_far.SendDlgMessage(h_dlg, DM_GETDLGITEM, ctrl_id, reinterpret_cast<INT_PTR>(dlg_item));
  dlg_item->Flags &= ~DIF_COLORMASK;
  dlg_item->Flags |= DIF_SETCOLOR | color;
  g_far.SendDlgMessage(h_dlg, DM_SETDLGITEM, ctrl_id, reinterpret_cast<INT_PTR>(dlg_item));
}

void Dialog::set_focus(unsigned ctrl_id) {
  g_far.SendDlgMessage(h_dlg, DM_SETFOCUS, ctrl_id, 0);
}

void Dialog::enable(unsigned ctrl_id, bool enable) {
  g_far.SendDlgMessage(h_dlg, DM_ENABLE, ctrl_id, enable ? TRUE : FALSE);
}

void Dialog::set_visible(unsigned ctrl_id, bool visible) {
  g_far.SendDlgMessage(h_dlg, DM_SHOWITEM, ctrl_id, visible ? 1 : 0);
}

Regex::Regex(): h_regex(INVALID_HANDLE_VALUE) {
  CHECK(g_far.RegExpControl(0, RECTL_CREATE, 0, reinterpret_cast<INT_PTR>(&h_regex)));
}

Regex::~Regex() {
  if (h_regex != INVALID_HANDLE_VALUE)
    CHECK(g_far.RegExpControl(h_regex, RECTL_FREE, 0, 0));
}

size_t Regex::search(const wstring& expr, const wstring& text) {
  CHECK(g_far.RegExpControl(h_regex, RECTL_COMPILE, 0, reinterpret_cast<INT_PTR>((L"/" + expr + L"/").c_str())));
  CHECK(g_far.RegExpControl(h_regex, RECTL_OPTIMIZE, 0, 0));
  RegExpSearch regex_search;
  memzero(regex_search);
  regex_search.Text = text.c_str();
  regex_search.Position = 0;
  regex_search.Length = static_cast<int>(text.size());
  RegExpMatch regex_match;
  regex_search.Match = &regex_match;
  regex_search.Count = 1;
  if (g_far.RegExpControl(h_regex, RECTL_SEARCHEX, 0, reinterpret_cast<INT_PTR>(&regex_search)))
    return regex_search.Match[0].start;
  else
    return -1;
}

Selection::Selection(HANDLE h_plugin): h_plugin(h_plugin) {
  g_far.Control(h_plugin, FCTL_BEGINSELECTION, 0, 0);
}

Selection::~Selection() {
  g_far.Control(h_plugin, FCTL_ENDSELECTION, 0, 0);
}

void Selection::select(unsigned idx, bool value) {
  g_far.Control(h_plugin, FCTL_SETSELECTION, idx, value ? TRUE : FALSE);
}

FileFilter::FileFilter(): h_filter(INVALID_HANDLE_VALUE) {
}

FileFilter::~FileFilter() {
  clean();
}

void FileFilter::clean() {
  if (h_filter != INVALID_HANDLE_VALUE) {
    g_far.FileFilterControl(h_filter, FFCTL_FREEFILEFILTER, 0, 0);
    h_filter = INVALID_HANDLE_VALUE;
  }
}

bool FileFilter::create(HANDLE h_panel, int type) {
  clean();
  return g_far.FileFilterControl(h_panel, FFCTL_CREATEFILEFILTER, type, reinterpret_cast<INT_PTR>(&h_filter)) != FALSE;
}

bool FileFilter::menu() {
  return g_far.FileFilterControl(h_filter, FFCTL_OPENFILTERSMENU, 0, 0) != FALSE;
}

void FileFilter::start() {
  g_far.FileFilterControl(h_filter, FFCTL_STARTINGTOFILTER, 0, 0);
}

bool FileFilter::match(const PluginPanelItem& panel_item) {
  return g_far.FileFilterControl(h_filter, FFCTL_ISFILEINFILTER, 0, reinterpret_cast<INT_PTR>(&panel_item)) != FALSE;
}

wstring get_absolute_path(const wstring& rel_path) {
  Buffer<wchar_t> buf(MAX_PATH);
  unsigned len = g_fsf.ConvertPath(CPM_FULL, rel_path.c_str(), buf.data(), static_cast<int>(buf.size()));
  if (len > buf.size()) {
    buf.resize(len);
    len = g_fsf.ConvertPath(CPM_FULL, rel_path.c_str(), buf.data(), static_cast<int>(buf.size()));
  }
  return buf.data();
}

INT_PTR adv_control(int command, void* param) {
  return g_far.AdvControl(&c_plugin_guid, command, param);
}

bool match_masks(const wstring& file_name, const wstring& masks) {
  return g_fsf.ProcessName(masks.c_str(), const_cast<wchar_t*>(file_name.c_str()), 0, PN_CMPNAMELIST) != 0;
}

unsigned char get_colors(PaletteColors color_id) {
  return static_cast<unsigned char>(g_far.AdvControl(&c_plugin_guid, ACTL_GETCOLOR, reinterpret_cast<void*>(color_id)));
}

bool panel_go_to_dir(HANDLE h_panel, const wstring& dir) {
  return g_far.Control(h_panel, FCTL_SETPANELDIR, 0, reinterpret_cast<INT_PTR>(dir.c_str())) != 0;
}

// set current file on panel to file_path
bool panel_go_to_file(HANDLE h_panel, const wstring& file_path) {
  wstring dir = extract_file_path(file_path);
  if (!g_far.Control(h_panel, FCTL_SETPANELDIR, 0, reinterpret_cast<INT_PTR>(dir.c_str())))
    return false;
  PanelInfo panel_info;
  if (!g_far.Control(h_panel, FCTL_GETPANELINFO, 0, reinterpret_cast<INT_PTR>(&panel_info)))
    return false;
  wstring file_name = upcase(extract_file_name(file_path));
  PanelRedrawInfo panel_ri = { 0 };
  Buffer<unsigned char> buf(0x1000);
  int i;
  for (i = 0; i < panel_info.ItemsNumber; i++) {
    unsigned size = g_far.Control(h_panel, FCTL_GETPANELITEM, i, NULL);
    if (size > buf.size())
      buf.resize(size);
    size = g_far.Control(h_panel, FCTL_GETPANELITEM, i, reinterpret_cast<INT_PTR>(buf.data()));
    assert(size);
    const PluginPanelItem* panel_item = reinterpret_cast<const PluginPanelItem*>(buf.data());
    if (file_name == upcase(panel_item->FileName)) {
      panel_ri.CurrentItem = i;
      break;
    }
  }
  if (i == panel_info.ItemsNumber)
    return false;
  if (!g_far.Control(h_panel, FCTL_REDRAWPANEL, 0, reinterpret_cast<INT_PTR>(&panel_ri)))
    return false;
  return true;
}

DWORD get_lang_id() {
  DWORD lang_id = 0;
  wstring lang_key_path = add_trailing_slash(extract_file_path(g_far.RootKey)) + L"Language";
  Key lang_key;
  if (!lang_key.open_nt(HKEY_CURRENT_USER, lang_key_path.c_str(), KEY_QUERY_VALUE, false))
    return lang_id;
  wstring lang_name;
  if (!lang_key.query_str_nt(lang_name, L"Main"))
    return lang_id;
  if (lang_name == L"English") lang_id = MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT);
  else if (lang_name == L"Russian") lang_id = MAKELANGID(LANG_RUSSIAN, SUBLANG_DEFAULT);
  else if (lang_name == L"Czech") lang_id = MAKELANGID(LANG_CZECH, SUBLANG_DEFAULT);
  else if (lang_name == L"German") lang_id = MAKELANGID(LANG_GERMAN, SUBLANG_DEFAULT);
  else if (lang_name == L"Hungarian") lang_id = MAKELANGID(LANG_HUNGARIAN, SUBLANG_DEFAULT);
  else if (lang_name == L"Polish") lang_id = MAKELANGID(LANG_POLISH, SUBLANG_DEFAULT);
  else if (lang_name == L"Spanish") lang_id = MAKELANGID(LANG_SPANISH, SUBLANG_DEFAULT);
  return lang_id;
}

void close_plugin(HANDLE h_plugin, const wstring& dir) {
  g_far.Control(h_plugin, FCTL_CLOSEPLUGIN, 0, reinterpret_cast<INT_PTR>(dir.c_str()));
}

void open_help(const wstring& topic) {
  g_far.ShowHelp(g_far.ModuleName, topic.c_str(), FHELP_SELFHELP);
}

};
