#include "Basic.hlsli"

[maxvertexcount(6)]
void GS_3D(triangle VertexPosHWNormalColor input[3], inout TriangleStream<VertexPosHWNormalColor> output)
{
    output.Append(input[0]);
    output.Append(input[1]);
    output.Append(input[2]);
    output.RestartStrip();
    
    VertexPosHWNormalColor buf;
    float X = 30.0f;
    matrix viewProj = mul(g_View, g_Proj);
    [unroll]
    for (int i = 0; i < 3; i++)
    {
        buf = input[i];
        buf.PosW = float3(2 * X - buf.PosW.x, buf.PosW.y, buf.PosW.z);
        buf.PosH = mul(float4(buf.PosW, 1.0f), viewProj);
        buf.NormalW = -buf.NormalW;
        buf.Color = float4(0.3f, 0.3f, 0.3f, 1.0f);
        
        output.Append(buf);
    }
    output.RestartStrip();
}
