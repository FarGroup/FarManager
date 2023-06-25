#include "msg.hpp"
#include "utils.hpp"
#include "sysutils.hpp"
#include "farutils.hpp"
#include "common.hpp"
#include "ui.hpp"
#include "archive.hpp"

class ArchiveTester: public IArchiveExtractCallback, public ICryptoGetTextPassword, public ComBase, public ProgressMonitor {
private:
  UInt32 src_dir_index;
  std::shared_ptr<Archive> archive;

  std::wstring file_path;
  UInt64 completed;
  UInt64 total;
  void do_update_ui() override {
    const unsigned c_width = 60;

    percent_done = calc_percent(completed, total);

    UInt64 speed;
    if (time_elapsed() == 0)
      speed = 0;
    else
      speed = al_round(static_cast<double>(completed) / static_cast<double>(time_elapsed()) * static_cast<double>(ticks_per_sec()));

    std::wostringstream st;
    st << fit_str(file_path, c_width) << L'\n';
    st << std::setw(7) << format_data_size(completed, get_size_suffixes()) << L" / " << format_data_size(total, get_size_suffixes()) << L" @ " << std::setw(9) << format_data_size(speed, get_speed_suffixes()) << L'\n';
    st << Far::get_progress_bar_str(c_width, percent_done, 100) << L'\n';
    progress_text = st.str();
  }

public:
  ArchiveTester(UInt32 src_dir_index, std::shared_ptr<Archive> archive): ProgressMonitor(Far::get_msg(MSG_PROGRESS_TEST)), src_dir_index(src_dir_index), archive(archive), completed(0), total(0) {
  }

  UNKNOWN_IMPL_BEGIN
  UNKNOWN_IMPL_ITF(IProgress)
  UNKNOWN_IMPL_ITF(IArchiveExtractCallback)
  UNKNOWN_IMPL_ITF(ICryptoGetTextPassword)
  UNKNOWN_IMPL_END

  STDMETHODIMP SetTotal(UInt64 total_value) noexcept override {
    COM_ERROR_HANDLER_BEGIN
    total = total_value;
    update_ui();
    return S_OK;
    COM_ERROR_HANDLER_END
  }
  STDMETHODIMP SetCompleted(const UInt64 *completeValue) noexcept override {
    COM_ERROR_HANDLER_BEGIN
    if (completeValue) {
      completed = *completeValue;
      update_ui();
    }
    return S_OK;
    COM_ERROR_HANDLER_END
  }

  STDMETHODIMP GetStream(UInt32 index, ISequentialOutStream **outStream,  Int32 askExtractMode) noexcept override {
    COM_ERROR_HANDLER_BEGIN
    const ArcFileInfo& file_info = archive->file_list[index];
    file_path = file_info.name;
    UInt32 parent_index = file_info.parent;
    while (parent_index != src_dir_index) {
      const ArcFileInfo& parent_file_info = archive->file_list[parent_index];
      file_path.insert(0, 1, L'\\').insert(0, parent_file_info.name);
      parent_index = parent_file_info.parent;
    }
    update_ui();
    *outStream = nullptr;
    return S_OK;
    COM_ERROR_HANDLER_END
  }
  STDMETHODIMP PrepareOperation(Int32 askExtractMode) noexcept override {
    COM_ERROR_HANDLER_BEGIN
    return S_OK;
    COM_ERROR_HANDLER_END
  }
  STDMETHODIMP SetOperationResult(Int32 resultEOperationResult) noexcept override {
    COM_ERROR_HANDLER_BEGIN
    if (resultEOperationResult == NArchive::NExtract::NOperationResult::kOK)
      return S_OK;
    bool encrypted = !archive->m_password.empty();
    Error error;
    error.code = E_MESSAGE;
    if (resultEOperationResult == NArchive::NExtract::NOperationResult::kUnsupportedMethod)
      error.messages.emplace_back(Far::get_msg(MSG_ERROR_EXTRACT_UNSUPPORTED_METHOD));
    else if (resultEOperationResult == NArchive::NExtract::NOperationResult::kDataError)
      error.messages.emplace_back(Far::get_msg(encrypted ? MSG_ERROR_EXTRACT_DATA_ERROR_ENCRYPTED : MSG_ERROR_EXTRACT_DATA_ERROR));
    else if (resultEOperationResult == NArchive::NExtract::NOperationResult::kCRCError)
      error.messages.emplace_back(Far::get_msg(encrypted ? MSG_ERROR_EXTRACT_CRC_ERROR_ENCRYPTED : MSG_ERROR_EXTRACT_CRC_ERROR));
    else
      error.messages.emplace_back(Far::get_msg(MSG_ERROR_EXTRACT_UNKNOWN));
    error.messages.emplace_back(file_path);
    error.messages.emplace_back(archive->arc_path);
    throw error;
    COM_ERROR_HANDLER_END
  }

  STDMETHODIMP CryptoGetTextPassword(BSTR *password) noexcept override {
    COM_ERROR_HANDLER_BEGIN
    if (archive->m_password.empty()) {
      ProgressSuspend ps(*this);
      if (!password_dialog(archive->m_password, archive->arc_path))
        FAIL(E_ABORT);
    }
    BStr(archive->m_password).detach(password);
    return S_OK;
    COM_ERROR_HANDLER_END
  }
};

void Archive::prepare_test(UInt32 file_index, std::list<UInt32>& indices) {
  const ArcFileInfo& file_info = file_list[file_index];
  if (file_info.is_dir) {
    FileIndexRange dir_list = get_dir_list(file_index);
    std::for_each(dir_list.first, dir_list.second, [&] (UInt32 item) {
      prepare_test(item, indices);
    });
  }
  else {
    indices.push_back(file_index);
  }
}

void Archive::test(UInt32 src_dir_index, const std::vector<UInt32>& src_indices) {
  DisableSleepMode dsm;

  std::list<UInt32> file_indices;
  for (unsigned i = 0; i < src_indices.size(); i++) {
    prepare_test(src_indices[i], file_indices);
  }

  std::vector<UInt32> indices;
  indices.reserve(file_indices.size());
  std::copy(file_indices.begin(), file_indices.end(), std::back_insert_iterator<std::vector<UInt32>>(indices));
  std::sort(indices.begin(), indices.end());

  ComObject<IArchiveExtractCallback> tester(new ArchiveTester(src_dir_index, shared_from_this()));
  COM_ERROR_CHECK(in_arc->Extract(indices.data(), static_cast<UInt32>(indices.size()), 1, tester));
}
