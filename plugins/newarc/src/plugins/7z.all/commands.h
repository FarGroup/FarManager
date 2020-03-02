static const char *pRAR[]={
/*Extract               */"rar x {-p%%P} {-ap%%R} -y -c- -kb -- %%A @%%LNM",
/*Extract without paths */"rar e {-p%%P} -y -c- -kb -- %%A @%%LNM",
/*Test                  */"rar t -y {-p%%P} -- %%A",
/*Delete                */"rar d -y {-p%%P} {-w%%W} -- %%A @%%LNM",
/*Comment archive       */"rar c -y {-w%%W} -- %%A",
/*Comment files         */"rar cf -y {-w%%W} -- %%A @%%LNM",
/*Convert to SFX        */"rar s -y -- %%A",
/*Lock archive          */"rar k -y -- %%A",
/*Protect archive       */"rar rr -y -- %%A",
/*Recover archive       */"rar r -y -- %%A",
/*Add files             */"rar a -y {-p%%P} {-ap%%R} {-w%%W} {%%S} -- %%A @%%LNM"
};

static const char *p7Z[]={
/*Extract               */"7z x {-p%%P} -r0 -y -scsDOS -- %%A @%%LQMN",
/*Extract without paths */"7z e {-p%%P} -r0 -y -scsDOS -- %%A @%%LQMN",
/*Test                  */"7z t {-p%%P} -r0 -scsDOS -- %%A @%%LQMN",
/*Delete                */"7z d {-p%%P} -r0 -ms=off -scsDOS -- %%A @%%LQMN",
/*Comment archive       */"",
/*Comment files         */"",
/*Convert to SFX        */"",
/*Lock archive          */"",
/*Protect archive       */"",
/*Recover archive       */"",
/*Add files             */"7z a {-p%%P} -r0 -t7z {%%S} -scsDOS -- %%A @%%LQMN"
};

static const char *pARJ[]={
/*Extract               */"arj32 x -+ {-g%%P} -v -y -p1 -- %%A !%%LM",
/*Extract without paths */"arj32 e -+ {-g%%P} -v -y -p1 -- %%A !%%LM",
/*Test                  */"arj32 t -+ -y {-g%%P} -v -p1 -- %%A",
/*Delete                */"arj32 d -+ -y {-w%%W} -p1 -- %%A !%%LNM",
/*Comment archive       */"arj32 c -+ -y {-w%%W} -z -- %%A",
/*Comment files         */"arj32 c -+ -y {-w%%W} -p1 -- %%A !%%LM",
/*Convert to SFX        */"arj32 y -+ -je -y -p %%A",
/*Lock archive          */"",
/*Protect archive       */"arj32 t -hk -y %%A",
/*Recover archive       */"arj32 q -y %%A",
/*Add files             */"arj32 a -+ -y -a1 {-g%%P} {-w%%W} -p {%%S} -- %%A !%%LM",
};

static const char *pZIP[]={
/*Extract               */"7z x {-p%%P} -r0 -y {-w%%W} -scsDOS -- %%A @%%LQMN",
/*Extract without paths */"7z e {-p%%P} -r0 -y {-w%%W} -scsDOS -- %%A @%%LQMN",
/*Test                  */"7z t {-p%%P} -r0 -scsDOS -- %%A @%%LQMN",
/*Delete                */"7z d {-p%%P} -r0 {-w%%W} -scsDOS -- %%A @%%LQMN",
/*Comment archive       */"",
/*Comment files         */"",
/*Convert to SFX        */"",
/*Lock archive          */"",
/*Protect archive       */"",
/*Recover archive       */"",
/*Add files             */"7z a {-p%%P} -r0 -tzip {-w%%W} {%%S} -scsDOS -- %%A @%%LQMN",
};

static const char *pTAR[]={
/*Extract               */"7z x -r0 -y {-w%%W} -scsDOS -- %%A @%%LQMN",
/*Extract without paths */"7z e -r0 -y {-w%%W} -scsDOS -- %%A @%%LQMN",
/*Test                  */"7z t -r0 -scsDOS -- %%A @%%LQMN",
/*Delete                */"7z d -r0 {-w%%W} -scsDOS -- %%A @%%LQMN",
/*Comment archive       */"",
/*Comment files         */"",
/*Convert to SFX        */"",
/*Lock archive          */"",
/*Protect archive       */"",
/*Recover archive       */"",
/*Add files             */"7z a -r0 -y -ttar {-w%%W} {%%S} -scsDOS -- %%A @%%LQMN",
};

static const char *pGZIP[]={
/*Extract               */"7z x -r0 -y {-w%%W} -scsDOS -- %%A @%%LQMN",
/*Extract without paths */"7z e -r0 -y {-w%%W} -scsDOS -- %%A @%%LQMN",
/*Test                  */"7z t -r0 -scsDOS -- %%A @%%LQMN",
/*Delete                */"7z d -r0 {-w%%W} -scsDOS -- %%A @%%LQMN",
/*Comment archive       */"",
/*Comment files         */"",
/*Convert to SFX        */"",
/*Lock archive          */"",
/*Protect archive       */"",
/*Recover archive       */"",
/*Add files             */"7z a -r0 -tgzip {-w%%W} {%%S} -scsDOS -- %%A @%%LQMN",
};

