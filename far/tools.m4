m4_define(CMDDATE,m4_ifelse(HOSTTYPE,`Windows',`.\scripts\gendate.awk',`./scripts/gendate.awk'))m4_dnl
m4_define(CMDPLUGINS,m4_ifelse(HOSTTYPE,`Windows',`.\scripts\plugins.awk',`./scripts/plugins.awk'))m4_dnl
m4_define(CMDAWK,m4_ifelse(HOSTTYPE,`Windows',`tools\gawk.exe',m4_ifelse(HOSTTYPE,`Msys',`./tools/gawk',`gawk')))m4_dnl
