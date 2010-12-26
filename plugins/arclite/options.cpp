#include "utils.hpp"
#include "sysutils.hpp"
#include "farutils.hpp"
#include "common.hpp"
#include "archive.hpp"
#include "options.hpp"


class OptionsKey: public Key {
public:
  unsigned get_int(const wchar_t* name, unsigned def_value) {
    unsigned value;
    if (query_int_nt(value, name))
      return value;
    else
      return def_value;
  }

  bool get_bool(const wchar_t* name, bool def_value) {
    bool value;
    if (query_bool_nt(value, name))
      return value;
    else
      return def_value;
  }

  wstring get_str(const wchar_t* name, const wstring& def_value) {
    wstring value;
    if (query_str_nt(value, name))
      return value;
    else
      return def_value;
  }

  ByteVector get_binary(const wchar_t* name, const ByteVector& def_value) {
    ByteVector value;
    if (query_binary_nt(value, name))
      return value;
    else
      return def_value;
  }

  TriState get_tri_state(const wchar_t* name, TriState def_value) {
    unsigned value;
    if (query_int_nt(value, name))
      return static_cast<TriState>(value);
    else
      return def_value;
  }

  void set_int(const wchar_t* name, unsigned value, unsigned def_value) {
    if (value == def_value)
      delete_value_nt(name);
    else
      set_int_nt(name, value);
  }

  void set_bool(const wchar_t* name, bool value, bool def_value) {
    if (value == def_value)
      delete_value_nt(name);
    else
      set_bool_nt(name, value);
  }

  void set_str(const wchar_t* name, const wstring& value, const wstring& def_value) {
    if (value == def_value)
      delete_value_nt(name);
    else
      set_str_nt(name, value);
  }

  void set_binary(const wchar_t* name, const ByteVector& value, const ByteVector& def_value) {
    if (value == def_value)
      delete_value_nt(name);
    else
      set_binary_nt(name, value.data(), static_cast<unsigned>(value.size()));
  }

  void set_tri_state(const wchar_t* name, TriState value, TriState def_value) {
    if (value == def_value)
      delete_value_nt(name);
    else
      set_int_nt(name, static_cast<unsigned>(value));
  }
};

Options g_options;
UpdateProfiles g_profiles;

const wchar_t* c_plugin_key_name = L"arclite";
const wchar_t* c_profiles_key_name = L"profiles";

wstring get_plugin_key_name() {
  return add_trailing_slash(Far::get_root_key_name()) + c_plugin_key_name;
}

wstring get_profiles_key_name() {
  return add_trailing_slash(add_trailing_slash(Far::get_root_key_name()) + c_plugin_key_name) + c_profiles_key_name;
}

wstring get_profile_key_name(const wstring& name) {
  return add_trailing_slash(add_trailing_slash(add_trailing_slash(Far::get_root_key_name()) + c_plugin_key_name )+ c_profiles_key_name) + name;
}

const wchar_t* c_param_handle_create = L"handle_create";
const wchar_t* c_param_handle_commands = L"handle_commands";
const wchar_t* c_param_plugin_prefix = L"plugin_prefix";
const wchar_t* c_param_max_check_size = L"max_check_size";
const wchar_t* c_param_extract_ignore_errors = L"extract_ignore_errors";
const wchar_t* c_param_extract_overwrite = L"extract_overwrite";
const wchar_t* c_param_extract_separate_dir = L"extract_separate_dir";
const wchar_t* c_param_update_arc_format_name = L"update_arc_type";
const wchar_t* c_param_update_level = L"update_level";
const wchar_t* c_param_update_method = L"update_method";
const wchar_t* c_param_update_solid = L"update_solid";
const wchar_t* c_param_update_show_password = L"update_show_password";
const wchar_t* c_param_update_encrypt_header = L"update_encrypt_header";
const wchar_t* c_param_update_volume_size = L"update_volume_size";
const wchar_t* c_param_update_ignore_errors = L"update_ignore_errors";
const wchar_t* c_param_update_overwrite = L"update_overwrite";
const wchar_t* c_param_panel_view_mode = L"panel_view_mode";
const wchar_t* c_param_panel_sort_mode = L"panel_sort_mode";
const wchar_t* c_param_panel_reverse_sort = L"panel_reverse_sort";
const wchar_t* c_param_use_include_masks = L"use_include_masks";
const wchar_t* c_param_include_masks = L"include_masks";
const wchar_t* c_param_use_exclude_masks = L"use_exclude_masks";
const wchar_t* c_param_exclude_masks = L"exclude_masks";
const wchar_t* c_param_use_enabled_formats = L"use_enabled_formats";
const wchar_t* c_param_enabled_formats = L"enabled_formats";
const wchar_t* c_param_use_disabled_formats = L"use_disabled_formats";
const wchar_t* c_param_disabled_formats = L"disabled_formats";

