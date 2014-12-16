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
};

// Defines the output data of our vertex shader
// - At a minimum, you'll need an SV_POSITION
// - Should match the pixel shader's input
struct VertexToPixel
{
	float4 position		 : SV_POSITION;	// System Value Position - Has specific meaning to the pipeline!
	float3 posW			 : POSITION;
	float3 normal		 : NORMAL;
	float2 uv		     : TEXCOORD1;
	float3 tangent		 : TANGENT;
};

// The entry point for our vertex shader
VertexToPixel main(VertexShaderInput vin)
{
	// Set up output
	VertexToPixel vout;

	matrix worldViewProj = mul(mul(world, view), projection);

	// Transform to world space space.
	vout.posW = mul(float4(vin.position, 1.0f), world).xyz;
	vout.normal = normalize(mul(vin.normal, (float3x3)world));
	vout.tangent = normalize(mul(vin.tangent, (float3x3)world));

	// Transform to homogeneous clip space.
	vout.position = mul(float4(vin.position, 1.0f), worldViewProj);
	// Output vertex attributes for interpolation across triangle.

	vout.uv = vin.uv;
	return vout;
}