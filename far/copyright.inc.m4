m4_include(`farversion.m4')m4_dnl
string_view version_string() { return L"Far Manager, version M4_MACRO_GET(FULLVERSION)"sv; }
string_view copyright() { return L"Copyright © 1996-2000 Eugene Roshal, Copyright © 2000-M4_MACRO_GET(COPYRIGHTYEAR) Far Group"sv; }
string_view platform() { return L"M4_MACRO_GET(BUILD_PLATFORM)"sv; }
string_view scm_revision() { return L"M4_MACRO_GET(BUILD_SCM_REVISION)"sv; }
