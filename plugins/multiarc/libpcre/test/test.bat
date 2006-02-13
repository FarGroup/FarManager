@echo off
copy ..\pcre.h .
copy "..\pcre++.h" .
copy ..\pcre.lib .
cl /nologo /GX /MD test.cpp

test.exe < test.in > test.out2

fc /b test.out test.out2
