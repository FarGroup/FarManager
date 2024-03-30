.SILENT:

!if !defined(VC)
!if "$(_NMAKE_VER)">="14.30"
VC = 17
!else
#default
VC = 16
!endif
!endif

# Toolchain setup
LINK = link.exe
# Toolchain setup end

# Output directory setup
!ifndef DEBUG
DIRNAME = Release
!else
DIRNAME = Debug
!endif

!if defined(AMD64) || "$(CPU)" == "AMD64" || "$(PLATFORM)" == "X64" || "$(PLATFORM)" == "x64"
!undef CPU
BUILD_PLATFORM = AMD64
DIRBIT = 64
!elseif defined(ARM64) || "$(CPU)" == "ARM64" || "$(PLATFORM)" == "arm64"
!undef CPU
BUILD_PLATFORM = ARM64
DIRBIT = ARM64
!elseif defined(ARM) || "$(CPU)" == "ARM" || "$(PLATFORM)" == "arm"
!undef CPU
BUILD_PLATFORM = ARM
DIRBIT = ARM
!else
BUILD_PLATFORM = X86
DIRBIT = 32
!endif

OUTDIR=$(DIRNAME).$(DIRBIT).vc
INTDIR=$(DIRNAME).$(DIRBIT).vc/obj
# Output directory setup end

!ifndef FARDIR
FARDIR=$(ROOTDIR)\far
!endif

# Main flags setup
CFLAGS = $(CFLAGS)\
	/nologo\
	/c\
	/J\
	/permissive-\
	/volatile:iso\
	/Wall\
	/we4013\
	/wd4464\
	/wd4668\
	/utf-8\
	/Gy\
	/Gw\
	/GF\
	/Fd"$(INTDIR)/"\
	/Fo"$(INTDIR)/"\
	/diagnostics:caret\
	/MP$(MP_LIMIT)\
	/FI$(FARDIR)\disabled_warnings.hpp\
	/Zi\
	/D "NOMINMAX"\
	/D "WIN32_LEAN_AND_MEAN"\
	/D "VC_EXTRALEAN"\
	/D "PSAPI_VERSION=1"\
	/D "_CRT_SECURE_NO_WARNINGS"\

!if "$(VC)">="17"
CFLAGS = $(CFLAGS)\
	/Zc:__STDC__,enumTypes,templateScope\

!endif

!ifndef ANSI
CFLAGS = $(CFLAGS)\
	/D "UNICODE"\
	/D "_UNICODE"\

!endif

CPPFLAGS = $(CPPFLAGS)\
	$(CFLAGS)\
	/EHsc\
	/std:c++latest\
	/Zc:__cplusplus,externConstexpr,inline,preprocessor,throwingNew\
	/D "_ENABLE_EXTENDED_ALIGNED_STORAGE"\
	/D "_CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES=1"\
	/D "_CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES_COUNT=1"\
	/D "MICROSOFT_WINDOWS_WINBASE_H_DEFINE_INTERLOCKED_CPLUSPLUS_OVERLOADS=1"\

AFLAGS =\
	/nologo\
	/c\
	/Fo"$(INTDIR)/"\

RFLAGS = $(RFLAGS)\
	/nologo\
	/l 0x409\

LINKFLAGS = $(LINKFLAGS)\
	/nologo\
	/subsystem:console\
	/release\
	/debug\
	/nxcompat\
	/largeaddressaware\
	/dynamicbase\
	/map\
	/merge:_RDATA=.rdata

ULINKFLAGS = $(ULINKFLAGS) -q -m- -ap -Gz -O- -o- -Gh -Gh- -b* \
             -GF:NXCOMPAT -GF:LARGEADDRESSAWARE
!if "$(DIRBIT)"=="64"
ULINKFLAGS = $(ULINKFLAGS) -GM:_RDATA=.rdata
!endif

# Configuration-specific flags
!ifdef DEBUG
# Debug mode
CFLAGS = $(CFLAGS) /MTd /Od /D "_DEBUG"
RFLAGS = $(RFLAGS) /D "_DEBUG"
LINKFLAGS = $(LINKFLAGS) /debug
ULINKFLAGS = $(ULINKFLAGS) -v
!else # DEBUG
# Release mode
CFLAGS = $(CFLAGS) /MT /O2 /D "NDEBUG"
RFLAGS = $(RFLAGS) /D "NDEBUG"
LINKFLAGS = $(LINKFLAGS) /incremental:no /OPT:REF /OPT:ICF /pdbaltpath:%_PDB%

!ifndef NO_RELEASE_LTCG
CFLAGS = $(CFLAGS) /GL
LINKFLAGS = $(LINKFLAGS) /ltcg
!ifdef LTCG_STATUS
LINKFLAGS = $(LINKFLAGS) /ltcg:status
!endif
!endif # NO_RELEASE_LTCG
!endif # DEBUG

!ifdef USE_ANALYZE
CFLAGS = $(CFLAGS) /analyze
!endif
# Configuration-specific flags end

# Platform-specific flags
!if "$(BUILD_PLATFORM)" == "X86"
CFLAGS = $(CFLAGS) /arch:IA32
!ifndef DEBUG
CFLAGS = $(CFLAGS) /Oy-
LINKFLAGS = $(LINKFLAGS) /safeseh
ULINKFLAGS = $(ULINKFLAGS) -RS
!endif # DEBUG
LINKFLAGS = $(LINKFLAGS) /machine:i386
ULINKFLAGS = $(ULINKFLAGS) -W5.1 -V5.1
OS_VERSION = 5.0
MASM = ml
!elseif "$(BUILD_PLATFORM)" == "AMD64"
LINKFLAGS = $(LINKFLAGS) /machine:amd64
ULINKFLAGS = $(ULINKFLAGS) -V5.2 -W5.2
OS_VERSION = 5.2
MASM = ml64
AFLAGS=$(AFLAGS) /D "X64"
!elseif "$(BUILD_PLATFORM)" == "ARM"
LINKFLAGS=$(LINKFLAGS) /machine:ARM
!elseif defined(ARM64)
LINKFLAGS = $(LINKFLAGS) /machine:ARM64
!endif
# Platform-specific flags end

# Compiler-specific flags
!ifdef CLANG
CC = clang-cl
CPP = clang-cl
CFLAGS = $(CFLAGS)\
	-Qunused-arguments\
	/clang:-fvisibility=hidden\
	-Weverything\
	-Werror=array-bounds\
	-Werror=dangling\
	-Werror=odr\
	-Werror=return-type\

CPPFLAGS = $(CPPFLAGS)\
	-Werror=old-style-cast\
	-Werror=reorder\

NOBATCH = 1
NO_RELEASE_LTCG = 1
!endif
# Compiler-specific flags end

LINK_LIBS =\
	kernel32.lib\
	advapi32.lib\
	user32.lib\
	shell32.lib\
	netapi32.lib\
	winspool.lib\
	mpr.lib\
	ole32.lib\
	oleaut32.lib\
	psapi.lib\
	secur32.lib\
	setupapi.lib\
	rpcrt4.lib\
	version.lib\
	userenv.lib\
	comdlg32.lib\
	imm32.lib\
	wbemuuid.lib\

