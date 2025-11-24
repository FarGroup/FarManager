# Far Manager - Copilot Coding Agent Instructions

## Repository Overview

**Far Manager** is a file and archive manager for Windows operating systems that works in text mode. It provides a simple and intuitive interface for file operations including viewing, editing, copying, renaming files and directories. The project is written primarily in C++ and supports plugins through a special Plugin API to extend functionality.

## Repository Statistics

- **Size**: ~77 MB
- **Primary Language**: C++ (1,553 source files: .cpp, .hpp, .c, .h)
- **Build Systems**: Visual Studio (56 .sln files), nmake, mingw32-make
- **Supported Platforms**: Windows (x86, x64, ARM64)
- **Supported Compilers**: MSVC (cl), GCC (MinGW), Clang
- **Runtimes**: Windows native (no external runtime dependencies)
- **Build Configurations**: Debug, Release

## Project Structure

### Root Directory Layout
```
/
├── .github/               # GitHub workflows and CI/CD configuration
│   └── workflows/         # Build and validation workflows
├── _build/                # Build configuration and solution files
│   └── vc/                # Visual Studio configuration
│       ├── all.sln        # Main solution file for VS builds
│       └── config/        # Common build configuration (common.mak)
├── far/                   # Main Far Manager source code
│   ├── far.sln            # Far-specific VS solution
│   ├── far.vcxproj        # Far VS project file
│   ├── makefile_vc        # nmake makefile for MSVC
│   ├── makefile_gcc       # makefile for GCC/MinGW
│   ├── makefile_gcc_common # Common GCC build configuration
│   ├── build.sh           # Build script for Unix-like environments
│   ├── tools/             # Build tools (m4, gawk, lng.generator, Python scripts)
│   ├── common/            # Common utility code
│   ├── platform.sdk/      # Platform-specific SDK wrappers
│   ├── scripts/           # Build and utility scripts
│   └── thirdparty/        # Third-party dependencies (uchardet, sqlite)
├── plugins/               # Plugin source code (20+ plugins)
│   ├── makefile_all_vc    # Build all plugins with MSVC
│   └── makefile_all_gcc   # Build all plugins with GCC
├── misc/                  # Miscellaneous tools and installers
│   ├── build-checks/      # .NET-based validation tools
│   │   ├── ChangelogChecker/ # Validates changelog format
│   │   └── HlfChecker/       # Validates help file format
│   ├── msi-installer/     # MSI installer builder (requires WiX)
│   └── lng/               # Language file generator (lng.generator.py)
├── enc/                   # Documentation and API reference
│   ├── tools/             # CHM documentation builder tools
│   └── README.md          # Documentation build instructions
└── extra/                 # Additional files for distribution
```

### Key Configuration Files

- **`.editorconfig`**: Code style settings (tabs for indentation, UTF-8 with BOM for C/C++)
- **`far/farversion.m4`**: Version configuration (Major: 3, Minor: 0)
- **`far/vbuild.m4`**: Build number (auto-incremented)
- **`far/changelog`**: Change history (required format, checked by ChangelogChecker)
- **`far/farlang.templ.m4`**: Language file template (NEVER edit .lng files directly)

## Build Instructions

### Prerequisites

**ALWAYS verify build prerequisites before building:**

#### For Windows with Visual Studio (MSVC):
- Visual Studio 2019 or 2022 with C++ toolchain
- Windows SDK (automatically installed with VS)
- Optional: Python 3.x (for language file generation with `PYTHON=1`)

#### For MinGW/GCC (Windows or Cross-compile from Linux):
- MinGW-w64 toolchain (mingw32-make, gcc, g++)
- For cross-compile on Linux: `i686-w64-mingw32-gcc` (32-bit) or `x86_64-w64-mingw32-gcc` (64-bit)
- Optional: Python 3.x (for language file generation)

#### For Validation Tools:
- .NET 6.0 SDK (for ChangelogChecker and HlfChecker in `misc/build-checks/`)
  - **NOTE**: CI uses .NET 6.0, but .NET 8.0+ can also work
- Python 3.x (for source_validator.py)

### Building Far Manager

**IMPORTANT**: ALWAYS build from the appropriate directory. Far and plugins are built separately.

#### Method 1: Visual Studio (msbuild)

```bash
# Build Far only
cd _build/vc
msbuild /property:Configuration=Release /property:Platform=x64 all.sln

# Or build from far/ directory
cd far
msbuild /property:Configuration=Release /property:Platform=x64 far.vcxproj
```

**Platforms**: `Win32` (x86), `x64`, `ARM64`  
**Configurations**: `Debug`, `Release`

