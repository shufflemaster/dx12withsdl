#pragma once

using namespace DirectX;

namespace GAL
{

    class alignas(16) TransformComponent
    {
    public:
        TransformComponent();
        ~TransformComponent();
        
        XMFLOAT4X4A m_matrix;
    }; //class TransformComponent

}; //namespace GAL

