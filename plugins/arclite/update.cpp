#include "msg.h"
#include "utils.hpp"
#include "sysutils.hpp"
#include "farutils.hpp"
#include "common.hpp"
#include "ui.hpp"
#include "archive.hpp"
#include "options.hpp"

std::wstring format_time(UInt64 t) {
  UInt64 s = t % 60;
  UInt64 m = (t / 60) % 60;
  UInt64 h = t / 60 / 60;
  std::wostringstream st;
  st << std::setfill(L'0') << std::setw(2) << h << L":" << std::setw(2) << m << L":" << std::setw(2) << s;
  return st.str();
}

class ArchiveUpdateProgress: public ProgressMonitor {
private:
  bool new_arc;
  UInt64 total;
  UInt64 completed;
  std::wstring arc_path;
  std::wstring file_path;
  UInt64 file_total;
  UInt64 file_completed;
  UInt64 total_data_read;
  UInt64 total_data_written;

  virtual void do_update_ui() {
    const unsigned c_width = 60;

    percent_done = calc_percent(completed, total);

    UInt64 time = time_elapsed();
    UInt64 speed;
    if (time == 0)
      speed = 0;
    else
      speed = al_round(static_cast<double>(completed) / time * ticks_per_sec());

    UInt64 total_time;
    if (completed)
      total_time = static_cast<UInt64>(static_cast<double>(total) / completed * time);
    else
      total_time = 0;
    if (total_time < time)
      total_time = time;

    std::wostringstream st;
    st << fit_str(arc_path, c_width) << L'\n';
    st << L"\x1\n";
    st << fit_str(file_path, c_width) << L'\n';
    st << std::setw(7) << format_data_size(file_completed, get_size_suffixes()) << L" / " <<
      format_data_size(file_total, get_size_suffixes()) << L'\n';
    st << Far::get_progress_bar_str(c_width, calc_percent(file_completed, file_total), 100) << L'\n';
    st << L"\x1\n";
    st << std::setw(7) << format_data_size(completed, get_size_suffixes()) << L" / " <<
      format_data_size(total, get_size_suffixes()) << L" @ " << std::setw(9) <<
      format_data_size(speed, get_speed_suffixes()) << L" -" << format_time((total_time - time) / ticks_per_sec()) << L'\n';
    st << std::setw(7) << format_data_size(total_data_read, get_size_suffixes()) << L" \x2192 " <<
      std::setw(7) << format_data_size(total_data_written, get_size_suffixes()) << L" = " <<
      std::setw(2) << calc_percent(total_data_written, total_data_read) << L"%" << L'\n';
    st << Far::get_progress_bar_str(c_width, percent_done, 100) << L'\n';
    progress_text = st.str();
  }

public:
  ArchiveUpdateProgress(bool new_arc, const std::wstring& arcpath):
    ProgressMonitor(Far::get_msg(new_arc ? MSG_PROGRESS_CREATE : MSG_PROGRESS_UPDATE)),
    new_arc(new_arc),
    total(0),
    completed(0),
    arc_path(arcpath),
    file_total(0),
    file_completed(0),
    total_data_read(0),
    total_data_written(0) {
  }

  void on_open_file(const std::wstring& file_path, UInt64 size) {
    CriticalSectionLock lock(GetSync());
    this->file_path = file_path;
    file_total = size;
    file_completed = 0;
    update_ui();
  }
  void on_read_file(unsigned size) {
    CriticalSectionLock lock(GetSync());
    file_completed += size;
    total_data_read += size;
    update_ui();
  }
  void on_write_archive(unsigned size) {
    CriticalSectionLock lock(GetSync());
    total_data_written += size;
    update_ui();
  }
  void on_total_update(UInt64 total) {
    CriticalSectionLock lock(GetSync());
    this->total = total;
    update_ui();
  }
  void on_completed_update(UInt64 completed) {
    CriticalSectionLock lock(GetSync());
    this->completed = completed;
    update_ui();
  }

};


DWORD translate_seek_method(UInt32 seek_origin) {
  DWORD method;
  switch (seek_origin) {
  case STREAM_SEEK_SET:
    method = FILE_BEGIN; break;
  case STREAM_SEEK_CUR:
    method = FILE_CURRENT; break;
  case STREAM_SEEK_END:
    method = FILE_END; break;
  default:
    FAIL(E_INVALIDARG);
  }
  return method;
}

class UpdateStream: public IOutStream {
protected:
  std::shared_ptr<ArchiveUpdateProgress> progress;
public:
  UpdateStream(std::shared_ptr<ArchiveUpdateProgress> progress): progress(progress) {
  }
  virtual ~UpdateStream() {
  }
  virtual void clean_files() throw() = 0;
};


class SimpleUpdateStream: public UpdateStream, public ComBase, private File {
public:
  SimpleUpdateStream(const std::wstring& file_path, std::shared_ptr<ArchiveUpdateProgress> progress): UpdateStream(progress) {
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
    progress->on_write_archive(size_written);
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

  virtual void clean_files() throw() {
    close();
    File::delete_file_nt(file_path);
  }
};


void create_sfx_module(const std::wstring& file_path, const SfxOptions& sfx_options);

class SfxUpdateStream: public UpdateStream, public ComBase, private File {
private:
  UInt64 start_offset;

public:
  SfxUpdateStream(const std::wstring& file_path, const SfxOptions& sfx_options, std::shared_ptr<ArchiveUpdateProgress> progress): UpdateStream(progress) {
    RETRY_OR_IGNORE_BEGIN
    try {
      create_sfx_module(file_path, sfx_options);
      open(file_path, FILE_WRITE_DATA, FILE_SHARE_READ, OPEN_EXISTING, 0);
      start_offset = set_pos(0, FILE_END);
    }
    catch (...) {
      File::delete_file_nt(file_path);
      throw;
    }
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
    progress->on_write_archive(size_written);
    if (processedSize)
      *processedSize = size_written;
    return S_OK;
    COM_ERROR_HANDLER_END
  }

