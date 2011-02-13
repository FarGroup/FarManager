#include "msg.h"

#include "utils.hpp"
#include "sysutils.hpp"
#include "farutils.hpp"
#include "common.hpp"
#include "ui.hpp"
#include "archive.hpp"
#include "options.hpp"
#include "cmdline.hpp"

wstring g_plugin_prefix;

void attach_sfx_module(const wstring& file_path, const SfxOptions& sfx_options);

class Plugin {
private:
  ComObject<Archive> archive;

  wstring current_dir;
  wstring extract_dir;
  wstring created_dir;
  wstring host_file;
  wstring panel_title;
  vector<InfoPanelLine> info_lines;

public:
  Plugin(): archive(new Archive()) {
  }

  static Plugin* open(const OpenOptions& options) {
    vector<ComObject<Archive>> archives = Archive::open(options);

    if (archives.size() == 0)
      FAIL(E_ABORT);

    int format_idx;
    if (archives.size() == 1) {
      format_idx = 0;
    }
    else {
      Far::MenuItems format_names;
      for (unsigned i = 0; i < archives.size(); i++) {
        format_names.add(archives[i]->arc_chain.to_string());
      }
      format_idx = Far::menu(Far::get_msg(MSG_PLUGIN_NAME), format_names);
      if (format_idx == -1)
        FAIL(E_ABORT);
    }

    Plugin* plugin = new Plugin();
    try {
      plugin->archive = archives[format_idx];
    }
    catch (...) {
      delete plugin;
      throw;
    }
    return plugin;
  }

  void info(OpenPluginInfo* opi) {
    opi->StructSize = sizeof(OpenPluginInfo);
    opi->Flags = OPIF_USEFILTER | OPIF_USESORTGROUPS | OPIF_USEHIGHLIGHTING | OPIF_ADDDOTS;
    opi->CurDir = current_dir.c_str();
    panel_title = Far::get_msg(MSG_PLUGIN_NAME);
    if (archive->is_open()) {
      panel_title += L":" + archive->arc_chain.to_string() + L":" + archive->arc_name();
      if (!current_dir.empty())
        panel_title += L":" + current_dir;
      host_file = archive->arc_path;
      if (archive->has_crc)
        opi->Flags |= OPIF_USECRC32;
    }
    opi->HostFile = host_file.c_str();
    opi->Format = g_plugin_prefix.c_str();
    opi->PanelTitle = panel_title.c_str();
    opi->StartPanelMode = '0' + g_options.panel_view_mode;
    opi->StartSortMode = g_options.panel_sort_mode;
    opi->StartSortOrder = g_options.panel_reverse_sort;

    info_lines.clear();
    info_lines.reserve(archive->arc_attr.size() + 1);
    InfoPanelLine ipl;
    ipl.Text = panel_title.c_str();
    ipl.Data = nullptr;
    ipl.Separator = 1;
    info_lines.push_back(ipl);
    for_each(archive->arc_attr.begin(), archive->arc_attr.end(), [&] (const Attr& attr) {
      ipl.Text = attr.name.c_str();
      ipl.Data = attr.value.c_str();
      ipl.Separator = 0;
      info_lines.push_back(ipl);
    });
    opi->InfoLines = info_lines.data();
    opi->InfoLinesNumber = static_cast<int>(info_lines.size());
  }

  void set_dir(const wstring& dir) {
    if (!archive->is_open())
      FAIL(E_ABORT);
    wstring new_dir;
    if (dir == L"\\")
      new_dir.assign(dir);
    else if (dir == L"..")
      new_dir = extract_file_path(current_dir);
    else if (dir[0] == L'\\') // absolute path
      new_dir.assign(dir);
    else
      new_dir.assign(L"\\").append(add_trailing_slash(remove_path_root(current_dir))).append(dir);

    if (new_dir == L"\\")
      new_dir.clear();

    archive->find_dir(new_dir);
    current_dir = new_dir;
  }

  void list(PluginPanelItem** panel_items, int* items_number) {
    if (!archive->is_open())
      FAIL(E_ABORT);
    UInt32 dir_index = archive->find_dir(current_dir);
    FileIndexRange dir_list = archive->get_dir_list(dir_index);
    size_t size = dir_list.second - dir_list.first;
    PluginPanelItem* items = new PluginPanelItem[size];
    memset(items, 0, size * sizeof(PluginPanelItem));
    try {
      unsigned idx = 0;
      for_each(dir_list.first, dir_list.second, [&] (UInt32 file_index) {
        const ArcFileInfo& file_info = archive->file_list[file_index];
        FAR_FIND_DATA& fdata = items[idx].FindData;
        const DWORD c_valid_attributes = FILE_ATTRIBUTE_ARCHIVE | FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM;
        fdata.dwFileAttributes = archive->get_attr(file_index) & c_valid_attributes;
        fdata.ftCreationTime = archive->get_ctime(file_index);
        fdata.ftLastAccessTime = archive->get_atime(file_index);
        fdata.ftLastWriteTime = archive->get_mtime(file_index);
        fdata.nFileSize = archive->get_size(file_index);
        fdata.nPackSize = archive->get_psize(file_index);
        fdata.lpwszFileName = file_info.name.c_str();
        items[idx].UserData = file_index;
        items[idx].CRC32 = archive->get_crc(file_index);
        idx++;
      });
    }
    catch (...) {
      delete[] items;
      throw;
    }
    *panel_items = items;
    *items_number = static_cast<int>(size);
  }

  static wstring get_separate_dir_path(const wstring& dst_dir, const wstring& arc_name) {
    wstring final_dir = add_trailing_slash(dst_dir) + arc_name;
    wstring ext = extract_file_ext(final_dir);
    final_dir.erase(final_dir.size() - ext.size(), ext.size());
    final_dir = auto_rename(final_dir);
    return final_dir;
  }

