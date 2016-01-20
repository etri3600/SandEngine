struct PixelShaderInput
{
	float4 fragPos : SV_POSITION;
	float3 PosW : POSITION0;
	float4 color : COLOR0;
	float3 NormalW : NORMAL0;
	float3 TangentW : TANGENT0;
	float2 uv : TEXCOORD0;
};

Texture2D diff[] : register(t0);
SamplerState samp : register(s0);

float4 main(PixelShaderInput input) : SV_TARGET
{
	return input.color;
}