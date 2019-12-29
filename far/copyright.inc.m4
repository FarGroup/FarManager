m4_include(`farversion.m4')m4_dnl
string_view version_string() { return L"Far Manager, version FULLVERSION"sv; }
string_view copyright() { return L"Copyright © 1996-2000 Eugene Roshal, Copyright © 2000-COPYRIGHTYEAR Far Group"sv; }
string_view platform() { return L"BUILD_PLATFORM"sv; }
string_view scm_revision() { return L"BUILD_SCM_REVISION"sv; }
