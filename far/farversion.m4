m4_divert(`-1')

# Uncomment to create a special version
#m4_define(`SPECIAL_VERSION', epic feature)

m4_define(VERSION_MAJOR, 3)
m4_define(VERSION_MINOR, 0)
m4_define(VERSION_REVISION, 0)
m4_define(VERSION_BUILD, m4_patsubst(m4_include(`vbuild.m4'),`
',`'))

m4_define(`VERSION_TYPE', m4_ifdef(`SPECIAL_VERSION', VS_SPECIAL, m4_ifelse(BUILD_TYPE, `', VS_PRIVATE, BUILD_TYPE)))

m4_define(BUILD_SPECIAL, m4_ifelse(VERSION_TYPE, VS_SPECIAL, 1, 0))
m4_define(BUILD_PRIVATE, m4_ifelse(VERSION_TYPE, VS_PRIVATE, 1, 0))
m4_define(BUILD_PRERELEASE, m4_ifelse(
	VERSION_TYPE, VS_ALPHA,   1,
	VERSION_TYPE, VS_PRIVATE, 1,
	VERSION_TYPE, VS_SPECIAL, 1,
	0))

m4_include(`tools.m4')

m4_define(BUILD_PLATFORM, m4_ifelse(
	FARBIT, 64, x64,
	FARBIT, 32, x86,
	FARBIT))

m4_define(BUILD_SCM_REVISION, m4_ifelse(SCM_REVISION, `', m4_patsubst(m4_esyscmd(git rev-parse HEAD 2>nul),`
',`'), SCM_REVISION))

m4_define(BUILD_DATE, m4_esyscmd(CMDAWK -f ./scripts/gendate.awk))
m4_define(BUILD_YEAR, m4_substr(BUILD_DATE,6,4))
m4_define(BUILD_MONTH, m4_substr(BUILD_DATE,3,2))
m4_define(BUILD_DAY, m4_substr(BUILD_DATE,0,2))
m4_define(COPYRIGHTYEAR, BUILD_YEAR)

m4_define(FULLVERSION, VERSION_MAJOR.VERSION_MINOR.VERSION_BUILD.VERSION_REVISION`'m4_ifelse(
	VERSION_TYPE, VS_SPECIAL, ` (Special: 'SPECIAL_VERSION`)',
	VERSION_TYPE, VS_PRIVATE, ` (Private)',
	VERSION_TYPE, VS_ALPHA,   ` (Alpha)',
	VERSION_TYPE, VS_BETA,    ` (Beta)',
	VERSION_TYPE, VS_RC,      ` (RC)',
	VERSION_TYPE, VS_RELEASE, `',
	` (Unknown)') BUILD_PLATFORM)

m4_divert(0)m4_dnl