#### Method 2: nmake (MSVC command-line)

**CRITICAL**: ALWAYS set MSVC environment variables first using `vcvarsall.bat`:

```bash
# Set environment for x64 build
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" amd64

# Build Far
cd far
nmake /f makefile_vc

# Build plugins
cd plugins
nmake /f makefile_all_vc
```

**Environment variables for nmake:**
- `DEBUG=1` - Build debug version
- `AMD64=1` or `CPU=AMD64` - Build x64 (auto-detected if vcvarsall was run)
- `ARM64=1` or `CPU=ARM64` - Build ARM64
- `CLANG=1` - Use clang-cl instead of cl
- `PYTHON=1` - Use Python for language file generation

#### Method 3: GCC/MinGW

**For Windows (MSYS2/MinGW):**
```bash
cd far
mingw32-make -j 4 -f makefile_gcc

cd ../plugins
mingw32-make -j 4 -f makefile_all_gcc
```

**For Linux cross-compilation:**
```bash
cd far
# 64-bit build
GCC_PREFIX=x86_64-w64-mingw32- make -j 4 -f makefile_gcc

# 32-bit build  
GCC_PREFIX=i686-w64-mingw32- make -j 4 -f makefile_gcc DIRBIT=32
```

**Environment variables for GCC:**
- `DEBUG=1` - Build debug version
- `CLANG=1` - Use clang instead of gcc
- `PYTHON=1` - Use Python for language file generation
- `DIRBIT=32` or `DIRBIT=64` - Specify bitness
- `GCC_PREFIX=<prefix>-` - Compiler prefix for cross-compilation

**Using build.sh helper script:**
```bash
cd far
./build.sh            # Build both 32 and 64-bit
./build.sh 64         # Build 64-bit only
./build.sh clean 64   # Clean and rebuild 64-bit
./build.sh debug 64   # Build 64-bit debug
```

### Build Output Locations

- **nmake**: `far/<config>.<platform>.vc/Far.exe`
- **GCC**: `far/<dirname>.<bitness>.<suffix>/Far.exe`
- **msbuild**: `_build/vc/_output/product/<config>.<platform>/Far.exe`

## Validation and CI Checks

**ALWAYS run these validation steps before submitting code:**

### 1. Code Style Validation

```bash
cd far
python tools/source_validator.py
```

**What it checks:**
- UTF-8 BOM presence in C/C++ files
- Correct include guards in headers
- Self-include as first include in .cpp files
- License header format
- Tab formatting (tabs for indentation, no tabs in middle of lines)
- No trailing whitespace
- Final newline in files

**Common Issues:**
- Missing UTF-8 BOM: Add BOM to .cpp/.hpp/.c/.h files
- Include guards: Must follow format `#ifndef FILENAME_EXT_<UUID>` and include `#pragma once`
- Self-include: .cpp files must include their corresponding .hpp first

### 2. Changelog Validation

**NOTE**: Requires .NET 6.0 SDK (CI uses this version)

```bash
cd misc/build-checks/ChangelogChecker
dotnet run --project ./ChangelogChecker.csproj --configuration Release --property UseSharedCompilation=false
```

**Changelog format** (in `far/changelog`):
```
--------------------------------------------------------------------------------
username YYYY-MM-DD HH:MM:SS+TZ:TZ - build NNNN

1. Description of change.
2. Another change.
```

**Important**: Recent changes go on top. Use `misc/changelog/ChangelogHeader.lua` macro to generate headers.

### 3. Help File (.hlf) Validation

```bash
cd misc/build-checks/HlfChecker
dotnet run --project ./HlfChecker.csproj --configuration Release --property UseSharedCompilation=false -- Verbose
```

### 4. Build Verification

**ALWAYS test your changes with both Debug and Release builds when possible.**

For critical changes, test across multiple compilers:
- MSVC cl
- Clang (with `CLANG=1`)
- GCC/MinGW

## Continuous Integration (CI/CD)

The repository has three CI systems that run on pull requests:

### GitHub Actions (.github/workflows/build.yml)

**Triggers**: Push to master, pull requests, manual dispatch

**Jobs:**
1. **code-style-checks** (Ubuntu): Runs `python tools/source_validator.py` in `far/`
2. **changelog-hlf-checks** (Ubuntu): 
   - Builds and runs ChangelogChecker (.NET 6.0)
   - Builds and runs HlfChecker (.NET 6.0)
3. **build-msvc** (Windows): Matrix build with msbuild and nmake
   - Platforms: x86, x64, ARM64
   - Configurations: Debug, Release
   - Compilers: cl, clang-cl
   - CodeQL analysis on x64 Debug builds
