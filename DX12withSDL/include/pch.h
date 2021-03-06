// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file

#ifndef PCH_H
#define PCH_H

//#ifndef WIN32_LEAN_AND_MEAN
//#define WIN32_LEAN_AND_MEAN    // Exclude rarely-used stuff from Windows headers.
//#endif

// TODO: add headers that you want to pre-compile here
#include "ecs_typedefs.h"
#include <stdio.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <windows.h>
#include <windowsx.h>
#include <assert.h>

#include <ctime>
#include <vector>
#include <memory>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <map>
#include <string>
#include <stack>
#include <unordered_map>

#include <d3d12.h>
#include <dxgi1_4.h>
#include "D3dx12.h"
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <wrl/client.h>

#include <SDL.h>

#endif //PCH_H
