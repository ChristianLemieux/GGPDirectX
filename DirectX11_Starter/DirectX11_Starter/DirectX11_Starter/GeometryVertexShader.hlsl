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

float3 calculatePosition(float2 vel, float2 acceleration, float3 position)
{
	float2 velocity = float2(acceleration.x * age + vel.x, acceleration.y * age + vel.y);
	return float3(age * velocity.x, age* velocity.y, position.z);
}

VertexToPixel main(VertexShaderInput input)
{
	VertexToPixel output;

	// Calculate output position
	matrix worldViewProj = mul(mul(world, view), projection);
	float3 position = input.position + calculatePosition(input.velocity, input.acceleration, input.position);
	output.position = mul(float4(position, 1.0f), worldViewProj);

	return output;
}