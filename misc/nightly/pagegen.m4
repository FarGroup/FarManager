m4_include(`farversion.m4')m4_dnl
m4_divert(-1)
M4_MACRO_DEFINE(NEWARC, M4_MACRO_CONCAT(
	Far,
	M4_MACRO_GET(VERSION_MAJOR),
	M4_MACRO_GET(VERSION_MINOR),
	b,
	M4_MACRO_GET(VERSION_BUILD).M4_MACRO_GET(BUILD_PLATFORM).M4_MACRO_GET(BUILD_YEAR),
	M4_MACRO_GET(BUILD_MONTH),
	M4_MACRO_GET(BUILD_DAY)))

m4_syscmd(cp -f M4_MACRO_CONCAT(ARC, .7z     /var/www/html/nightly/, M4_MACRO_GET(NEWARC), .7z))
m4_syscmd(cp -f M4_MACRO_CONCAT(ARC, .msi    /var/www/html/nightly/, M4_MACRO_GET(NEWARC), .msi))
m4_syscmd(cp -f M4_MACRO_CONCAT(ARC, .pdb.7z /var/www/html/nightly/, M4_MACRO_GET(NEWARC), .pdb.7z))

m4_divert(0)m4_dnl
<?php
M4_MACRO_CONCAT($far, FARVAR, _major=",          M4_MACRO_GET(VERSION_MAJOR)";)
M4_MACRO_CONCAT($far, FARVAR, _minor=",          M4_MACRO_GET(VERSION_MINOR)";)
M4_MACRO_CONCAT($far, FARVAR, _build=",          M4_MACRO_GET(VERSION_BUILD)";)
M4_MACRO_CONCAT($far, FARVAR, _platform=",       M4_MACRO_GET(BUILD_PLATFORM)";)
M4_MACRO_CONCAT($far, FARVAR, _arc=",            M4_MACRO_GET(NEWARC), .7z";)
M4_MACRO_CONCAT($far, FARVAR, _msi=",            M4_MACRO_GET(NEWARC),.msi";)
M4_MACRO_CONCAT($far, FARVAR, _pdb=",            M4_MACRO_GET(NEWARC),.pdb.7z";)
M4_MACRO_CONCAT($far, FARVAR, _date=",           M4_MACRO_GET(BUILD_YEAR)-M4_MACRO_GET(BUILD_MONTH)-M4_MACRO_GET(BUILD_DAY)";)
M4_MACRO_CONCAT($far, FARVAR, _scm_revision=",   SCM_REVISION";)
M4_MACRO_CONCAT($far, FARVAR, _lastchange=",     LASTCHANGE";)
?>
