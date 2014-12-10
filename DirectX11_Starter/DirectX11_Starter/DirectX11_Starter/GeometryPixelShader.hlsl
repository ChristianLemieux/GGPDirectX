Texture2D myTexture: register(t0);
SamplerState mySampler: register(s0);

struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float2 uv			: TEXCOORD0;
};

float4 main(VertexToPixel input) : SV_TARGET
{
	return myTexture.Sample(mySampler, input.uv);
}