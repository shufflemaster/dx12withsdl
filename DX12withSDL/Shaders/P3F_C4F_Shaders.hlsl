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
VSOutput VSMain(float3 pos : POSITION, float3 normal : NORMAL, float4 color : COLOR)
{
    float3 light = normalize(float3(1.0, -1.0f, 0.0f));
    VSOutput vsOut;
    vsOut.pos = mul(float4(pos, 1.0), wvpMat);
    float dotto = -dot(normal, light);
    vsOut.color = color * dotto;
    return vsOut;
}

// Pixel Shader
float4 PSMain(VSOutput vsOut) : SV_Target
{
	return vsOut.color;
}
