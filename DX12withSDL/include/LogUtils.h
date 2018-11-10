#pragma once

namespace GAL {

    void __cdecl odprintf(const char *format, ...);

}


#define ODERROR(format, ...) GAL::odprintf("ERROR: %s %s %d "format, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#define ODINFO(format, ...) GAL::odprintf("INFO: "format, __VA_ARGS__)