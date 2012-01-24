#include "farversion.hpp"
#define PLUGIN_NAME L"<(NAME)>"
#define PLUGIN_VERSION MAKEFARVERSION(<(VER_MAJOR)>, <(VER_MINOR)>, <(VER_PATCH)>, 0, VS_RELEASE)
#define PLUGIN_DESCRIPTION L"Archive support (based on 7-Zip project)"
