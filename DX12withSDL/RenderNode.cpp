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

    bool RenderNode::InitWithVertices(const ColorVertex* vertices, uint32_t numVertices,
                                      const DWORD* indices, uint32_t numIndices)
    {
        ID3D12Device* device = RendererD3D12::GetRenderer().GetDevice();

        // create root signature
        CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init(
            0, nullptr, //numParameters and parameters. 
            0, nullptr, //num static samplers and static samplers.
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS|
            D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS);

        //Serialize the root signature as a stream of bytes.
        ID3DBlob* signature;
        hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, nullptr);
        if (FAILED(hr))
        {
            ODERROR("Failed to serialize the root signature hr=%d", hr);
            return false;
        }

        hr = device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature));
        if (FAILED(hr))
        {
            ODERROR("Failed to create the root signature hr=%d", hr);
            return false;
        }

        
        return true;
    }

};
