#pragma once

static const TCHAR *pRAR[]={
/*Extract               */_T("rar x {-p%%P} {-ap%%R} -y -c- -kb -- %%A @%%LNM"),
/*Extract without paths */_T("rar e {-p%%P} -y -c- -kb -- %%A @%%LNM"),
/*Test                  */_T("rar t -y {-p%%P} -- %%A"),
/*Delete                */_T("rar d -y {-p%%P} {-w%%W} -- %%A @%%LNM"),
/*Comment archive       */_T("rar c -y {-w%%W} -- %%A"),
/*Comment files         */_T("rar cf -y {-w%%W} -- %%A @%%LNM"),
/*Convert to SFX        */_T("rar s -y -- %%A"),
/*Lock archive          */_T("rar k -y -- %%A"),
/*Protect archive       */_T("rar rr -y -- %%A"),
/*Recover archive       */_T("rar r -y -- %%A"),
/*Add files             */_T("rar a -y {-p%%P} {-ap%%R} {-w%%W} {%%S} -- %%A @%%LNM")
};

static const TCHAR *p7Z[]={
/*Extract               */_T("7z x {-p%%P} -r0 -y -scsDOS -- %%A @%%LQMN"),
/*Extract without paths */_T("7z e {-p%%P} -r0 -y -scsDOS -- %%A @%%LQMN"),
/*Test                  */_T("7z t {-p%%P} -r0 -scsDOS -- %%A @%%LQMN"),
/*Delete                */_T("7z d {-p%%P} -r0 -ms=off -scsDOS -- %%A @%%LQMN"),
/*Comment archive       */_T(""),
/*Comment files         */_T(""),
/*Convert to SFX        */_T(""),
/*Lock archive          */_T(""),
/*Protect archive       */_T(""),
/*Recover archive       */_T(""),
/*Add files             */_T("7z a {-p%%P} -r0 -t7z {%%S} -scsDOS -- %%A @%%LQMN")
};

static const TCHAR *pARJ[]={
/*Extract               */_T("arj32 x -+ {-g%%P} -v -y -p1 -- %%A !%%LM"),
/*Extract without paths */_T("arj32 e -+ {-g%%P} -v -y -p1 -- %%A !%%LM"),
/*Test                  */_T("arj32 t -+ -y {-g%%P} -v -p1 -- %%A"),
/*Delete                */_T("arj32 d -+ -y {-w%%W} -p1 -- %%A !%%LNM"),
/*Comment archive       */_T("arj32 c -+ -y {-w%%W} -z -- %%A"),
/*Comment files         */_T("arj32 c -+ -y {-w%%W} -p1 -- %%A !%%LM"),
/*Convert to SFX        */_T("arj32 y -+ -je -y -p %%A"),
/*Lock archive          */_T(""),
/*Protect archive       */_T("arj32 t -hk -y %%A"),
/*Recover archive       */_T("arj32 q -y %%A"),
/*Add files             */_T("arj32 a -+ -y -a1 {-g%%P} {-w%%W} -p {%%S} -- %%A !%%LM"),
};

static const TCHAR *pZIP[]={
/*Extract               */_T("7z x {-p%%P} -r0 -y {-w%%W} -scsDOS -- %%A @%%LQMN"),
/*Extract without paths */_T("7z e {-p%%P} -r0 -y {-w%%W} -scsDOS -- %%A @%%LQMN"),
/*Test                  */_T("7z t {-p%%P} -r0 -scsDOS -- %%A @%%LQMN"),
/*Delete                */_T("7z d {-p%%P} -r0 {-w%%W} -scsDOS -- %%A @%%LQMN"),
/*Comment archive       */_T(""),
/*Comment files         */_T(""),
/*Convert to SFX        */_T(""),
/*Lock archive          */_T(""),
/*Protect archive       */_T(""),
/*Recover archive       */_T(""),
/*Add files             */_T("7z a {-p%%P} -r0 -tzip {-w%%W} {%%S} -scsDOS -- %%A @%%LQMN"),
};

static const TCHAR *pTAR[]={
/*Extract               */_T("7z x -r0 -y {-w%%W} -scsDOS -- %%A @%%LQMN"),
/*Extract without paths */_T("7z e -r0 -y {-w%%W} -scsDOS -- %%A @%%LQMN"),
/*Test                  */_T("7z t -r0 -scsDOS -- %%A @%%LQMN"),
/*Delete                */_T("7z d -r0 {-w%%W} -scsDOS -- %%A @%%LQMN"),
/*Comment archive       */_T(""),
/*Comment files         */_T(""),
/*Convert to SFX        */_T(""),
/*Lock archive          */_T(""),
/*Protect archive       */_T(""),
/*Recover archive       */_T(""),
/*Add files             */_T("7z a -r0 -y -ttar {-w%%W} {%%S} -scsDOS -- %%A @%%LQMN"),
};

static const TCHAR *pGZIP[]={
/*Extract               */_T("7z x -r0 -y {-w%%W} -scsDOS -- %%A @%%LQMN"),
/*Extract without paths */_T("7z e -r0 -y {-w%%W} -scsDOS -- %%A @%%LQMN"),
/*Test                  */_T("7z t -r0 -scsDOS -- %%A @%%LQMN"),
/*Delete                */_T("7z d -r0 {-w%%W} -scsDOS -- %%A @%%LQMN"),
/*Comment archive       */_T(""),
/*Comment files         */_T(""),
/*Convert to SFX        */_T(""),
/*Lock archive          */_T(""),
/*Protect archive       */_T(""),
/*Recover archive       */_T(""),
/*Add files             */_T("7z a -r0 -tgzip {-w%%W} {%%S} -scsDOS -- %%A @%%LQMN"),
};

