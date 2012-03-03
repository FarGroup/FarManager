void geninst(const deque<wstring>& params) {
  wstring install_target(L"install_project: $(INSTALL)");
  for (unsigned i = 0; i < params.size(); ++i) {
    wstring file_path(params[i]);
    wstring target(L"$(INSTALL)\\" + extract_file_name(file_path));
    wcout << target << L": " << file_path << L"\n";
    wcout << L"  $(COPY)\n";
    install_target += L' ';
    install_target += target;
  }
  wcout << install_target << L"\n";
  wcout << L"$(INSTALL):\n";
  wcout << L"  if not exist $(INSTALL) mkdir $(INSTALL)\n";
}
