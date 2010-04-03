#include <windows.h>

#include "consize.hpp"

void main() {
  HANDLE h_file = CreateFile(c_pipe_name, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
  if (h_file == INVALID_HANDLE_VALUE) return;
  
  DWORD process_id = GetCurrentProcessId();
  DWORD nwritten;
  WriteFile(h_file, &process_id, sizeof(process_id), &nwritten, NULL);

  COORD size = GetLargestConsoleWindowSize(GetStdHandle(STD_OUTPUT_HANDLE));
  if (size.X == 0 || size.Y == 0) return;
  WriteFile(h_file, &size, sizeof(size), &nwritten, NULL);

  CloseHandle(h_file);
}
