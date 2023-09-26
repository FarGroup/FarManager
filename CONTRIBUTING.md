|[Polski](CONTRIBUTING-PL.md)|[Русский](CONTRIBUTING-RU.md)|
|-|-|

### CONTRIBUTING

> Sections below will help you get familiar with our development process.

#### Submitting patches

When you feel comfortable with the code and decide to make your contribution
to the project, please follow those guidelines:

1. One logical change per patch, the smaller the patch the easier it is for us
   to review and commit it.
2. Try to keep inline with the overall style of the code.
3. Provide detailed changelog for your patch.
4. If your patch requires updating the documentation (help, Encyclopedia, etc.)
   please provide the needed updates.
   Please create the patch against the latest code from the repository.
5. Patches should be submitted as pull requests to the repository
   or as diff files to our [bugtracker](https://bugs.farmanager.com)
   or [forum](https://forum.farmanager.com/viewforum.php?f=54)
6. If you plan to create large patches or want to keep current with the
   development of Far Manager, subscribe to the
   [Developers mailing list](https://groups.google.com/group/fardeven)
   (<fardeven@googlegroups.com>).
7. Frequent patchers will be eligible for full repository access, by our discretion.


#### Compilation

```
cd far
```

1. To compile with Visual Studio you can either use the IDE project or makefile.<br/>
   Example for msbuild & vcxproj:<br/>
   `msbuild /property:Configuration=Release;platform=x64 far.vcxproj`<br/>
   Example for nmake & makefile:<br/>
   `nmake /f makefile_vc`

2. To compile with GCC you can use makefile.<br/>
   Example for MinGW & makefile:<br/>
   `mingw32-make -f makefile_gcc`

> Also see comments in makefile_* for additional build parameters.


#### Changelog - `changelog` file

1. All comments on committed changes should be written to the `changelog` file.
   You should leave comments in the source code only if you think the code is
   not self-explanatory and won't be understood by "future generations".
2. Recent changes go on top.
3. Each entry starts with a header of the form:
```
--------------------------------------------------------------------------------
warp 2006-12-05 01:39:38+03:00 - build 2149
```
4. Changes not always require a build increase (i.e. cosmetic or non code
   related changes).
5. Sample macro to generate the header: [ChangelogHeader.lua](./misc/changelog/ChangelogHeader.lua)

#### farversion.m4

1. This file contains information used to generate Far version:
   * `SPECIAL_VERSION`, a string that, if set, marks the build as special.
      - Intended for Far versions which code has not been
        committed yet to the repository so we and the users will not be confused.
      - If not set, the build type will be defined by FARMANAGER_BUILD_TYPE environment
        variable. If the variable is not set, the build type will be Private.
        For the supported build types see VERSION_STAGE enumeration in plugin.hpp.
   * `VERSION_MAJOR` - major Far version (e.g. 3).
   * `VERSION_MINOR` - minor Far version (e.g. 0).
   * `VERSION_REVISION` - Far version revision (e.g. 0).
   * `VERSION_BUILD` - set in `vbuild.m4` file.


#### vbuild.m4

1. If the build number in `vbuild.m4`  was changed then after committing the
   changes to the repository you should call `tag_build.bat`.


#### Adding new lines to language files

1. Lng files are generated automatically.
   All changes must be made in `farlang.templ.m4`.
   If you don't know how to translate your changes to other languages,
   use the English version with the `upd:` prefix.


#### x64 - ensuring successful compilation for x86 and x64

1. DWORD_PTR, LONG_PTR, etc. are used instead of DWORD/long/etc. in the
   following cases:

   - needed where previously int/long/dword/or_any_other_non_pointer_type
      were used and a pointer value was assigned to it.
   - ...
   - ...


> Reminders, todos, notes should go to the [bugtracker](https://bugs.farmanager.com).
