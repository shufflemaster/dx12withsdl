//
// Shaders
//

struct VSOutput
{
	float4 pos : SV_POSITION;
	float4 color : COLOR;
};

cbuffer ConstantBuffer : register(b0)
{
    float4x4 wvpMat;
};

// Vertex Shader
VSOutput VSMain(float3 pos : POSITION, float4 color : COLOR)
{
    VSOutput vsOut;
    vsOut.pos = mul(float4(pos, 1.0), wvpMat);
    vsOut.color = color;
    return vsOut;
}

// Pixel Shader
float4 PSMain(VSOutput vsOut) : SV_Target
{
	return vsOut.color;
}
