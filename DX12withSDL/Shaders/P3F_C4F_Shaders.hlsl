//
// Shaders
//

struct VSOutput
{
    float3 posVS    : TEXCOORD0;   // View space position.
    float3 normalVS : NORMAL;      // View space normal.
	float4 pos      : SV_POSITION; // Clip space position.
	float4 color    : COLOR;
};

//We are doing View Space lighting calculations
//instead of World Space.
cbuffer ConstantBuffer : register(b0)
{
    float4x4 worldViewProjectionMat;
    float4x4 worldViewMat;
};

// Vertex Shader
// We only do transformations here. All the lighting will be done in the pixel shader.
VSOutput VSMain(float3 pos : POSITION, float3 normal : NORMAL, float4 color : COLOR)
{
    VSOutput vsOut;
    vsOut.posVS = mul(float4(pos, 1.0f), worldViewMat).xyz;
    vsOut.normalVS = mul(normal, (float3x3)worldViewMat);
    vsOut.pos = mul(float4(pos, 1.0), worldViewProjectionMat);
    vsOut.color = color;
    return vsOut;
}

struct LightResult
{
    float4 diffuse;
    float4 specular;
};

float CalcAttenuation(float lkConstantAtt, float lkLinearAtt, float lkQuadraticAtt, float pointToLightDistance)
{
    return 1.0f / (lkConstantAtt +
                   lkLinearAtt * pointToLightDistance +
                   lkQuadraticAtt * pointToLightDistance * pointToLightDistance);
}

/*
@pointNormalVS: Pixel Normal is View Space.

NOTES about specular.:
 P is the point in 3D space that we want to shade,
 N is the surface normal at point we want to shade,
 Lp is the position of the light in 3D space,
 L is the normalized direction vector from the point we want to shade to the light source,
 Ep is the eye position (position of the camera),
 V is the view vector and is the normalized direction vector from the point we want to shade to the eye,
 R is the reflection vector from the light source to the point we are shading about the surface normal ,
 Ls is the specular contribution of the light source,
 Ks is the specular component of the material,
 Sp is the "shininess" of the material. The higher the "shininess" value, the smaller the specular highlight.
Then using the Phong lighting model, the specular component can be calculated as follows:

L = normalize(Lp - P)
V = normalize(Ep - P)
R = 2(L.N)N - L
Specular = ((R.V)^Sp)*Ls*Ks
*/
LightResult CalcDirectionalLight(float3 pointPositionVS, float3 pointNormalVS)
{
    LightResult ret;

    //WS means WorldSpace
    float3 lightDirectionWS = float3(1.0, 1.0f, 1.0f);

    float4 lightDiffuseColor = float4(0.5f, 0.5f, 0.5f, 1.0f);
    //Transform light direction from world space to view space.
    float3 L = -normalize(mul(lightDirectionWS, (float3x3)worldViewMat));

    float NdotL = dot(pointNormalVS, L);
    ret.diffuse = max(0, NdotL) * lightDiffuseColor;

    //Specular calculation.
    float materialSpecularPower = 32.0f;
    float4 lightSpecularColor = float4(1.0, 1.0f, 1.0f, 1.0f);

    float3 R = (2.0f * NdotL * pointNormalVS) - L;
    //In ViewSpace eyePosition is (0,0,0)
    float3 V = normalize(/*eyePosition*/ -pointPositionVS);

    float RdotV = max(0, dot(R, V));
    ret.specular = pow(RdotV, materialSpecularPower) * lightSpecularColor;

    return ret;
}

LightResult CalcPointLight(float3 pointPositionVS, float3 pointNormalVS)
{
    LightResult ret;

    float4 lightPositionWS = float4(3.0f, 1.0f, -4.0f, 1.0f);
    float3 lightPositionVS = mul(lightPositionWS, worldViewMat).xyz;
    float4 lightDiffuseColor = float4(1.0f, 1.0f, 1.0f, 1.0f);

    float3 Ldelta = lightPositionVS - pointPositionVS;
    float LdeltaLength = length(Ldelta);
    float attenuation = CalcAttenuation(1.0f, 0.2f, 0.1f, LdeltaLength);

    float3 L = Ldelta / LdeltaLength;
    float NdotL = dot(pointNormalVS, L);
    ret.diffuse = max(0, NdotL) * lightDiffuseColor * attenuation;

    //Specular contribution.
    float materialSpecularPower = 32.0f;
    float4 lightSpecularColor = float4(1.0, 1.0f, 1.0f, 1.0f);

    float3 R = (2.0f * NdotL * pointNormalVS) - L;
    //In ViewSpace eyePosition is (0,0,0)
    float3 V = normalize(/*eyePosition*/ -pointPositionVS);

    float RdotV = max(0, dot(R, V));
    ret.specular = pow(RdotV, materialSpecularPower) * lightSpecularColor * attenuation;

    return ret;
}

// Pixel Shader
float4 PSMain(VSOutput vsOut) : SV_Target
{
    //Equations of lighting
     //From: https://www.3dgep.com/texturing-lighting-directx-11/
     //LightingResult lit = ComputeLighting(IN.PositionWS, normalize(IN.NormalWS));

     //float4 emissive = Material.Emissive;
     //float4 ambient = Material.Ambient * GlobalAmbient;
     //float4 diffuse = Material.Diffuse * lit.Diffuse;
     //float4 specular = Material.Specular * lit.Specular;

     //float4 texColor = { 1, 1, 1, 1 };

     //if (Material.UseTexture)
     //{
     //    texColor = Texture.Sample(Sampler, IN.TexCoord);
     //}
     //float4 finalColor = (emissive + ambient + diffuse + specular) * texColor;
     //return finalColor;

    //We are going to simulate one directional light, and one point light.
    float4 ambient = float4(0.05, 0.05, 0.05, 1.0f);
    float4 materialDiffuse = float4(1.0f, 1.0f, 1.0f, 1.0f);
    float4 materialSpecular = float4(1.0f, 1.0f, 1.0f, 1.0f);

    LightResult directionalLightResult;
    directionalLightResult = CalcDirectionalLight(vsOut.posVS, vsOut.normalVS);

    LightResult pointLightResult;
    pointLightResult = CalcPointLight(vsOut.posVS, vsOut.normalVS);

    LightResult allLightsResult;
    allLightsResult.diffuse = directionalLightResult.diffuse + pointLightResult.diffuse;
    allLightsResult.specular = directionalLightResult.specular + pointLightResult.specular;

    float4 diffuse = materialDiffuse * allLightsResult.diffuse;
    float4 specular = materialSpecular * allLightsResult.specular;

    float4 finalColor = (ambient + diffuse + specular) * vsOut.color;
	return finalColor;
}
