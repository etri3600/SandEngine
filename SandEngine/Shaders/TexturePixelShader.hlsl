struct PixelShaderInput
{
	float4 fragPos : SV_POSITION;
	float3 PosW : POSITION0;
	float4 color : COLOR0;
	float3 NormalW : NORMAL0;
	float3 TangentW : TANGENT0;
	float2 uv : TEXCOORD0;
};

struct FragmentOutput
{
	float4 Color : SV_Target0;
	float4 Depth : SV_Target1;
	float4 Normal : SV_Target2;
};

Texture2D<half4> diff : register(t0);
SamplerState samp : register(s0);

FragmentOutput main(PixelShaderInput input)
{
	FragmentOutput output;
	output.Color = diff.Sample(samp, input.uv);
	output.Depth = float4(1, 1, 0, 1);
	output.Normal = float4(1, 0, 1, 1);
	return output;
}