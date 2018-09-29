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

        const D3D12_INPUT_LAYOUT_DESC& inputLayoutDesc = P3F_C4F::GetInputLayoutDesc();

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
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; // format of the render target
        psoDesc.SampleDesc = sampleDesc; // must be the same sample description as the swapchain and depth/stencil buffer
        psoDesc.SampleMask = 0xffffffff; // sample mask has to do with multi-sampling. 0xffffffff means point sampling is done
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT); // a default rasterizer state.
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT); // a default blent state.
        psoDesc.NumRenderTargets = 1; // we are only binding one render target

        // create the pso
        hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineStateObject));
        if (FAILED(hr))
        {
            return false;
        }


        return true;
    }

};
