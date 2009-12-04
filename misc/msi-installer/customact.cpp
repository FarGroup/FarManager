#include <windows.h>
#include <msiquery.h>

bool is_inst(MSIHANDLE h_install, const char* name) {
  INSTALLSTATE st_inst, st_action;
  if (MsiGetFeatureState(h_install, name, &st_inst, &st_action) != ERROR_SUCCESS) return false;
  INSTALLSTATE st = st_action;
  if (st <= 0) st = st_inst;
  if (st <= 0) return false;
  if ((st == INSTALLSTATE_REMOVED) || (st == INSTALLSTATE_ABSENT)) return false;
  return true;
}

UINT __stdcall UpdateFeatureState(MSIHANDLE h_install) {
  PMSIHANDLE h_db = MsiGetActiveDatabase(h_install);
  if (h_db == 0) return ERROR_INSTALL_FAILURE;
  PMSIHANDLE h_view;
  if (MsiDatabaseOpenView(h_db, "SELECT Feature FROM Feature WHERE Display = 0", &h_view) != ERROR_SUCCESS) return ERROR_INSTALL_FAILURE;
  if (MsiViewExecute(h_view, 0) != ERROR_SUCCESS) return ERROR_INSTALL_FAILURE;
  PMSIHANDLE h_record;
  while (true) {
    UINT res = MsiViewFetch(h_view, &h_record);
    if (res == ERROR_NO_MORE_ITEMS) break;
    if (res != ERROR_SUCCESS) return ERROR_INSTALL_FAILURE;

    const unsigned c_buf_size = 256;
    char feature_id[c_buf_size];
    DWORD size = ARRAYSIZE(feature_id);
    if (MsiRecordGetString(h_record, 1, feature_id, &size) != ERROR_SUCCESS) return ERROR_INSTALL_FAILURE;

    char names[c_buf_size];
    lstrcpy(names, feature_id);
    unsigned name_cnt = 1;
    for (char* ch = names; *ch; ch++) {
      if (*ch == '.') {
        *ch = 0;
        ch++;
        name_cnt++;
      }
    }
    if (name_cnt <= 1) return ERROR_INSTALL_FAILURE;

    bool inst = true;
    const char* name = names;
    for (unsigned i = 0; i < name_cnt; i++) {
      if (!is_inst(h_install, name)) {
        inst = false;
        break;
      }
      name += lstrlen(name) + 1;
    }
    if (MsiSetFeatureState(h_install, feature_id, inst ? INSTALLSTATE_LOCAL : INSTALLSTATE_ABSENT) != ERROR_SUCCESS) return ERROR_INSTALL_FAILURE;
  }

  return ERROR_SUCCESS;
}
