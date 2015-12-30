cbuffer ViewProjectionConstantBuffer : register(b0)
{
	column_major matrix model;
	column_major matrix view;
	column_major matrix projection;
};

Texture2D<float4> tex : register(t0);
SamplerState samp : register(s0);

struct VertexShaderInput
{
	float3 pos : POSITION;
	float4 color : COLOR0;
	float3 normal : NORMAL0;
	float2 uv : TEXCOORD0;
};

struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float4 color : COLOR0;
};

PixelShaderInput main(VertexShaderInput input)
{
	PixelShaderInput output;
	float4 pos = float4(input.pos, 1.0f);

	pos = mul(model, pos);
	pos = mul(view, pos);
	pos = mul(projection, pos);

	output.pos = pos;
	output.color = input.color;

	return output;
}