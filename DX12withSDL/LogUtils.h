#pragma once

namespace GAL {

    void __cdecl odprintf(const char *format, ...);

#define ODERROR(format, ...) odprintf("%s %s %d "format, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)

}
