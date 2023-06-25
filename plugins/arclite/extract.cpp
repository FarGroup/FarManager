#include "msg.hpp"
#include "utils.hpp"
#include "sysutils.hpp"
#include "farutils.hpp"
#include "common.hpp"
#include "ui.hpp"
#include "archive.hpp"
#include "options.hpp"

ExtractOptions::ExtractOptions():
  ignore_errors(false),
  overwrite(oaOverwrite),
  move_files(triUndef),
  separate_dir(triFalse),
  delete_archive(false),
  disable_delete_archive(false),
  open_dir(triFalse) {
}

static std::wstring get_progress_bar_str(unsigned width, unsigned percent1, unsigned percent2) {
  const wchar_t c_pb_black = 9608;
  const wchar_t c_pb_gray = 9619;
  const wchar_t c_pb_white = 9617;
  unsigned len1 = al_round(static_cast<double>(percent1) / 100 * width);
  if (len1 > width)
    len1 = width;
  unsigned len2 = al_round(static_cast<double>(percent2) / 100 * width);
  if (len2 > width)
    len2 = width;
  if (len2 > len1)
    len2 -= len1;
  else
    len2 = 0;
  unsigned len3 = width - (len1 + len2);
  std::wstring result;
  result.append(len1, c_pb_black);
  result.append(len2, c_pb_gray);
  result.append(len3, c_pb_white);
  return result;
}

class ExtractProgress: public ProgressMonitor {
private:
  std::wstring arc_path;
  UInt64 extract_completed;
  UInt64 extract_total;
  std::wstring extract_file_path;
  UInt64 cache_stored;
  UInt64 cache_written;
  UInt64 cache_total;
  std::wstring cache_file_path;

  void do_update_ui() override {
    const unsigned c_width = 60;

    percent_done = calc_percent(extract_completed, extract_total);

    UInt64 extract_speed;
    if (time_elapsed() == 0)
      extract_speed = 0;
    else
      extract_speed = al_round(static_cast<double>(extract_completed) / static_cast<double>(time_elapsed()) * static_cast<double>(ticks_per_sec()));

    if (extract_total && cache_total > extract_total)
      cache_total = extract_total;

    unsigned cache_stored_percent = calc_percent(cache_stored, cache_total);
    unsigned cache_written_percent = calc_percent(cache_written, cache_total);

    std::wostringstream st;
    st << fit_str(arc_path, c_width) << L'\n';
    st << L"\x1\n";
    st << fit_str(extract_file_path, c_width) << L'\n';
    st << std::setw(7) << format_data_size(extract_completed, get_size_suffixes()) << L" / " << format_data_size(extract_total, get_size_suffixes()) << L" @ " << std::setw(9) << format_data_size(extract_speed, get_speed_suffixes()) << L'\n';
    st << Far::get_progress_bar_str(c_width, percent_done, 100) << L'\n';
    st << L"\x1\n";
    st << fit_str(cache_file_path, c_width) << L'\n';
    st << L"(" << format_data_size(cache_stored, get_size_suffixes()) << L" - " << format_data_size(cache_written, get_size_suffixes()) << L") / " << format_data_size(cache_total, get_size_suffixes()) << L'\n';
    st << get_progress_bar_str(c_width, cache_written_percent, cache_stored_percent) << L'\n';
    progress_text = st.str();
  }

public:
  ExtractProgress(const std::wstring& arc_path):
    ProgressMonitor(Far::get_msg(MSG_PROGRESS_EXTRACT)),
    arc_path(arc_path),
    extract_completed(0),
    extract_total(0),
    cache_stored(0),
    cache_written(0),
    cache_total(0) {
  }

  void update_extract_file(const std::wstring& file_path) {
    CriticalSectionLock lock(GetSync());
    extract_file_path = file_path;
    update_ui();
  }
  void set_extract_total(UInt64 size) {
    CriticalSectionLock lock(GetSync());
    extract_total = size;
  }
  void update_extract_completed(UInt64 size) {
    CriticalSectionLock lock(GetSync());
    extract_completed = size;
    update_ui();
  }
  void update_cache_file(const std::wstring& file_path) {
    CriticalSectionLock lock(GetSync());
    cache_file_path = file_path;
    update_ui();
  }
  void set_cache_total(UInt64 size) {
    CriticalSectionLock lock(GetSync());
    cache_total = size;
  }
  void update_cache_stored(UInt64 size) {
    CriticalSectionLock lock(GetSync());
    cache_stored += size;
    update_ui();
  }
  void update_cache_written(UInt64 size) {
    CriticalSectionLock lock(GetSync());
    cache_written += size;
    update_ui();
  }
  void reset_cache_stats() {
    cache_stored = 0;
    cache_written = 0;
  }
};


