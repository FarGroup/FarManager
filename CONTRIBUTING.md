﻿### CONTRIBUTING

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

1. To compile with Visual Studio you can either use the IDE project or makefile.  
   Example for msbuild & vcxproj:  
     `msbuild /property:Configuration=Release;platform=x64 far.vcxproj`  
   Example for nmake & makefile:  
     `nmake /f makefile_vc`

2. To compile with GCC you can use makefile.
   Example for MinGW & makefile:  
     `mingw32-make -f makefile_gcc`

> Also see comments in makefile_* for additional build parameters.


#### Changelog - `changelog` file

1. All comments on committed changes should be written to the `changelog` file.
   You should leave comments in the source code only if you think the code is
   not self-explanatory and won't be understood by "future generations".
2. Recent changes go on top.
3. Each entry starts with a header of the form:
>   warp 05.12.2006 01:39:38 +0300 - build 2149
4. Changes not always require a build increase (i.e. cosmetic or non code
   related changes).
5. Sample macro to generate the header:  
```lua
   Macro {
     area="Editor"; key="Ctrl`"; action = function()
       Keys("CtrlHome End CtrlLeft")
       build=mf.int(mf.substr(Editor.Value,Editor.RealPos-1))+1;
       Keys("CtrlHome")
       print(mf.date("name %d.%m0.%Y %H:%M:%S %z - build ")) print(build)
       for RCounter=4,1,-1 do  Keys("Enter") end
       Keys("Up Up 1 . Space")
     end;
   }
```

#### farversion.m4

1. This file contains information used to generate Far version:
   * `BUILDTYPE`, a string that defines the build type:
      - '' (an empty string) - release version
      - `alpha` - alpha version
      - `beta` - beta version
      - `RC` - release candidate
      - `AnythingElse` - intended for Far versions which code has not been
        committed yet to the repository so we and the users will not be confused.  
        Far version will look something like that:  
        `FarVersion alpha AnythingElse based on build BuildNumber`
   * `MAJOR` - major Far version (i.e. 3).
   * `MINOR` - minor Far version (i.e. 0).


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
