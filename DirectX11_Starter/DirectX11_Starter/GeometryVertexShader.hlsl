cbuffer geoBuffer : register(b0)
{
	float age;
};

struct VertexShaderInput
{
	float4 position		: POSITION;
	float2 velocity		: TEXCOORD0;
	float2 acceleration	: TEXCOORD1;
};

struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float2 velocity : TEXCOORD0;
	float2 acceleration : TEXCOORD1;
};

float4 calculatePosition(float2 velocity, float2 acceleration, float4 position)
{
	return float4(0.5f * age * acceleration.x + age * velocity.x + position.x, 0.5f * age * acceleration.y + age * velocity.y + position.y, position.z, 1.0f);
}

VertexToPixel main(VertexShaderInput input)
{
	VertexToPixel output;

	output.position = calculatePosition(input.velocity, input.acceleration, input.position);
	output.velocity = input.velocity;
	output.acceleration = input.acceleration;
	return output;
}