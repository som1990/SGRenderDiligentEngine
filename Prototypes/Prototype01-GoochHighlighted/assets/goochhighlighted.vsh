#include "structures.fxh"

// Note that if separate shader objects are not supported (this is only the case for old GLES3.0 devices), vertex
// shader output variable name must match exactly the name of the pixel shader input variable.
// If the variable has structure type (like in this example), the structure declarations must also be identical.

cbuffer cbConstants
{
    Constants g_Constants;
}


void main(in VSInput VSIn,
          out PSInput PSIn)
{
    PSIn.Pos = mul(float4(VSIn.Pos, 1.0), g_Constants.g_WorldViewProj);
    PSIn.Normal = mul(float4(VSIn.Normal, 0.0), g_Constants.g_NormalMatrix).xyz;
    PSIn.EyePos = g_Constants.g_EyePos.xyz;
}