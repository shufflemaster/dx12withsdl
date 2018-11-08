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

        //Helper methods.
        inline XMFLOAT4A* GetRight() { return GetRow(0); }
        inline XMFLOAT4A* GetUp() { return GetRow(1); }
        inline XMFLOAT4A* GetForward() { return GetRow(2); }
        inline XMFLOAT4A* GetPosition() { return GetRow(3); }
        inline XMFLOAT4A* GetRow(const int rowIdx) { return reinterpret_cast<XMFLOAT4A*>(m_matrix.m[rowIdx]); }
    }; //class TransformComponent

}; //namespace GAL

