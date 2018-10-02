#include "pch.h"
#include "RenderNode.h"
#include "VertexTypes.h"

#include "d3dx12.h"
#include "LogUtils.h"
#include "RendererD3D12.h"

namespace GAL
{

    RenderNode::RenderNode()
    {
    }


    RenderNode::~RenderNode()
    {
    }

    bool RenderNode::InitWithVertices(const P3F_C4F* vertices, uint32_t numVertices,
        const DWORD* indices, uint32_t numIndices)
    {
        HRESULT hr;
        ID3D12Device* device = RendererD3D12::GetRenderer().GetDevice();

        // create root signature
        CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init(
            0, nullptr, //numParameters and parameters. 
            0, nullptr, //num static samplers and static samplers.
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS);

        //Serialize the root signature as a stream of bytes.
        ID3DBlob* signature;
        hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, nullptr);
        if (FAILED(hr))
        {
            ODERROR("Failed to serialize the root signature hr=%d", hr);
            return false;
        }

        hr = device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature.Get()));
        if (FAILED(hr))
        {
            ODERROR("Failed to create the root signature hr=%d", hr);
            return false;
        }

        //Create vertex and pixel shader
        m_vertexShader = std::make_unique<Shader>("P3F_C4F_Shaders.hlsl", "VSMain", Shader::TargetType::VertexShader);
        if (m_vertexShader->GetCompileError()) {
            ODERROR("Failed to compile vertex shader: %s", m_vertexShader->GetCompileError());
            return false;
        }

        m_pixelShader = std::make_unique<Shader>("P3F_C4F_Shaders.hlsl", "PSMain", Shader::TargetType::PixelShader);
        if (m_pixelShader->GetCompileError()) {
            ODERROR("Failed to compile pixel shader: %s", m_pixelShader->GetCompileError());
            return false;
        }

        //FIXME: PSO should not be created/owned by render nodes.
        // create a pipeline state object (PSO)

        // In a real application, you will have many pso's. for each different shader
        // or different combinations of shaders, different blend states or different rasterizer states,
        // different topology types (point, line, triangle, patch), or a different number
        // of render targets you will need a pso

        // VS is the only required shader for a pso. You might be wondering when a case would be where
        // you only set the VS. It's possible that you have a pso that only outputs data with the stream
        // output, and not on a render target, which means you would not need anything after the stream
        // output.
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {}; // a structure to define a pso
        psoDesc.InputLayout = P3F_C4F::GetInputLayoutDesc(); // the structure describing our input layout
        psoDesc.pRootSignature = m_rootSignature.Get(); // the root signature that describes the input data this pso needs
        psoDesc.VS = m_vertexShader->GetCompiledByteCode(); // structure describing where to find the vertex shader bytecode and how large it is
        psoDesc.PS = m_pixelShader->GetCompiledByteCode(); // same as VS but for pixel shader
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; // type of topology we are drawing
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT); // a default rasterizer state.
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT); // a default blent state.
        RendererD3D12::GetRenderer().fillPSOSampleDescription(psoDesc);


        // create the pso
        hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineStateObject.Get()));
        if (FAILED(hr))
        {
            ODERROR("Failed to create PSO: result=0x%08X", hr);
            return false;
        }

        //FIXME: Move this into some kind of mesh object for efficient instancing
        //Create the vertex buffer
        // create default heap.
        // default heap is memory on the GPU. Only the GPU has access to this memory
        // To get data into this heap, we will have to upload the data using
        // an upload heap
        UINT vBufferSize = sizeof(*vertices) * numVertices;
        device->CreateCommittedResource(
                &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), // a default heap
                D3D12_HEAP_FLAG_NONE, // no flags
                &CD3DX12_RESOURCE_DESC::Buffer(vBufferSize), // resource description for a buffer
                D3D12_RESOURCE_STATE_COPY_DEST, // we will start this heap in the copy destination state since we will copy data
                                                // from the upload heap to this heap
                nullptr, // optimized clear value must be null for this type of resource. used for render targets and depth/stencil buffers
                IID_PPV_ARGS(&m_vertexBuffer.Get()));
        // we can give resource heaps a name so when we debug with the graphics debugger we know what resource we are looking at
        m_vertexBuffer->SetName(L"Vertex Buffer Resource Heap");

        // create upload heap
        // upload heaps are used to upload data to the GPU. CPU can write to it, GPU can read from it
        // We will upload the vertex buffer using this heap to the default heap
        ID3D12Resource* vBufferUploadHeap;
        device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // upload heap
            D3D12_HEAP_FLAG_NONE, // no flags
            &CD3DX12_RESOURCE_DESC::Buffer(vBufferSize), // resource description for a buffer
            D3D12_RESOURCE_STATE_GENERIC_READ, // GPU will read from this buffer and copy its contents to the default heap
            nullptr,
            IID_PPV_ARGS(&vBufferUploadHeap));
        vBufferUploadHeap->SetName(L"Vertex Buffer Upload Resource Heap");

        // store vertex buffer in upload heap
        D3D12_SUBRESOURCE_DATA vertexData = {};
        vertexData.pData = reinterpret_cast<BYTE*>(vertices); // pointer to our vertex array
        vertexData.RowPitch = vBufferSize; // size of all our triangle vertex data
        vertexData.SlicePitch = vBufferSize; // also the size of our triangle vertex data

        // we are now creating a command with the command list to copy the data from
        // the upload heap to the default heap
        UpdateSubresources(commandList, vertexBuffer, vBufferUploadHeap, 0, 0, 1, &vertexData);

        // transition the vertex buffer data from copy destination state to vertex buffer state
        commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(vertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

        // Now we execute the command list to upload the initial assets (triangle data)
        commandList->Close();
        ID3D12CommandList* ppCommandLists[] = { commandList };
        commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

        // increment the fence value now, otherwise the buffer might not be uploaded by the time we start drawing
        fenceValue[frameIndex]++;
        hr = commandQueue->Signal(fence[frameIndex], fenceValue[frameIndex]);
        if (FAILED(hr))
        {
            Running = false;
        }

        // create a vertex buffer view for the triangle. We get the GPU memory address to the vertex pointer using the GetGPUVirtualAddress() method
        vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
        vertexBufferView.StrideInBytes = sizeof(Vertex);
        vertexBufferView.SizeInBytes = vBufferSize;




        return true;
    }

};