  void get_files(const PluginPanelItem* panel_items, int items_number, int move, const wchar_t** dest_path, int op_mode) {
    bool single_item = items_number == 1;
    if (single_item && wcscmp(panel_items[0].FindData.lpwszFileName, L"..") == 0) return;
    ExtractOptions options;
    options.dst_dir = *dest_path;
    options.move_files = archive->updatable() ? (move ? triTrue : triFalse) : triUndef;
    options.delete_archive = false;
    bool show_dialog = (op_mode & (OPM_FIND | OPM_VIEW | OPM_EDIT | OPM_QUICKVIEW)) == 0;
    if (show_dialog && (op_mode & OPM_SILENT) && (op_mode & OPM_TOPLEVEL) == 0)
      show_dialog = false;
    if (op_mode & (OPM_FIND | OPM_QUICKVIEW))
      options.ignore_errors = true;
    else
      options.ignore_errors = g_options.extract_ignore_errors;
    if (show_dialog) {
      options.overwrite = g_options.extract_overwrite;
      options.separate_dir = g_options.extract_separate_dir;
      options.open_dir = g_options.extract_open_dir;
    }
    else {
      options.overwrite = oaOverwrite;
      options.separate_dir = triFalse;
    }
    if (show_dialog) {
      if (!extract_dialog(options))
        FAIL(E_ABORT);
      if (options.dst_dir.empty())
        options.dst_dir = L".";
      if (!is_absolute_path(options.dst_dir))
        options.dst_dir = Far::get_absolute_path(options.dst_dir);
      if (options.dst_dir != *dest_path) {
        extract_dir = options.dst_dir;
        *dest_path = extract_dir.c_str();
      }
      if (options.separate_dir == triTrue || (options.separate_dir == triUndef && !single_item && (op_mode & OPM_TOPLEVEL))) {
        options.dst_dir = get_separate_dir_path(options.dst_dir, archive->arc_name());
      }
      if (!options.password.empty())
        archive->password = options.password;
      if (options.save_params) {
        g_options.extract_ignore_errors = options.ignore_errors;
        g_options.extract_overwrite = options.overwrite;
        g_options.extract_separate_dir = options.separate_dir;
        g_options.extract_open_dir = options.open_dir;
        g_options.save();
      }
    }

    UInt32 src_dir_index = archive->find_dir(current_dir);

    vector<UInt32> indices;
    indices.reserve(items_number);
    for (int i = 0; i < items_number; i++) {
      indices.push_back(static_cast<UInt32>(panel_items[i].UserData));
    }

    ErrorLog error_log;
    archive->extract(src_dir_index, indices, options, error_log);

    if (!error_log.empty() && show_dialog) {
      show_error_log(error_log);
    }

    if (error_log.empty()) {
      if (options.delete_archive) {
        archive->close();
        archive->delete_archive();
        Far::close_plugin(this, archive->arc_dir());
      }
      else if (options.move_files == triTrue)
        archive->delete_files(indices);
      Far::progress_notify();
    }

    if (options.open_dir) {
      if (single_item)
        Far::panel_go_to_file(PANEL_ACTIVE, add_trailing_slash(options.dst_dir) + panel_items[0].FindData.lpwszFileName);
      else
        Far::panel_go_to_dir(PANEL_ACTIVE, options.dst_dir);
    }
  }

  static void extract(const vector<wstring>& arc_list, ExtractOptions options) {
    wstring dst_dir = options.dst_dir;
    wstring dst_file_name;
    ErrorLog error_log;
    for (unsigned i = 0; i < arc_list.size(); i++) {
      vector<ComObject<Archive>> archives;
      try {
        OpenOptions open_options;
        open_options.arc_path = arc_list[i];
        open_options.detect = false;
        open_options.password = options.password;
        open_options.arc_types = ArcAPI::formats().get_arc_types();
        archives = Archive::open(open_options);
        if (archives.empty())
          throw Error(Far::get_msg(MSG_ERROR_NOT_ARCHIVE), arc_list[i], __FILE__, __LINE__);
      }
      catch (const Error& error) {
        if (error.code == E_ABORT)
          throw;
        error_log.push_back(error);
        continue;
      }

      ComObject<Archive> archive = archives[0];
      if (archive->password.empty())
        archive->password = options.password;
      archive->make_index();

      FileIndexRange dir_list = archive->get_dir_list(c_root_index);

      unsigned num_items = dir_list.second - dir_list.first;
      if (arc_list.size() == 1 && num_items == 1) {
        dst_file_name = archive->file_list[*dir_list.first].name;
      }

      vector<UInt32> indices;
      indices.reserve(num_items);
      for_each(dir_list.first, dir_list.second, [&] (UInt32 file_index) {
        indices.push_back(file_index);
      });

      if (options.separate_dir == triTrue || (options.separate_dir == triUndef && indices.size() > 1))
        options.dst_dir = get_separate_dir_path(dst_dir, archive->arc_name());
      else
        options.dst_dir = dst_dir;

      size_t error_count = error_log.size();
      archive->extract(c_root_index, indices, options, error_log);

      if (options.delete_archive && error_count == error_log.size()) {
        archive->close();
        archive->delete_archive();
      }
    }

    if (!error_log.empty()) {
      show_error_log(error_log);
    }
    else {
      Far::update_panel(PANEL_ACTIVE, false);
      Far::progress_notify();
    }

    if (options.open_dir) {
      if (arc_list.size() > 1)
        Far::panel_go_to_dir(PANEL_ACTIVE, dst_dir);
      else if (dst_file_name.empty())
        Far::panel_go_to_dir(PANEL_ACTIVE, options.dst_dir);
      else
        Far::panel_go_to_file(PANEL_ACTIVE, add_trailing_slash(options.dst_dir) + dst_file_name);
    }
  }

