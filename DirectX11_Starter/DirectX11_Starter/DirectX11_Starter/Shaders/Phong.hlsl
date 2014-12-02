//http://www.gamasutra.com/view/feature/131275/implementing_lighting_models_with_.php?print=1


// The constant buffer that holds our "per model" data
// - Each object you draw with this shader will probably have
//   slightly different data (at least for the world matrix)
cbuffer perModel : register(b0)
{
	matrix world;
	matrix view;
	matrix projection;
};

// Defines what kind of data to expect as input
// - This should match our input layout!
struct VertexShaderInput
{
	float3 position		: POSITION;
	float4 color		: COLOR;
	float2 uv		    : TEXCOORD0;
	float3 normal		: NORMAL0;
	float3 lightPos		: TEXCOORD2;
};

// Defines the output data of our vertex shader
// - At a minimum, you'll need an SV_POSITION
// - Should match the pixel shader's input
struct VertexToPixel
{
	float4 position		: SV_POSITION;	// System Value Position - Has specific meaning to the pipeline!
	float4 color		: COLOR;
	float2 uv		    : TEXCOORD0;
	float3 normal		: TEXCOORD1;
	float3 lightPos		: TEXCOORD2;
};

// The entry point for our vertex shader
VertexToPixel main(VertexShaderInput input)
{
	// Set up output
	VertexToPixel output;

	// Calculate output position
	matrix worldViewProj = mul(mul(world, view), projection);
	output.position = mul(float4(input.position, 1.0f), worldViewProj);
	output.normal = input.normal;
	output.uv = input.uv;
	output.lightPos = input.lightPos;
	
	float3 norm = normalize(input.normal);
	float3 LightDir = normalize(input.lightPos);
	float3 viewDir = normalize(float3(0.0f, 0.0f, -1.0f) - normalize(mul(input.position, world)));
	float4 diffuse = { 1.0f, 0.0f, 0.0f, 1.0f };
	float4 diff = saturate(dot(LightDir, norm));

	float3 reflect = normalize(2 * diff * norm - LightDir);
	float4 specular = pow(saturate(dot(reflect, viewDir)), 8);
	output.color = input.color + diffuse * diff + specular;
	return output;
}