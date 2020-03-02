#include "msg.h"
#include "guids.hpp"
#include "utils.hpp"
#include "farutils.hpp"
#include "sysutils.hpp"
#include "common.hpp"
#include "archive.hpp"
#include "ui.hpp"

class AttachSfxModuleProgress: public ProgressMonitor {
private:
  std::wstring file_path;
  UInt64 completed;
  UInt64 total;

  virtual void do_update_ui() {
    const unsigned c_width = 60;

    percent_done = calc_percent(completed, total);

    std::wostringstream st;
    st << fit_str(file_path, c_width) << L'\n';
    st << Far::get_progress_bar_str(c_width, percent_done, 100) << L'\n';
    progress_text = st.str();
  }

public:
  AttachSfxModuleProgress(const std::wstring& file_path): ProgressMonitor(Far::get_msg(MSG_PROGRESS_SFX_CONVERT)), file_path(file_path), completed(0), total(0) {
  }

  void set_total(UInt64 size) {
    total = size;
  }
  void update_completed(UInt64 size) {
    completed += size;
    update_ui();
  }
};

void replace_icon(const std::wstring& pe_path, const std::wstring& ico_path);
void replace_ver_info(const std::wstring& pe_path, const SfxVersionInfo& ver_info);

ByteVector generate_install_config(const SfxInstallConfig& config) {
  std::wstring text;
  text += L";!@Install@!UTF-8!\n";
  if (!config.title.empty())
    text += L"Title=\"" + config.title + L"\"\n";
  if (!config.begin_prompt.empty())
    text += L"BeginPrompt=\"" + config.begin_prompt + L"\"\n";
  if (!config.progress.empty())
    text += L"Progress=\"" + config.progress + L"\"\n";
  if (!config.run_program.empty())
    text += L"RunProgram=\"" + config.run_program + L"\"\n";
  if (!config.directory.empty())
    text += L"Directory=\"" + config.directory + L"\"\n";
  if (!config.execute_file.empty())
    text += L"ExecuteFile=\"" + config.execute_file + L"\"\n";
  if (!config.execute_parameters.empty())
    text += L"ExecuteParameters=\"" + config.execute_parameters + L"\"\n";
  text += L";!@InstallEnd@!\n";
  std::string utf8_text = unicode_to_ansi(text, CP_UTF8);
  return ByteVector(utf8_text.begin(), utf8_text.end());
}

void create_sfx_module(const std::wstring& file_path, const SfxOptions& sfx_options) {
  uintptr_t sfx_id = ArcAPI::sfx().find_by_name(sfx_options.name);
  CHECK(sfx_id < ArcAPI::sfx().size());
  std::wstring sfx_path = ArcAPI::sfx()[sfx_id].path;

  File file;
  file.open(file_path, FILE_WRITE_DATA, FILE_SHARE_READ, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL);

  File sfx_file(sfx_path, FILE_READ_DATA, FILE_SHARE_READ, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN);
  Buffer<unsigned char> buf(static_cast<size_t>(sfx_file.size()));
  sfx_file.read(buf.data(), static_cast<unsigned>(buf.size()));
  file.write(buf.data(), buf.size());

  file.close();

  if (sfx_options.replace_icon)
    replace_icon(file_path, sfx_options.icon_path);

  if (sfx_options.replace_version)
    replace_ver_info(file_path, sfx_options.ver_info);

  if (sfx_options.append_install_config) {
    file.open(file_path, FILE_WRITE_DATA, FILE_SHARE_READ, OPEN_EXISTING, 0);
    file.set_pos(0, FILE_END);

    ByteVector install_config = generate_install_config(sfx_options.install_config);
    file.write(install_config.data(), install_config.size());
  }
}

