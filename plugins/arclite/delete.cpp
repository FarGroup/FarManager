#include "msg.h"
#include "utils.hpp"
#include "sysutils.hpp"
#include "farutils.hpp"
#include "common.hpp"
#include "ui.hpp"
#include "archive.hpp"

class ArchiveFileDeleterProgress: public ProgressMonitor {
private:
  UInt64 total;
  UInt64 completed;

  virtual void do_update_ui() {
    const unsigned c_width = 60;

    if (total == 0)
      percent_done = 0;
    else
      percent_done = al_round(static_cast<double>(completed) * 100 / total);
    if (percent_done > 100)
      percent_done = 100;

    UInt64 speed;
    if (time_elapsed() == 0)
      speed = 0;
    else
      speed = al_round(static_cast<double>(completed) / time_elapsed() * ticks_per_sec());

    std::wostringstream st;
    st << std::setw(7) << format_data_size(completed, get_size_suffixes()) << L" / " << format_data_size(total, get_size_suffixes()) << L" @ " << std::setw(9) << format_data_size(speed, get_speed_suffixes()) << L'\n';
    st << Far::get_progress_bar_str(c_width, percent_done, 100) << L'\n';
    progress_text = st.str();
  }

public:
  ArchiveFileDeleterProgress(): ProgressMonitor(Far::get_msg(MSG_PROGRESS_UPDATE)), total(0), completed(0) {
  }

  void update_total(UInt64 total) {
    this->total = total;
    update_ui();
  }

  void update_completed(UInt64 completed) {
    this->completed = completed;
    update_ui();
  }
};

class ArchiveFileDeleterStream: public IOutStream, public ComBase, private File {
private:
  std::shared_ptr<ProgressMonitor> progress;
public:
  ArchiveFileDeleterStream(const std::wstring& file_path, std::shared_ptr<ProgressMonitor> progress): progress(progress) {
    RETRY_OR_IGNORE_BEGIN
    open(file_path, GENERIC_WRITE, FILE_SHARE_READ, CREATE_ALWAYS, 0);
    RETRY_END(*progress)
  }

  UNKNOWN_IMPL_BEGIN
  UNKNOWN_IMPL_ITF(ISequentialOutStream)
  UNKNOWN_IMPL_ITF(IOutStream)
  UNKNOWN_IMPL_END

  STDMETHODIMP Write(const void *data, UInt32 size, UInt32 *processedSize) {
    COM_ERROR_HANDLER_BEGIN
    if (processedSize)
      *processedSize = 0;
    unsigned size_written;
    RETRY_OR_IGNORE_BEGIN
    size_written = static_cast<unsigned>(write(data, size));
    RETRY_END(*progress)
    if (processedSize)
      *processedSize = size_written;
    return S_OK;
    COM_ERROR_HANDLER_END
  }

  STDMETHODIMP Seek(Int64 offset, UInt32 seekOrigin, UInt64 *newPosition) {
    COM_ERROR_HANDLER_BEGIN
    if (newPosition)
      *newPosition = 0;
    UInt64 new_position = set_pos(offset, translate_seek_method(seekOrigin));
    if (newPosition)
      *newPosition = new_position;
    return S_OK;
    COM_ERROR_HANDLER_END
  }
  STDMETHODIMP SetSize(UInt64 newSize) {
    COM_ERROR_HANDLER_BEGIN
    RETRY_OR_IGNORE_BEGIN
    set_pos(newSize, FILE_BEGIN);
    set_end();
    RETRY_END(*progress)
    return S_OK;
    COM_ERROR_HANDLER_END
  }
};

class ArchiveFileDeleter: public IArchiveUpdateCallback, public ComBase {
private:
  std::vector<UInt32> new_indices;
  std::shared_ptr<ArchiveFileDeleterProgress> progress;

public:
  ArchiveFileDeleter(const std::vector<UInt32>& new_indices, std::shared_ptr<ArchiveFileDeleterProgress> progress): new_indices(new_indices), progress(progress) {
  }

