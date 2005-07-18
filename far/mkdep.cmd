@echo off
awk -f mkdep.awk -v out=Release mkdep.list.txt > far.release.dep
awk -f mkdep.awk -v out=Debug   mkdep.list.txt > far.debug.dep