class FileWriteCache {
private:
  static const size_t c_min_cache_size = 10 * 1024 * 1024;
  static const size_t c_max_cache_size = 100 * 1024 * 1024;

  struct CacheRecord {
    std::wstring file_path;
    UInt32 file_id{};
    OverwriteAction overwrite{ OverwriteAction::oaAsk };
    size_t buffer_pos{};
    size_t buffer_size{};
  };

  std::shared_ptr<Archive> archive;
  unsigned char* buffer;
  size_t buffer_size;
  size_t commit_size{};
  size_t buffer_pos{};
  std::list<CacheRecord> cache_records;
  File file;
  CacheRecord current_rec;
  bool continue_file{};
  bool error_state{};
  std::shared_ptr<bool> ignore_errors;
  std::shared_ptr<ErrorLog> error_log;
  std::shared_ptr<ExtractProgress> progress;

  size_t get_max_cache_size() const {
    MEMORYSTATUSEX mem_st{sizeof(mem_st)};
    GlobalMemoryStatusEx(&mem_st);
    auto size = static_cast<size_t>(mem_st.ullAvailPhys);
    if (size < c_min_cache_size)
      size = c_min_cache_size;
    if (size > c_max_cache_size)
      size = c_max_cache_size;
    return size;
  }
  // create new file
  void create_file() {
    std::wstring file_path;
    if (current_rec.overwrite == oaRename)
      file_path = auto_rename(current_rec.file_path);
    else
      file_path = current_rec.file_path;
    if (current_rec.overwrite == oaOverwrite || current_rec.overwrite == oaOverwriteCase || current_rec.overwrite == oaAppend)
      File::set_attr_nt(file_path, FILE_ATTRIBUTE_NORMAL);
    RETRY_OR_IGNORE_BEGIN
    const DWORD access = FILE_WRITE_DATA | FILE_WRITE_ATTRIBUTES;
    const DWORD shares = FILE_SHARE_READ;
    const DWORD attrib = FILE_ATTRIBUTE_TEMPORARY;
    if (current_rec.overwrite == oaAppend) {
      file.open(file_path, access, shares, OPEN_EXISTING, attrib);
    } else {
      bool opened = current_rec.overwrite != oaOverwriteCase && file.open_nt(file_path, access, shares, CREATE_ALWAYS, attrib);
      if (!opened) {
        File::delete_file_nt(file_path); // sometimes can help
        file.open(file_path, access, shares, CREATE_ALWAYS, attrib);
      }
    }
    RETRY_OR_IGNORE_END(*ignore_errors, *error_log, *progress)
    if (error_ignored) error_state = true;
    progress->update_cache_file(current_rec.file_path);
  }
  // allocate file
  void allocate_file() {
    if (error_state) return;
    if (archive->get_size(current_rec.file_id) == 0) return;
    UInt64 size;
    if (current_rec.overwrite == oaAppend)
      size = file.size();
    else
      size = 0;
    RETRY_OR_IGNORE_BEGIN
    file.set_pos(size + archive->get_size(current_rec.file_id), FILE_BEGIN);
    file.set_end();
    file.set_pos(size, FILE_BEGIN);
    RETRY_OR_IGNORE_END(*ignore_errors, *error_log, *progress)
    if (error_ignored) error_state = true;
  }
  // write file using 1 MB blocks
  void write_file() {
    if (error_state) return;
    const unsigned c_block_size = 1 * 1024 * 1024;
    size_t pos = 0;
    while (pos < current_rec.buffer_size) {
      DWORD size;
      if (pos + c_block_size > current_rec.buffer_size)
        size = static_cast<DWORD>(current_rec.buffer_size - pos);
      else
        size = c_block_size;
      size_t size_written = 0;
      RETRY_OR_IGNORE_BEGIN
      size_written = file.write(buffer + current_rec.buffer_pos + pos, size);
      RETRY_OR_IGNORE_END(*ignore_errors, *error_log, *progress)
      if (error_ignored) {
        error_state = true;
        return;
      }
      pos += size_written;
      progress->update_cache_written(size_written);
    }
  }
  void close_file() {
    if (file.is_open()) {
      if (!error_state) {
        RETRY_OR_IGNORE_BEGIN
        file.set_end(); // ensure end of file is set correctly
        File::set_attr_nt(current_rec.file_path, archive->get_attr(current_rec.file_id));
        file.set_time_nt(archive->get_ctime(current_rec.file_id), archive->get_atime(current_rec.file_id), archive->get_mtime(current_rec.file_id));
        IGNORE_END(*ignore_errors, *error_log, *progress)
        if (error_ignored) error_state = true;
      }
      file.close();
      if (error_state)
        File::delete_file_nt(current_rec.file_path);
    }
  }
  void write() {
    std::for_each(cache_records.begin(), cache_records.end(), [&] (const CacheRecord& rec) {
      if (continue_file) {
        continue_file = false;
        current_rec = rec;
      }
      else {
        close_file();
        error_state = false; // reset error flag on each file
        current_rec = rec;
        create_file();
        allocate_file();
      }
      write_file();
    });
    // leave only last file record in cache
    if (!cache_records.empty()) {
      current_rec = cache_records.back();
      current_rec.buffer_pos = 0;
      current_rec.buffer_size = 0;
      cache_records.assign(1, current_rec);
      continue_file = true; // last file is not written in its entirety (possibly)
    }
    buffer_pos = 0;
    progress->reset_cache_stats();
  }
  void store(const unsigned char* data, size_t size) {
    assert(!cache_records.empty());
    assert(size <= buffer_size);
    if (buffer_pos + size > buffer_size) {
      write();
    }
    CacheRecord& rec = cache_records.back();
    size_t new_size = buffer_pos + size;
    if (new_size > commit_size) {
      CHECK_SYS(VirtualAlloc(buffer + commit_size, new_size - commit_size, MEM_COMMIT, PAGE_READWRITE));
      commit_size = new_size;
    }
    std::memcpy(buffer + buffer_pos, data, size);
    rec.buffer_size += size;
    buffer_pos += size;
    progress->update_cache_stored(size);
  }
public:
  FileWriteCache(std::shared_ptr<Archive> archive, std::shared_ptr<bool> ignore_errors, std::shared_ptr<ErrorLog> error_log, std::shared_ptr<ExtractProgress> progress):
    archive(archive),
    buffer_size(get_max_cache_size()),
    ignore_errors(ignore_errors),
    error_log(error_log),
    progress(progress)
  {
    progress->set_cache_total(buffer_size);
    buffer = reinterpret_cast<unsigned char*>(VirtualAlloc(nullptr, buffer_size, MEM_RESERVE, PAGE_NOACCESS));
    CHECK_SYS(buffer);
  }
  ~FileWriteCache() {
    VirtualFree(buffer, 0, MEM_RELEASE);
    if (file.is_open()) {
      file.close();
      File::delete_file_nt(current_rec.file_path);
    }
  }
  void store_file(const std::wstring& file_path, UInt32 file_id, OverwriteAction overwrite_action) {
    CacheRecord rec;
    rec.file_path = file_path;
    rec.file_id = file_id;
    rec.overwrite = overwrite_action;
    rec.buffer_pos = buffer_pos;
    rec.buffer_size = 0;
    cache_records.push_back(rec);
  }
  void store_data(const unsigned char* data, size_t size) {
    unsigned full_buffer_cnt = static_cast<unsigned>(size / buffer_size);
    for (unsigned i = 0; i < full_buffer_cnt; i++) {
      store(data + i * buffer_size, buffer_size);
    }
    store(data + full_buffer_cnt * buffer_size, size % buffer_size);
  }
  void finalize() {
    write();
    close_file();
  }
};