const bool c_def_handle_create = true;
const bool c_def_handle_commands = true;
const wchar_t* c_def_plugin_prefix = L"arc";
const unsigned c_def_max_check_size = 1 << 20;
const bool c_def_extract_ignore_errors = false;
const OverwriteAction c_def_extract_overwrite = oaAsk;
const TriState c_def_extract_separate_dir = triUndef;
const wchar_t* c_def_update_arc_format_name = L"7z";
const unsigned c_def_update_level = 5;
const wchar_t* c_def_update_method = L"";
const bool c_def_update_solid = true;
const bool c_def_update_show_password = false;
const TriState c_def_update_encrypt_header = triUndef;
const wchar_t* c_def_update_volume_size = L"";
const bool c_def_update_ignore_errors = false;
const OverwriteAction c_def_update_overwrite = oaAsk;
const unsigned c_def_panel_view_mode = 2;
const unsigned c_def_panel_sort_mode = SM_NAME;
const bool c_def_panel_reverse_sort = false;
const bool c_def_use_include_masks = false;
const wchar_t* c_def_include_masks = L"";
const bool c_def_use_exclude_masks = false;
const wchar_t* c_def_exclude_masks = L"";
const bool c_def_use_enabled_formats = false;
const wchar_t* c_def_enabled_formats = L"";
const bool c_def_use_disabled_formats = false;
const wchar_t* c_def_disabled_formats = L"";

void load_sfx_options(OptionsKey& key, SfxOptions& sfx_options) {
  SfxOptions def_sfx_options;
#define GET_VALUE(name, type) sfx_options.name = key.get_##type(L"sfx."L#name, def_sfx_options.name)
  GET_VALUE(name, str);
  GET_VALUE(replace_icon, bool);
  GET_VALUE(icon_path, str);
  GET_VALUE(replace_version, bool);
  GET_VALUE(ver_info.version, str);
  GET_VALUE(ver_info.comments, str);
  GET_VALUE(ver_info.company_name, str);
  GET_VALUE(ver_info.file_description, str);
  GET_VALUE(ver_info.legal_copyright, str);
  GET_VALUE(ver_info.product_name, str);
  GET_VALUE(append_install_config, bool);
  GET_VALUE(install_config.title, str);
  GET_VALUE(install_config.begin_prompt, str);
  GET_VALUE(install_config.progress, str);
  GET_VALUE(install_config.run_program, str);
  GET_VALUE(install_config.directory, str);
  GET_VALUE(install_config.execute_file, str);
  GET_VALUE(install_config.execute_parameters, str);
#undef GET_VALUE
}

void save_sfx_options(OptionsKey& key, const SfxOptions& sfx_options) {
  SfxOptions def_sfx_options;
#define SET_VALUE(name, type) key.set_##type(L"sfx."L#name, sfx_options.name, def_sfx_options.name)
  SET_VALUE(name, str);
  SET_VALUE(replace_icon, bool);
  SET_VALUE(icon_path, str);
  SET_VALUE(replace_version, bool);
  SET_VALUE(ver_info.version, str);
  SET_VALUE(ver_info.comments, str);
  SET_VALUE(ver_info.company_name, str);
  SET_VALUE(ver_info.file_description, str);
  SET_VALUE(ver_info.legal_copyright, str);
  SET_VALUE(ver_info.product_name, str);
  SET_VALUE(append_install_config, bool);
  SET_VALUE(install_config.title, str);
  SET_VALUE(install_config.begin_prompt, str);
  SET_VALUE(install_config.progress, str);
  SET_VALUE(install_config.run_program, str);
  SET_VALUE(install_config.directory, str);
  SET_VALUE(install_config.execute_file, str);
  SET_VALUE(install_config.execute_parameters, str);
#undef SET_VALUE
}

