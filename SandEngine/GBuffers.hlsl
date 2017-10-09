struct ModelViewProjection
{
	column_major matrix model;
	column_major matrix view;
	column_major matrix projection;
	column_major matrix normalMatrix;
};

ConstantBuffer<ModelViewProjection> MVPConstantBuffer : register(b0);

struct GBufferVertexPosInput
{
	float3 pos : POSITION;
};

struct GBufferFragmentPosInput
{
	float4 fragPos : SV_POSITION;
	float4 viewPos : TEXCOORD0;
};

GBufferFragmentPosInput vertPos(GBufferVertexPosInput input)
{
	GBufferFragmentPosInput output;

	float4 posW = mul(MVPConstantBuffer.model, float4(input.pos, 1.0f));
	float4 posV = mul(MVPConstantBuffer.view, posW);
	pos = mul(MVPConstantBuffer.projection, posV);

	output.fragPos = pos;
	output.viewPos = posV;

	return output;
}

float4 fragPos(GBufferFragmentPosInput input) : SV_TARGET
{
	return input.viewPos;
}

struct GBufferVertexNormalInput
{
	float3 pos : POSITION;
	float3 normal : NORMAL;
};

struct GBufferFragmentNormalInput{
	float4 fragPos : SV_POSITION;
	float4 viewNormal : NORMAL;
};

GBufferFragmentNormalInput vertNormal(GBufferVertexNormalInput input)
{
	GBufferFragmentNormalInput output;

	float4 posW = mul(MVPConstantBuffer.model, float4(input.pos, 1.0f));
	float4 posV = mul(MVPConstantBuffer.view, posW);
	pos = mul(MVPConstantBuffer.projection, posV);

	float4 normalW = mul(ViewProjectionConstantBuffer.normalMatrix, input.normal);
	
	output.fragPos = pos;
	output.viewNormal = mul(ViewProjectionConstantBuffer.view, normalW);

	return output;
}

float4 fragNormal(GBufferFragmentNormalInput input) : SV_TARGET
{
	return input.viewNormal;
}