  STDMETHODIMP Seek(Int64 offset, UInt32 seekOrigin, UInt64 *newPosition) {
    COM_ERROR_HANDLER_BEGIN
    if (newPosition)
      *newPosition = 0;
    Int64 real_offset = offset;
    if (seekOrigin == STREAM_SEEK_SET)
      real_offset += start_offset;
    UInt64 new_position = set_pos(real_offset, translate_seek_method(seekOrigin));
    if (new_position < start_offset)
      FAIL(E_INVALIDARG);
    new_position -= start_offset;
    if (newPosition)
      *newPosition = new_position;
    return S_OK;
    COM_ERROR_HANDLER_END
  }
  STDMETHODIMP SetSize(UInt64 newSize) {
    COM_ERROR_HANDLER_BEGIN
    RETRY_OR_IGNORE_BEGIN
    set_pos(newSize + start_offset);
    set_end();
    RETRY_END(*progress)
    return S_OK;
    COM_ERROR_HANDLER_END
  }

  virtual void clean_files() throw() {
    close();
    File::delete_file_nt(file_path);
  }
};


class MultiVolumeUpdateStream: public UpdateStream, public ComBase {
private:
  std::wstring file_path;
  UInt64 volume_size;

  UInt64 stream_pos;
  UInt64 seek_stream_pos;
  UInt64 stream_size;
  bool next_volume;
  File volume;

  std::wstring get_volume_path(UInt64 volume_idx) {
    std::wstring volume_ext = uint_to_str(volume_idx + 1);
    if (volume_ext.size() < 3)
      volume_ext.insert(0, 3 - volume_ext.size(), L'0');
    volume_ext.insert(0, 1, L'.');

    size_t pos = file_path.find_last_of(L'.');
    if (pos != std::wstring::npos && pos != 0) {
      std::wstring ext = file_path.substr(pos);
      if (_wcsicmp(ext.c_str(), c_volume_ext) == 0)
        return file_path.substr(0, pos) + volume_ext;
    }
    return file_path + volume_ext;
  }

  UInt64 get_last_volume_idx() {
    return stream_size ? (stream_size - 1) / volume_size : 0;
  }

public:
  MultiVolumeUpdateStream(const std::wstring& file_path, UInt64 volume_size, std::shared_ptr<ArchiveUpdateProgress> progress): UpdateStream(progress), file_path(file_path), volume_size(volume_size), stream_pos(0), seek_stream_pos(0), stream_size(0), next_volume(false) {
    RETRY_OR_IGNORE_BEGIN
    volume.open(get_volume_path(0), GENERIC_WRITE, FILE_SHARE_READ, CREATE_ALWAYS, 0);
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
    if (seek_stream_pos != stream_pos) {
      UInt64 volume_idx = seek_stream_pos / volume_size;
      UInt64 last_volume_idx = get_last_volume_idx();
      while (last_volume_idx + 1 < volume_idx) {
        last_volume_idx += 1;
        RETRY_OR_IGNORE_BEGIN
        volume.open(get_volume_path(last_volume_idx), GENERIC_WRITE, FILE_SHARE_READ, CREATE_ALWAYS, 0);
        volume.set_pos(volume_size);
        volume.set_end();
        RETRY_END(*progress)
      }
      if (last_volume_idx < volume_idx) {
        last_volume_idx += 1;
        assert(last_volume_idx == volume_idx);
        RETRY_OR_IGNORE_BEGIN
        volume.open(get_volume_path(last_volume_idx), GENERIC_WRITE, FILE_SHARE_READ, CREATE_ALWAYS, 0);
        RETRY_END(*progress)
      }
      else {
        RETRY_OR_IGNORE_BEGIN
        volume.open(get_volume_path(volume_idx), GENERIC_WRITE, FILE_SHARE_READ, OPEN_EXISTING, 0);
        RETRY_END(*progress)
      }
      volume.set_pos(seek_stream_pos - volume_idx * volume_size);
      stream_pos = seek_stream_pos;
      next_volume = false;
    }

    unsigned data_off = 0;
    do {
      UInt64 volume_idx = stream_pos / volume_size;

      if (next_volume) { // advance to next volume
        if (volume_idx > get_last_volume_idx()) {
          RETRY_OR_IGNORE_BEGIN
          volume.open(get_volume_path(volume_idx), GENERIC_WRITE, FILE_SHARE_READ, CREATE_ALWAYS, 0);
          RETRY_END(*progress)
        }
        else {
          RETRY_OR_IGNORE_BEGIN
          volume.open(get_volume_path(volume_idx), GENERIC_WRITE, FILE_SHARE_READ, OPEN_EXISTING, 0);
          RETRY_END(*progress)
        }
        next_volume = false;
      }

      UInt64 volume_upper_bound = (volume_idx + 1) * volume_size;
      unsigned write_size;
      if (stream_pos + (size - data_off) >= volume_upper_bound) {
        write_size = static_cast<unsigned>(volume_upper_bound - stream_pos);
        next_volume = true;
      }
      else
        write_size = size - data_off;
      RETRY_OR_IGNORE_BEGIN
      write_size = static_cast<unsigned>(volume.write(reinterpret_cast<const unsigned char*>(data) + data_off, write_size));
      RETRY_END(*progress)
      CHECK(write_size != 0);
      data_off += write_size;
      stream_pos += write_size;
      seek_stream_pos = stream_pos;
      if (stream_size < stream_pos)
        stream_size = stream_pos;
    }
    while (data_off < size);
    progress->on_write_archive(size);
    if (processedSize)
      *processedSize = size;
    return S_OK;
    COM_ERROR_HANDLER_END
  }

