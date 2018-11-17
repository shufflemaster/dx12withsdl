#pragma once

using namespace DirectX;

namespace GAL
{
    class Camera
    {
    public:
        Camera();
        ~Camera();
        
        XMFLOAT4X4A m_viewMatrix;
        XMFLOAT4X4A m_projMatrix;
        //XMFLOAT4X4A m_viewProjMatrix;
    };
}; //namespace GAL
