@echo off
awk -f mkdep.awk -v out=Release.vc mkdep.list.txt > far.release.dep
awk -f mkdep.awk -v out=Debug.vc   mkdep.list.txt > far.debug.dep
