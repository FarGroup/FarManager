#include <windows.h>
#include <imagehlp.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
using namespace std;

int main(int argc, char* argv[]) {
  try {
    if (argc < 2)
      throw exception("source path not specified");

#ifdef NIGHTLY
    if (argc < 3)
      throw exception("platform not specified");
#endif

    string source_dir = argv[1];

    // determine Far version
    string plugin_hpp_path = source_dir + "\\PluginSDK\\Headers.c\\plugin.hpp";
    ifstream header_file;
    header_file.exceptions(ios_base::badbit | ios_base::failbit | ios_base::eofbit);
    header_file.open(plugin_hpp_path.c_str());
    string line;
    unsigned ver_major, ver_minor, ver_build;
    unsigned found_cnt = 0;
    while (found_cnt < 3) {
      header_file >> line;
      if (line == "FARMANAGERVERSION_MAJOR") {
        header_file >> ver_major;
        found_cnt++;
      }
      if (line == "FARMANAGERVERSION_MINOR") {
        header_file >> ver_minor;
        found_cnt++;
      }
      if (line == "FARMANAGERVERSION_BUILD") {
        header_file >> ver_build;
        found_cnt++;
      }
    }

    // determine Far platform
#ifdef NIGHTLY
    string platform = argv[2];
#else
    string far_exe_path = source_dir + "\\Far.exe";
    LOADED_IMAGE* far_exe = ImageLoad(far_exe_path.c_str(), "");
    if (!far_exe)
      throw exception("cannot load Far.exe");
    WORD machine = far_exe->FileHeader->FileHeader.Machine;
    ImageUnload(far_exe);
    string platform;
    if (machine == IMAGE_FILE_MACHINE_I386) {
      platform = "x86";
    }
    else if (machine == IMAGE_FILE_MACHINE_AMD64) {
      platform = "x64";
    }
    else
      throw exception("unknown machine type");
#endif

    // generate params
    ofstream output;
    output.exceptions(ios_base::badbit | ios_base::failbit | ios_base::eofbit);
    output.open("params.mak");
    output << "SOURCE_DIR = " << source_dir << endl;
    output << "VER_MAJOR = " << ver_major << endl;
    output << "VER_MINOR = " << ver_minor << endl;
    output << "VER_BUILD = " << ver_build << endl;
    output << "PLATFORM = " << platform << endl;
#ifdef NIGHTLY
    output << "NIGHTLY = 1" << endl;
#endif

    return 0;
  }
  catch (const exception& e) {
    cerr << "genparams: error: " << typeid(e).name() << ": " << e.what() << endl;
  }
  catch (...) {
    cerr << "genparams: unknown error" << endl;
  }
  return 1;
}
