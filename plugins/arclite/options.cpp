#include "utils.hpp"
#include "sysutils.hpp"
#include "farutils.hpp"
#include "common.hpp"
#include "archive.hpp"
#include "options.hpp"


class OptionsKey: public Far::Settings {
public:
  template<class Integer>
  Integer get_int(const wchar_t* name, Integer def_value) {
    unsigned __int64 value;
    if (get(name, value))
      return static_cast<Integer>(value);
    else
      return def_value;
  }

  bool get_bool(const wchar_t* name, bool def_value) {
    unsigned __int64 value;
    if (get(name, value))
      return value != 0;
    else
      return def_value;
  }

  wstring get_str(const wchar_t* name, const wstring& def_value) {
    wstring value;
    if (get(name, value))
      return value;
    else
      return def_value;
  }

  ByteVector get_binary(const wchar_t* name, const ByteVector& def_value) {
    ByteVector value;
    if (get(name, value))
      return value;
    else
      return def_value;
  }

  void set_int(const wchar_t* name, unsigned value, unsigned def_value) {
    if (value == def_value)
      del(name);
    else
      set(name, value);
  }

  void set_bool(const wchar_t* name, bool value, bool def_value) {
    if (value == def_value)
      del(name);
    else
      set(name, value ? 1 : 0);
  }

  void set_str(const wchar_t* name, const wstring& value, const wstring& def_value) {
    if (value == def_value)
      del(name);
    else
      set(name, value);
  }

  void set_binary(const wchar_t* name, const ByteVector& value, const ByteVector& def_value) {
    if (value == def_value)
      del(name);
    else
      set(name, value.data(), value.size());
  }
};

Options g_options;

Options::Options():
  handle_create(true),
  handle_commands(true),
  plugin_prefix(L"arc"),
  max_check_size(1 << 20),
  extract_ignore_errors(false),
  extract_overwrite(oaAsk),
  extract_separate_dir(triUndef),
  extract_open_dir(false),
  update_arc_format_name(L"7z"),
  update_level(5),
  update_method(),
  update_solid(true),
  update_show_password(false),
  update_encrypt_header(triUndef),
  update_sfx_options(),
  update_volume_size(),
  update_ignore_errors(false),
  update_overwrite(oaAsk),
  update_append_ext(false),
  panel_view_mode(2),
  panel_sort_mode(SM_NAME),
  panel_reverse_sort(false),
  use_include_masks(true),
  include_masks(L"*.rar,*.zip,*.[zj],*.[bg7]z,*.[bg]zip,*.tar,*.t[agbx]z,*.ar[cj],*.r[0-9][0-9],*.a[0-9][0-9],*.bz2,*.cab,*.jar,*.lha,*.lzh,*.ha,*.ac[bei],*.pa[ck],*.rk,*.cpio,*.rpm,*.zoo,*.hqx,*.sit,*.ice,*.uc2,*.ain,*.imp,*.777,*.ufa,*.boa,*.bs[2a],*.sea,*.hpk,*.ddi,*.x2,*.rkv,*.[lw]sz,*.h[ay]p,*.lim,*.sqz,*.chz"),
  use_exclude_masks(false),
  exclude_masks(),
  pgdn_masks(false),
  use_enabled_formats(false),
  enabled_formats(),
  use_disabled_formats(false),
  disabled_formats(),
  pgdn_formats(false)
{}

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
  key.create();
  Options def_options;
#define GET_VALUE(name, type) name = key.get_##type(L#name, def_options.name)
  GET_VALUE(handle_create, bool);
  GET_VALUE(handle_commands, bool);
  GET_VALUE(plugin_prefix, str);
  GET_VALUE(max_check_size, int);
  GET_VALUE(extract_ignore_errors, bool);
  GET_VALUE(extract_overwrite, int);
  GET_VALUE(extract_separate_dir, int);
  GET_VALUE(extract_open_dir, bool);
  GET_VALUE(update_arc_format_name, str);
  GET_VALUE(update_level, int);
  GET_VALUE(update_method, str);
  GET_VALUE(update_solid, bool);
  GET_VALUE(update_show_password, bool);
  GET_VALUE(update_encrypt_header, int);
  load_sfx_options(key, update_sfx_options);
  GET_VALUE(update_volume_size, str);
  GET_VALUE(update_ignore_errors, bool);
  GET_VALUE(update_overwrite, int);
  GET_VALUE(update_append_ext, bool);
  GET_VALUE(panel_view_mode, int);
  GET_VALUE(panel_sort_mode, int);
  GET_VALUE(panel_reverse_sort, bool);
  GET_VALUE(use_include_masks, bool);
  GET_VALUE(include_masks, str);
  GET_VALUE(use_exclude_masks, bool);
  GET_VALUE(exclude_masks, str);
  GET_VALUE(pgdn_masks, bool);
  GET_VALUE(use_enabled_formats, bool);
  GET_VALUE(enabled_formats, str);
  GET_VALUE(use_disabled_formats, bool);
  GET_VALUE(disabled_formats, str);
  GET_VALUE(pgdn_formats, bool);