class CachedFileExtractStream: public ISequentialOutStream, public ComBase {
private:
  std::shared_ptr<FileWriteCache> cache;

public:
  CachedFileExtractStream(std::shared_ptr<FileWriteCache> cache): cache(cache) {
  }

  UNKNOWN_IMPL_BEGIN
  UNKNOWN_IMPL_ITF(ISequentialOutStream)
  UNKNOWN_IMPL_END

  STDMETHODIMP Write(const void *data, UInt32 size, UInt32 *processedSize) noexcept override {
    COM_ERROR_HANDLER_BEGIN
    if (processedSize)
      *processedSize = 0;
    cache->store_data(static_cast<const unsigned char*>(data), size);
    if (processedSize)
      *processedSize = size;
    return S_OK;
    COM_ERROR_HANDLER_END
  }
};


class ArchiveExtractor: public IArchiveExtractCallback, public ICryptoGetTextPassword, public ComBase {
private:
  std::wstring file_path;
  ArcFileInfo file_info;
  UInt32 src_dir_index;
  std::wstring dst_dir;
  std::shared_ptr<Archive> archive;
  std::shared_ptr<OverwriteAction> overwrite_action;
  std::shared_ptr<bool> ignore_errors;
  std::shared_ptr<ErrorLog> error_log;
  std::shared_ptr<FileWriteCache> cache;
  std::shared_ptr<ExtractProgress> progress;
  std::shared_ptr<std::set<UInt32>> skipped_indices;

public:
  ArchiveExtractor(
    UInt32 src_dir_index,
    const std::wstring& dst_dir,
    std::shared_ptr<Archive> archive,
    std::shared_ptr<OverwriteAction> overwrite_action,
    std::shared_ptr<bool> ignore_errors,
    std::shared_ptr<ErrorLog> error_log,
    std::shared_ptr<FileWriteCache> cache,
    std::shared_ptr<ExtractProgress> progress,
    std::shared_ptr<std::set<UInt32>> skipped_indices):
      src_dir_index(src_dir_index),
      dst_dir(dst_dir),
      archive(archive),
      overwrite_action(overwrite_action),
      ignore_errors(ignore_errors),
      error_log(error_log),
      cache(cache),
      progress(progress),
      skipped_indices(skipped_indices) {
  }

