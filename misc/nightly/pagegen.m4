m4_include(`farversion.m4')m4_dnl
m4_define(NEWARC,`Far'MAJOR`'MINOR`b'BUILD`.'FROMBIT(`x86',`x64',`IA64')`.'BLD_YEAR`'BLD_MONTH`'BLD_DAY)m4_dnl
m4_define(COPY,m4_esyscmd(cp -f ARC`.7z' `/var/www/html/nightly/'NEWARC`.7z'))m4_dnl
m4_define(COPY2,m4_esyscmd(cp -f ARC`.msi' `/var/www/html/nightly/'NEWARC`.msi'))m4_dnl
`<?php'
`$far'FARVAR`_major="'MAJOR`";'
`$far'FARVAR`_minor="'MINOR`";'
`$far'FARVAR`_build="'BUILD`";'
`$far'FARVAR`_platform="'FROMBIT(`x86',`x64',`IA64')`";'
`$far'FARVAR`_arc="'NEWARC`.7z";'
`$far'FARVAR`_msi="'NEWARC`.msi";'
`$far'FARVAR`_date="'BLD_YEAR`-'BLD_MONTH`-'BLD_DAY`";'
`$far'FARVAR`_lastchange="'LASTCHANGE`";'
`?>'
