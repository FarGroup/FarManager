/* lf_common.h */
#ifndef LF_COMMON_H
#define LF_COMMON_H

/* Common library prefix to prevent Lua registry conflicts */
#define LUAFAR_PREFIX "LuaFAR."

/* LuaFAR Metatable / Registry Keys */
#define TYPE_ADDMACRODATA  LUAFAR_PREFIX "AddMacroData"
#define TYPE_BIT64         LUAFAR_PREFIX "Bit64"
#define TYPE_DIALOG        LUAFAR_PREFIX "Dialog"
#define TYPE_FILEFILTER    LUAFAR_PREFIX "FileFilter"
#define TYPE_PLUGINHANDLE  LUAFAR_PREFIX "PluginHandle"
#define TYPE_REGEX         LUAFAR_PREFIX "Regex"
#define TYPE_SAVEDSCREEN   LUAFAR_PREFIX "SavedScreen"
#define TYPE_SETTINGS      LUAFAR_PREFIX "Settings"
#define TYPE_TIMER         LUAFAR_PREFIX "Timer"
#define TYPE_USERCONTROL   LUAFAR_PREFIX "UserControl"

#endif /* LF_COMMON_H */