void Options::load() {
  OptionsKey key;
  key.open_nt(HKEY_CURRENT_USER, get_plugin_key_name().c_str(), KEY_QUERY_VALUE, false);
  handle_create = key.get_bool(c_param_handle_create, c_def_handle_create);
  handle_commands = key.get_bool(c_param_handle_commands, c_def_handle_commands);
  plugin_prefix = key.get_str(c_param_plugin_prefix, c_def_plugin_prefix);
  max_check_size = key.get_int(c_param_max_check_size, c_def_max_check_size);
  extract_ignore_errors = key.get_bool(c_param_extract_ignore_errors, c_def_extract_ignore_errors);
  extract_overwrite = static_cast<OverwriteAction>(key.get_int(c_param_extract_overwrite, c_def_extract_overwrite));
  extract_separate_dir = key.get_tri_state(c_param_extract_separate_dir, c_def_extract_separate_dir);
  update_arc_format_name = key.get_str(c_param_update_arc_format_name, c_def_update_arc_format_name);
  update_level = key.get_int(c_param_update_level, c_def_update_level);
  update_method = key.get_str(c_param_update_method, c_def_update_method);
  update_solid = key.get_bool(c_param_update_solid, c_def_update_solid);
  update_show_password = key.get_bool(c_param_update_show_password, c_def_update_show_password);
  update_encrypt_header = key.get_tri_state(c_param_update_encrypt_header, c_def_update_encrypt_header);
  load_sfx_options(key, update_sfx_options);
  update_volume_size = key.get_str(c_param_update_volume_size, c_def_update_volume_size);
  update_ignore_errors = key.get_bool(c_param_update_ignore_errors, c_def_update_ignore_errors);
  update_overwrite = static_cast<OverwriteAction>(key.get_int(c_param_update_overwrite, c_def_update_overwrite));
  panel_view_mode = key.get_int(c_param_panel_view_mode, c_def_panel_view_mode);
  panel_sort_mode = key.get_int(c_param_panel_sort_mode, c_def_panel_sort_mode);
  panel_reverse_sort = key.get_bool(c_param_panel_reverse_sort, c_def_panel_reverse_sort);
  use_include_masks = key.get_bool(c_param_use_include_masks, c_def_use_include_masks);
  include_masks = key.get_str(c_param_include_masks, c_def_include_masks);
  use_exclude_masks = key.get_bool(c_param_use_exclude_masks, c_def_use_exclude_masks);
  exclude_masks = key.get_str(c_param_exclude_masks, c_def_exclude_masks);
  use_enabled_formats = key.get_bool(c_param_use_enabled_formats, c_def_use_enabled_formats);
  enabled_formats = key.get_str(c_param_enabled_formats, c_def_enabled_formats);
  use_disabled_formats = key.get_bool(c_param_use_disabled_formats, c_def_use_disabled_formats);
  disabled_formats = key.get_str(c_param_disabled_formats, c_def_disabled_formats);
};

void Options::save() const {
  OptionsKey key;
  key.open_nt(HKEY_CURRENT_USER, get_plugin_key_name().c_str(), KEY_SET_VALUE, true);
  key.set_bool(c_param_handle_create, handle_create, c_def_handle_create);
  key.set_bool(c_param_handle_commands, handle_commands, c_def_handle_commands);
  key.set_str(c_param_plugin_prefix, plugin_prefix, c_def_plugin_prefix);
  key.set_int(c_param_max_check_size, max_check_size, c_def_max_check_size);
  key.set_bool(c_param_extract_ignore_errors, extract_ignore_errors, c_def_extract_ignore_errors);
  key.set_int(c_param_extract_overwrite, extract_overwrite, c_def_extract_overwrite);
  key.set_tri_state(c_param_extract_separate_dir, extract_separate_dir, c_def_extract_separate_dir);
  key.set_str(c_param_update_arc_format_name, update_arc_format_name, c_def_update_arc_format_name);
  key.set_int(c_param_update_level, update_level, c_def_update_level);
  key.set_str(c_param_update_method, update_method, c_def_update_method);
  key.set_bool(c_param_update_solid, update_solid, c_def_update_solid);
  key.set_bool(c_param_update_show_password, update_show_password, c_def_update_show_password);
  key.set_tri_state(c_param_update_encrypt_header, update_encrypt_header, c_def_update_encrypt_header);
  save_sfx_options(key, update_sfx_options);
  key.set_str(c_param_update_volume_size, update_volume_size, c_def_update_volume_size);
  key.set_bool(c_param_update_ignore_errors, update_ignore_errors, c_def_update_ignore_errors);
  key.set_int(c_param_update_overwrite, update_overwrite, c_def_update_overwrite);
  key.set_int(c_param_panel_view_mode, panel_view_mode, c_def_panel_view_mode);
  key.set_int(c_param_panel_sort_mode, panel_sort_mode, c_def_panel_sort_mode);
  key.set_bool(c_param_panel_reverse_sort, panel_reverse_sort, c_def_panel_reverse_sort);
  key.set_bool(c_param_use_include_masks, use_include_masks, c_def_use_include_masks);
  key.set_str(c_param_include_masks, include_masks, c_def_include_masks);
  key.set_bool(c_param_use_exclude_masks, use_exclude_masks, c_def_use_exclude_masks);
  key.set_str(c_param_exclude_masks, exclude_masks, c_def_exclude_masks);
  key.set_bool(c_param_use_enabled_formats, use_enabled_formats, c_def_use_enabled_formats);
  key.set_str(c_param_enabled_formats, enabled_formats, c_def_enabled_formats);
  key.set_bool(c_param_use_disabled_formats, use_disabled_formats, c_def_use_disabled_formats);
  key.set_str(c_param_disabled_formats, disabled_formats, c_def_disabled_formats);
}