  STDMETHODIMP Seek(Int64 offset, UInt32 seekOrigin, UInt64 *newPosition) {
    COM_ERROR_HANDLER_BEGIN
    if (newPosition)
      *newPosition = 0;
    switch (seekOrigin) {
    case STREAM_SEEK_SET:
      seek_stream_pos = offset;
      break;
    case STREAM_SEEK_CUR:
      seek_stream_pos += offset;
      break;
    case STREAM_SEEK_END:
      if (offset < 0 && static_cast<unsigned>(-offset) > stream_size)
        FAIL(E_INVALIDARG);
      seek_stream_pos = stream_size + offset;
      break;
    default:
      FAIL(E_INVALIDARG);
    }
    if (newPosition)
      *newPosition = seek_stream_pos;
    return S_OK;
    COM_ERROR_HANDLER_END
  }

  STDMETHODIMP SetSize(UInt64 newSize) {
    COM_ERROR_HANDLER_BEGIN
    if (stream_size == newSize)
      return S_OK;

    UInt64 last_volume_idx = get_last_volume_idx();
    UInt64 volume_idx = static_cast<unsigned>(newSize / volume_size);
    while (last_volume_idx + 1 < volume_idx) {
      last_volume_idx += 1;
      RETRY_OR_IGNORE_BEGIN
      volume.open(get_volume_path(last_volume_idx), GENERIC_WRITE, FILE_SHARE_READ, CREATE_ALWAYS, 0);
      volume.set_pos(volume_size);
      volume.set_end();
      RETRY_END(*progress)
    }
    RETRY_OR_IGNORE_BEGIN
    if (last_volume_idx < volume_idx) {
      last_volume_idx += 1;
      assert(last_volume_idx == volume_idx);
      volume.open(get_volume_path(last_volume_idx), GENERIC_WRITE, FILE_SHARE_READ, CREATE_ALWAYS, 0);
    }
    else {
      volume.open(get_volume_path(volume_idx), GENERIC_WRITE, FILE_SHARE_READ, OPEN_EXISTING, 0);
    }
    volume.set_pos(newSize - volume_idx * volume_size);
    volume.set_end();
    RETRY_END(*progress)

    for (UInt64 extra_idx = volume_idx + 1; extra_idx <= last_volume_idx; extra_idx++) {
      File::delete_file(get_volume_path(extra_idx));
    }

    stream_size = newSize;

    return S_OK;
    COM_ERROR_HANDLER_END
  }

  virtual void clean_files() noexcept {
    volume.close();
    UInt64 last_volume_idx = get_last_volume_idx();
    for (UInt64 volume_idx = 0; volume_idx <= last_volume_idx; volume_idx++) {
      File::delete_file_nt(get_volume_path(volume_idx));
    }
  }
};


class FileReadStream: public IInStream, public IStreamGetSize, public ComBase, private File {
private:
  std::shared_ptr<ArchiveUpdateProgress> progress;

public:
  FileReadStream(const std::wstring& file_path, bool open_shared, std::shared_ptr<ArchiveUpdateProgress> progress): progress(progress) {
    open(file_path, FILE_READ_DATA, FILE_SHARE_READ | (open_shared ? FILE_SHARE_WRITE | FILE_SHARE_DELETE : 0), OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN);
  }

  UNKNOWN_IMPL_BEGIN
  UNKNOWN_IMPL_ITF(ISequentialInStream)
  UNKNOWN_IMPL_ITF(IInStream)
  UNKNOWN_IMPL_ITF(IStreamGetSize)
  UNKNOWN_IMPL_END

  STDMETHODIMP Read(void *data, UInt32 size, UInt32 *processedSize) {
    COM_ERROR_HANDLER_BEGIN
    if (processedSize)
      *processedSize = 0;
    unsigned size_read = static_cast<unsigned>(read(data, size));
    progress->on_read_file(size_read);
    if (processedSize)
      *processedSize = size_read;
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

  STDMETHODIMP GetSize(UInt64 *pSize) {
    COM_ERROR_HANDLER_BEGIN
    if (!pSize) {
      FAIL(E_INVALIDARG);
    }
    else {
      *pSize = size();
      return S_OK;
    }
    COM_ERROR_HANDLER_END
  }
};


struct FileIndexInfo {
  std::wstring rel_path;
  FindData find_data;
};
typedef std::map<UInt32, FileIndexInfo> FileIndexMap;

class PrepareUpdate: private ProgressMonitor {
private:
  std::wstring src_dir;
  Archive& archive;
  FileIndexMap& file_index_map;
  UInt32& new_index;
  bool& ignore_errors;
  ErrorLog& error_log;
  OverwriteAction overwrite_action;
  Far::FileFilter* filter;
  bool& skipped_files;

  const std::wstring* file_path;

