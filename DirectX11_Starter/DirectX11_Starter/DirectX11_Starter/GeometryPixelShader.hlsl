Texture2D myTexture: register(t0);
Texture2D alphaMap: register(t1);
SamplerState mySampler: register(s0);

struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float2 uv			: TEXCOORD0;
};

float opacity(float4 color)
{
	return color.r + color.g + color.b/ 3;
}

float4 main(VertexToPixel input) : SV_TARGET
{
	float4 color = myTexture.Sample(mySampler, input.uv);
	float4 alpha = alphaMap.Sample(mySampler, input.uv);
	color.a = 0.5f;

	return color;
}