void attach_sfx_module(const std::wstring& file_path, const SfxOptions& sfx_options) {
  AttachSfxModuleProgress progress(file_path);

  {
    OpenOptions options;
    options.arc_path = file_path;
    options.detect = false;
    options.arc_types.push_back(c_7z);
    std::unique_ptr<Archives> archives(Archive::open(options));
    if (archives->empty())
      FAIL_MSG(Far::get_msg(MSG_ERROR_SFX_CONVERT));
    if (!archives->front()->is_pure_7z())
      FAIL_MSG(Far::get_msg(MSG_ERROR_SFX_CONVERT));
  }

  FindData file_data = File::get_find_data(file_path);
  progress.set_total(file_data.size());
  std::wstring dst_path = file_path + c_sfx_ext;
  try {
    create_sfx_module(dst_path, sfx_options);

    File dst_file(dst_path, FILE_WRITE_DATA | FILE_WRITE_ATTRIBUTES, FILE_SHARE_READ, OPEN_EXISTING, 0);
    dst_file.set_pos(0, FILE_END);

    File src_file(file_path, FILE_READ_DATA, FILE_SHARE_READ, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN);
    Buffer<char> buf(1024 * 1024);
    while (true) {
      size_t size_read = src_file.read(buf.data(), static_cast<unsigned>(buf.size()));
      if (size_read == 0)
        break;
      CHECK(dst_file.write(buf.data(), size_read) == size_read);
      progress.update_completed(size_read);
    }

    File::set_attr(file_path, file_data.dwFileAttributes);
    dst_file.set_time(file_data.ftCreationTime, file_data.ftLastAccessTime, file_data.ftLastWriteTime);
  }
  catch (...) {
    File::delete_file_nt(dst_path);
    throw;
  }
  File::delete_file_nt(file_path);
}


struct SfxProfile {
  std::wstring name;
  SfxOptions options;
};

typedef std::vector<SfxProfile> SfxProfiles;

class SfxOptionsDialog: public Far::Dialog {
private:
  enum {
    c_client_xs = 69
  };

  SfxOptions& options;
  SfxProfiles profiles;

  int profile_ctrl_id;
  int module_ctrl_id;
  int replace_icon_ctrl_id;
  int icon_path_ctrl_id;
  int replace_version_ctrl_id;
  int ver_info_version_ctrl_id;
  int ver_info_comments_ctrl_id;
  int ver_info_company_name_ctrl_id;
  int ver_info_file_description_ctrl_id;
  int ver_info_legal_copyright_ctrl_id;
  int ver_info_product_name_ctrl_id;
  int append_install_config_ctrl_id;
  int install_config_title_ctrl_id;
  int install_config_begin_prompt_ctrl_id;
  int install_config_progress_ctrl_id;
  int install_config_run_program_ctrl_id;
  int install_config_directory_ctrl_id;
  int install_config_execute_file_ctrl_id;
  int install_config_execute_parameters_ctrl_id;
  int ok_ctrl_id;
  int cancel_ctrl_id;

  void set_control_state() {
    DisableEvents de(*this);

    bool replace_icon = get_check(replace_icon_ctrl_id);
    for (int ctrl_id = replace_icon_ctrl_id + 1; ctrl_id < replace_version_ctrl_id; ctrl_id++)
      enable(ctrl_id, replace_icon);

    bool replace_version = get_check(replace_version_ctrl_id);
    for (int ctrl_id = replace_version_ctrl_id + 1; ctrl_id < append_install_config_ctrl_id; ctrl_id++)
      enable(ctrl_id, replace_version);

    unsigned module_id = get_list_pos(module_ctrl_id);
    bool install_config_enabled = module_id < ArcAPI::sfx().size() && ArcAPI::sfx()[module_id].install_config();
    enable(append_install_config_ctrl_id, install_config_enabled);

    bool append_install_config = get_check(append_install_config_ctrl_id);
    for (int ctrl_id = append_install_config_ctrl_id + 1; ctrl_id <= install_config_execute_parameters_ctrl_id; ctrl_id++)
      enable(ctrl_id, install_config_enabled && append_install_config);
  }

