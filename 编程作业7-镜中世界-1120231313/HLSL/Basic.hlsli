#include "LightHelper.hlsli"

Texture2D g_Tex : register(t0);
SamplerState g_SamLinear : register(s0);


cbuffer CBChangesEveryDrawing : register(b0)
{
    matrix g_World;
    matrix g_WorldInvTranspose;
    Material g_Material;
    float4 m_Color;
    float2 m_TexOffset;
    float m_TexScale;
}

cbuffer CBChangesEveryFrame : register(b1)
{
    matrix g_View;
    float3 g_EyePosW;
    DirectionalLight g_DirLight[10];
    PointLight g_PointLight[10];
    SpotLight g_SpotLight[10];
    int g_NumDirLight;
    int g_NumPointLight;
    int g_NumSpotLight;
}

cbuffer CBChangesOnResize : register(b2)
{
    matrix g_Proj;
}

cbuffer CBChangesRarely : register(b3)
{
    int g_IsReflection;
    matrix g_Reflection;
    float3 g_Pad;
}



struct VertexPosNormalTex
{
    float3 PosL : POSITION;
    float3 NormalL : NORMAL;
    float2 Tex : TEXCOORD;
};

struct VertexPosNormalColor
{
    float3 PosL : POSITION;
    float3 NormalL : NORMAL;
    float4 Color : COLOR;
};

struct VertexPosTex
{
    float3 PosL : POSITION;
    float2 Tex : TEXCOORD;
};


struct VertexPosHWNormalTex
{
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION; // 在世界中的位置
    float3 NormalW : NORMAL; // 法向量在世界中的方向
    float2 Tex : TEXCOORD;
};

struct VertexPosHWNormalColor
{
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION; // 在世界中的位置
    float3 NormalW : NORMAL; // 法向量在世界中的方向
    float4 Color : COLOR;
};

struct VertexPosHTex
{
    float4 PosH : SV_POSITION;
    float2 Tex : TEXCOORD;
};