  virtual void do_update_ui() {
    const unsigned c_width = 60;
    std::wostringstream st;
    st << std::left << std::setw(c_width) << fit_str(*file_path, c_width) << L'\n';
    progress_text = st.str();
  }

  void update_progress(const std::wstring& file_path) {
    this->file_path = &file_path;
    update_ui();
  }

  bool process_file(const std::wstring& sub_dir, const FindData& src_find_data, UInt32 dst_dir_index, UInt32& file_index) {
    if (filter) {
      PluginPanelItem filter_data{};
      filter_data.FileAttributes = src_find_data.dwFileAttributes;
      filter_data.CreationTime = src_find_data.ftCreationTime;
      filter_data.LastAccessTime = src_find_data.ftLastAccessTime;
      filter_data.LastWriteTime = src_find_data.ftLastWriteTime;
      filter_data.FileSize = src_find_data.size();
      filter_data.AllocationSize = 0;
      filter_data.FileName = const_cast<wchar_t*>(src_find_data.cFileName);
      if (!filter->match(filter_data))
        return false;
    }

    FileIndexInfo file_index_info;
    file_index_info.rel_path = sub_dir;
    file_index_info.find_data = src_find_data;

    ArcFileInfo file_info;
    file_info.is_dir = src_find_data.is_dir();
    file_info.parent = dst_dir_index;
    file_info.name = src_find_data.cFileName;
    FileIndexRange fi_range = std::equal_range(archive.file_list_index.begin(), archive.file_list_index.end(), -1, [&] (UInt32 left, UInt32 right) -> bool {
      const ArcFileInfo& fi_left = left == (UInt32)-1 ? file_info : archive.file_list[left];
      const ArcFileInfo& fi_right = right == (UInt32)-1 ? file_info : archive.file_list[right];
      return fi_left < fi_right;
    });
    if (fi_range.first == fi_range.second) {
      // new file
      file_index = new_index;
      file_index_map[new_index] = file_index_info;
      new_index++;
    }
    else {
      // updated file
      file_index = *fi_range.first;
      if (file_index >= archive.num_indices) { // fake index
        file_index_map[new_index] = file_index_info;
        new_index++;
      }
      else if (!file_info.is_dir) {
        OverwriteAction overwrite;
        if (overwrite_action == oaAsk) {
          OverwriteFileInfo src_ov_info, dst_ov_info;
          src_ov_info.is_dir = src_find_data.is_dir();
          src_ov_info.size = src_find_data.size();
          src_ov_info.mtime = src_find_data.ftLastWriteTime;
          dst_ov_info.is_dir = file_info.is_dir;
          dst_ov_info.size = archive.get_size(file_index);
          dst_ov_info.mtime = archive.get_mtime(file_index);
          ProgressSuspend ps(*this);
          OverwriteOptions ov_options;
          if (!overwrite_dialog(add_trailing_slash(sub_dir) + file_info.name, src_ov_info, dst_ov_info, odkUpdate, ov_options))
            FAIL(E_ABORT);
          overwrite = ov_options.action;
          if (ov_options.all)
            overwrite_action = ov_options.action;
        }
        else
          overwrite = overwrite_action;
        if (overwrite == oaSkip) {
          skipped_files = true;
          return false;
        }
        file_index_map[file_index] = file_index_info;
      }
    }
    return true;
  }

  bool process_file_enum(FileEnum& file_enum, const std::wstring& sub_dir, UInt32 dst_dir_index) {
    bool not_empty = false;
    while (true) {
      bool more = false;
      RETRY_OR_IGNORE_BEGIN
      more = file_enum.next();
      RETRY_OR_IGNORE_END(ignore_errors, error_log, *this)
      if (error_ignored || !more) break;
      UInt32 saved_new_index = new_index;
      UInt32 file_index;
      if (process_file(sub_dir, file_enum.data(), dst_dir_index, file_index)) {
        if (file_enum.data().is_dir()) {
          std::wstring rel_path = add_trailing_slash(sub_dir) + file_enum.data().cFileName;
          std::wstring full_path = add_trailing_slash(src_dir) + rel_path;
          update_progress(full_path);
          DirList dir_list(full_path);
          if (!process_file_enum(dir_list, rel_path, file_index)) {
            if (filter) {
              file_index_map.erase(file_index);
              new_index = saved_new_index;
            }
          }
          else
            not_empty = true;
        }
        else
          not_empty = true;
      }
    }
    return not_empty;
  }

public:
  PrepareUpdate(
    const std::wstring& src_dir,
    const std::vector<std::wstring>& file_names,
    UInt32 dst_dir_index,
    Archive& archive,
    FileIndexMap& file_index_map,
    UInt32& new_index,
    OverwriteAction overwrite_action,
    bool& ignore_errors,
    ErrorLog& error_log,
    Far::FileFilter* filter,
    bool& skipped_files
  ) : ProgressMonitor(Far::get_msg(MSG_PROGRESS_SCAN_DIRS), false),
    src_dir(src_dir),
    archive(archive),
    file_index_map(file_index_map),
    new_index(new_index),
    ignore_errors(ignore_errors),
    error_log(error_log),
    overwrite_action(overwrite_action),
    filter(filter),
    skipped_files(skipped_files)
  {
    skipped_files = false;
    if (filter)
      filter->start();
    for (unsigned i = 0; i < file_names.size(); i++) {
      std::wstring full_path = add_trailing_slash(src_dir) + file_names[i];
      FileEnum file_enum(full_path);
      process_file_enum(file_enum, extract_file_path(file_names[i]), dst_dir_index);
    }
  }
};

class ArchiveUpdater: public IArchiveUpdateCallback, public ICryptoGetTextPassword2, public ComBase {
private:
  std::wstring src_dir;
  std::wstring dst_dir;
  UInt32 num_indices;
  std::shared_ptr<FileIndexMap> file_index_map;
  const UpdateOptions& options;
  std::shared_ptr<bool> ignore_errors;
  std::shared_ptr<ErrorLog> error_log;
  std::shared_ptr<ArchiveUpdateProgress> progress;

public:
  ArchiveUpdater(const std::wstring& src_dir, const std::wstring& dst_dir, UInt32 num_indices, std::shared_ptr<FileIndexMap> file_index_map, const UpdateOptions& options, std::shared_ptr<bool> ignore_errors, std::shared_ptr<ErrorLog> error_log, std::shared_ptr<ArchiveUpdateProgress> progress): src_dir(src_dir), dst_dir(dst_dir), num_indices(num_indices), file_index_map(file_index_map), options(options), ignore_errors(ignore_errors), error_log(error_log), progress(progress) {
  }