#undef GET_VALUE
};

void Options::save() const {
  OptionsKey key;
  key.create();
  Options def_options;
#define SET_VALUE(name, type) key.set_##type(L#name, name, def_options.name)
  SET_VALUE(handle_create, bool);
  SET_VALUE(handle_commands, bool);
  SET_VALUE(plugin_prefix, str);
  SET_VALUE(max_check_size, int);
  SET_VALUE(extract_ignore_errors, bool);
  SET_VALUE(extract_overwrite, int);
  SET_VALUE(extract_separate_dir, int);
  SET_VALUE(extract_open_dir, bool);
  SET_VALUE(update_arc_format_name, str);
  SET_VALUE(update_level, int);
  SET_VALUE(update_method, str);
  SET_VALUE(update_solid, bool);
  SET_VALUE(update_show_password, bool);
  SET_VALUE(update_encrypt_header, int);
  save_sfx_options(key, update_sfx_options);
  SET_VALUE(update_volume_size, str);
  SET_VALUE(update_ignore_errors, bool);
  SET_VALUE(update_overwrite, int);
  SET_VALUE(update_append_ext, bool);
  SET_VALUE(panel_view_mode, int);
  SET_VALUE(panel_sort_mode, int);
  SET_VALUE(panel_reverse_sort, bool);
  SET_VALUE(use_include_masks, bool);
  SET_VALUE(include_masks, str);
  SET_VALUE(use_exclude_masks, bool);
  SET_VALUE(exclude_masks, str);
  SET_VALUE(pgdn_masks, bool);
  SET_VALUE(use_enabled_formats, bool);
  SET_VALUE(enabled_formats, str);
  SET_VALUE(use_disabled_formats, bool);
  SET_VALUE(disabled_formats, str);
  SET_VALUE(pgdn_formats, bool);
#undef SET_VALUE
}


UpdateProfiles g_profiles;

const wchar_t* c_profiles_key_name = L"profiles";

wstring get_profile_key_name(const wstring& name) {
  return add_trailing_slash(c_profiles_key_name) + name;
}

ProfileOptions::ProfileOptions():
  arc_type(c_7z),
  level(5),
  method(),
  solid(true),
  password(),
  encrypt(false),
  encrypt_header(triUndef),
  create_sfx(false),
  enable_volumes(false),
  volume_size(),
  move_files(false),
  ignore_errors(false),
  advanced()
{}

UpdateOptions::UpdateOptions():
  arc_path(),
  show_password(false),
  open_shared(false),
  overwrite(oaAsk),
  append_ext(false)
{}

SfxOptions::SfxOptions():
  replace_icon(false),
  replace_version(false),
  append_install_config(false)
{}

void UpdateProfiles::load() {
  clear();
  OptionsKey key;
  key.create();
  key.set_dir(c_profiles_key_name);
  vector<wstring> profile_names;
  key.list_dir(profile_names);
  ProfileOptions def_profile_options;
  for (unsigned i = 0; i < profile_names.size(); i++) {
    UpdateProfile profile;
    profile.name = profile_names[i];
    key.set_dir(get_profile_key_name(profile.name));
#define GET_VALUE(name, type) profile.options.name = key.get_##type(L#name, def_profile_options.name)
    GET_VALUE(arc_type, binary);
    GET_VALUE(level, int);
    GET_VALUE(method, str);
    GET_VALUE(solid, bool);
    GET_VALUE(password, str);
    GET_VALUE(encrypt, bool);
    GET_VALUE(encrypt_header, int);
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
  sort_by_name();
}

void UpdateProfiles::save() const {
  OptionsKey key;
  key.create();
  key.set_dir(c_profiles_key_name);
  vector<wstring> profile_names;
  key.list_dir(profile_names);
  for (unsigned i = 0; i < profile_names.size(); i++) {
    key.del(profile_names[i].c_str());
  }
  ProfileOptions def_profile_options;
  for_each(cbegin(), cend(), [&] (const UpdateProfile& profile) {
    key.set_dir(get_profile_key_name(profile.name));
#define SET_VALUE(name, type) key.set_##type(L#name, profile.options.name, def_profile_options.name)
    SET_VALUE(arc_type, binary);
    SET_VALUE(level, int);
    SET_VALUE(method, str);
    SET_VALUE(solid, bool);
    SET_VALUE(password, str);
    SET_VALUE(encrypt, bool);
    SET_VALUE(encrypt_header, int);
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
