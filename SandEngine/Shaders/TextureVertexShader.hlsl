struct ViewProjection
{
	column_major matrix model;
	column_major matrix view;
	column_major matrix projection;
	column_major matrix normalMatrix;
};

ConstantBuffer<ViewProjection> ViewProjectionConstantBuffer : register(b0);

struct VertexShaderInput
{
	float3 pos : POSITION;
	float4 color : COLOR0;
	float3 normal : NORMAL0;
	float4 tangent : TANGENT0;
	float2 uv : TEXCOORD0;
};

struct PixelShaderInput
{
	float4 fragPos : SV_POSITION;
	float3 PosW : POSITION0;
	float4 color : COLOR0;
	float3 NormalW : NORMAL0;
	float3 TangentW : TANGENT0;
	float2 uv : TEXCOORD0;
};

PixelShaderInput main(VertexShaderInput input)
{
	PixelShaderInput output;

	float4 posW = mul(ViewProjectionConstantBuffer.model, float4(input.pos, 1.0f));
	float4 pos = mul(ViewProjectionConstantBuffer.view, posW);
	pos = mul(ViewProjectionConstantBuffer.projection, pos);

	output.fragPos = pos;
	output.PosW = posW.xyz;
	output.color = input.color;
	output.NormalW = mul(ViewProjectionConstantBuffer.normalMatrix, input.normal).xyz;
	output.TangentW = mul(ViewProjectionConstantBuffer.normalMatrix, input.tangent).xyz;
	output.uv = input.uv;

	return output;
}