  static void bulk_extract(const vector<wstring>& arc_list) {
    ExtractOptions options;
    PanelInfo panel_info;
    if (Far::get_panel_info(PANEL_PASSIVE, panel_info) && Far::is_real_file_panel(panel_info))
      options.dst_dir = Far::get_panel_dir(PANEL_PASSIVE);
    options.move_files = triUndef;
    options.delete_archive = false;
    options.ignore_errors = g_options.extract_ignore_errors;
    options.overwrite = g_options.extract_overwrite;
    options.separate_dir = g_options.extract_separate_dir;
    options.open_dir = g_options.extract_open_dir;

    if (!extract_dialog(options))
      FAIL(E_ABORT);
    if (options.dst_dir.empty())
      options.dst_dir = L".";
    if (!is_absolute_path(options.dst_dir))
      options.dst_dir = Far::get_absolute_path(options.dst_dir);

    if (options.save_params) {
      g_options.extract_ignore_errors = options.ignore_errors;
      g_options.extract_overwrite = options.overwrite;
      g_options.extract_separate_dir = options.separate_dir;
      g_options.extract_open_dir = options.open_dir;
      g_options.save();
    }

    extract(arc_list, options);
  }

  static void cmdline_extract(const ExtractCommand& cmd) {
    vector<wstring> arc_list;
    arc_list.reserve(cmd.arc_list.size());
    for_each(cmd.arc_list.begin(), cmd.arc_list.end(), [&] (const wstring& arc_name) {
      arc_list.push_back(Far::get_absolute_path(arc_name));
    });
    ExtractOptions options = cmd.options;
    options.dst_dir = Far::get_absolute_path(cmd.options.dst_dir);
    extract(arc_list, options);
  }

  void test_files(struct PluginPanelItem* panel_items, int items_number, int op_mode) {
    UInt32 src_dir_index = archive->find_dir(current_dir);
    vector<UInt32> indices;
    indices.reserve(items_number);
    for (int i = 0; i < items_number; i++) {
      indices.push_back(static_cast<UInt32>(panel_items[i].UserData));
    }
    archive->test(src_dir_index, indices);
    Far::info_dlg(Far::get_msg(MSG_PLUGIN_NAME), Far::get_msg(MSG_TEST_OK));
  }

  static void bulk_test(const vector<wstring>& arc_list) {
    ErrorLog error_log;
    for (unsigned i = 0; i < arc_list.size(); i++) {
      vector<ComObject<Archive>> archives;
      try {
        OpenOptions open_options;
        open_options.arc_path = arc_list[i];
        open_options.detect = false;
        open_options.arc_types = ArcAPI::formats().get_arc_types();
        archives = Archive::open(open_options);
        if (archives.empty())
          throw Error(Far::get_msg(MSG_ERROR_NOT_ARCHIVE), arc_list[i], __FILE__, __LINE__);
      }
      catch (const Error& error) {
        if (error.code == E_ABORT)
          throw;
        error_log.push_back(error);
        continue;
      }

      ComObject<Archive> archive = archives[0];
      archive->make_index();

      FileIndexRange dir_list = archive->get_dir_list(c_root_index);
      vector<UInt32> indices;
      indices.reserve(dir_list.second - dir_list.first);
      for_each(dir_list.first, dir_list.second, [&] (UInt32 file_index) {
        indices.push_back(file_index);
      });

      try {
        archive->test(c_root_index, indices);
      }
      catch (const Error& error) {
        if (error.code == E_ABORT)
          throw;
        error_log.push_back(error);
      }
    }

    if (!error_log.empty()) {
      show_error_log(error_log);
    }
    else {
      Far::update_panel(PANEL_ACTIVE, false);
      Far::info_dlg(Far::get_msg(MSG_PLUGIN_NAME), Far::get_msg(MSG_TEST_OK));
    }
  }

  static void cmdline_test(const TestCommand& cmd) {
    vector<wstring> arc_list;
    arc_list.reserve(cmd.arc_list.size());
    for_each(cmd.arc_list.begin(), cmd.arc_list.end(), [&] (const wstring& arc_name) {
      arc_list.push_back(Far::get_absolute_path(arc_name));
    });
    Plugin::bulk_test(arc_list);
  }

