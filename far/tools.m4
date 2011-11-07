m4_define(HOST_TYPE,m4_ifdef(`HOSTTYPE',HOSTTYPE,`Windows'))m4_dnl
m4_define(CMDDATE,m4_ifelse(HOST_TYPE,`Windows',`.\scripts\gendate.awk',`./scripts/gendate.awk'))m4_dnl
m4_define(CMDPLUGINS,m4_ifelse(HOST_TYPE,`Windows',`.\scripts\plugins.awk',`./scripts/plugins.awk'))m4_dnl
m4_define(CMDAWK,m4_ifelse(HOST_TYPE,`Windows',`tools\gawk.exe',HOST_TYPE,`Msys',`./tools/gawk',`gawk'))m4_dnl