static const char *pBZIP[]={
/*Extract               */"7z x -r0 -y {-w%%W} -scsDOS -- %%A @%%LQMN",
/*Extract without paths */"7z e -r0 -y {-w%%W} -scsDOS -- %%A @%%LQMN",
/*Test                  */"7z t -r0 -scsDOS -- %%A @%%LQMN",
/*Delete                */"7z d -r0 {-w%%W} -scsDOS -- %%A @%%LQMN",
/*Comment archive       */"",
/*Comment files         */"",
/*Convert to SFX        */"",
/*Lock archive          */"",
/*Protect archive       */"",
/*Recover archive       */"",
/*Add files             */"7z a -r0 -tbzip2 {-w%%W} {%%S} -scsDOS -- %%A @%%LQMN",
};

static const char *pZ[]={
/*Extract               */"7z x -r0 -y {-w%%W} -scsDOS -- %%A @%%LQMN",
/*Extract without paths */"7z e -r0 -y {-w%%W} -scsDOS -- %%A @%%LQMN",
/*Test                  */"7z t -r0 -scsDOS -- %%A @%%LQMN",
/*Delete                */"",
/*Comment archive       */"",
/*Comment files         */"",
/*Convert to SFX        */"",
/*Lock archive          */"",
/*Protect archive       */"",
/*Recover archive       */"",
/*Add files             */"",
};

static const char *pCPIO[]={
/*Extract               */"7z x {-p%%P} -r0 -y -scsDOS -- %%A @%%LQMN",
/*Extract without paths */"7z e {-p%%P} -r0 -y -scsDOS -- %%A @%%LQMN",
/*Test                  */"7z t {-p%%P} -r0 -scsDOS -- %%A @%%LQMN",
/*Delete                */"",
/*Comment archive       */"",
/*Comment files         */"",
/*Convert to SFX        */"",
/*Lock archive          */"",
/*Protect archive       */"",
/*Recover archive       */"",
/*Add files             */"",
};

static const char *pDEB[]={
/*Extract               */"7z x {-p%%P} -r0 -y -scsDOS -- %%A @%%LQMN",
/*Extract without paths */"7z e {-p%%P} -r0 -y -scsDOS -- %%A @%%LQMN",
/*Test                  */"7z t {-p%%P} -r0 -scsDOS -- %%A @%%LQMN",
/*Delete                */"",
/*Comment archive       */"",
/*Comment files         */"",
/*Convert to SFX        */"",
/*Lock archive          */"",
/*Protect archive       */"",
/*Recover archive       */"",
/*Add files             */"",
};

static const char *pRPM[]={
/*Extract               */"7z x {-p%%P} -r0 -y -scsDOS -- %%A @%%LQMN",
/*Extract without paths */"7z e {-p%%P} -r0 -y -scsDOS -- %%A @%%LQMN",
/*Test                  */"7z t {-p%%P} -r0 -scsDOS -- %%A @%%LQMN",
/*Delete                */"",
/*Comment archive       */"",
/*Comment files         */"",
/*Convert to SFX        */"",
/*Lock archive          */"",
/*Protect archive       */"",
/*Recover archive       */"",
/*Add files             */"",
};

static const char *pCAB[]={
/*Extract               */"MsCab -i0 -FAR {-ap%%R} {-p%%P} {%%S} x %%A @%%LMA",
/*Extract without paths */"MsCab -i0 -FAR {-p%%P} {%%S} e %%A @%%LMA",
/*Test                  */"MsCab -i0 {-p%%P} {%%S} t %%A",
/*Delete                */"MsCab -i0 -FAR {-p%%P} {%%S} d %%A @%%LMA",
/*Comment archive       */"",
/*Comment files         */"",
/*Convert to SFX        */"MsCab {%%S} s %%A",
/*Lock archive          */"",
/*Protect archive       */"",
/*Recover archive       */"",
/*Add files             */"MsCab -i0 -dirs {-ap%%R} {-p%%P} {%%S} a %%A @%%LNMA",
};

static const char *pLZH[]={
/*Extract               */"lha x -a -c -d -m {-w%%W} %%a @%%lM",
/*Extract without paths */"lha e -a -c -m {-w%%W} %%a @%%lM",
/*Test                  */"lha t -r2 -a -m {-w%%W} %%a",
/*Delete                */"lha d -r2 -a -m {-w%%W} %%a @%%lM",
/*Comment archive       */"",
/*Comment files         */"",
/*Convert to SFX        */"lha s -x1 -a -m {-w%%W} %%a",
/*Lock archive          */"",
/*Protect archive       */"",
/*Recover archive       */"",
/*Add files             */"lha a -a -m {-w%%W} %%a @%%lM",
};
