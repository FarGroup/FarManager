m4_include(`farversion.m4')m4_dnl
m4_define(NEWARC,`Far'VERSION_MAJOR`'VERSION_MINOR`b'VERSION_BUILD`.'m4_ifelse(FARBIT, 64, `x64', `x86')`.'BUILD_YEAR`'BUILD_MONTH`'BUILD_DAY)m4_dnl
m4_define(COPY,m4_esyscmd(cp -f ARC`.7z' `/var/www/html/nightly/'NEWARC`.7z'))m4_dnl
m4_define(COPY2,m4_esyscmd(cp -f ARC`.msi' `/var/www/html/nightly/'NEWARC`.msi'))m4_dnl
m4_define(COPY3,m4_esyscmd(cp -f ARC`.pdb.7z' `/var/www/html/nightly/'NEWARC`.pdb.7z'))m4_dnl
`<?php'
`$far'FARVAR`_major="'VERSION_MAJOR`";'
`$far'FARVAR`_minor="'VERSION_MINOR`";'
`$far'FARVAR`_build="'VERSION_BUILD`";'
`$far'FARVAR`_platform="'m4_ifelse(FARBIT, 64, `x64', `x86')`";'
`$far'FARVAR`_arc="'NEWARC`.7z";'
`$far'FARVAR`_msi="'NEWARC`.msi";'
`$far'FARVAR`_pdb="'NEWARC`.pdb.7z";'
`$far'FARVAR`_date="'BUILD_YEAR`-'BUILD_MONTH`-'BUILD_DAY`";'
`$far'FARVAR`_scm_revision="'SCM_REVISION`";'
`$far'FARVAR`_lastchange="'LASTCHANGE`";'
`?>'
