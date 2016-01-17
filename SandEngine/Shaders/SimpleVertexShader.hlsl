struct ViewProjection
{
	column_major matrix model;
	column_major matrix view;
	column_major matrix projection;
};

ConstantBuffer<ViewProjection> ViewProjectionConstantBuffer : register(b0);

struct VertexShaderInput
{
	float3 pos : POSITION;
	float4 color : COLOR0;
	float3 normal : NORMAL0;
	float2 uv : TEXCOORD0;
	uint4 bone : BLENDINDICES0;
	float wieght : BLENDWEIGHT0;
};

struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float4 color : COLOR0;
	float3 normal : NORMAL0;
	float2 uv : TEXCOORD0;
};

PixelShaderInput main(VertexShaderInput input)
{
	PixelShaderInput output;
	float4 pos = float4(input.pos, 1.0f);

	pos = mul(ViewProjectionConstantBuffer.model, pos);
	pos = mul(ViewProjectionConstantBuffer.view, pos);
	pos = mul(ViewProjectionConstantBuffer.projection, pos);

	output.pos = pos;
	output.color = input.color;
	output.normal = input.normal;
	output.uv = input.uv;

	return output;
}