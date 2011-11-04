m4_define(CMDDATE,m4_ifelse(HOSTTYPE,`Windows',`.\scripts\gendate.awk',`./scripts/gendate.awk'))m4_dnl
m4_define(CMDPLUGINS,m4_ifelse(HOSTTYPE,`Windows',`.\scripts\plugins.awk',`./scripts/plugins.awk'))m4_dnl
m4_define(CMDAWK,m4_ifelse(HOSTTYPE,`Windows',`tools\gawk.exe',`gawk'))m4_dnl
