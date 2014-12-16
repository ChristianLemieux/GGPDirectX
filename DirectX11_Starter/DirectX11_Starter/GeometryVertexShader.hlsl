cbuffer geoBuffer : register(b0)
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

float3 calculatePosition(float2 velocity, float2 acceleration, float3 position)
{
	return float3(0.5f * age * acceleration.x + age * velocity.x + position.x, 0.5f * age * acceleration.y + age * velocity.y + position.y, position.z);
}

VertexToPixel main(VertexShaderInput input)
{
	VertexToPixel output;

	// Calculate output position
	//matrix worldViewProj = mul(mul(world, view), projection);
	//float3 
	output.position = float4(calculatePosition(input.velocity, input.acceleration, input.position), 1.0);
	//output.position = mul(float4(position, 1.0f), worldViewProj);
	return output;
}