  UNKNOWN_IMPL_BEGIN
  UNKNOWN_IMPL_ITF(IProgress)
  UNKNOWN_IMPL_ITF(IArchiveUpdateCallback)
  UNKNOWN_IMPL_END

  STDMETHODIMP SetTotal(UInt64 total) {
    COM_ERROR_HANDLER_BEGIN
    progress->update_total(total);
    return S_OK;
    COM_ERROR_HANDLER_END
  }
  STDMETHODIMP SetCompleted(const UInt64 *completeValue) {
    COM_ERROR_HANDLER_BEGIN
    if (completeValue)
      progress->update_completed(*completeValue);
    return S_OK;
    COM_ERROR_HANDLER_END
  }

  STDMETHODIMP GetUpdateItemInfo(UInt32 index, Int32 *newData, Int32 *newProperties, UInt32 *indexInArchive) {
    COM_ERROR_HANDLER_BEGIN
    *newData = 0;
    *newProperties = 0;
    *indexInArchive = new_indices[index];
    return S_OK;
    COM_ERROR_HANDLER_END
  }
  STDMETHODIMP GetProperty(UInt32 index, PROPID propID, PROPVARIANT *value) {
    COM_ERROR_HANDLER_BEGIN
    PropVariant prop;
    prop.detach(value);
    return S_OK;
    COM_ERROR_HANDLER_END
  }
  STDMETHODIMP GetStream(UInt32 index, ISequentialInStream **inStream) {
    COM_ERROR_HANDLER_BEGIN
    *inStream = nullptr;
    return S_OK;
    COM_ERROR_HANDLER_END
  }
  STDMETHODIMP SetOperationResult(Int32 operationResult) {
    COM_ERROR_HANDLER_BEGIN
    return S_OK;
    COM_ERROR_HANDLER_END
  }
};


void Archive::enum_deleted_indices(UInt32 file_index, std::vector<UInt32>& indices) {
  const ArcFileInfo& file_info = file_list[file_index];
  indices.push_back(file_index);
  if (file_info.is_dir) {
    FileIndexRange dir_list = get_dir_list(file_index);
    std::for_each(dir_list.first, dir_list.second, [&] (UInt32 file_index) {
      enum_deleted_indices(file_index, indices);
    });
  }
}

void Archive::delete_files(const std::vector<UInt32>& src_indices) {
  std::vector<UInt32> deleted_indices;
  deleted_indices.reserve(file_list.size());
  std::for_each(src_indices.begin(), src_indices.end(), [&] (UInt32 src_index) {
    enum_deleted_indices(src_index, deleted_indices);
  });
  std::sort(deleted_indices.begin(), deleted_indices.end());

  std::vector<UInt32> file_indices;
  file_indices.reserve(num_indices);
  for(UInt32 i = 0; i < num_indices; i++)
    file_indices.push_back(i);

  std::vector<UInt32> new_indices;
  new_indices.reserve(num_indices);
  std::set_difference(file_indices.begin(), file_indices.end(), deleted_indices.begin(), deleted_indices.end(), back_inserter(new_indices));

  std::wstring temp_arc_name = get_temp_file_name();
  try {
    ComObject<IOutArchive> out_arc;
    CHECK_COM(in_arc->QueryInterface(IID_IOutArchive, reinterpret_cast<void**>(&out_arc)));

    std::shared_ptr<ArchiveFileDeleterProgress> progress(new ArchiveFileDeleterProgress());
    ComObject<IArchiveUpdateCallback> deleter(new ArchiveFileDeleter(new_indices, progress));
    ComObject<IOutStream> update_stream(new ArchiveFileDeleterStream(temp_arc_name, progress));

    COM_ERROR_CHECK(copy_prologue(update_stream));

    COM_ERROR_CHECK(out_arc->UpdateItems(update_stream, static_cast<UInt32>(new_indices.size()), deleter));
    close();
    update_stream.Release();
    File::move_file(temp_arc_name, arc_path, MOVEFILE_REPLACE_EXISTING);
  }
  catch (...) {
    File::delete_file_nt(temp_arc_name);
    throw;
  }

  reopen();
}
