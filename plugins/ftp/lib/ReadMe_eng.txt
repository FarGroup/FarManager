
Description
~~~~~~~~~~~
Sub plugins used by FAR FTP client with version greater 1.5.

Contacts \ Copyrights
~~~~~~~~~~~~~~~~~~~~~
  The part of sub-plugins source code are written by "Eugene Roshal",
all modification and futures created by "Jouri Mamaev".

  Current sub-plugin developer:
    JouriM<at>uran.ru

  Sub-plugins and ftp plugin news and latest downloads:
      ftp://JouriWarez:WareZ@194.226.246.33/Popa/Far

Sub-Plugins search method
~~~~~~~~~~~~~~~~~~~~~~~~~~~
FTP client searches sub plugins in next order:
  1. Already loaded module (by other plugin or other FTP
     client copy);
  2. Base FAR directory (from where far.exe start);
  3. System wide search (uses system rules to locate DLL);
  4. FTP client plugin directory (from where farftp.dll
     loaded by FAR);
  5. FTP client library directory ("LIB" subdirectory in
     FTP plugin directory);

Fail to load/locate
~~~~~~~~~~~~~~~~~~~
If FTP client plugin can not load some of sub-plugins it will
fail to start with message:

  "Error loading: Plugin ... is not valid FTP plugin."

in case found old or incorrect module version, and with message:

  "Error loading: Plugin ... can not be found."

in case module can not be found.

Sub-Plugins usage
~~~~~~~~~~~~~~~~~
Every time you open FTP client plugin panel the plugin tries to load all
plugins it need.
If all modules already loaded plugin do nothing, or try to locate all
modules if last try fail.

If FTP client plugin successfully locate and load sub-plugins it silently
use it and release module handles only before finally FAR shutdown.
So, if you need to reinstall or delete sub-plugins modules you need to
exit all FAR copies which use FTP client.

Sub-plugin sources
~~~~~~~~~~~~~~~~~~
You can download sources for all sub-plugins from FarFtp ftp site with archive
named like:
  Ftpp_DDMMYY.zip
 where:
   DD - day of month;
   MM - month;
   YY - last two digits of year;

For example file with name "Ftpp_251102.zip" means package with sources
for 25 Dec 2002.

Sub-Plugins compilation
~~~~~~~~~~~~~~~~~~~~~~~
To compile plugins you must have:
  - sub-plugins sources;

  - FarFtp with maximum closest version to sub-plugin package;
      ftp://JouriWarez:WareZ@194.226.246.33/Popa/Far

  - FStdLib installed;
      ftp://JouriWarez:WareZ@194.226.246.33/Popa/Far/FStdLib

  - Borland CPP compiler version later 5.2
      or
    Visual CPP compiler version later 6.0.

To compile plugins you may use makefiles provided with sub-plugins.
Use "makefile.bcb" file to compile with Borland compiler and
"makefile.vc6" file to compile with Visual C compiler.

Sub-Plugins modification
~~~~~~~~~~~~~~~~~~~~~~~~
You may freely modificate sub-plugin sources.

If you change it to fix some bugs or to implement any futures and wish
to share this changes with other FTP users please send your modifications
to current developer with changes description.
