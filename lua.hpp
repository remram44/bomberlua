/**
 * @file lua.h Ce header inclut les headers de Lua, dans un bloc extern "C", le
 * header <lua.hpp> étant fourni uniquement sous Debian.
 */

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}
