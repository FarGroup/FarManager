tools\m4 -P copyright.inc.m4 | tools\gawk -f scripts\enc.awk > copyright.inc
tools\m4 -P farversion.inc.m4                                > farversion.inc
tools\m4 -P Far.exe.manifest.m4                              > Far.exe.manifest
tools\m4 -P far.rc.m4                                        > far.rc
tools\lng.generator.exe -nc -i lang.ini farlang.templ
