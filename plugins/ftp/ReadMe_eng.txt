Read-ME file for FAR FTP client
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Plugin versions
~~~~~~~~~~~~~~~
  Plugin includes in FAR package, but upgrades more frequently then base FAR
  release, so you must visit and monitor plugin version on developer site in
  case you need to have latest plugin versions.

  The site of current plugin developer is:
     ftp://far:far@194.226.246.33

Files
~~~~~~
  FTP archive can contains different files depends on plugin version you
  receive.
  Here is description for full files list:
    - !WhatsNew          File contains the latest news in the text form for all
                         plugin versions;
    - file_id.diz      + Short description file;
    - FarFtp.dll         Plugin main DLL;
    - FtpEng.hlf       + English help file;
    - FtpRus.hlf       + Russian help file;
    - FtpEng.lng         English language file;
    - FtpRus.lng         Russian language file;
    - CopyDlg_*.reg    + Registry files contains design for copy progress dialog
                         designed by different person;
    - TechInfo.reg     + English registry file contains technical options for
                         configure FTP plugin;
    - TechInfo_rus.reg + English registry file contains technical options for
                         configure FTP plugin;
    - FtpCmds*.txt     + Describes how to configure plugin to use custom
                         ftp commands;
    - Notes*.txt       + Text document contains notes on the features of the
                         FTP plugin;
    - WhatsNew*.txt    + Contains news information since last public FAR
                         release;
    - Lib              + Directory contains sub-plugins for FTP client;
                         Note: plugin must have all plugins installed to run
                           properly but package may not contain plugins
                           itself in case then are not changed since last
                           plugin version;

  Files marked by "+" symbol are optional and may not exist in some
  plugin packages;

Upgrade existing versions
~~~~~~~~~~~~~~~~~~~~~~~~~
  1. Upgrading using full package

  If you receive full package simply delete all old files from FTP directory
(including subdirectories) and extract received package to old directory;

  2. Upgrading using partial package

  If you receive partial package, be sure you have all other files from
correct version of plugin package;

  Upgrading using partial packages are safe only in case if:
    - you periodically receive new versions;
    - you have the last full plugin package installed;
    - you is a bug-reporter and receive plugin package directly from developer;
    - you know exactly what are you doing;

  Always read next files before installing:
    - "!WhatsNew"     - to be sure which new futures or bug fixes are included
                        in package;
    - "WhatsNew*.txt" - read this file only if "!WhatsNew" file does not
                        exist in package;
    - Check changes in all other text files from package;

  You should overwrite next files if they contains in package:
      - *.dll - FTP client or plugins files;
                Client may not run or may fail if you will use it with sub-plugins
                from different versions;

      - *.lng - language files.
                Plugin will fail showing message if you use old language files;
      - all other files are optional and you may not extract if from package;