const wchar_t* c_param_profile_arc_path = L"arc_path";
const wchar_t* c_param_profile_arc_type = L"arc_type";
const wchar_t* c_param_profile_level = L"level";
const wchar_t* c_param_profile_method = L"method";
const wchar_t* c_param_profile_solid = L"solid";
const wchar_t* c_param_profile_password = L"password";
const wchar_t* c_param_profile_show_password = L"show_password";
const wchar_t* c_param_profile_encrypt = L"encrypt";
const wchar_t* c_param_profile_encrypt_header = L"encrypt_header";
const wchar_t* c_param_profile_encrypt_header_defined = L"encrypt_header_defined";
const wchar_t* c_param_profile_create_sfx = L"create_sfx";
const wchar_t* c_param_profile_enable_volumes = L"enable_volumes";
const wchar_t* c_param_profile_volume_size = L"volume_size";
const wchar_t* c_param_profile_move_files = L"move_files";
const wchar_t* c_param_profile_open_shared = L"open_shared";
const wchar_t* c_param_profile_ignore_errors = L"ignore_errors";
const wchar_t* c_param_profile_advanced = L"advanced";

const wchar_t* c_def_profile_arc_path = L"";
const ArcType c_def_profile_arc_type = c_7z;
const unsigned c_def_profile_level = 5;
const wchar_t* c_def_profile_method = L"";
const bool c_def_profile_solid = true;
const wchar_t* c_def_profile_password = L"";
const bool c_def_profile_show_password = false;
const bool c_def_profile_encrypt = false;
const TriState c_def_profile_encrypt_header = triUndef;
const bool c_def_profile_create_sfx = false;
const bool c_def_profile_enable_volumes = false;
const wchar_t* c_def_profile_volume_size = L"";
const bool c_def_profile_move_files = false;
const bool c_def_profile_open_shared = false;
const bool c_def_profile_ignore_errors = false;
const wchar_t* c_def_profile_advanced = L"";

ProfileOptions::ProfileOptions():
  arc_type(c_def_profile_arc_type),
  level(c_def_profile_level),
  method(c_def_profile_method),
  solid(c_def_profile_solid),
  password(c_def_profile_password),
  encrypt(c_def_profile_encrypt),
  encrypt_header(c_def_profile_encrypt_header),
  create_sfx(c_def_profile_create_sfx),
  enable_volumes(c_def_profile_enable_volumes),
  volume_size(c_def_profile_volume_size),
  move_files(c_def_profile_move_files),
  ignore_errors(c_def_profile_ignore_errors),
  advanced(c_def_profile_advanced)
{}

UpdateOptions::UpdateOptions():
  arc_path(c_def_profile_arc_path),
  show_password(c_def_profile_show_password),
  open_shared(c_def_profile_open_shared),
  overwrite(oaAsk)
{}

SfxOptions::SfxOptions():
  replace_icon(false),
  replace_version(false),
  append_install_config(false)
{}