  void put_files(const PluginPanelItem* panel_items, int items_number, int move, const wchar_t* src_path, int op_mode) {
    if (items_number == 1 && wcscmp(panel_items[0].FindData.lpwszFileName, L"..") == 0)
      return;
    UpdateOptions options;
    bool new_arc = !archive->is_open();
    if (!new_arc && !archive->updatable()) {
      FAIL_MSG(Far::get_msg(MSG_ERROR_NOT_UPDATABLE));
    }
    if (new_arc) {
      if (items_number == 1 || is_root_path(src_path))
        options.arc_path = panel_items[0].FindData.lpwszFileName;
      else
        options.arc_path = extract_file_name(src_path);
      ArcTypes arc_types = ArcAPI::formats().find_by_name(g_options.update_arc_format_name);
      if (arc_types.empty())
        options.arc_type = c_7z;
      else
        options.arc_type = arc_types.front();
      options.sfx_options = g_options.update_sfx_options;
      if (ArcAPI::formats().count(options.arc_type))
        options.arc_path += ArcAPI::formats().at(options.arc_type).default_extension();

      options.level = g_options.update_level;
      options.method = g_options.update_method;
      options.solid = g_options.update_solid;
      options.encrypt = false;
      options.encrypt_header = g_options.update_encrypt_header;
      options.volume_size = g_options.update_volume_size;
    }
    else {
      options.arc_type = archive->arc_chain.back().type; // required to set update properties
      archive->load_update_props();
      options.level = archive->level;
      options.method = archive->method;
      options.solid = archive->solid;
      options.encrypt = archive->encrypted;
      options.encrypt_header = triUndef;
      options.password = archive->password;
      options.overwrite = g_options.update_overwrite;
      if (op_mode & OPM_EDIT)
        options.overwrite = oaOverwrite;
    }
    options.create_sfx = false;
    options.enable_volumes = false;
    options.show_password = g_options.update_show_password;
    options.move_files = move != 0;
    options.open_shared = (Far::adv_control(ACTL_GETSYSTEMSETTINGS) & FSS_COPYFILESOPENEDFORWRITING) != 0;
    options.ignore_errors = g_options.update_ignore_errors;

    bool res = update_dialog(new_arc, options, g_profiles);
    g_profiles.save();
    if (!res)
      FAIL(E_ABORT);
    if (ArcAPI::formats().count(options.arc_type) == 0)
      FAIL_MSG(Far::get_msg(MSG_ERROR_NO_FORMAT));
    if (new_arc) {
      if (!is_absolute_path(options.arc_path))
        options.arc_path = Far::get_absolute_path(options.arc_path);
      if (File::exists(options.arc_path)) {
        if (Far::message(Far::get_msg(MSG_PLUGIN_NAME) + L"\n" + Far::get_msg(MSG_UPDATE_DLG_CONFIRM_OVERWRITE), 0, FMSG_MB_YESNO) != 0)
          FAIL(E_ABORT);
      }
      g_options.update_arc_format_name = ArcAPI::formats().at(options.arc_type).name;
      g_options.update_sfx_options = options.sfx_options;
      g_options.update_volume_size = options.volume_size;
      g_options.update_level = options.level;
      g_options.update_method = options.method;
      g_options.update_solid = options.solid;
      g_options.update_encrypt_header = options.encrypt_header;
    }
    else {
      archive->level = options.level;
      archive->method = options.method;
      archive->solid = options.solid;
      archive->encrypted = options.encrypt;
      g_options.update_overwrite = options.overwrite;
    }
    g_options.update_show_password = options.show_password;
    g_options.update_ignore_errors = options.ignore_errors;
    g_options.save();

    vector<wstring> file_names;
    file_names.reserve(items_number);
    for (int i = 0; i < items_number; i++) {
      file_names.push_back(panel_items[i].FindData.lpwszFileName);
    }

    ErrorLog error_log;
    if (new_arc)
      archive->create(src_path, file_names, options, error_log);
    else
      archive->update(src_path, file_names, remove_path_root(current_dir), options, error_log);

    if (!error_log.empty()) {
      show_error_log(error_log);
    }
    else {
      Far::progress_notify();
    }

    if (new_arc) {
      if (upcase(Far::get_panel_dir(PANEL_ACTIVE)) == upcase(extract_file_path(options.arc_path)))
        Far::panel_go_to_file(PANEL_ACTIVE, options.arc_path);
      if (upcase(Far::get_panel_dir(PANEL_PASSIVE)) == upcase(extract_file_path(options.arc_path)))
        Far::panel_go_to_file(PANEL_PASSIVE, options.arc_path);
    }
  }

  static void create_archive() {
    PanelInfo panel_info;
    if (!Far::get_panel_info(PANEL_ACTIVE, panel_info))
      FAIL(E_ABORT);
    if (!Far::is_real_file_panel(panel_info))
      FAIL(E_ABORT);
    vector<wstring> file_list;
    file_list.reserve(panel_info.SelectedItemsNumber);
    wstring src_path = Far::get_panel_dir(PANEL_ACTIVE);
    for (int i = 0; i < panel_info.SelectedItemsNumber; i++) {
      Far::PanelItem panel_item = Far::get_selected_panel_item(PANEL_ACTIVE, i);
      file_list.push_back(panel_item.file_name);
    }
    if (file_list.empty())
      FAIL(E_ABORT);
    if (file_list.size() == 1 && file_list[0] == L"..")
      FAIL(E_ABORT);

    UpdateOptions options;

    if (file_list.size() == 1 || is_root_path(src_path))
      options.arc_path = file_list[0];
    else
      options.arc_path = extract_file_name(src_path);
    ArcTypes arc_types = ArcAPI::formats().find_by_name(g_options.update_arc_format_name);
    if (arc_types.empty())
      options.arc_type = c_7z;
    else
      options.arc_type = arc_types.front();
    options.sfx_options = g_options.update_sfx_options;
    if (ArcAPI::formats().count(options.arc_type))
      options.arc_path += ArcAPI::formats().at(options.arc_type).default_extension();

    options.level = g_options.update_level;
    options.method = g_options.update_method;
    options.solid = g_options.update_solid;
    options.show_password = g_options.update_show_password;
    options.encrypt = false;
    options.encrypt_header = g_options.update_encrypt_header;
    options.create_sfx = false;
    options.enable_volumes = false;
    options.volume_size = g_options.update_volume_size;
    options.move_files = false;
    options.open_shared = (Far::adv_control(ACTL_GETSYSTEMSETTINGS) & FSS_COPYFILESOPENEDFORWRITING) != 0;
    options.ignore_errors = g_options.update_ignore_errors;

    bool res = update_dialog(true, options, g_profiles);
    g_profiles.save();
    if (!res)
      FAIL(E_ABORT);
    if (ArcAPI::formats().count(options.arc_type) == 0)
      FAIL_MSG(Far::get_msg(MSG_ERROR_NO_FORMAT));

    if (!is_absolute_path(options.arc_path))
      options.arc_path = Far::get_absolute_path(options.arc_path);
    if (File::exists(options.arc_path)) {
      if (Far::message(Far::get_msg(MSG_PLUGIN_NAME) + L"\n" + Far::get_msg(MSG_UPDATE_DLG_CONFIRM_OVERWRITE), 0, FMSG_MB_YESNO) != 0)
        FAIL(E_ABORT);
    }
    g_options.update_arc_format_name = ArcAPI::formats().at(options.arc_type).name;
    g_options.update_sfx_options = options.sfx_options;
    g_options.update_volume_size = options.volume_size;
    g_options.update_level = options.level;
    g_options.update_method = options.method;
    g_options.update_solid = options.solid;
    g_options.update_encrypt_header = options.encrypt_header;
    g_options.update_show_password = options.show_password;
    g_options.update_ignore_errors = options.ignore_errors;
    g_options.save();

    ErrorLog error_log;
    Archive().create(src_path, file_list, options, error_log);

    if (!error_log.empty()) {
      show_error_log(error_log);
    }
    else {
      Far::progress_notify();
    }

    if (upcase(Far::get_panel_dir(PANEL_ACTIVE)) == upcase(extract_file_path(options.arc_path)))
      Far::panel_go_to_file(PANEL_ACTIVE, options.arc_path);
    if (upcase(Far::get_panel_dir(PANEL_PASSIVE)) == upcase(extract_file_path(options.arc_path)))
      Far::panel_go_to_file(PANEL_PASSIVE, options.arc_path);
  }

