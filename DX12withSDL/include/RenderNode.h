#pragma once

using namespace std;

using namespace DirectX;

#include "Shader.h"
using Microsoft::WRL::ComPtr;

namespace GAL
{
    struct P3F_C4F;

    class RenderNode
    {
    public:
        RenderNode();
        virtual ~RenderNode();

        bool InitWithVertices(const P3F_C4F* vertices, uint32_t numVertices,
                              const DWORD* indices, uint32_t numIndices);



        uint32_t m_numIndices;

        XMFLOAT4X4 m_worldMatrix;
    };
};
