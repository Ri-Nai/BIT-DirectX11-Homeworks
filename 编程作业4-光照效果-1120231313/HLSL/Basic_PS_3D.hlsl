#include "Basic.hlsli"

// 像素着色器(3D)
float4 PS_3D(VertexPosHWNormalColor pIn) : SV_Target
{
    // 标准化法向量
    pIn.NormalW = normalize(pIn.NormalW);
    
    // 初始化材质
    Material mat = g_Material;

    // 顶点指向眼睛的向量
    float3 toEyeW = normalize(g_EyePosW - pIn.PosW);

    // 初始化为0 
    float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 A = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 D = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 S = float4(0.0f, 0.0f, 0.0f, 0.0f);
    int i;
	// 强制展开循环以减少指令数
	[unroll]
    for (i = 0; i < g_NumDirLight; ++i)
    {
        ComputeDirectionalLight(mat, g_DirLight[i], pIn.NormalW, toEyeW, A, D, S);
        ambient += A;
        diffuse += D;
        spec += S;
    }
    
	[unroll]
    for (i = 0; i < g_NumPointLight; ++i)
    {
        ComputePointLight(mat, g_PointLight[i], pIn.PosW, pIn.NormalW, toEyeW, A, D, S);
        ambient += A;
        diffuse += D;
        spec += S;
    }
    
	[unroll]
    for (i = 0; i < g_NumSpotLight; ++i)
    {
        ComputeSpotLight(mat, g_SpotLight[i], pIn.PosW, pIn.NormalW, toEyeW, A, D, S);
        ambient += A;
        diffuse += D;
        spec += S;
    }
  
    float4 litColor = pIn.Color * (ambient + diffuse) + spec;
    litColor.a = pIn.Color.a * mat.Diffuse.a;
	
    return litColor;
}