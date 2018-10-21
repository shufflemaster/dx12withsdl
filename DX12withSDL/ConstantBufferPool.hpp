#pragma once

#include <d3d12.h>
#include "D3dx12.h"
#include <wrl/client.h>
using Microsoft::WRL::ComPtr;

#include "LogUtils.h"

namespace GAL {

    /*
    This class owns a pool of constant buffers
    */
    template<typename T, const int backBufferCount, const int maxItems>
    class ConstantBufferPool
    {
    public:
        ConstantBufferPool();
        virtual ~ConstantBufferPool();

        bool Init(ID3D12Device* device);
        D3D12_GPU_VIRTUAL_ADDRESS UploadData(const T* srcDataPtr, int backBufferIdx, int itemIdx);

    private:
        static const int D3D12_CONSTANT_BUFFER_SIZE = 64 * 1024;
        static const int D3D12_CBV_ALIGNMENT = 256;
        static const int m_maxItems = maxItems;
        static const int m_alignedItemSize = (sizeof(T) + (D3D12_CBV_ALIGNMENT - 1)) & ~(D3D12_CBV_ALIGNMENT - 1);
        static const int m_numItemsPerCB = D3D12_CONSTANT_BUFFER_SIZE / m_alignedItemSize;
        static const int m_numCBs = (maxItems / m_numItemsPerCB) + ((maxItems % m_numItemsPerCB) ? 1 : 0);

        ComPtr<ID3D12Resource> m_ConstantBufferUploadHeaps[backBufferCount][m_numCBs];
    };

    template<typename T, const int backBufferCount, const int maxItems>
    ConstantBufferPool<T, backBufferCount, maxItems>::ConstantBufferPool()
    {


    }

    template<typename T, const int backBufferCount, const int maxItems>
    ConstantBufferPool<T, backBufferCount, maxItems>::~ConstantBufferPool()
    {

    }

    template<typename T, const int backBufferCount, const int maxItems>
    bool ConstantBufferPool<T, backBufferCount, maxItems>::Init(ID3D12Device* device)
    {
        HRESULT hr;

        for (int backBufferIdx = 0; backBufferIdx < backBufferCount; ++backBufferIdx)
        {
            for (int constantBufferIdx = 0; constantBufferIdx < m_numCBs; ++constantBufferIdx)
            {
                hr = device->CreateCommittedResource(
                    &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // this heap will be used to upload the constant buffer data
                    D3D12_HEAP_FLAG_NONE, // no flags
                    &CD3DX12_RESOURCE_DESC::Buffer(D3D12_CONSTANT_BUFFER_SIZE), // size of the resource heap. Must be a multiple of 64KB for single-textures and constant buffers
                    D3D12_RESOURCE_STATE_GENERIC_READ, // will be data that is read from so we keep it in the generic read state
                    nullptr, // we do not have use an optimized clear value for constant buffers
                    IID_PPV_ARGS(&m_ConstantBufferUploadHeaps[backBufferIdx][constantBufferIdx]));
                if (FAILED(hr))
                {
                    ODERROR("Failed to create constant buffer %d of %d, for backBuffer Index %d", constantBufferIdx, m_numCBs, backBufferIdx);
                    return false;
                }
            }
        }
        return true;
    }

    template<typename T, const int backBufferCount, const int maxItems>
    D3D12_GPU_VIRTUAL_ADDRESS ConstantBufferPool<T, backBufferCount, maxItems>::UploadData(const T* srcDataPtr, int backBufferIdx, int itemIdx)
    {
        assert(backBufferIdx < backBufferCount);
        assert(itemIdx < maxItems);

        int constantBufferIdx = itemIdx / m_numItemsPerCB;
        int subItemIdx = itemIdx % m_numItemsPerCB;
        ID3D12Resource* uploadHeap = m_ConstantBufferUploadHeaps[backBufferIdx][constantBufferIdx].Get();

        //Upload the new matrix data.
        CD3DX12_RANGE readRange(0, 0);
        void* cpuAddress;
        uploadHeap->Map(0, &readRange, &cpuAddress);
        memcpy((char*)cpuAddress + (subItemIdx * m_alignedItemSize), srcDataPtr, sizeof(T));
        uploadHeap->Unmap(0, nullptr);

        return uploadHeap->GetGPUVirtualAddress() + (subItemIdx * m_alignedItemSize);
    }

};