  static void cmdline_update(const UpdateCommand& cmd) {
    UpdateOptions options = cmd.options;
    options.arc_path = Far::get_absolute_path(options.arc_path);

    vector<wstring> files;
    files.reserve(cmd.files.size());
    for_each(cmd.files.begin(), cmd.files.end(), [&] (const wstring& file) {
      files.push_back(Far::get_absolute_path(file));
    });

    // load listfiles
    for_each(cmd.listfiles.begin(), cmd.listfiles.end(), [&] (const wstring& listfile) {
      wstring str = load_file(Far::get_absolute_path(listfile));
      std::list<wstring> fl = parse_listfile(str);
      files.reserve(files.size() + fl.size());
      for_each(fl.begin(), fl.end(), [&] (const wstring& file) {
        files.push_back(Far::get_absolute_path(file));
      });
    });

    if (files.empty())
      FAIL(E_ABORT);

    // common source directory
    wstring src_path = extract_file_path(Far::get_absolute_path(files.front()));
    wstring src_path_upcase = upcase(src_path);
    wstring full_path;
    for_each(files.begin(), files.end(), [&] (const wstring& file) {
      while (!substr_match(upcase(file), 0, src_path_upcase.c_str())) {
        if (is_root_path(src_path))
          src_path.clear();
        else
          src_path = extract_file_path(src_path);
        src_path_upcase = upcase(src_path);
      }
    });

    // relative paths
    if (!src_path.empty()) {
      for_each(files.begin(), files.end(), [&] (wstring& file) {
        file.erase(0, src_path.size() + 1);
      });
    }

    options.open_shared = (Far::adv_control(ACTL_GETSYSTEMSETTINGS) & FSS_COPYFILESOPENEDFORWRITING) != 0;

    ErrorLog error_log;
    if (cmd.new_arc) {
      Archive().create(src_path, files, options, error_log);

      if (upcase(Far::get_panel_dir(PANEL_ACTIVE)) == upcase(extract_file_path(options.arc_path)))
        Far::panel_go_to_file(PANEL_ACTIVE, options.arc_path);
      if (upcase(Far::get_panel_dir(PANEL_PASSIVE)) == upcase(extract_file_path(options.arc_path)))
        Far::panel_go_to_file(PANEL_PASSIVE, options.arc_path);
    }
    else {
      OpenOptions open_options;
      open_options.arc_path = options.arc_path;
      open_options.detect = false;
      open_options.password = options.password;
      open_options.arc_types = ArcAPI::formats().get_arc_types();
      vector<ComObject<Archive>> archives = Archive::open(open_options);
      if (archives.empty())
        throw Error(Far::get_msg(MSG_ERROR_NOT_ARCHIVE), options.arc_path, __FILE__, __LINE__);

      ComObject<Archive> archive = archives[0];
      if (!archive->updatable())
        throw Error(Far::get_msg(MSG_ERROR_NOT_UPDATABLE), options.arc_path, __FILE__, __LINE__);

      archive->make_index();

      options.arc_type = archive->arc_chain.back().type;
      archive->load_update_props();
      if (!cmd.level_defined)
        options.level = archive->level;
      if (!cmd.method_defined)
        options.method = archive->method;
      if (!cmd.solid_defined)
        options.solid = archive->solid;
      if (!cmd.encrypt_defined) {
        options.encrypt = archive->encrypted;
        options.password = archive->password;
      }

      archive->update(src_path, files, wstring(), options, error_log);
    }
  }

  void delete_files(const PluginPanelItem* panel_items, int items_number, int op_mode) {
    if (items_number == 1 && wcscmp(panel_items[0].FindData.lpwszFileName, L"..") == 0) return;

    if (!archive->updatable()) {
      FAIL_MSG(Far::get_msg(MSG_ERROR_NOT_UPDATABLE));
    }

    bool show_dialog = (op_mode & (OPM_SILENT | OPM_FIND | OPM_VIEW | OPM_EDIT | OPM_QUICKVIEW)) == 0;
    if (show_dialog) {
      if (Far::message(Far::get_msg(MSG_PLUGIN_NAME) + L"\n" + Far::get_msg(MSG_DELETE_DLG_CONFIRM), 0, FMSG_MB_OKCANCEL) != 0)
        FAIL(E_ABORT);
    }

    vector<UInt32> indices;
    indices.reserve(items_number);
    for (int i = 0; i < items_number; i++) {
      indices.push_back(static_cast<UInt32>(panel_items[i].UserData));
    }
    archive->delete_files(indices);

    Far::progress_notify();
  }

