
// The constant buffer that holds our "per model" data
// - Each object you draw with this shader will probably have
//   slightly different data (at least for the world matrix)
cbuffer perModel : register(b0)
{
	matrix world;
	matrix view;
	matrix projection;
};

cbuffer CameraBuffer : register(b1)
{
	float3 cameraPosition;
	float padding;
};

// Defines what kind of data to expect as input
// - This should match our input layout!
struct VertexShaderInput
{
	float3 position		: POSITION;
	float3 normal		: NORMAL;
	float2 uv		    : TEXCOORD1;
	float3 tangent		: TANGENT;
	float3 binormal		: BINORMAL;
};

// Defines the output data of our vertex shader
// - At a minimum, you'll need an SV_POSITION
// - Should match the pixel shader's input
struct VertexToPixel
{
	float4 position		 : SV_POSITION;	// System Value Position - Has specific meaning to the pipeline!
	float3 normal		 : TEXCOORD0;
	float2 uv		     : TEXCOORD1;
	float3 viewDirection : TEXCOORD2;
	float3 tangent		 : TANGENT;
	float3 binormal		 : BINORMAL;
};

// The entry point for our vertex shader
VertexToPixel main(VertexShaderInput input)
{
	// Set up output
	VertexToPixel output;
	float4 worldPosition;

	// Calculate output position
	matrix worldViewProj = mul(mul(world, view), projection);
	output.position = mul(float4(input.position, 1.0f), worldViewProj);
	output.uv = input.uv;

	worldPosition = mul(input.position, world);
	output.viewDirection = normalize(cameraPosition.xyz - worldPosition.xyz);

	// Calculate the normal vector against the world matrix only and then normalize the final value.
	output.normal = normalize(mul(input.normal, (float3x3)world));

	// Calculate the tangent vector against the world matrix only and then normalize the final value.
	output.tangent = normalize(mul(input.tangent, (float3x3)world));

	// Calculate the binormal vector against the world matrix only and then normalize the final value.
	output.binormal = normalize(mul(input.binormal, (float3x3)world));

	output.uv = input.uv;

	return output;
}