
Description
~~~~~~~~~~~

Contacts \ Copyrights
~~~~~~~~~~~~~~~~~~~~~
  The sub-plugins source code is written by "Eugene Roshal" and "Jouri Mamaev".

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
If FTP client plugin can not load some sub-plugins it will
fail to start with the message:

  "Error loading: Plugin ... is not valid FTP plugin."

in case it found an old or incorrect module version, and with the message:

  "Error loading: Plugin ... can not be found."

in case a module can not be found.

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

Sub-Plugins modification
~~~~~~~~~~~~~~~~~~~~~~~~
You may freely modificate sub-plugin sources.

If you change it to fix some bugs or to implement any futures and wish
to share this changes with other FTP users please send your modifications
to current developer with changes description.
