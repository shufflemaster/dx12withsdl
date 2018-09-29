#pragma once

/*
 Here we define the basic structs/class for the vertices we support
*/

namespace GAL
{
    struct P3F_C4F
    {
        float pos[3]; //x, y, z
        float color[4]; //r, g, b, a

        static const D3D12_INPUT_LAYOUT_DESC& GetInputLayoutDesc()
        {
            static D3D12_INPUT_ELEMENT_DESC elements[] =
            {
                { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
                { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
            };
            static D3D12_INPUT_LAYOUT_DESC desc = { elements, sizeof(elements) / sizeof(elements[0]) };
            return desc;
        }
    };

};
