#pragma once

struct ExternalCodec {
  std::wstring name;
  unsigned minL{}, maxL{},  mod0L{};
  unsigned L1{}, L3{}, L5{}, L7{}, L9{};
  bool bcj_only{};
  std::wstring adv;
  void reset() {
    name.clear();
    minL = 1; maxL = 9; mod0L = 0;
    L1 = 1; L3 = 3; L5 = 5; L7 = 7; L9 = 9;
    bcj_only = false;
    adv.clear();
  }
};

struct Options {
  bool handle_create;
  bool handle_commands;
  std::wstring plugin_prefix;
  unsigned max_check_size;
  // extract
  bool extract_ignore_errors;
  OverwriteAction extract_overwrite;
  TriState extract_separate_dir;
  bool extract_open_dir;
  // update
  std::wstring update_arc_format_name;
  unsigned update_level;
  std::wstring update_levels; // for example: '7z=7;zip=5;bzip2=9;xz=5;wim=0;tar=0;gzip=5'
  std::wstring update_method;
  bool update_solid;
  std::wstring update_advanced;
  bool update_encrypt;
  bool update_show_password;
  TriState update_encrypt_header;
  std::wstring update_password;
  bool update_create_sfx;
  SfxOptions update_sfx_options;
  bool update_enable_volumes;
  std::wstring update_volume_size;
  bool update_move_files;
  bool update_ignore_errors;
  OverwriteAction update_overwrite;
  bool update_append_ext;
  // panel mode
  bool own_panel_view_mode;
  uintptr_t panel_view_mode;
  OPENPANELINFO_SORTMODES panel_sort_mode;
  bool panel_reverse_sort;
  // masks
  bool use_include_masks;
  std::wstring include_masks;
  bool use_exclude_masks;
  std::wstring exclude_masks;
  bool pgdn_masks;
  // archive formats
  bool use_enabled_formats;
  std::wstring enabled_formats;
  bool use_disabled_formats;
  std::wstring disabled_formats;
  bool pgdn_formats;
  bool saveCP;
  uintptr_t oemCP;
  uintptr_t ansiCP;
  unsigned correct_name_mode;
  bool qs_by_default;
  bool strict_case;

  struct LoadedFromXML {
    bool max_check_size{};
    bool correct_name_mode{};
    bool qs_by_default{};
    bool strict_case{};
  } loaded_from_xml;

  std::vector<ExternalCodec> codecs;

  Options();
  // profiles
  void load();
  void save() const;
};

extern Options g_options;
extern UpdateProfiles g_profiles;

extern const wchar_t* c_copy_opened_files_option;
extern const wchar_t* c_esc_confirmation_option;
bool get_app_option(size_t category, const wchar_t* name, bool& value);
