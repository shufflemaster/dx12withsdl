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
        inline XMFLOAT4A* GetRow(const int rowIdx) { return reinterpret_cast<XMFLOAT4A*>(m_matrix.m[rowIdx]); }

        inline XMFLOAT4A* GetRight() { return GetRow(0); }
        inline XMFLOAT4A* GetUp() { return GetRow(1); }
        inline XMFLOAT4A* GetForward() { return GetRow(2); }
        inline XMFLOAT4A* GetPosition() { return GetRow(3); }

        inline void SetRight(const XMFLOAT4A& right) { *GetRight() = right; }
        inline void SetUp(const XMFLOAT4A& up) { *GetUp() = up; }
        inline void SetForward(const XMFLOAT4A& forward) { *GetForward() = forward; }
        inline void SetPosition(const XMFLOAT4A& pos) { *GetPosition() = pos; }

    }; //class TransformComponent

}; //namespace GAL

