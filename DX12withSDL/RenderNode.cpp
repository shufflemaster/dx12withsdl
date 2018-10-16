#include "pch.h"
#include "RenderNode.h"
#include "VertexTypes.h"

#include "d3dx12.h"
#include "LogUtils.h"
#include "RendererD3D12.h"

namespace GAL
{

    RenderNode::RenderNode() : m_numIndices(0)
    {
    }


    RenderNode::~RenderNode()
    {
    }

    bool RenderNode::InitWithVertices(const P3F_C4F* vertices, uint32_t numVertices,
        const DWORD* indices, uint32_t numIndices)
    {
        ID3D12Device* device = RendererD3D12::GetRenderer().GetDevice();

        m_numIndices = numIndices;

        //FIXME: Move this into some kind of mesh object for efficient instancing
        //Create the vertex buffer
        // create default heap.
        // default heap is memory on the GPU. Only the GPU has access to this memory
        // To get data into this heap, we will have to upload the data using
        // an upload heap
        UINT vertexDataSize = sizeof(*vertices) * numVertices;
        device->CreateCommittedResource(
                &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), // a default heap
                D3D12_HEAP_FLAG_NONE, // no flags
                &CD3DX12_RESOURCE_DESC::Buffer(vertexDataSize), // resource description for a buffer
                D3D12_RESOURCE_STATE_COPY_DEST, // we will start this heap in the copy destination state since we will copy data
                                                // from the upload heap to this heap
                nullptr, // optimized clear value must be null for this type of resource. used for render targets and depth/stencil buffers
                IID_PPV_ARGS(&m_vertexBuffer));
        // we can give resource heaps a name so when we debug with the graphics debugger we know what resource we are looking at
        m_vertexBuffer->SetName(L"Vertex Buffer Resource Heap");

        UINT indexDataSize = sizeof(*indices) * numIndices;
        device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), // a default heap
            D3D12_HEAP_FLAG_NONE, // no flags
            &CD3DX12_RESOURCE_DESC::Buffer(indexDataSize), // resource description for a buffer
            D3D12_RESOURCE_STATE_COPY_DEST, // we will start this heap in the copy destination state since we will copy data
                                            // from the upload heap to this heap
            nullptr, // optimized clear value must be null for this type of resource. used for render targets and depth/stencil buffers
            IID_PPV_ARGS(&m_indexBuffer));
        // we can give resource heaps a name so when we debug with the graphics debugger we know what resource we are looking at
        m_indexBuffer->SetName(L"Index Buffer Resource Heap");

        // create upload heap
        // upload heaps are used to upload data to the GPU. CPU can write to it, GPU can read from it
        // We will upload the vertex buffer using this heap to the default heap
        ComPtr<ID3D12Resource> vBufferUploadHeap;
        device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // upload heap
            D3D12_HEAP_FLAG_NONE, // no flags
            &CD3DX12_RESOURCE_DESC::Buffer(vertexDataSize + indexDataSize), // resource description for a buffer
            D3D12_RESOURCE_STATE_GENERIC_READ, // GPU will read from this buffer and copy its contents to the default heap
            nullptr,
            IID_PPV_ARGS(&vBufferUploadHeap));
        vBufferUploadHeap->SetName(L"V and I Buffer Upload Resource Heap");

        //Create the buffer views
        m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
        m_vertexBufferView.SizeInBytes = vertexDataSize;
        m_vertexBufferView.StrideInBytes = sizeof(*vertices);

        m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
        m_indexBufferView.SizeInBytes = sizeof(indices);
        m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;

        // Copy data on CPU into the upload buffer
        void* p;
        vBufferUploadHeap->Map(0, nullptr, &p);
        ::memcpy(p, vertices, vertexDataSize);
        ::memcpy(static_cast<unsigned char*>(p) + vertexDataSize,
            indices, indexDataSize);
        vBufferUploadHeap->Unmap(0, nullptr);

        // Create our temporary upload fence, command list and command allocator
        // This will be only used while creating the mesh buffer
        // to upload data to the GPU.
        ComPtr<ID3D12Fence> uploadFence;
        device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&uploadFence));

        ComPtr<ID3D12CommandAllocator> uploadCommandAllocator;
        device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
            IID_PPV_ARGS(&uploadCommandAllocator));
        ComPtr<ID3D12GraphicsCommandList> uploadCommandList;
        device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
            uploadCommandAllocator.Get(), nullptr,
            IID_PPV_ARGS(&uploadCommandList));

        // Copy data from upload buffer on CPU into the vertex and index buffers on 
        // the GPU
        uploadCommandList->CopyBufferRegion(m_vertexBuffer.Get(), 0,
            vBufferUploadHeap.Get(), 0, vertexDataSize);
        uploadCommandList->CopyBufferRegion(m_indexBuffer.Get(), 0,
            vBufferUploadHeap.Get(), vertexDataSize, indexDataSize);

        // Barriers, batch them together
        CD3DX12_RESOURCE_BARRIER barriers[2] = {
            CD3DX12_RESOURCE_BARRIER::Transition(m_vertexBuffer.Get(),
            D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER),
            CD3DX12_RESOURCE_BARRIER::Transition(m_indexBuffer.Get(),
                D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER)
        };

        uploadCommandList->ResourceBarrier(2, barriers);

        uploadCommandList->Close();

        // Execute the upload and finish the command list
        ID3D12CommandQueue* commandQueue = RendererD3D12::GetRenderer().GetCommandQueue();
        ID3D12CommandList* commandLists[] = { uploadCommandList.Get() };
        commandQueue->ExecuteCommandLists(std::extent<decltype(commandLists)>::value, commandLists);
        commandQueue->Signal(uploadFence.Get(), 1);

        auto waitEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        RendererD3D12::WaitForFence(uploadFence.Get(), 1, waitEvent);

        // Cleanup our upload handle
        uploadCommandAllocator->Reset();

        CloseHandle(waitEvent);

        return true;
    }

};
