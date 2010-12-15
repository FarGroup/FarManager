#pragma once

const wchar_t** get_size_suffixes();
const wchar_t** get_speed_suffixes();

bool password_dialog(wstring& password, const wstring& arc_path);

struct OverwriteOptions {
  OverwriteAction action;
  bool all;
};
struct OverwriteFileInfo {
  bool is_dir;
  unsigned __int64 size;
  FILETIME mtime;
};
enum OverwriteDialogKind {
  odkExtract,
  odkUpdate,
};
bool overwrite_dialog(const wstring& file_path, const OverwriteFileInfo& src_file_info, const OverwriteFileInfo& dst_file_info, OverwriteDialogKind kind, OverwriteOptions& options);

bool extract_dialog(ExtractOptions& options);

void show_error_log(const ErrorLog& error_log);

bool update_dialog(bool new_arc, UpdateOptions& options, UpdateProfiles& profiles);

struct PluginSettings {
  bool handle_create;
  bool handle_commands;
  bool use_include_masks;
  wstring include_masks;
  bool use_exclude_masks;
  wstring exclude_masks;
  bool use_enabled_formats;
  wstring enabled_formats;
  bool use_disabled_formats;
  wstring disabled_formats;
};

bool settings_dialog(PluginSettings& settings);

void attr_dialog(const AttrList& attr_list);

bool sfx_options_dialog(SfxOptions& sfx_options, const UpdateProfiles& update_profiles);
