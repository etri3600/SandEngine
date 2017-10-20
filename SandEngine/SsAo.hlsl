Texture2D<half4> colorTexture : register(t0);
Texture2D<half4> depthTexture : register(t1);
Texture2D<half4> normalTexture : register(t2);

SamplerState linearClampSampler : register(s0);

struct ViewProjection
{
	column_major matrix View;
	column_major matrix Projection;
	column_major matrix InvView;
	column_major matrix InvProjection;
};

ConstantBuffer<ViewProjection> ViewProjectionConstantBuffer : register(b0);

static const float2 gTexCoords[6] =
{
	float2(0.0f, 1.0f),
	float2(1.0f, 0.0f),
	float2(0.0f, 0.0f),
	float2(0.0f, 1.0f),
	float2(1.0f, 1.0f),
	float2(1.0f, 0.0f),
};

struct FragmentInput
{
	float4 SvPosition : SV_POSITION;
	float3 ViewPosition : POSITION;
	float2 TexCoord : TEXCOORD0;
};

FragmentInput vert(uint vid : SV_VertexID)
{
	FragmentInput output;
	output.TexCoord = gTexCoords[vid];
	output.SvPosition = float4(2.0f * output.TexCoord.x - 1.0f, 1.0f - 2.0f * output.TexCoord.y, 0.0f, 1.0f);
	float4 hPos = mul(output.SvPosition, ViewProjectionConstantBuffer.InvProjection);
	output.ViewPosition = hPos.xyz / hPos.w;

	return output;
}

float4 frag(FragmentInput input) : SV_Target0
{
	half4 color = colorTexture.Sample(linearClampSampler, input.TexCoord);
	half depth = depthTexture.Sample(linearClampSampler, input.TexCoord).z;
	half4 normal = normalTexture.Sample(linearClampSampler, input.TexCoord);

	return color;
}