  UNKNOWN_IMPL_BEGIN
  UNKNOWN_IMPL_ITF(IProgress)
  UNKNOWN_IMPL_ITF(IArchiveUpdateCallback)
  UNKNOWN_IMPL_ITF(ICryptoGetTextPassword2)
  UNKNOWN_IMPL_END

  STDMETHODIMP SetTotal(UInt64 total) {
    CriticalSectionLock lock(GetSync());
    COM_ERROR_HANDLER_BEGIN
    progress->on_total_update(total);
    return S_OK;
    COM_ERROR_HANDLER_END
  }
  STDMETHODIMP SetCompleted(const UInt64 *completeValue) {
    CriticalSectionLock lock(GetSync());
    COM_ERROR_HANDLER_BEGIN
    if (completeValue)
      progress->on_completed_update(*completeValue);
    return S_OK;
    COM_ERROR_HANDLER_END
  }

  STDMETHODIMP GetUpdateItemInfo(UInt32 index, Int32 *newData, Int32 *newProperties, UInt32 *indexInArchive) {
    COM_ERROR_HANDLER_BEGIN
    auto found = file_index_map->find(index);
    if (found == file_index_map->end()) {
      *newData = 0;
      *newProperties = 0;
      *indexInArchive = index;
    }
    else {
      *newData = 1;
      *newProperties = 1;
      *indexInArchive = found->first < num_indices ? found->first : -1;
    }
    return S_OK;
    COM_ERROR_HANDLER_END
  }
  STDMETHODIMP GetProperty(UInt32 index, PROPID propID, PROPVARIANT *value) {
    COM_ERROR_HANDLER_BEGIN
    const FileIndexInfo& file_index_info = file_index_map->at(index);
    PropVariant prop;
    switch (propID) {
    case kpidPath:
      prop = add_trailing_slash(add_trailing_slash(dst_dir) + file_index_info.rel_path) + file_index_info.find_data.cFileName; break;
    case kpidName:
      prop = file_index_info.find_data.cFileName; break;
    case kpidIsDir:
      prop = file_index_info.find_data.is_dir(); break;
    case kpidSize:
      prop = file_index_info.find_data.size(); break;
    case kpidAttrib:
      prop = static_cast<UInt32>(file_index_info.find_data.dwFileAttributes); break;
    case kpidCTime:
      prop = file_index_info.find_data.ftCreationTime; break;
    case kpidATime:
      prop = file_index_info.find_data.ftLastAccessTime; break;
    case kpidMTime:
      prop = file_index_info.find_data.ftLastWriteTime; break;
    }
    prop.detach(value);
    return S_OK;
    COM_ERROR_HANDLER_END
  }
  STDMETHODIMP GetStream(UInt32 index, ISequentialInStream **inStream) {
    COM_ERROR_HANDLER_BEGIN
    *inStream = nullptr;
    const FileIndexInfo& file_index_info = file_index_map->at(index);

    if (file_index_info.find_data.is_dir())
      return S_OK;

    std::wstring file_path = add_trailing_slash(add_trailing_slash(src_dir) + file_index_info.rel_path) + file_index_info.find_data.cFileName;
    progress->on_open_file(file_path, file_index_info.find_data.size());

    ComObject<ISequentialInStream> stream;
    RETRY_OR_IGNORE_BEGIN
    stream = new FileReadStream(file_path, options.open_shared, progress);
    RETRY_OR_IGNORE_END(*ignore_errors, *error_log, *progress)
    if (error_ignored)
      return S_FALSE;

    stream.detach(inStream);
    return S_OK;

    COM_ERROR_HANDLER_END
  }
  STDMETHODIMP SetOperationResult(Int32 operationResult) {
    CriticalSectionLock lock(GetSync());
    COM_ERROR_HANDLER_BEGIN
    if (operationResult == NArchive::NUpdate::NOperationResult::kOK)
      return S_OK;
/*
  if (operationResult == NArchive::NUpdate::NOperationResult::kError)
    FAIL_MSG(Far::get_msg(MSG_ERROR_UPDATE_ERROR));
    else
*/
    FAIL_MSG(Far::get_msg(MSG_ERROR_UPDATE_UNKNOWN));
    COM_ERROR_HANDLER_END
  }

