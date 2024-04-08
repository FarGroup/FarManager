lfjit.exe is a "Unicode Lua Interpreter". In addition to lua51.dll,
it is dynamically linked with luafar3.dll. The interpreter works identically
to the standard luajit.exe except the following additions and changes:

* Additional libraries: 'bit64', 'utf8', 'win'.
* Unicode version of library 'io'.
* Unicode versions of functions 'loadfile', 'require', 'package.loadlib'.
* Methods called on strings via ':' access 'utf8' table rather than
  'string' table.

These libraries and functions are described in the LuaFAR manual.

Limitation: non-Unicode command line interface.