4. **build-msys2** (Windows): GCC and Clang builds via MSYS2
5. **build-llvm-mingw** (Ubuntu): Cross-compilation with llvm-mingw

### AppVeyor (appveyor.yml, appveyor-gcc.yml, appveyor-clang.yml)

- Builds release and debug for x86, x64, ARM64
- Generates MSI installers (requires WiX)
- Downloads and includes NetBox and FarColorer plugins
- Runs macro tests (`Far.exe -service "macro:test"`)
- Creates distribution archives

### Azure Pipelines (azure-pipelines.yml)

- Uses `_build/vc/all.sln`
- Builds all platforms and configurations

**IMPORTANT**: All three CI systems must pass for PRs to be merged.

## Common Build Issues and Workarounds

### Issue: Language file generation errors
**Cause**: Missing or incorrect m4/gawk tools  
**Solution**: Use `PYTHON=1` to use Python-based generator instead:
```bash
nmake /f makefile_vc PYTHON=1
# or
make -f makefile_gcc PYTHON=1
```

### Issue: Missing .NET 6.0 for build-checks
**Cause**: Only .NET 8+ installed  
**Workaround**: Build-checks can run on .NET 8, but CI uses 6.0. For local testing, upgrade the target framework in .csproj files or install .NET 6.0 SDK.

### Issue: Debug build linking errors with ucrtbased
**Cause**: Missing debug C runtime  
**Solution**: Add `USER_LIBS=-lucrtbased` for UCRT-based builds:
```bash
make -f makefile_gcc DEBUG=1 USER_LIBS=-lucrtbased
```

### Issue: nmake fails with "command not found"
**Cause**: MSVC environment not initialized  
**Solution**: ALWAYS run vcvarsall.bat first:
```bash
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" amd64
```

### Issue: Build times out on slow systems
**Cause**: Full rebuild with dependencies takes 3-10+ minutes  
**Solution**: 
- Use incremental builds (don't clean unless necessary)
- Use parallel builds: `make -j N` or `msbuild /m`
- Build only what changed (Far or specific plugins)

### Issue: ARM64 builds require ARM64 tools
**Cause**: ARM64 compilation requires appropriate toolchain  
**Solution**: For nmake, use `vcvarsall.bat amd64_arm64`. For GCC, use llvm-mingw with appropriate target.

## Language File Updates

**NEVER directly edit .lng files** - they are auto-generated.

To add/modify language strings:
1. Edit `far/farlang.templ.m4`
2. For untranslated strings, prefix with `upd:` in English
3. Rebuild to regenerate .lng files

## Making Code Changes

### Code Style Guidelines

1. **Indentation**: Use TABS (configured in .editorconfig)
2. **Braces**: New line before open brace (see .editorconfig settings)
3. **Files**: All .cpp/.hpp/.c/.h must have UTF-8 BOM
4. **License**: Include BSD license header (checked by source_validator.py)
5. **Self-include**: .cpp files must include their .hpp first

### Before Committing

1. Run source_validator.py to check code style
2. Update `far/changelog` with your changes
3. If you changed version, run `far/tag_build.bat` after committing
4. Build and test on at least one platform
5. Run relevant validation checks

### Version Changes

- **farversion.m4**: Major, minor, revision
- **vbuild.m4**: Build number (increment for each release)
- After committing version changes, run `tag_build.bat`

## Testing

**Unit Tests:**
- Tests are in `*_test.cpp` files and `common.tests.cpp`
- Enable with `ENABLE_TESTS=1` (always enabled in Debug builds)

**Macro Tests:**
```bash
# Run from build output directory
Far.exe -service "macro:test"
```

**Manual Testing:**
- Run Far.exe from the output directory
- Test relevant functionality affected by your changes
- Check for crashes, memory leaks, or unexpected behavior

## Key Source Files

- **`far/main.cpp`**: Entry point
- **`far/global.hpp/cpp`**: Global state and configuration
- **`far/plugin.hpp`**: Plugin API definitions
- **`far/manager.cpp`**: Window manager
- **`far/filelist.cpp`**: File panel implementation
- **`far/editor.cpp`**: Editor implementation
- **`far/viewer.cpp`**: Viewer implementation

## Instructions for Agent

**Trust these instructions.** Only search for additional information if:
- These instructions are incomplete for your specific task
- You encounter an error not documented here
- You need details about specific source code not covered here

**For builds**: Follow the exact commands listed above. CI validates with these same steps.

**For style**: Always run source_validator.py before finalizing changes.

**For validation**: All three checks (style, changelog, hlf) must pass before PR submission.