  UNKNOWN_IMPL_BEGIN
  UNKNOWN_IMPL_ITF(IProgress)
  UNKNOWN_IMPL_ITF(IArchiveExtractCallback)
  UNKNOWN_IMPL_ITF(ICryptoGetTextPassword)
  UNKNOWN_IMPL_END

  STDMETHODIMP SetTotal(UInt64 total) noexcept override {
    CriticalSectionLock lock(GetSync());
    COM_ERROR_HANDLER_BEGIN
    progress->set_extract_total(total);
    return S_OK;
    COM_ERROR_HANDLER_END
  }
  STDMETHODIMP SetCompleted(const UInt64 *completeValue) noexcept override {
    CriticalSectionLock lock(GetSync());
    COM_ERROR_HANDLER_BEGIN
    if (completeValue)
      progress->update_extract_completed(*completeValue);
    return S_OK;
    COM_ERROR_HANDLER_END
  }

  STDMETHODIMP GetStream(UInt32 index, ISequentialOutStream **outStream,  Int32 askExtractMode) noexcept override {
    COM_ERROR_HANDLER_BEGIN
    *outStream = nullptr;
    file_info = archive->file_list[index];
    if (file_info.is_dir)
      return S_OK;

    const auto cmode = static_cast<int>(g_options.correct_name_mode);
    file_path = correct_filename(file_info.name, cmode, file_info.is_altstream);
    UInt32 parent_index = file_info.parent;
    while (parent_index != src_dir_index && parent_index != c_root_index) {
      const ArcFileInfo& parent_file_info = archive->file_list[parent_index];
      file_path.insert(0, 1, L'\\').insert(0, correct_filename(parent_file_info.name, cmode & ~(0x10 | 0x40), false));
      parent_index = parent_file_info.parent;
    }
    file_path.insert(0, add_trailing_slash(dst_dir));

    if (askExtractMode != NArchive::NExtract::NAskMode::kExtract)
      return S_OK;

    FindData dst_file_info;
    OverwriteAction overwrite;
    if (File::get_find_data_nt(file_path, dst_file_info)) {
      if (*overwrite_action == oaAsk) {
        OverwriteFileInfo src_ov_info, dst_ov_info;
        src_ov_info.is_dir = file_info.is_dir;
        src_ov_info.size = archive->get_size(index);
        src_ov_info.mtime = archive->get_mtime(index);
        dst_ov_info.is_dir = dst_file_info.is_dir();
        dst_ov_info.size = dst_file_info.size();
        dst_ov_info.mtime = dst_file_info.ftLastWriteTime;
        ProgressSuspend ps(*progress);
        OverwriteOptions ov_options;
        if (!overwrite_dialog(file_path, src_ov_info, dst_ov_info, odkExtract, ov_options))
          return E_ABORT;
        if (g_options.strict_case && ov_options.action == oaOverwrite) {
          auto dst_len = std::wcslen(dst_file_info.cFileName);
          if (file_path.size() > dst_len && file_path.substr(file_path.size()-dst_len) != dst_file_info.cFileName)
            ov_options.action = oaOverwriteCase;
        }
        overwrite = ov_options.action;
        if (ov_options.all)
          *overwrite_action = ov_options.action;
      }
      else
        overwrite = *overwrite_action;
      if (overwrite == oaSkip) {
        if (skipped_indices) {
          skipped_indices->insert(index);
          for (UInt32 idx = file_info.parent; idx != c_root_index; idx = archive->file_list[idx].parent) {
            skipped_indices->insert(idx);
          }
        }
        return S_OK;
      }
    }
    else
      overwrite = oaAsk;

    if (archive->get_anti(index))
    {
      if (File::exists(file_path)) File::delete_file(file_path);
      return S_OK;
    }

    progress->update_extract_file(file_path);
    cache->store_file(file_path, index, overwrite);
    ComObject<ISequentialOutStream> out_stream(new CachedFileExtractStream(cache));
    out_stream.detach(outStream);

    return S_OK;
    COM_ERROR_HANDLER_END
  }
  STDMETHODIMP PrepareOperation(Int32 askExtractMode) noexcept override {
    CriticalSectionLock lock(GetSync());
    COM_ERROR_HANDLER_BEGIN
    return S_OK;
    COM_ERROR_HANDLER_END
  }
  STDMETHODIMP SetOperationResult(Int32 resultEOperationResult) noexcept override {
    CriticalSectionLock lock(GetSync());
    COM_ERROR_HANDLER_BEGIN
    RETRY_OR_IGNORE_BEGIN
    bool encrypted = !archive->m_password.empty();
    Error error;
    switch (resultEOperationResult) {
    case NArchive::NExtract::NOperationResult::kOK:
    case NArchive::NExtract::NOperationResult::kDataAfterEnd:
      return S_OK;
    case NArchive::NExtract::NOperationResult::kUnsupportedMethod:
      error.messages.emplace_back(Far::get_msg(MSG_ERROR_EXTRACT_UNSUPPORTED_METHOD));
      break;
    case NArchive::NExtract::NOperationResult::kDataError:
      archive->m_password.clear();
      error.messages.emplace_back(Far::get_msg(encrypted ? MSG_ERROR_EXTRACT_DATA_ERROR_ENCRYPTED : MSG_ERROR_EXTRACT_DATA_ERROR));
      break;
    case NArchive::NExtract::NOperationResult::kCRCError:
      archive->m_password.clear();
      error.messages.emplace_back(Far::get_msg(encrypted ? MSG_ERROR_EXTRACT_CRC_ERROR_ENCRYPTED : MSG_ERROR_EXTRACT_CRC_ERROR));
      break;
    case NArchive::NExtract::NOperationResult::kUnavailable:
      error.messages.emplace_back(Far::get_msg(MSG_ERROR_EXTRACT_UNAVAILABLE_DATA));
      break;
    case NArchive::NExtract::NOperationResult::kUnexpectedEnd:
      error.messages.emplace_back(Far::get_msg(MSG_ERROR_EXTRACT_UNEXPECTED_END_DATA));
      break;
    //case NArchive::NExtract::NOperationResult::kDataAfterEnd:
    //  error.messages.emplace_back(Far::get_msg(MSG_ERROR_EXTRACT_DATA_AFTER_END));
    //  break;
    case NArchive::NExtract::NOperationResult::kIsNotArc:
      error.messages.emplace_back(Far::get_msg(MSG_ERROR_EXTRACT_IS_NOT_ARCHIVE));
      break;
    case NArchive::NExtract::NOperationResult::kHeadersError:
      error.messages.emplace_back(Far::get_msg(MSG_ERROR_EXTRACT_HEADERS_ERROR));
      break;
    case NArchive::NExtract::NOperationResult::kWrongPassword:
      error.messages.emplace_back(Far::get_msg(MSG_ERROR_EXTRACT_WRONG_PASSWORD));
      break;
    default:
      error.messages.emplace_back(Far::get_msg(MSG_ERROR_EXTRACT_UNKNOWN));
      break;
    }
    error.code = E_MESSAGE;
    error.messages.emplace_back(file_path);
    error.messages.emplace_back(archive->arc_path);
    throw error;
    IGNORE_END(*ignore_errors, *error_log, *progress)
    COM_ERROR_HANDLER_END
  }

