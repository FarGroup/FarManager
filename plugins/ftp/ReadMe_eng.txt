Read-ME file for FAR FTP client
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Files
~~~~~~
  FTP archive can contains different files depends on plugin version you
  receive.
  Here is description for full files list:
    - FarFtp.dll         Plugin main DLL;
    - FtpEng.hlf       + English help file;
    - FtpRus.hlf       + Russian help file;
    - FtpEng.lng         English language file;
    - FtpRus.lng         Russian language file;
    - TechInfo.reg     + English registry file contains technical options for
                         configure FTP plugin;
    - TechInfo_rus.reg + Russian registry file contains technical options for
                         configure FTP plugin;
    - FtpCmds*.txt     + Describes how to configure plugin to use custom
                         ftp commands;
    - Notes*.txt       + Text document contains notes on the features of the
                         FTP plugin;
    - Lib              + Directory contains sub-plugins for FTP client;
                         Note: plugin must have all sub-plugins installed to
                           run properly but package may not contain plugins
                           themself in case they are not changed since last
                           plugin version;

  Files marked by "+" symbol are optional and may not exist in some
  plugin packages;

Upgrade existing version
~~~~~~~~~~~~~~~~~~~~~~~~
  1. Upgrading using a full package

  If you receive full package simply delete all old files from FTP directory
(including subdirectories) and extract received package to old directory;

  2. Upgrading using a partial package

  If you receive partial package, be sure you have all other files from
correct version of plugin package;

  Upgrading using partial packages are safe only in case if:
    - you periodically receive new versions;
    - you have the last full plugin package installed;
    - you are a bug-reporter and receive plugin package directly from developer;
    - you know exactly what you are doing;

  Always read next files before installing:
    - Check changes in all text files from package;

  You should overwrite next files if they are contained in the package:
      - *.dll - FTP client or plugins files;
                Client may not run or may fail if you use it with sub-plugins
                from different versions;
      - *.lng - language files.
                Plugin will fail showing message if you use old language files;
      - all other files are optional;
