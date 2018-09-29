#pragma once

#include <d3d12.h>
#include <wrl/client.h>

namespace GAL {
    class Shader
    {
    public:
        enum TargetType
        {
            ComputeShader,
            DomainShader,
            GeometryShader,
            HullShader,
            PixelShader,
            VertexShader,
        };

        Shader(LPCWSTR filePath, const char * entryPoint, TargetType targetType);
        static Shader* Compile(LPCWSTR filePath, const char * entryPoint, TargetType targetType, char** error = nullptr);
        virtual ~Shader();
        
        const char * GetCompileError() const;
        D3D12_SHADER_BYTECODE GetCompiledByteCode() const;

    private:
        Shader();

        Microsoft::WRL::ComPtr<ID3DBlob> m_compilationError;
        Microsoft::WRL::ComPtr<ID3DBlob> m_compiledShader;

        static const char * GetTargetTypeString(TargetType targetType);
    }; //class Shader
};//namepsace GAL