  STDMETHODIMP CryptoGetTextPassword(BSTR *password) noexcept override {
    CriticalSectionLock lock(GetSync());
    COM_ERROR_HANDLER_BEGIN
    if (archive->m_password.empty()) {
      ProgressSuspend ps(*progress);
      if (!password_dialog(archive->m_password, archive->arc_path))
        FAIL(E_ABORT);
    }
    BStr(archive->m_password).detach(password);
    return S_OK;
    COM_ERROR_HANDLER_END
  }
};

void Archive::prepare_dst_dir(const std::wstring& path) {
  if (!is_root_path(path)) {
    prepare_dst_dir(extract_file_path(path));
    File::create_dir_nt(path);
  }
}

class PrepareExtract: public ProgressMonitor {
private:
  Archive& archive;
  std::list<UInt32>& indices;
  bool& ignore_errors;
  ErrorLog& error_log;
  const std::wstring* file_path{};

  void do_update_ui() override {
    const unsigned c_width = 60;
    std::wostringstream st;
    st << std::left << std::setw(c_width) << fit_str(*file_path, c_width) << L'\n';
    progress_text = st.str();
  }

  void update_progress(const std::wstring& file_path_value) {
    CriticalSectionLock lock(GetSync());
    file_path = &file_path_value;
    update_ui();
  }