  STDMETHODIMP CryptoGetTextPassword2(Int32 *passwordIsDefined, BSTR *password) {
    CriticalSectionLock lock(GetSync());
    COM_ERROR_HANDLER_BEGIN
    *passwordIsDefined = !options.password.empty();
    BStr(options.password).detach(password);
    return S_OK;
    COM_ERROR_HANDLER_END
  }
};


void Archive::set_properties(IOutArchive* out_arc, const UpdateOptions& options) {
  ComObject<ISetProperties> set_props;
  if (SUCCEEDED(out_arc->QueryInterface(IID_ISetProperties, reinterpret_cast<void**>(&set_props)))) {
    static const ExternalCodec defopts { L"", 1,9,0, 1,3,5,7,9, false };

    std::wstring method;
    if (options.arc_type == c_7z)
      method = options.method;
    else if (ArcAPI::formats().count(options.arc_type))
      method = ArcAPI::formats().at(options.arc_type).name;

    auto method_params = &defopts;
    for (size_t i = 0; i < g_options.codecs.size(); ++i) {
      if (method == g_options.codecs[i].name) {
        method_params = &g_options.codecs[i];
        break;
      }
    }

    const auto is_digit = [](const std::wstring& n, const wchar_t up) {
      return n.size() == 1 && n[0] >= L'0' && n[0] <= up;
    };
    const auto variant = [](const std::wstring& v) {
      PropVariant var;
      if (!v.empty()) {
        wchar_t *endptr = nullptr;
        UINT64 v64 = _wcstoui64(v.c_str(), &endptr, 10);
        if (endptr && !*endptr) {
          if (v64 >= 0 && v64 <= UINT_MAX)
            var = static_cast<UInt32>(v64);
          else
            var = v64;
        }
        else
          var = v;
      }
      return var;
    };

    auto adv = options.advanced;
    bool ignore_method = false;
    if (!adv.empty() && adv[0] == L'-') {
      adv.erase(0, 1);
      if (!adv.empty() && adv[0] == L'-') {
        adv.erase(0, 1);
        ignore_method = true;
      }
    }
    else
      adv = method_params->adv + L' ' + adv;

    auto adv_params = split(adv, L' ');
    int adv_level = -1;
    bool adv_have_0 = false, adv_have_1 = false, adv_have_bcj = false, adv_have_qs = false;
    for (auto it = adv_params.begin(); it != adv_params.end(); ) {
      auto param = *it;
      if (param == L"s" || param == L"s1" || param == L"s+") { // s s+ s1
        *it = param = L"s=on";
      }
      else if (param == L"s0" || param == L"s-") {
        *it = param = L"s=off";
      }
      else if (param == L"qs1" || param == L"qs+" || param == L"qs=on") {
        *it = param = L"qs=on"; adv_have_qs = true;
      }
      else if (param == L"qs0" || param == L"qs-" || param == L"qs=off") {
        *it = param = L"qs=off"; adv_have_qs = true;
      }
      else if (param.size() >= 2 && param[0] == L'x' && param[1] >= L'0' && param[1] <= L'9') { // xN
        *it = param.insert(1, 1, L'=');
      }
      else if (param.size() >= 3 && param[0] == L'm' && param[1] == 't' && param[2] >= L'1' && param[2] <= L'9') { // mtN
        *it = param.insert(2, 1, L'='); // mt=N
      }
      else if (param.size() >= 3 && param[0] == L'y' && param[1] == 'x' && param[2] >= L'1' && param[2] <= L'9') { // yxN
        *it = param.insert(2, 1, L'='); // yx=N
      }
      auto sep = param.find(L'=');
      if (param.empty() || sep == 0) {
        it = adv_params.erase(it);
        continue;
      }
      else if (sep != std::wstring::npos) {
        auto name = param.substr(0, sep);
        auto value = param.substr(sep + 1);
        if (0 == _wcsicmp(name.c_str(), L"x")) {
          it = adv_params.erase(it);
          if (!value.empty() && value[0] >= L'0' && value[0] <= L'9')
            adv_level = static_cast<int>(str_to_uint(value));
          continue;
        }
        adv_have_0 = adv_have_0 || name == L"0";
        adv_have_1 = adv_have_0 || name == L"1";
        if (is_digit(name, L'9') && value.size() >= 3)
          adv_have_bcj = adv_have_bcj || 0 == _wcsicmp(value.substr(0,3).c_str(), L"BCJ");
      }
      it++;
    }
    if (!adv_have_qs && g_options.qs_by_default && options.arc_type == c_7z)
      adv_params.emplace_back(L"qs=on");

    auto level = options.level;
    if (adv_level < 0) {
      if (level == 1) level = method_params->L1;
      else if (level == 3) level = method_params->L3;
      else if (level == 5) level = method_params->L5;
      else if (level == 7) level = method_params->L7;
      else if (level == 9) level = method_params->L9;
    }
    else {
      level = adv_level;
      if (method_params->mod0L && level % method_params->mod0L == 0)
        level = 0;
      else if (level && level < method_params->minL)
        level = method_params->minL;
      else if (level > method_params->maxL)
        level = method_params->maxL;
    }

    std::vector<std::wstring> names;
    std::vector<PropVariant> values;
    int n_01 = 0;

    if (options.arc_type == c_7z) {
      if (!ignore_method) {
        names.push_back(L"0"); values.push_back(method);
        ++n_01;
      }
      if (method_params->bcj_only && !adv_have_bcj && !ignore_method) {
        names.push_back(L"1"); values.push_back(L"BCJ");
        ++n_01;
      }
      names.push_back(L"x"); values.push_back(level);
      if (level != 0) {
        names.push_back(L"s"); values.push_back(options.solid);
      }
      if (options.encrypt) {
        if (options.encrypt_header != triUndef) {
          names.push_back(L"he"); values.push_back(options.encrypt_header == triTrue);
        }
      }
    }
    else if (options.arc_type != c_bzip2 || level != 0) {
      names.push_back(L"x"); values.push_back(level);
    }

    int n_shift = (adv_have_0 || adv_have_1) ? n_01 : 0;
    for (const auto& param : adv_params) {
      auto sep = param.find(L'=');
      std::wstring name = sep != std::wstring::npos ? param.substr(0, sep) : param;
      std::wstring value = sep != std::wstring::npos ? param.substr(sep + 1) : std::wstring();
      if (n_shift && is_digit(name, L'7'))
        name[0] = static_cast<wchar_t>(name[0] + n_shift);
      bool found = false;
      unsigned i = 0;
      for (const auto& n : names) {
        std::wstring v = values[i].is_str() ? values[i].get_str() : std::wstring();
        if (0 == _wcsicmp(n.c_str(), name.c_str()) || ((int)i < n_01 && is_digit(n,L'9') && is_digit(name,L'9') && !v.empty() && substr_match(upcase(value), 0, upcase(v).c_str()))) {
          found = true;
          values[i] = variant(value);
          break;
        }
        ++i;
      }
      if (!found) {
        names.push_back(name);
        values.push_back(variant(value));
      }
    }

    // normalize {N}=... parameter names (start from '0', no gaps): {'1' '5' '3'} -> {'0' '2' '1'}
    //
    int gaps[10] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
    for (const auto& n : names)
      if (is_digit(n,L'9'))
        gaps[n[0] - L'0'] = 0;
    int gap = 0;
    for (int i = 0; i < 10; ++i) {
      auto t = gaps[i]; gaps[i] = gap; gap += t;
    }
    for (size_t i = 0; i < names.size(); ++i)
      if (is_digit(names[i],L'9'))
        names[i][0] = static_cast<wchar_t>(names[i][0] - gaps[names[i][0]-L'0']);

    std::vector<const wchar_t*> name_ptrs;
    name_ptrs.reserve(names.size());
    for (unsigned i = 0; i < names.size(); i++) {
      name_ptrs.push_back(names[i].c_str());
    }

    CHECK_COM(set_props->SetProperties(name_ptrs.data(), values.data(), static_cast<UInt32>(values.size())));
  }
}

class DeleteSrcFiles: public ProgressMonitor {
private:
  bool& ignore_errors;
  ErrorLog& error_log;

