m4_define(HOST_TYPE,m4_ifdef(`HOSTTYPE',HOSTTYPE,`Windows'))m4_dnl
m4_define(CMDAWK,m4_ifelse(HOST_TYPE,`Windows',`tools\gawk.exe',HOST_TYPE,`Msys',`./tools/gawk',`gawk'))m4_dnl