  void prepare_extract(const FileIndexRange& index_range, const std::wstring& parent_dir) {
    const auto cmode = static_cast<int>(g_options.correct_name_mode);
    std::for_each(index_range.first, index_range.second, [&] (UInt32 file_index) {
      const ArcFileInfo& file_info = archive.file_list[file_index];
      if (file_info.is_dir) {
        std::wstring dir_path = add_trailing_slash(parent_dir) + correct_filename(file_info.name, cmode, file_info.is_altstream);
        update_progress(dir_path);

        RETRY_OR_IGNORE_BEGIN
        try {
          File::create_dir(dir_path);
        }
        catch (const Error& e) {
          if (e.code != HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS))
            throw;
        }
        RETRY_OR_IGNORE_END(ignore_errors, error_log, *this)

        FileIndexRange dir_list = archive.get_dir_list(file_index);
        prepare_extract(dir_list, dir_path);
      }
      else {
        indices.push_back(file_index);
      }
    });
  }

public:
  PrepareExtract(const FileIndexRange& index_range, const std::wstring& parent_dir, Archive& archive, std::list<UInt32>& indices, bool& ignore_errors, ErrorLog& error_log): ProgressMonitor(Far::get_msg(MSG_PROGRESS_CREATE_DIRS), false), archive(archive), indices(indices), ignore_errors(ignore_errors), error_log(error_log) {
    prepare_extract(index_range, parent_dir);
  }
};


class SetDirAttr: public ProgressMonitor {
private:
  Archive& archive;
  bool& ignore_errors;
  ErrorLog& error_log;
  const std::wstring* m_file_path{};

  void do_update_ui() override {
    const unsigned c_width = 60;
    std::wostringstream st;
    st << std::left << std::setw(c_width) << fit_str(*m_file_path, c_width) << L'\n';
    progress_text = st.str();
  }

  void update_progress(const std::wstring& file_path_value) {
    CriticalSectionLock lock(GetSync());
	m_file_path = &file_path_value;
    update_ui();
  }

