cbuffer perModel : register(b0)
{
	matrix world;
	matrix view;
	matrix projection;
};

cbuffer geoBuffer : register(b1)
{
	float age;
};

struct VertexShaderInput
{
	float3 position		: POSITION;
	float2 velocity		: TEXCOORD2;
	float2 acceleration	: TEXCOORD3;
};

struct VertexToPixel
{
	float4 position		: SV_POSITION;
};

float3 calculatePosition(float2 velocity, float3 position)
{
	return float3(age * velocity.x, age* velocity.y, position.z);
}

VertexToPixel main(VertexShaderInput input)
{
	VertexToPixel output;

	// Calculate output position
	matrix worldViewProj = mul(mul(world, view), projection);
	float3 position = input.position + calculatePosition(input.velocity, input.position);
	output.position = mul(float4(position, 1.0f), worldViewProj);

	return output;
}