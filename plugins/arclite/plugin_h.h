#include "farversion.hpp"
#define PLUGIN_NAME L"<(NAME)>"
#define PLUGIN_VERSION MAKEFARVERSION(<(VER_MAJOR)>, <(VER_MINOR)>, 0, <(VER_BUILD)>, VS_RELEASE)
#define PLUGIN_DESCRIPTION L"Archive support (based on 7-Zip project)"