  const std::wstring* file_path;

  virtual void do_update_ui() {
    const unsigned c_width = 60;
    std::wostringstream st;
    st << std::left << std::setw(c_width) << fit_str(*file_path, c_width) << L'\n';
    progress_text = st.str();
  }

  void update_progress(const std::wstring& file_path) {
    this->file_path = &file_path;
    update_ui();
  }

  void delete_src_file(const std::wstring& file_path) {
    update_progress(file_path);
    RETRY_OR_IGNORE_BEGIN
    try {
      File::delete_file(file_path);
    }
    catch (const Error& e) {
      if (e.code != HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED))
        throw;
      File::set_attr_nt(file_path, FILE_ATTRIBUTE_NORMAL);
      File::delete_file(file_path);
    }
    RETRY_OR_IGNORE_END(ignore_errors, error_log, *this)
  }

  void delete_src_dir(const std::wstring& dir_path) {
    {
      DirList dir_list(dir_path);
      while (dir_list.next()) {
        std::wstring path = add_trailing_slash(dir_path) + dir_list.data().cFileName;
        update_progress(path);
        if (dir_list.data().is_dir())
          delete_src_dir(path);
        else
          delete_src_file(path);
      }
    }

    RETRY_OR_IGNORE_BEGIN
    try {
      File::remove_dir(dir_path);
    }
    catch (const Error& e) {
      if (e.code != HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED))
        throw;
      File::set_attr_nt(dir_path, FILE_ATTRIBUTE_NORMAL);
      File::remove_dir(dir_path);
    }
    RETRY_OR_IGNORE_END(ignore_errors, error_log, *this)
  }

public:
  DeleteSrcFiles(const std::wstring& src_dir, const std::vector<std::wstring>& file_names, bool& ignore_errors, ErrorLog& error_log): ProgressMonitor(Far::get_msg(MSG_PROGRESS_DELETE_FILES), false), ignore_errors(ignore_errors), error_log(error_log) {
    for (unsigned i = 0; i < file_names.size(); i++) {
      std::wstring file_path = add_trailing_slash(src_dir) + file_names[i];
      FindData find_data = File::get_find_data(file_path);
      if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        delete_src_dir(file_path);
      else
        delete_src_file(file_path);
    }
  }
};