static const TCHAR *pBZIP[]={
/*Extract               */_T("7z x -r0 -y {-w%%W} -scsDOS -- %%A @%%LQMN"),
/*Extract without paths */_T("7z e -r0 -y {-w%%W} -scsDOS -- %%A @%%LQMN"),
/*Test                  */_T("7z t -r0 -scsDOS -- %%A @%%LQMN"),
/*Delete                */_T("7z d -r0 {-w%%W} -scsDOS -- %%A @%%LQMN"),
/*Comment archive       */_T(""),
/*Comment files         */_T(""),
/*Convert to SFX        */_T(""),
/*Lock archive          */_T(""),
/*Protect archive       */_T(""),
/*Recover archive       */_T(""),
/*Add files             */_T("7z a -r0 -tbzip2 {-w%%W} {%%S} -scsDOS -- %%A @%%LQMN"),
};

static const TCHAR *pZ[]={
/*Extract               */_T("7z x -r0 -y {-w%%W} -scsDOS -- %%A @%%LQMN"),
/*Extract without paths */_T("7z e -r0 -y {-w%%W} -scsDOS -- %%A @%%LQMN"),
/*Test                  */_T("7z t -r0 -scsDOS -- %%A @%%LQMN"),
/*Delete                */_T(""),
/*Comment archive       */_T(""),
/*Comment files         */_T(""),
/*Convert to SFX        */_T(""),
/*Lock archive          */_T(""),
/*Protect archive       */_T(""),
/*Recover archive       */_T(""),
/*Add files             */_T(""),
};

static const TCHAR *pCPIO[]={
/*Extract               */_T("7z x {-p%%P} -r0 -y -scsDOS -- %%A @%%LQMN"),
/*Extract without paths */_T("7z e {-p%%P} -r0 -y -scsDOS -- %%A @%%LQMN"),
/*Test                  */_T("7z t {-p%%P} -r0 -scsDOS -- %%A @%%LQMN"),
/*Delete                */_T(""),
/*Comment archive       */_T(""),
/*Comment files         */_T(""),
/*Convert to SFX        */_T(""),
/*Lock archive          */_T(""),
/*Protect archive       */_T(""),
/*Recover archive       */_T(""),
/*Add files             */_T("")
};

static const TCHAR *pDEB[]={
/*Extract               */_T("7z x {-p%%P} -r0 -y -scsDOS -- %%A @%%LQMN"),
/*Extract without paths */_T("7z e {-p%%P} -r0 -y -scsDOS -- %%A @%%LQMN"),
/*Test                  */_T("7z t {-p%%P} -r0 -scsDOS -- %%A @%%LQMN"),
/*Delete                */_T(""),
/*Comment archive       */_T(""),
/*Comment files         */_T(""),
/*Convert to SFX        */_T(""),
/*Lock archive          */_T(""),
/*Protect archive       */_T(""),
/*Recover archive       */_T(""),
/*Add files             */_T("")
};

static const TCHAR *pRPM[]={
/*Extract               */_T("7z x {-p%%P} -r0 -y -scsDOS -- %%A @%%LQMN"),
/*Extract without paths */_T("7z e {-p%%P} -r0 -y -scsDOS -- %%A @%%LQMN"),
/*Test                  */_T("7z t {-p%%P} -r0 -scsDOS -- %%A @%%LQMN"),
/*Delete                */_T(""),
/*Comment archive       */_T(""),
/*Comment files         */_T(""),
/*Convert to SFX        */_T(""),
/*Lock archive          */_T(""),
/*Protect archive       */_T(""),
/*Recover archive       */_T(""),
/*Add files             */_T(""),
};

static const TCHAR *pCAB[]={
/*Extract               */_T("MsCab -i0 -FAR {-ap%%R} {-p%%P} {%%S} x %%A @%%LMA"),
/*Extract without paths */_T("MsCab -i0 -FAR {-p%%P} {%%S} e %%A @%%LMA"),
/*Test                  */_T("MsCab -i0 {-p%%P} {%%S} t %%A"),
/*Delete                */_T("MsCab -i0 -FAR {-p%%P} {%%S} d %%A @%%LMA"),
/*Comment archive       */_T(""),
/*Comment files         */_T(""),
/*Convert to SFX        */_T("MsCab {%%S} s %%A"),
/*Lock archive          */_T(""),
/*Protect archive       */_T(""),
/*Recover archive       */_T(""),
/*Add files             */_T("MsCab -i0 -dirs {-ap%%R} {-p%%P} {%%S} a %%A @%%LNMA"),
};

static const TCHAR *pLZH[]={
/*Extract               */_T("lha x -a -c -d -m {-w%%W} %%a @%%lM"),
/*Extract without paths */_T("lha e -a -c -m {-w%%W} %%a @%%lM"),
/*Test                  */_T("lha t -r2 -a -m {-w%%W} %%a"),
/*Delete                */_T("lha d -r2 -a -m {-w%%W} %%a @%%lM"),
/*Comment archive       */_T(""),
/*Comment files         */_T(""),
/*Convert to SFX        */_T("lha s -x1 -a -m {-w%%W} %%a"),
/*Lock archive          */_T(""),
/*Protect archive       */_T(""),
/*Recover archive       */_T(""),
/*Add files             */_T("lha a -a -m {-w%%W} %%a @%%lM"),
};
