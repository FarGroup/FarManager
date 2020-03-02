#pragma once

const wchar_t** get_size_suffixes();
const wchar_t** get_speed_suffixes();

bool password_dialog(std::wstring& password, const std::wstring& arc_path);

struct OverwriteOptions {
  OverwriteAction action;
  bool all;
};
struct OverwriteFileInfo {
  bool is_dir;
  UInt64 size;
  FILETIME mtime;
};
enum OverwriteDialogKind {
  odkExtract,
  odkUpdate,
};
bool overwrite_dialog(const std::wstring& file_path, const OverwriteFileInfo& src_file_info, const OverwriteFileInfo& dst_file_info, OverwriteDialogKind kind, OverwriteOptions& options);

bool extract_dialog(ExtractOptions& options);

void show_error_log(const ErrorLog& error_log);

bool update_dialog(bool new_arc, bool multifile, UpdateOptions& options, UpdateProfiles& profiles);

struct PluginSettings {
  bool handle_create;
  bool handle_commands;
  bool own_panel_view_mode;
  bool use_include_masks;
  std::wstring include_masks;
  bool use_exclude_masks;
  std::wstring exclude_masks;
  bool pgdn_masks;
  bool use_enabled_formats;
  std::wstring enabled_formats;
  bool use_disabled_formats;
  std::wstring disabled_formats;
  bool pgdn_formats;
  bool saveCP;
  UINT oemCP;
  UINT ansiCP;
};

bool settings_dialog(PluginSettings& settings);

void attr_dialog(const AttrList& attr_list);

bool sfx_options_dialog(SfxOptions& sfx_options, const UpdateProfiles& update_profiles);
