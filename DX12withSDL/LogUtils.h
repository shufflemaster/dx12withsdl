#pragma once

namespace GAL {

    void __cdecl odprintf(const char *format, ...);

#define ODERROR(fmt, ...) odprintf("%s %s %d "format, __FILE__, __FUNC__, __LINE__, __VA_ARGS__)

}