  SfxOptions read_controls() {
    SfxOptions options;
    const SfxModules& sfx_modules = ArcAPI::sfx();
    unsigned sfx_id = get_list_pos(module_ctrl_id);
    if (sfx_id >= sfx_modules.size()) {
      FAIL_MSG(Far::get_msg(MSG_SFX_OPTIONS_DLG_WRONG_MODULE));
    }
    options.name = extract_file_name(sfx_modules[sfx_id].path);
    options.replace_icon = get_check(replace_icon_ctrl_id);
    if (options.replace_icon) {
      options.icon_path = get_text(icon_path_ctrl_id);
      if (!File::exists(options.icon_path)) {
        FAIL_MSG(Far::get_msg(MSG_SFX_OPTIONS_DLG_INVALID_ICON_PATH));
      }
    }
    else
      options.icon_path.clear();
    options.replace_version = get_check(replace_version_ctrl_id);
    if (options.replace_version) {
      options.ver_info.version = get_text(ver_info_version_ctrl_id);
      options.ver_info.comments = get_text(ver_info_comments_ctrl_id);
      options.ver_info.company_name = get_text(ver_info_company_name_ctrl_id);
      options.ver_info.file_description = get_text(ver_info_file_description_ctrl_id);
      options.ver_info.legal_copyright = get_text(ver_info_legal_copyright_ctrl_id);
      options.ver_info.product_name = get_text(ver_info_product_name_ctrl_id);
    }
    else {
      options.ver_info.version.clear();
      options.ver_info.comments.clear();
      options.ver_info.company_name.clear();
      options.ver_info.file_description.clear();
      options.ver_info.legal_copyright.clear();
      options.ver_info.product_name.clear();
    }
    bool install_config_enabled = sfx_modules[sfx_id].install_config();
    options.append_install_config = install_config_enabled && get_check(append_install_config_ctrl_id);
    if (options.append_install_config) {
      options.install_config.title = get_text(install_config_title_ctrl_id);
      options.install_config.begin_prompt = get_text(install_config_begin_prompt_ctrl_id);
      TriState value = get_check3(install_config_progress_ctrl_id);
      if (value == triUndef)
        options.install_config.progress.clear();
      else
        options.install_config.progress = value == triTrue ? L"yes" : L"no";
      options.install_config.run_program = get_text(install_config_run_program_ctrl_id);
      options.install_config.directory = get_text(install_config_directory_ctrl_id);
      options.install_config.execute_file = get_text(install_config_execute_file_ctrl_id);
      options.install_config.execute_parameters = get_text(install_config_execute_parameters_ctrl_id);
    }
    else {
      options.install_config.title.clear();
      options.install_config.begin_prompt.clear();
      options.install_config.progress.clear();
      options.install_config.run_program.clear();
      options.install_config.directory.clear();
      options.install_config.execute_file.clear();
      options.install_config.execute_parameters.clear();
    }
    return options;
  }

  void write_controls(const SfxOptions& options) {
    DisableEvents de(*this);

    set_list_pos(module_ctrl_id, ArcAPI::sfx().find_by_name(options.name));

    set_check(replace_icon_ctrl_id, options.replace_icon);
    set_text(icon_path_ctrl_id, options.icon_path);

    set_check(replace_version_ctrl_id, options.replace_version);
    set_text(ver_info_product_name_ctrl_id, options.ver_info.product_name);
    set_text(ver_info_version_ctrl_id, options.ver_info.version);
    set_text(ver_info_company_name_ctrl_id, options.ver_info.company_name);
    set_text(ver_info_file_description_ctrl_id, options.ver_info.file_description);
    set_text(ver_info_comments_ctrl_id, options.ver_info.comments);
    set_text(ver_info_legal_copyright_ctrl_id, options.ver_info.legal_copyright);

    set_check(append_install_config_ctrl_id, options.append_install_config);
    set_text(install_config_title_ctrl_id, options.install_config.title);
    set_text(install_config_begin_prompt_ctrl_id, options.install_config.begin_prompt);
    TriState value;
    if (options.install_config.progress == L"yes")
      value = triTrue;
    else if (options.install_config.progress == L"no")
      value = triFalse;
    else
      value = triUndef;
    set_check3(install_config_progress_ctrl_id, value);
    set_text(install_config_run_program_ctrl_id, options.install_config.run_program);
    set_text(install_config_directory_ctrl_id, options.install_config.directory);
    set_text(install_config_execute_file_ctrl_id, options.install_config.execute_file);
    set_text(install_config_execute_parameters_ctrl_id, options.install_config.execute_parameters);
  }

