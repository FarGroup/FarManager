#pragma once

extern const wchar_t* c_plugin_key_name;

struct Options {
  bool handle_create;
  bool handle_commands;
  wstring plugin_prefix;
  unsigned max_check_size;
  // extract
  bool extract_ignore_errors;
  OverwriteAction extract_overwrite;
  TriState extract_separate_dir;
  bool extract_open_dir;
  // update
  wstring update_arc_format_name;
  unsigned update_level;
  wstring update_method;
  bool update_solid;
  bool update_show_password;
  TriState update_encrypt_header;
  SfxOptions update_sfx_options;
  wstring update_volume_size;
  bool update_ignore_errors;
  OverwriteAction update_overwrite;
  // panel mode
  unsigned panel_view_mode;
  unsigned panel_sort_mode;
  bool panel_reverse_sort;
  // masks
  bool use_include_masks;
  wstring include_masks;
  bool use_exclude_masks;
  wstring exclude_masks;
  // archive formats
  bool use_enabled_formats;
  wstring enabled_formats;
  bool use_disabled_formats;
  wstring disabled_formats;
  // profiles
  void load();
  void save() const;
};

extern Options g_options;
extern UpdateProfiles g_profiles;