void UpdateProfiles::load() {
  clear();
  Key profiles_key;
  if (profiles_key.open_nt(HKEY_CURRENT_USER, get_profiles_key_name().c_str(), KEY_ENUMERATE_SUB_KEYS, false)) {
    vector<wstring> profile_names = profiles_key.enum_sub_keys();
    for (unsigned i = 0; i < profile_names.size(); i++) {
      UpdateProfile profile;
      profile.name = profile_names[i];
      OptionsKey key;
      key.open_nt(HKEY_CURRENT_USER, get_profile_key_name(profile.name).c_str(), KEY_QUERY_VALUE, false);
#define GET_VALUE(name, type) profile.options.name = key.get_##type(c_param_profile_##name, c_def_profile_##name)
      GET_VALUE(arc_type, binary);
      GET_VALUE(level, int);
      GET_VALUE(method, str);
      GET_VALUE(solid, bool);
      GET_VALUE(password, str);
      GET_VALUE(encrypt, bool);
      GET_VALUE(encrypt_header, tri_state);
      GET_VALUE(create_sfx, bool);
      load_sfx_options(key, profile.options.sfx_options);
      GET_VALUE(enable_volumes, bool);
      GET_VALUE(volume_size, str);
      GET_VALUE(move_files, bool);
      GET_VALUE(ignore_errors, bool);
      GET_VALUE(advanced, str);
#undef GET_VALUE
      push_back(profile);
    }
  }
  sort_by_name();
}

void UpdateProfiles::save() const {
  Key profiles_key;
  if (profiles_key.open_nt(HKEY_CURRENT_USER, get_profiles_key_name().c_str(), KEY_ENUMERATE_SUB_KEYS, true)) {
    vector<wstring> profile_names = profiles_key.enum_sub_keys();
    for (unsigned i = 0; i < profile_names.size(); i++) {
      profiles_key.delete_sub_key(profile_names[i].c_str());
    }
  }
  for_each(begin(), end(), [&] (const UpdateProfile& profile) {
    OptionsKey key;
    key.open_nt(HKEY_CURRENT_USER, get_profile_key_name(profile.name).c_str(), KEY_SET_VALUE, true);
#define SET_VALUE(name, type) key.set_##type(c_param_profile_##name, profile.options.name, c_def_profile_##name)
    SET_VALUE(arc_type, binary);
    SET_VALUE(level, int);
    SET_VALUE(method, str);
    SET_VALUE(solid, bool);
    SET_VALUE(password, str);
    SET_VALUE(encrypt, bool);
    SET_VALUE(encrypt_header, tri_state);
    SET_VALUE(create_sfx, bool);
    save_sfx_options(key, profile.options.sfx_options);
    SET_VALUE(enable_volumes, bool);
    SET_VALUE(volume_size, str);
    SET_VALUE(move_files, bool);
    SET_VALUE(ignore_errors, bool);
    SET_VALUE(advanced, str);
#undef SET_VALUE
  });
}

unsigned UpdateProfiles::find_by_name(const wstring& name) {
  unsigned idx = 0;
  for (idx = 0; idx < size() && upcase(at(idx).name) != upcase(name); idx++);
  return idx;
}

unsigned UpdateProfiles::find_by_options(const UpdateOptions& options) {
  unsigned idx;
  for (idx = 0; idx < size() && !(at(idx).options == options); idx++);
  return idx;
}

void UpdateProfiles::sort_by_name() {
  sort(begin(), end(), [] (const UpdateProfile& p1, const UpdateProfile& p2) -> bool {
    return upcase(p1.name) < upcase(p2.name);
  });
}

void UpdateProfiles::update(const wstring& name, const UpdateOptions& options) {
  unsigned name_idx = find_by_name(name);
  unsigned options_idx = find_by_options(options);
  if (name_idx < size() && options_idx < size()) {
    at(name_idx).name = name;
    at(name_idx).options = options;
    erase(begin() + options_idx);
  }
  else if (name_idx < size()) {
    at(name_idx).name = name;
    at(name_idx).options = options;
  }
  else if (options_idx < size()) {
    at(options_idx).name = name;
    at(options_idx).options = options;
  }
  else {
    UpdateProfile new_profile;
    new_profile.name = name;
    new_profile.options = options;
    push_back(new_profile);
  }
  sort_by_name();
}