  void set_dir_attr(const FileIndexRange& index_range, const std::wstring& parent_dir) {
    const auto cmode = static_cast<int>(g_options.correct_name_mode);
    for_each (index_range.first, index_range.second, [&] (UInt32 file_index) {
      const ArcFileInfo& file_info = archive.file_list[file_index];
      std::wstring file_path = add_trailing_slash(parent_dir) + correct_filename(file_info.name, cmode, file_info.is_altstream);
      update_progress(file_path);
      if (file_info.is_dir) {
        FileIndexRange dir_list = archive.get_dir_list(file_index);
        set_dir_attr(dir_list, file_path);
        RETRY_OR_IGNORE_BEGIN
        if (archive.get_anti(file_index))
        {
          if (File::exists(file_path)) File::remove_dir(file_path);
        }
        else
        {
          File::set_attr(file_path, FILE_ATTRIBUTE_NORMAL);
          File file(file_path, FILE_WRITE_ATTRIBUTES, FILE_SHARE_READ, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS);
          File::set_attr_nt(file_path, archive.get_attr(file_index));
          file.set_time_nt(archive.get_ctime(file_index), archive.get_atime(file_index), archive.get_mtime(file_index));
        }
        RETRY_OR_IGNORE_END(ignore_errors, error_log, *this)
      }
    });
  }

public:
  SetDirAttr(const FileIndexRange& index_range, const std::wstring& parent_dir, Archive& archive, bool& ignore_errors, ErrorLog& error_log): ProgressMonitor(Far::get_msg(MSG_PROGRESS_SET_ATTR), false), archive(archive), ignore_errors(ignore_errors), error_log(error_log) {
    set_dir_attr(index_range, parent_dir);
  }
};


void Archive::extract(UInt32 src_dir_index, const std::vector<UInt32>& src_indices, const ExtractOptions& options, std::shared_ptr<ErrorLog> error_log, std::vector<UInt32>* extracted_indices) {
  DisableSleepMode dsm;

  const auto ignore_errors = std::make_shared<bool>(options.ignore_errors);
  const auto overwrite_action = std::make_shared<OverwriteAction>(options.overwrite);

  prepare_dst_dir(options.dst_dir);

  std::list<UInt32> file_indices;
  PrepareExtract(FileIndexRange(src_indices.begin(), src_indices.end()), options.dst_dir, *this, file_indices, *ignore_errors, *error_log);

  std::vector<UInt32> indices;
  indices.reserve(file_indices.size());
  std::copy(file_indices.begin(), file_indices.end(), std::back_insert_iterator<std::vector<UInt32>>(indices));
  std::sort(indices.begin(), indices.end());

  const auto progress = std::make_shared<ExtractProgress>(arc_path);
  const auto cache = std::make_shared<FileWriteCache>(shared_from_this(), ignore_errors, error_log, progress);
  const auto skipped_indices = extracted_indices? std::make_shared<std::set<UInt32>>() : nullptr;
  ComObject<IArchiveExtractCallback> extractor(new ArchiveExtractor(src_dir_index, options.dst_dir, shared_from_this(), overwrite_action, ignore_errors, error_log, cache, progress, skipped_indices));
  COM_ERROR_CHECK(in_arc->Extract(indices.data(), static_cast<UInt32>(indices.size()), 0, extractor));
  cache->finalize();
  progress->clean();

  SetDirAttr(FileIndexRange(src_indices.begin(), src_indices.end()), options.dst_dir, *this, *ignore_errors, *error_log);

  if (extracted_indices) {
    std::vector<UInt32> sorted_src_indices(src_indices);
    std::sort(sorted_src_indices.begin(), sorted_src_indices.end());
    extracted_indices->clear();
    extracted_indices->reserve(src_indices.size());
    std::set_difference(sorted_src_indices.begin(), sorted_src_indices.end(), skipped_indices->begin(), skipped_indices->end(), back_inserter(*extracted_indices));
    extracted_indices->shrink_to_fit();
  }
}

void Archive::delete_archive() {
  File::delete_file_nt(arc_path);
  std::for_each(volume_names.begin(), volume_names.end(), [&] (const std::wstring& volume_name) {
    File::delete_file_nt(add_trailing_slash(arc_dir()) + volume_name);
  });
}