  void create_dir(const wchar_t** name, int op_mode) {
    if (!archive->updatable()) {
      FAIL_MSG(Far::get_msg(MSG_ERROR_NOT_UPDATABLE));
    }
    bool show_dialog = (op_mode & (OPM_SILENT | OPM_FIND | OPM_VIEW | OPM_EDIT | OPM_QUICKVIEW)) == 0;
    created_dir = *name;
    if (show_dialog) {
      if (!Far::input_dlg(Far::get_msg(MSG_PLUGIN_NAME), Far::get_msg(MSG_CREATE_DIR_DLG_NAME), created_dir))
        FAIL(E_ABORT);
      *name = created_dir.c_str();
    }
    archive->create_dir(created_dir, remove_path_root(current_dir));
  }

  void show_attr() {
    AttrList attr_list;
    Far::PanelItem panel_item = Far::get_current_panel_item(PANEL_ACTIVE);
    if (panel_item.file_name == L"..") {
      if (is_root_path(current_dir)) {
        attr_list = archive->arc_attr;
      }
      else {
        attr_list = archive->get_attr_list(archive->find_dir(current_dir));
      }
    }
    else {
      attr_list = archive->get_attr_list(static_cast<UInt32>(panel_item.user_data));
    }
    if (!attr_list.empty())
      attr_dialog(attr_list);
  }

  void close() {
    PanelInfo panel_info;
    if (Far::get_panel_info(this, panel_info)) {
      g_options.panel_view_mode = panel_info.ViewMode;
      g_options.panel_sort_mode = panel_info.SortMode;
      g_options.panel_reverse_sort = (panel_info.Flags & PFLAGS_REVERSESORTORDER) != 0;
      g_options.save();
    }
  }

  static void convert_to_sfx(const vector<wstring>& file_list) {
    SfxOptions sfx_options = g_options.update_sfx_options;
    if (!sfx_options_dialog(sfx_options, g_profiles))
      FAIL(E_ABORT);
    g_options.update_sfx_options = sfx_options;
    g_options.save();
    for (unsigned i = 0; i < file_list.size(); i++) {
      attach_sfx_module(file_list[i], sfx_options);
    }
  }
};

TriState detect_next_time = triUndef;

int WINAPI GetMinFarVersion(void) {
  return FARMANAGERVERSION;
}

int WINAPI GetMinFarVersionW(void) {
  return FARMANAGERVERSION;
}

void WINAPI SetStartupInfoW(const struct PluginStartupInfo *Info) {
  enable_lfh();
  Far::init(Info);
  g_options.load();
  g_profiles.load();
  g_plugin_prefix = g_options.plugin_prefix;
  Archive::max_check_size = g_options.max_check_size;
}

void WINAPI GetPluginInfoW(struct PluginInfo *Info) {
  FAR_ERROR_HANDLER_BEGIN;
  static const wchar_t* plugin_menu[1];
  static const wchar_t* config_menu[1];
  plugin_menu[0] = Far::msg_ptr(MSG_PLUGIN_NAME);
  config_menu[0] = Far::msg_ptr(MSG_PLUGIN_NAME);

  Info->StructSize = sizeof(PluginInfo);
  Info->PluginMenuStrings = plugin_menu;
  Info->PluginMenuStringsNumber = ARRAYSIZE(plugin_menu);
  Info->PluginConfigStrings = config_menu;
  Info->PluginConfigStringsNumber = ARRAYSIZE(config_menu);
  Info->CommandPrefix = g_plugin_prefix.c_str();
  FAR_ERROR_HANDLER_END(return, return, false);
}