void Archive::create(const std::wstring& src_dir, const std::vector<std::wstring>& file_names, const UpdateOptions& options, std::shared_ptr<ErrorLog> error_log) {
  DisableSleepMode dsm;

  std::shared_ptr<bool> ignore_errors(new bool(options.ignore_errors));
  UInt32 new_index = 0;
  bool skipped_files = false;

  std::shared_ptr<FileIndexMap> file_index_map(new FileIndexMap());
  PrepareUpdate(src_dir, file_names, c_root_index, *this, *file_index_map, new_index, oaOverwrite, *ignore_errors, *error_log, options.filter.get(), skipped_files);

  ComObject<IOutArchive> out_arc;
  ArcAPI::create_out_archive(options.arc_type, out_arc.ref());

  set_properties(out_arc, options);

  std::shared_ptr<ArchiveUpdateProgress> progress(new ArchiveUpdateProgress(true, options.arc_path));
  ComObject<IArchiveUpdateCallback> updater(new ArchiveUpdater(src_dir, std::wstring(), 0, file_index_map, options, ignore_errors, error_log, progress));

  prepare_dst_dir(extract_file_path(options.arc_path));
  UpdateStream* stream_impl;
  if (options.enable_volumes)
    stream_impl = new MultiVolumeUpdateStream(options.arc_path, parse_size_string(options.volume_size), progress);
  else if (options.create_sfx && options.arc_type == c_7z)
    stream_impl = new SfxUpdateStream(options.arc_path, options.sfx_options, progress);
  else
    stream_impl = new SimpleUpdateStream(options.arc_path, progress);
  ComObject<IOutStream> update_stream(stream_impl);

  try {
    COM_ERROR_CHECK(out_arc->UpdateItems(update_stream, new_index, updater));
  }
  catch (...) {
    stream_impl->clean_files();
    throw;
  }

  if (options.move_files && error_log->empty() && !options.filter && !skipped_files)
    DeleteSrcFiles(src_dir, file_names, *ignore_errors, *error_log);
}

void Archive::update(const std::wstring& src_dir, const std::vector<std::wstring>& file_names, const std::wstring& dst_dir, const UpdateOptions& options, std::shared_ptr<ErrorLog> error_log) {
  DisableSleepMode dsm;

  std::shared_ptr<bool> ignore_errors(new bool(options.ignore_errors));
  UInt32 new_index = num_indices; // starting index for new files
  bool skipped_files = false;

  std::shared_ptr<FileIndexMap> file_index_map(new FileIndexMap());
  PrepareUpdate(src_dir, file_names, find_dir(dst_dir), *this, *file_index_map, new_index, options.overwrite, *ignore_errors, *error_log, options.filter.get(), skipped_files);

  std::wstring temp_arc_name = get_temp_file_name();
  try {
    ComObject<IOutArchive> out_arc;
    CHECK_COM(in_arc->QueryInterface(IID_IOutArchive, reinterpret_cast<void**>(&out_arc)));

    set_properties(out_arc, options);

    std::shared_ptr<ArchiveUpdateProgress> progress(new ArchiveUpdateProgress(false, arc_path));
    ComObject<IArchiveUpdateCallback> updater(new ArchiveUpdater(src_dir, dst_dir, num_indices, file_index_map, options, ignore_errors, error_log, progress));
    ComObject<IOutStream> update_stream(new SimpleUpdateStream(temp_arc_name, progress));

    COM_ERROR_CHECK(copy_prologue(update_stream));

    COM_ERROR_CHECK(out_arc->UpdateItems(update_stream, new_index, updater));
    close();
    update_stream.Release();
    File::move_file(temp_arc_name, arc_path, MOVEFILE_REPLACE_EXISTING);
  }
  catch (...) {
    File::delete_file_nt(temp_arc_name);
    throw;
  }

  reopen();

  if (options.move_files && error_log->empty() && !options.filter && !skipped_files)
    DeleteSrcFiles(src_dir, file_names, *ignore_errors, *error_log);
}

void Archive::create_dir(const std::wstring& dir_name, const std::wstring& dst_dir) {
  DisableSleepMode dsm;

  std::shared_ptr<FileIndexMap> file_index_map(new FileIndexMap());
  FileIndexInfo file_index_info{};
  file_index_info.find_data.dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
  SYSTEMTIME sys_time;
  GetSystemTime(&sys_time);
  FILETIME file_time;
  SystemTimeToFileTime(&sys_time, &file_time);
  file_index_info.find_data.ftCreationTime = file_time;
  file_index_info.find_data.ftLastAccessTime = file_time;
  file_index_info.find_data.ftLastWriteTime = file_time;
  std::wcscpy(file_index_info.find_data.cFileName, dir_name.c_str());
  (*file_index_map)[num_indices] = file_index_info;

  UpdateOptions options;
  options.arc_type = arc_chain.back().type;
  load_update_props();
  options.level = level;
  options.method = method;
  options.solid = solid;
  options.encrypt = encrypted;
  options.password = password;
  options.overwrite = oaOverwrite;

  std::wstring temp_arc_name = get_temp_file_name();
  try {
    ComObject<IOutArchive> out_arc;
    CHECK_COM(in_arc->QueryInterface(IID_IOutArchive, reinterpret_cast<void**>(&out_arc)));

    std::shared_ptr<ErrorLog> error_log(new ErrorLog());
    std::shared_ptr<bool> ignore_errors(new bool(options.ignore_errors));

    std::shared_ptr<ArchiveUpdateProgress> progress(new ArchiveUpdateProgress(false, arc_path));
    ComObject<IArchiveUpdateCallback> updater(new ArchiveUpdater(std::wstring(), dst_dir, num_indices, file_index_map, options, ignore_errors, error_log, progress));
    ComObject<IOutStream> update_stream(new SimpleUpdateStream(temp_arc_name, progress));

    COM_ERROR_CHECK(out_arc->UpdateItems(update_stream, num_indices + 1, updater));
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
