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
    unsigned fount_cnt = 0;
    while (fount_cnt < 3) {
      header_file >> line;
      if (line == "FARMANAGERVERSION_MAJOR") {
        header_file >> ver_major;
        fount_cnt++;
      }
      if (line == "FARMANAGERVERSION_MINOR") {
        header_file >> ver_minor;
        fount_cnt++;
      }
      if (line == "FARMANAGERVERSION_BUILD") {
        header_file >> ver_build;
        fount_cnt++;
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

    // generate makefile
    ostringstream fmt;
    fmt << ver_major << "." << ver_minor << "." << ver_build;
    string version = fmt.str();

#ifdef NIGHTLY
    string msi_name = source_dir + "\\final.msi";
#else
    fmt.str(string());
    fmt << "Far" << ver_major << ver_minor << "b" << ver_build << "." << platform << ".msi";
    string msi_name = fmt.str();
#endif

    ofstream makefile;
    makefile.exceptions(ios_base::badbit | ios_base::failbit | ios_base::eofbit);
    makefile.open("makefile");
#ifdef NIGHTLY
    makefile << "all:" << endl;
//    makefile << "  cl -nologo -O1 -EHsc customact.cpp -link -dll -out:CustomActions.dll -export:UpdateFeatureState -export:SaveShortcutProps -export:RestoreShortcutProps msi.lib" << endl;
//    makefile << "  upx --lzma CustomActions.dll" << endl;
    makefile << "  candle -nologo -dSourceDir=\"" << source_dir << "\" -dBranch=" << ver_major << " -dPlatform=" << platform << " -dVersion=" << version << " -dCustomActions=CustomActions.dll installer.wxs ui.wxs" << endl;
    makefile << "  light -nologo -cultures:en-us -loc en-us.wxl -loc ui_en-us.wxl -spdb -sval -sh -dcl:high -out " << msi_name << " installer.wixobj ui.wixobj" << endl;
#else
    makefile << msi_name << ": installer.wixobj ui.wixobj en-us.wxl ui_en-us.wxl customact.dll" << endl;
    makefile << "  light -nologo -cultures:en-us -loc en-us.wxl -loc ui_en-us.wxl -spdb -sval -sh -cc . -reusecab -O2 -out " << msi_name << " installer.wixobj ui.wixobj" << endl;
    makefile << "customact.dll: customact.cpp" << endl;
    makefile << "  cl -nologo -Zi -EHsc customact.cpp -link -dll -debug -incremental:no -out:customact.dll -export:UpdateFeatureState -export:SaveShortcutProps -export:RestoreShortcutProps msi.lib" << endl;
    makefile << "installer.wxs: files.wxi features.wxi shortcuts.wxi guids_" << ver_major << "_" << platform << ".wxi" << endl;
    makefile << ".wxs.wixobj::" << endl;
    makefile << "  candle -nologo -dSourceDir=\"" << source_dir << "\" -dBranch=" << ver_major << " -dPlatform=" << platform << " -dVersion=" << version << " -dCustomActions=customact.dll $<" << endl;
    makefile << "clean:" << endl;
    makefile << "  del /q /f *.wixobj customact.dll customact.obj #far.cab" << endl;
    makefile << ".SUFFIXES: .wxs" << endl;
    makefile << ".PHONY: clean" << endl;
#endif

    return 0;
  }
  catch (const exception& e) {
    cerr << "genscript: error: " << typeid(e).name() << ": " << e.what() << endl;
  }
  catch (...) {
    cerr << "genscript: unknown error" << endl;
  }
  return 1;
}
