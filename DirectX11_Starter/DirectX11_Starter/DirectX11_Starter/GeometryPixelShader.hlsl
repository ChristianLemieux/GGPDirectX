Texture2D myTexture: register(t0);
SamplerState mySampler: register(s0);

struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float3 normal		: TEXCOORD0;
	float2 uv			: TEXCOORD1;
};

float4 main(VertexToPixel input) : SV_TARGET
{
	return myTexture.Sample(mySampler, input.uv);
}