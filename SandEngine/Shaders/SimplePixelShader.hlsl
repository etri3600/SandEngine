struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float4 color : COLOR0;
};

Texture2D diff[] : register(t0);
SamplerState samp : register(s0);

float4 main(PixelShaderInput input) : SV_TARGET
{
	return input.color;
}