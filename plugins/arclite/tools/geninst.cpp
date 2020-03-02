void geninst(const std::deque<std::wstring>& params) {
  std::wstring install_target(L"install_project: $(INSTALL)");
  for (unsigned i = 0; i < params.size(); ++i) {
    std::wstring file_path(params[i]);
    std::wstring target(L"$(INSTALL)\\" + extract_file_name(file_path));
    std::wcout << target << L": " << file_path << L"\n";
    std::wcout << L"  $(COPY)\n";
    install_target += L' ';
    install_target += target;
  }
  std::wcout << install_target << L"\n";
  std::wcout << L"$(INSTALL):\n";
  std::wcout << L"  if not exist $(INSTALL) mkdir $(INSTALL)\n";
}
