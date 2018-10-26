#include "pch.h"

#include "Shader.h"

#include "LogUtils.h"

namespace GAL
{
    Shader::Shader()
    {
    }

    Shader::Shader(const Shader & other)
    {
        if (other.m_compilationError.Get())
        {
            D3DCreateBlob(
                other.m_compilationError->GetBufferSize(),
                (ID3DBlob**)m_compilationError.GetAddressOf());
            memcpy(m_compilationError->GetBufferPointer(), other.m_compilationError->GetBufferPointer(), other.m_compilationError->GetBufferSize());
        }

        if (other.m_compiledShader.Get())
        {
            D3DCreateBlob(
                other.m_compiledShader->GetBufferSize(),
                (ID3DBlob**)m_compiledShader.GetAddressOf());
            memcpy(m_compiledShader->GetBufferPointer(), other.m_compiledShader->GetBufferPointer(), other.m_compiledShader->GetBufferSize());
        }

    }

    Shader::Shader(Shader && other)
    {
        other.m_compilationError.Swap(m_compilationError);
        other.m_compiledShader.Swap(m_compiledShader);
    }

    Shader::Shader(LPCWSTR filePath, const char * entryPoint, TargetType targetType) : Shader()
    {
        const char * shaderTarget = GetTargetTypeString(targetType);

        //FIXME: What about D3DCOMPILE_PACK_MATRIX_ROW_MAJOR or D3DCOMPILE_PACK_COLUMN_ROW_MAJOR
#if defined _DEBUG || defined DEBUG_SHADERS
        UINT shaderCompileFlags = D3DCOMPILE_WARNINGS_ARE_ERRORS | D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
        UINT shaderCompileFlags = D3DCOMPILE_WARNINGS_ARE_ERRORS | D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

        HRESULT hr = D3DCompileFromFile(
            filePath,
            nullptr, //No shader macros.
            nullptr, //No Includes, most likely will be changed in the future so the shader can #include.
            entryPoint, shaderTarget,
            shaderCompileFlags,
            0, //No effect compile option
            m_compiledShader.GetAddressOf(), m_compilationError.GetAddressOf());

        if (S_OK != hr) {
            ODERROR("Failed to compile shader: %s", reinterpret_cast<LPCSTR>(m_compilationError.Get()->GetBufferPointer()));
        }
    }

    Shader* Shader::Compile(LPCWSTR filePath, const char * entryPoint, TargetType targetType, char** error)
    {
        Shader* shader = new Shader(filePath, entryPoint, targetType);
        if (shader->GetCompileError()) {
            if (error) {
                SIZE_T bufferSize = shader->m_compilationError.Get()->GetBufferSize();
                char* err = new char[bufferSize];
                strcpy(err, shader->GetCompileError());
            }
            delete shader;
            return nullptr;
        }
        return shader;
    }

    const char * Shader::GetCompileError() const
    {
        if (m_compilationError.Get()) {
            return reinterpret_cast<LPCSTR>(m_compilationError.Get()->GetBufferPointer());
        }
        return nullptr;
    }

    D3D12_SHADER_BYTECODE Shader::GetCompiledByteCode() const
    {
        D3D12_SHADER_BYTECODE bc = { nullptr, 0};
        if (m_compiledShader.Get()) {
            bc.pShaderBytecode = m_compiledShader.Get()->GetBufferPointer();
            bc.BytecodeLength = m_compiledShader.Get()->GetBufferSize();
        }
        return bc;
    }

    Shader::~Shader()
    {
    }

    const char * Shader::GetTargetTypeString(TargetType targetType)
    {
        switch (targetType)
        {
        case PixelShader: return "ps_5_0";
        case VertexShader: return "vs_5_0";
        case ComputeShader: return "cs_5_0";
        case DomainShader: return "ds_5_0";
        case GeometryShader: return "gs_5_0";
        default: return "hs_5_0"; //HullShader
        }
    }
};//namespace GAL