  intptr_t dialog_proc(intptr_t msg, intptr_t param1, void* param2) {
    if (msg == DN_CLOSE && param1 >= 0 && param1 != cancel_ctrl_id) {
      options = read_controls();
    }
    else if (msg == DN_INITDIALOG) {
      set_control_state();
    }
    else if (msg == DN_BTNCLICK) {
      set_control_state();
    }
    else if (msg == DN_EDITCHANGE && param1 == module_ctrl_id) {
      set_control_state();
    }
    else if (msg == DN_EDITCHANGE && param1 == profile_ctrl_id) {
      unsigned profile_idx = get_list_pos(profile_ctrl_id);
      if (profile_idx != (unsigned)-1 && profile_idx < profiles.size()) {
        write_controls(profiles[profile_idx].options);
        set_control_state();
      }
    }

    if (msg == DN_EDITCHANGE || msg == DN_BTNCLICK) {
      unsigned profile_idx = static_cast<unsigned>(profiles.size());
      SfxOptions options;
      bool valid_options = true;
      try {
        options = read_controls();
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
  SfxOptionsDialog(SfxOptions& options, const UpdateProfiles& update_profiles): Far::Dialog(Far::get_msg(MSG_SFX_OPTIONS_DLG_TITLE), &c_sfx_options_dialog_guid, c_client_xs, L"SfxOptions"), options(options) {
    std::for_each(update_profiles.cbegin(), update_profiles.cend(), [&] (const UpdateProfile& update_profile) {
      if (update_profile.options.create_sfx) {
        SfxProfile sfx_profile;
        sfx_profile.name = update_profile.name;
        sfx_profile.options = update_profile.options.sfx_options;
        profiles.push_back(sfx_profile);
      }
    });
  }

  bool show() {
    label(Far::get_msg(MSG_SFX_OPTIONS_DLG_PROFILE));
    std::vector<std::wstring> profile_names;
    profile_names.reserve(profiles.size());
    for (unsigned i = 0; i < profiles.size(); i++) {
      profile_names.push_back(profiles[i].name);
    }
    profile_names.push_back(std::wstring());
    profile_ctrl_id = combo_box(profile_names, profiles.size(), 30, DIF_DROPDOWNLIST);
    new_line();
    separator();
    new_line();

    label(Far::get_msg(MSG_SFX_OPTIONS_DLG_MODULE));
    std::vector<std::wstring> module_names;
    const SfxModules& sfx_modules = ArcAPI::sfx();
    module_names.reserve(sfx_modules.size() + 1);
    size_t name_width = 0;
    std::for_each(sfx_modules.begin(), sfx_modules.end(), [&] (const SfxModule& sfx_module) {
      std::wstring name = sfx_module.description();
      module_names.push_back(name);
      if (name_width < name.size())
        name_width = name.size();
    });
    module_names.push_back(std::wstring());
    module_ctrl_id = combo_box(module_names, sfx_modules.find_by_name(options.name), name_width + 6, DIF_DROPDOWNLIST);
    new_line();

    replace_icon_ctrl_id = check_box(Far::get_msg(MSG_SFX_OPTIONS_DLG_REPLACE_ICON), options.replace_icon);
    new_line();
    spacer(2);
    label(Far::get_msg(MSG_SFX_OPTIONS_DLG_ICON_PATH));
    icon_path_ctrl_id = history_edit_box(options.icon_path, L"arclite.icon_path", AUTO_SIZE, DIF_EDITPATH | DIF_SELECTONENTRY);
    new_line();

    size_t label_len = 0;
    std::vector<std::wstring> labels;
    labels.push_back(Far::get_msg(MSG_SFX_OPTIONS_DLG_VER_INFO_PRODUCT_NAME));
    labels.push_back(Far::get_msg(MSG_SFX_OPTIONS_DLG_VER_INFO_VERSION));
    labels.push_back(Far::get_msg(MSG_SFX_OPTIONS_DLG_VER_INFO_COMPANY_NAME));
    labels.push_back(Far::get_msg(MSG_SFX_OPTIONS_DLG_VER_INFO_FILE_DESCRIPTION));
    labels.push_back(Far::get_msg(MSG_SFX_OPTIONS_DLG_VER_INFO_COMMENTS));
    labels.push_back(Far::get_msg(MSG_SFX_OPTIONS_DLG_VER_INFO_LEGAL_COPYRIGHT));
    for (unsigned i = 0; i < labels.size(); i++)
      if (label_len < labels[i].size())
        label_len = labels[i].size();
    label_len += 2;
    std::vector<std::wstring>::const_iterator label_text = labels.cbegin();
    replace_version_ctrl_id = check_box(Far::get_msg(MSG_SFX_OPTIONS_DLG_REPLACE_VERSION), options.replace_version);
    new_line();
    spacer(2);
    label(*label_text++);
    pad(label_len);
    ver_info_product_name_ctrl_id = edit_box(options.ver_info.product_name, AUTO_SIZE, DIF_SELECTONENTRY);
    new_line();
    spacer(2);
    label(*label_text++);
    pad(label_len);
    ver_info_version_ctrl_id = edit_box(options.ver_info.version, AUTO_SIZE, DIF_SELECTONENTRY);
    new_line();
    spacer(2);
    label(*label_text++);
    pad(label_len);
    ver_info_company_name_ctrl_id = edit_box(options.ver_info.company_name, AUTO_SIZE, DIF_SELECTONENTRY);
    new_line();
    spacer(2);
    label(*label_text++);
    pad(label_len);
    ver_info_file_description_ctrl_id = edit_box(options.ver_info.file_description, AUTO_SIZE, DIF_SELECTONENTRY);
    new_line();
    spacer(2);
    label(*label_text++);
    pad(label_len);
    ver_info_comments_ctrl_id = edit_box(options.ver_info.comments, AUTO_SIZE, DIF_SELECTONENTRY);
    new_line();
    spacer(2);
    label(*label_text++);
    pad(label_len);
    ver_info_legal_copyright_ctrl_id = edit_box(options.ver_info.legal_copyright, AUTO_SIZE, DIF_SELECTONENTRY);
    new_line();

    label_len = 0;
    labels.clear();
    labels.push_back(Far::get_msg(MSG_SFX_OPTIONS_DLG_INSTALL_CONFIG_TITLE));
    labels.push_back(Far::get_msg(MSG_SFX_OPTIONS_DLG_INSTALL_CONFIG_BEGIN_PROMPT));
    labels.push_back(Far::get_msg(MSG_SFX_OPTIONS_DLG_INSTALL_CONFIG_PROGRESS));
    labels.push_back(Far::get_msg(MSG_SFX_OPTIONS_DLG_INSTALL_CONFIG_RUN_PROGRAM));
    labels.push_back(Far::get_msg(MSG_SFX_OPTIONS_DLG_INSTALL_CONFIG_DIRECTORY));
    labels.push_back(Far::get_msg(MSG_SFX_OPTIONS_DLG_INSTALL_CONFIG_EXECUTE_FILE));
    labels.push_back(Far::get_msg(MSG_SFX_OPTIONS_DLG_INSTALL_CONFIG_EXECUTE_PARAMETERS));
    for (unsigned i = 0; i < labels.size(); i++)
      if (label_len < labels[i].size())
        label_len = labels[i].size();
    label_len += 2;
    label_text = labels.cbegin();
    append_install_config_ctrl_id = check_box(Far::get_msg(MSG_SFX_OPTIONS_DLG_APPEND_INSTALL_CONFIG), options.append_install_config);
    new_line();
    spacer(2);
    label(*label_text++);
    pad(label_len);
    install_config_title_ctrl_id = edit_box(options.install_config.title, AUTO_SIZE, DIF_SELECTONENTRY);
    new_line();
    spacer(2);
    label(*label_text++);
    pad(label_len);
    install_config_begin_prompt_ctrl_id = edit_box(options.install_config.begin_prompt, AUTO_SIZE, DIF_SELECTONENTRY);
    new_line();
    spacer(2);
    label(*label_text++);
    pad(label_len);
    TriState value;
    if (options.install_config.progress == L"yes")
      value = triTrue;
    else if (options.install_config.progress == L"no")
      value = triFalse;
    else
      value = triUndef;
    install_config_progress_ctrl_id = check_box3(std::wstring(), value);
    new_line();
    spacer(2);
    label(*label_text++);
    pad(label_len);
    install_config_run_program_ctrl_id = edit_box(options.install_config.run_program, AUTO_SIZE, DIF_SELECTONENTRY);
    new_line();
    spacer(2);
    label(*label_text++);
    pad(label_len);
    install_config_directory_ctrl_id = edit_box(options.install_config.directory, AUTO_SIZE, DIF_SELECTONENTRY);
    new_line();
    spacer(2);
    label(*label_text++);
    pad(label_len);
    install_config_execute_file_ctrl_id = edit_box(options.install_config.execute_file, AUTO_SIZE, DIF_SELECTONENTRY);
    new_line();
    spacer(2);
    label(*label_text++);
    pad(label_len);
    install_config_execute_parameters_ctrl_id = edit_box(options.install_config.execute_parameters, AUTO_SIZE, DIF_SELECTONENTRY);
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

bool sfx_options_dialog(SfxOptions& sfx_options, const UpdateProfiles& update_profiles) {
  return SfxOptionsDialog(sfx_options, update_profiles).show();
}
