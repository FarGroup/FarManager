m4_define(CMDDATE,m4_ifelse(HOSTTYPE,`Unix',`./scripts/gendate.awk',`.\scripts\gendate.awk'))m4_dnl
m4_define(CMDPLUGINS,m4_ifelse(HOSTTYPE,`Unix',`./scripts/plugins.awk',`.\scripts\plugins.awk'))m4_dnl
m4_define(CMDAWK,m4_ifelse(HOSTTYPE,`Unix',`gawk',m4_ifelse(HOSTTYPE,`Msys',`./tools/gawk',`tools\gawk.exe')))m4_dnl
