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
	float4 white = float4(1.0f, 1.0f, 1.0f, 1.0f);
	return myTexture.Sample(mySampler, input.uv);
}