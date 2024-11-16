#include "Basic.hlsli"

// 像素着色器(3D)
float4 PS_3D(VertexPosHWNormalTex pIn) : SV_Target
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
        DirectionalLight dirLight = g_DirLight[i];
        [flatten]
        if (g_IsReflection)
        {
            dirLight.Direction = mul(dirLight.Direction, (float3x3) (g_Reflection));
        }
        ComputeDirectionalLight(mat, dirLight, pIn.NormalW, toEyeW, A, D, S);
        ambient += A;
        diffuse += D;
        spec += S;
    }
    
    [unroll]
    for (i = 0; i < g_NumPointLight; ++i)
    {
        PointLight pointLight = g_PointLight[i];
        [flatten]
        if (g_IsReflection)
        {
            pointLight.Position = (float3) mul(float4(pointLight.Position, 1.0f), g_Reflection);
        }
        ComputePointLight(mat, pointLight, pIn.PosW, pIn.NormalW, toEyeW, A, D, S);
        ambient += A;
        diffuse += D;
        spec += S;
    }
    
    [unroll]
    for (i = 0; i < g_NumSpotLight; ++i)
    {
        SpotLight spotLight = g_SpotLight[i];
        [flatten]
        if (g_IsReflection)
        {
            spotLight.Position = (float3) mul(float4(spotLight.Position, 1.0f), g_Reflection);
            spotLight.Direction = mul(spotLight.Direction, (float3x3) g_Reflection);
        }
        ComputeSpotLight(mat, spotLight, pIn.PosW, pIn.NormalW, toEyeW, A, D, S);
        ambient += A;
        diffuse += D;
        spec += S;
    }
    
    float2 tex = pIn.Tex;
    tex += m_TexOffset;
    
    float4 texColor = g_Tex.Sample(g_SamLinear, tex);
    float4 litColor = texColor * (ambient + diffuse) + spec;
    litColor.a = texColor.a * g_Material.Diffuse.a;
    
    return litColor;
}