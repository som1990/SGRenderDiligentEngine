struct Constants
{
    float4x4 g_WorldViewProj;
    float4x4 g_NormalMatrix;
    float4 g_EyePos;
};


#ifndef __cplusplus
// Vertex shader takes two inputs: vertex position and normal.
// By convention, Diligent Engine expects vertex shader inputs to be 
// labeled 'ATTRIBn', where n is the attribute number.

struct VSInput
{
    float3 Pos : ATTRIB0;
    float3 Normal : ATTRIB1;
};

struct PSInput
{
    float4 Pos : SV_POSITION;
    float3 Normal : NORMAL;
    float3 EyePos : EYE_POS;
    
};

#endif // _cplusplus