HANDLE WINAPI OpenPluginW(int OpenFrom, INT_PTR Item) {
  FAR_ERROR_HANDLER_BEGIN;
  if (OpenFrom == OPEN_PLUGINSMENU) {
    Far::MenuItems menu_items;
    unsigned open_menu_id = menu_items.add(Far::get_msg(MSG_MENU_OPEN));
    unsigned detect_menu_id = menu_items.add(Far::get_msg(MSG_MENU_DETECT));
    unsigned create_menu_id = menu_items.add(Far::get_msg(MSG_MENU_CREATE));
    unsigned extract_menu_id = menu_items.add(Far::get_msg(MSG_MENU_EXTRACT));
    unsigned test_menu_id = menu_items.add(Far::get_msg(MSG_MENU_TEST));
    unsigned sfx_convert_menu_id = menu_items.add(Far::get_msg(MSG_MENU_SFX_CONVERT));
    unsigned item = Far::menu(Far::get_msg(MSG_PLUGIN_NAME), menu_items, L"Contents");
    if (item == open_menu_id || item == detect_menu_id) {
      OpenOptions options;
      options.detect = item == detect_menu_id;
      PanelInfo panel_info;
      if (!Far::get_panel_info(PANEL_ACTIVE, panel_info))
        return INVALID_HANDLE_VALUE;
      Far::PanelItem panel_item = Far::get_current_panel_item(PANEL_ACTIVE);
      if (panel_item.file_attributes & FILE_ATTRIBUTE_DIRECTORY)
        return INVALID_HANDLE_VALUE;
      if (!Far::is_real_file_panel(panel_info)) {
        if ((panel_item.file_attributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
          Far::post_keys(vector<DWORD>(1, KEY_CTRLPGDN));
          detect_next_time = options.detect ? triTrue : triFalse;
        }
        return INVALID_HANDLE_VALUE;
      }
      options.arc_path = add_trailing_slash(Far::get_panel_dir(PANEL_ACTIVE)) + panel_item.file_name;
      options.arc_types = ArcAPI::formats().get_arc_types();
      return Plugin::open(options);
    }
    else if (item == create_menu_id) {
      Plugin::create_archive();
    }
    else if (item == extract_menu_id || item == test_menu_id || item == sfx_convert_menu_id) {
      PanelInfo panel_info;
      if (!Far::get_panel_info(PANEL_ACTIVE, panel_info))
        return INVALID_HANDLE_VALUE;
      if (!Far::is_real_file_panel(panel_info))
        return INVALID_HANDLE_VALUE;
      vector<wstring> file_list;
      file_list.reserve(panel_info.SelectedItemsNumber);
      wstring dir = Far::get_panel_dir(PANEL_ACTIVE);
      for (int i = 0; i < panel_info.SelectedItemsNumber; i++) {
        Far::PanelItem panel_item = Far::get_selected_panel_item(PANEL_ACTIVE, i);
        if ((panel_item.file_attributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
          wstring file_path = add_trailing_slash(dir) + panel_item.file_name;
          file_list.push_back(file_path);
        }
      }
      if (file_list.empty())
        return INVALID_HANDLE_VALUE;
      if (item == extract_menu_id)
        Plugin::bulk_extract(file_list);
      else if (item == test_menu_id)
        Plugin::bulk_test(file_list);
      else if (item == sfx_convert_menu_id) {
        Plugin::convert_to_sfx(file_list);
      }
    }
  }
  else if (OpenFrom == OPEN_COMMANDLINE) {
    try {
      CommandArgs cmd_args = parse_command(reinterpret_cast<const wchar_t*>(Item));
      switch (cmd_args.cmd) {
      case cmdOpen: {
        OpenOptions options = parse_open_command(cmd_args).options;
        options.arc_path = Far::get_absolute_path(options.arc_path);
        return Plugin::open(options);
      }
      case cmdCreate:
      case cmdUpdate:
        Plugin::cmdline_update(parse_update_command(cmd_args));
        break;
      case cmdExtract:
        Plugin::cmdline_extract(parse_extract_command(cmd_args));
        break;
      case cmdTest:
        Plugin::cmdline_test(parse_test_command(cmd_args));
        break;
      }
    }
    catch (const Error& e) {
      if (e.code == E_BAD_FORMAT)
        Far::open_help(L"Prefix");
      else
        throw;
    }
  }
  return INVALID_HANDLE_VALUE;
  FAR_ERROR_HANDLER_END(return INVALID_HANDLE_VALUE, return INVALID_HANDLE_VALUE, false);
}

HANDLE WINAPI OpenFilePluginW(const wchar_t *Name,const unsigned char *Data,int DataSize,int OpMode) {
  FAR_ERROR_HANDLER_BEGIN;
  if (Name == nullptr) {
    if (!g_options.handle_create)
      FAIL(E_ABORT);
    return new Plugin();
  }
  else {
    OpenOptions options;
    options.arc_path = Name;
    options.arc_types = ArcAPI::formats().get_arc_types();
    if (detect_next_time == triUndef) {
      options.detect = false;
      if (!g_options.handle_commands)
        FAIL(E_ABORT);
      if (g_options.use_include_masks && !Far::match_masks(extract_file_name(Name), g_options.include_masks))
        FAIL(E_ABORT);
      if (g_options.use_exclude_masks && Far::match_masks(extract_file_name(Name), g_options.exclude_masks))
        FAIL(E_ABORT);
      if (g_options.use_enabled_formats || g_options.use_disabled_formats) {
        set<wstring> enabled_formats;
        if (g_options.use_enabled_formats) {
          list<wstring> name_list = split(upcase(g_options.enabled_formats), L',');
          copy(name_list.cbegin(), name_list.cend(), inserter(enabled_formats, enabled_formats.begin()));
        }
        set<wstring> disabled_formats;
        if (g_options.use_disabled_formats) {
          list<wstring> name_list = split(upcase(g_options.disabled_formats), L',');
          copy(name_list.cbegin(), name_list.cend(), inserter(disabled_formats, disabled_formats.begin()));
        }

        const ArcFormats& arc_formats = ArcAPI::formats();
        ArcTypes::iterator arc_type = options.arc_types.begin();
        while (arc_type != options.arc_types.end()) {
          wstring arc_name = upcase(arc_formats.at(*arc_type).name);
          if (g_options.use_enabled_formats && enabled_formats.count(arc_name) == 0)
            arc_type = options.arc_types.erase(arc_type);
          else if (g_options.use_disabled_formats && disabled_formats.count(arc_name) != 0)
            arc_type = options.arc_types.erase(arc_type);
          else
            arc_type++;
        }

        if (options.arc_types.empty())
          FAIL(E_ABORT);
      }
    }
    else
      options.detect = detect_next_time == triTrue;
    detect_next_time = triUndef;
    return Plugin::open(options);
  }
  FAR_ERROR_HANDLER_END(return INVALID_HANDLE_VALUE, return INVALID_HANDLE_VALUE, (OpMode & (OPM_SILENT | OPM_FIND)) != 0);
}

void WINAPI ClosePluginW(HANDLE hPlugin) {
  FAR_ERROR_HANDLER_BEGIN;
  Plugin* plugin = reinterpret_cast<Plugin*>(hPlugin);
  IGNORE_ERRORS(plugin->close());
  delete plugin;
  FAR_ERROR_HANDLER_END(return, return, true);
}

void WINAPI GetOpenPluginInfoW(HANDLE hPlugin,struct OpenPluginInfo *Info) {
  FAR_ERROR_HANDLER_BEGIN;
  reinterpret_cast<Plugin*>(hPlugin)->info(Info);
  FAR_ERROR_HANDLER_END(return, return, false);
}

int WINAPI SetDirectoryW(HANDLE hPlugin,const wchar_t *Dir,int OpMode) {
  FAR_ERROR_HANDLER_BEGIN;
  reinterpret_cast<Plugin*>(hPlugin)->set_dir(Dir);
  return TRUE;
  FAR_ERROR_HANDLER_END(return FALSE, return FALSE, (OpMode & (OPM_SILENT | OPM_FIND)) != 0);
}

int WINAPI GetFindDataW(HANDLE hPlugin,struct PluginPanelItem **pPanelItem,int *pItemsNumber,int OpMode) {
  FAR_ERROR_HANDLER_BEGIN;
  reinterpret_cast<Plugin*>(hPlugin)->list(pPanelItem, pItemsNumber);
  return TRUE;
  FAR_ERROR_HANDLER_END(return FALSE, return FALSE, (OpMode & (OPM_SILENT | OPM_FIND)) != 0);
}

void WINAPI FreeFindDataW(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber) {
  FAR_ERROR_HANDLER_BEGIN;
  delete[] PanelItem;
  FAR_ERROR_HANDLER_END(return, return, false);
}

int WINAPI GetFilesW(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int Move,const wchar_t **DestPath,int OpMode) {
  FAR_ERROR_HANDLER_BEGIN
  reinterpret_cast<Plugin*>(hPlugin)->get_files(PanelItem, ItemsNumber, Move, DestPath, OpMode);
  return 1;
  FAR_ERROR_HANDLER_END(return 0, return -1, (OpMode & (OPM_FIND | OPM_QUICKVIEW)) != 0);
}

int WINAPI PutFilesW(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int Move,const wchar_t *SrcPath,int OpMode) {
  FAR_ERROR_HANDLER_BEGIN;
  reinterpret_cast<Plugin*>(hPlugin)->put_files(PanelItem, ItemsNumber, Move, SrcPath, OpMode);
  return 2;
  FAR_ERROR_HANDLER_END(return 0, return -1, (OpMode & OPM_FIND) != 0);
}

int WINAPI DeleteFilesW(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int OpMode) {
  FAR_ERROR_HANDLER_BEGIN;
  reinterpret_cast<Plugin*>(hPlugin)->delete_files(PanelItem, ItemsNumber, OpMode);
  return TRUE;
  FAR_ERROR_HANDLER_END(return FALSE, return FALSE, (OpMode & OPM_SILENT) != 0);
}

int WINAPI MakeDirectoryW(HANDLE hPlugin, const wchar_t** Name, int OpMode) {
  FAR_ERROR_HANDLER_BEGIN;
  reinterpret_cast<Plugin*>(hPlugin)->create_dir(Name, OpMode);
  return 1;
  FAR_ERROR_HANDLER_END(return -1, return -1, (OpMode & OPM_SILENT) != 0);
}

int WINAPI ProcessHostFileW(HANDLE hPlugin, struct PluginPanelItem *PanelItem, int ItemsNumber, int OpMode) {
  FAR_ERROR_HANDLER_BEGIN;
  Far::MenuItems menu_items;
  menu_items.add(Far::get_msg(MSG_TEST_MENU));
  int item = Far::menu(Far::get_msg(MSG_PLUGIN_NAME), menu_items);
  if (item == 0)
    reinterpret_cast<Plugin*>(hPlugin)->test_files(PanelItem, ItemsNumber, OpMode);
  return TRUE;
  FAR_ERROR_HANDLER_END(return FALSE, return FALSE, (OpMode & OPM_SILENT) != 0);
}

int WINAPI ProcessKeyW(HANDLE hPlugin,int Key,unsigned int ControlState) {
  FAR_ERROR_HANDLER_BEGIN;
  if (ControlState == PKF_CONTROL && Key == 'A') {
    reinterpret_cast<Plugin*>(hPlugin)->show_attr();
    return TRUE;
  }
  return FALSE;
  FAR_ERROR_HANDLER_END(return FALSE, return FALSE, false);
}

int WINAPI ConfigureW(int ItemNumber) {
  FAR_ERROR_HANDLER_BEGIN;
  PluginSettings settings;
  settings.handle_create = g_options.handle_create;
  settings.handle_commands = g_options.handle_commands;
  settings.use_include_masks = g_options.use_include_masks;
  settings.include_masks = g_options.include_masks;
  settings.use_exclude_masks = g_options.use_exclude_masks;
  settings.exclude_masks = g_options.exclude_masks;
  settings.use_enabled_formats = g_options.use_enabled_formats;
  settings.enabled_formats = g_options.enabled_formats;
  settings.use_disabled_formats = g_options.use_disabled_formats;
  settings.disabled_formats = g_options.disabled_formats;
  if (settings_dialog(settings)) {
    g_options.handle_create = settings.handle_create;
    g_options.handle_commands = settings.handle_commands;
    g_options.use_include_masks = settings.use_include_masks;
    g_options.include_masks = settings.include_masks;
    g_options.use_exclude_masks = settings.use_exclude_masks;
    g_options.exclude_masks = settings.exclude_masks;
    g_options.use_enabled_formats = settings.use_enabled_formats;
    g_options.enabled_formats = settings.enabled_formats;
    g_options.use_disabled_formats = settings.use_disabled_formats;
    g_options.disabled_formats = settings.disabled_formats;
    g_options.save();
    return TRUE;
  }
  else
    return FALSE;
  FAR_ERROR_HANDLER_END(return FALSE, return FALSE, false);
}

void WINAPI ExitFARW() {
  ArcAPI::free();
}
