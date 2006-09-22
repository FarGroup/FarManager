@echo off
tools\gawk -f .\scripts\mkdep.awk -v out=Release.vc mkdep.list.txt > far.release.dep
tools\gawk -f .\scripts\mkdep.awk -v out=Debug.vc   mkdep.list.txt > far.debug.dep
