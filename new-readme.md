# Far Manager (Extended Overview)

| [Polski](new-readme-pl.md) | [Русский](README-RU.md) | [Short Version](README.md) |
| - | - | - |

[![Header][logo-img]][logo-url]

| | AppVeyor | Azure |
| - | - | - |
| VS | [![AppVeyor][VS-AppVeyor-img]][VS-AppVeyor-url] | [![Azure Pipelines][VS-Azure-img]][VS-Azure-url] |
| GCC | [![AppVeyor][GCC-AppVeyor-img]][GCC-AppVeyor-url] | TBD |
| Clang | [![AppVeyor][Clang-AppVeyor-img]][Clang-AppVeyor-url] | TBD |

## 1. What Is Far Manager?

Far Manager is a text‑mode file and archive manager for Windows. It focuses on speed, keyboard efficiency and extensibility through a powerful plugin system. Out of the box it lets you:

- Browse and manage files, folders and archives
- View and edit text and binary files
- Copy, move, rename, delete with advanced overwrite and queue logic
- Work with network resources, FTP / SFTP (via plugins) and virtual panels
- Automate tasks via macros and scripts

If you prefer a concise description, see the short base README: `README.md`.

## 2. Core Principles

- Text UI: Fast rendering, minimal distraction, full keyboard control
- Extensibility: Rich [Plugin API](https://api.farmanager.com/) enabling deep integration
- Stability: Mature codebase actively maintained for decades
- Customization: Themes, color highlighting, localized interface, macro engine

## 3. Key Features (Highlights)

| Category | Highlights |
|----------|------------|
| Navigation | Dual panels, drive bar, fast search, bookmarks |
| Editing | Internal editor with syntax highlighting (via plugins), large file support |
| Viewing | Flexible viewer (hex / Unicode), quick peek |
| Archives | Transparent browsing (ZIP, RAR, etc. via plugins) |
| Remote | FTP / network shares / temporary panels |
| Automation | Macro engine, scriptable actions, event hooks |
| UI Customization | Color groups, sorting profiles, per‑panel modes |
| Internationalization | Multi-language interface & translatable resource template |

## 4. Architecture Overview

```text
+---------------------------+
| Far.exe (Core)           |
|  Panels / UI / Kernel    |
+-------------+-------------+
              |
       Plugin Interfaces (C/C++)
              |
   +----------+----------+------------------+
   | Archive  | Network  | Viewer / Editor  | ... (20+ more)
   +----------+----------+------------------+
```

- Core is written in modern C++ (MSVC / GCC / Clang).
- Plugins are loaded as external DLLs; communication uses well-defined structs & callbacks.
- Language resources generated from templates (`far/farlang.templ.m4`). Do not edit `.lng` manually.

## 5. Building Far Manager

Multiple build systems are supported. Always consult `far/build.sh` and `_build/vc/all.sln` if in doubt.

### 5.1 Visual Studio (MSBuild)

```pwsh
cd _build/vc
msbuild /property:Configuration=Release /property:Platform=x64 all.sln
```

Output: `_build/vc/_output/product/Release.x64/Far.exe`

### 5.2 nmake (MSVC command line)

```pwsh
# Initialize environment (adjust VS path as needed)
& "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Auxiliary/Build/vcvarsall.bat" amd64
cd far
nmake /f makefile_vc AMD64=1
```

Flags:

- `DEBUG=1` build debug
- `CLANG=1` use clang-cl
- `PYTHON=1` alternative language file generator

### 5.3 GCC / MinGW

```pwsh
cd far
mingw32-make -j 4 -f makefile_gcc
```

Cross‑compile example (PowerShell invoking make may vary):

```pwsh
cd far
$env:GCC_PREFIX="x86_64-w64-mingw32-"; make -j 4 -f makefile_gcc DIRBIT=64
```

### 5.4 Helper Script (bash)

On environments with sh:

```sh
cd far
./build.sh 64        # 64-bit build
./build.sh debug 64  # Debug
```

### 5.5 Validation

Before contributing run:

```pwsh
cd far
python tools/source_validator.py
```

Also validate changelog and help files (see Section 10). Details: `enc/README.md` and `misc/build-checks/`.

## 6. Plugins

- Located in `plugins/`
- Built separately (`makefile_all_vc`, `makefile_all_gcc`)
- Examples include archive integration, temporary panels, network browsing
- Create new plugin with Visual Studio template or by mimicking existing layout

Basic build (MSVC):

```pwsh
cd plugins
nmake /f makefile_all_vc AMD64=1
```

## 7. Localization & Language Resources

- Template: `far/farlang.templ.m4`
- Generated files: `*.lng` (do not edit manually)
- To modify strings: edit template, rebuild with `PYTHON=1` if m4/gawk unavailable
- Documentation build steps: `enc/README.md`

## 8. Macro & Automation

Run macro tests (after build):

```pwsh
cd _build/vc/_output/product/Debug.x64
./Far.exe -service "macro:test"
```

Macro references reside in documentation under `enc/`.

## 9. Directory Highlights

| Path | Purpose |
|------|---------|
| `far/` | Core source, build scripts |
| `plugins/` | Official plugin sources |
| `misc/` | Installer scripts, validation tools |
| `enc/` | Documentation & CHM builder tools |
| `extra/` | Distribution extras (manifest, artwork) |
| `_build/vc/` | Visual Studio solutions |

## 10. Quality & CI

Three independent CI systems:

- GitHub Actions: style, changelog/help validation, multi-matrix builds
- AppVeyor: installers, macro tests, archives
- Azure Pipelines: full solution builds

You can inspect workflow files: `.github/workflows/`, `appveyor*.yml`, `azure-pipelines.yml`.

## 11. Contributing

Start with `CONTRIBUTING.md` (English) and localized versions (`CONTRIBUTING-PL.md`, `CONTRIBUTING-RU.md`).

Checklist before opening a PR:

1. Update `far/changelog` (top section) following required format
2. Run `python tools/source_validator.py`
3. Build at least one Release + one Debug configuration
4. Test affected functionality (editor, panels, plugin etc.)
5. Ensure no direct edits to generated language files

Add yourself (if appropriate) to `CONTRIBUTORS.md`.

## 12. Issue Reporting

Use the bug tracker: <https://bugs.farmanager.com/> — include:

- Far version & build number
- Reproduction steps
- Expected vs actual behavior
- Attach minimal logs or macro reproductions

Security concerns: report privately to maintainers (see forums / mailing lists).

## 13. License

BSD License — see `LICENSE`.

## 14. Quick Start (Minimal Flow)

```pwsh
# Clone

cd FarManager

# Build (VS Release x64)
cd _build/vc
msbuild /property:Configuration=Release /property:Platform=x64 all.sln

# Run
cd _output/product/Release.x64
./Far.exe
```

## 15. Helpful Links

- Official site: <https://www.farmanager.com/>
- Forums: <https://enforum.farmanager.com/> / <https://forum.farmanager.com/>
- Source: <https://github.com/FarGroup/FarManager>
- Plugin API: <https://api.farmanager.com/>

## 16. Frequently Asked (Abbreviated)

| Question | Pointer |
|----------|---------|
| How do I add a new language string? | Edit `farlang.templ.m4`, rebuild |
| Why does style validator fail? | Check BOM, tabs, include guards |
| Where is version info? | `far/farversion.m4`, `far/vbuild.m4` |
| Can I use Clang? | Yes: `CLANG=1` for nmake / GCC or clang-cl via VS |

---

Extended README maintained on branch `demo-readme`. Short form remains in `README.md`.
[logo-img]: ./logo.svg
[logo-url]: <https://www.farmanager.com>
[VS-AppVeyor-img]: <https://ci.appveyor.com/api/projects/status/6pca73evwo3oxvr9?svg=true>
[VS-AppVeyor-url]: <https://ci.appveyor.com/project/FarGroup/farmanager/history>
[GCC-AppVeyor-img]: <https://ci.appveyor.com/api/projects/status/k7ln3edp8nt5aoay?svg=true>
[GCC-AppVeyor-url]: <https://ci.appveyor.com/project/FarGroup/farmanager-5lhsj/history>
[Clang-AppVeyor-img]: <https://ci.appveyor.com/api/projects/status/pvwnc6gc5tjlpmti?svg=true>
[Clang-AppVeyor-url]: <https://ci.appveyor.com/project/FarGroup/farmanager-tgu1s/history>
[VS-Azure-img]: <https://img.shields.io/azure-devops/build/FarGroup/66d0ddcf-a098-4b98-9470-1c90632c4ba3/1.svg?logo=azuredevops>
[VS-Azure-url]: <https://dev.azure.com/FarGroup/FarManager/_build?definitionId=1>
