
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


float4 lightPos : POSITION
<
string UIName = "Light Position";
string Object = "PointLight";
string Space = "World";
int refID = 0;
> = { 100.0f, 100.0f, 100.0f, 0.0f };

float4 lightColor : LIGHTCOLOR
<
int LightRef = 0;
> = { 1.0f, 1.0f, 1.0f, 0.0f };

float4 ambiColor : Ambient
<
string UIName = "Ambient Color";
> = { 0.1f, 0.1f, 0.1f, 1.0f };

texture diffuseTexture : DiffuseMap
<
string name = "seafloor.dds";
string UIName = "Diffuse Texture";
>;

texture normalMap : NormalMap
<
string name = "NMP_Ripples2_512.dds";
string UIName = "Normal Texture";
>;

sampler2D cmap = sampler_state
{
	Texture = <diffuseTexture>;
	MinFilter = Linear;
	MagFilter = Linear;
	MipFilter = Linear;
};

sampler2D normalSampler = sampler_state
{
	Texture = <normalMap>;
	MinFilter = Linear;
	MagFilter = Linear;
	MipFilter = Linear;
};

// Defines what kind of data to expect as input
// - This should match our input layout!
struct VertexShaderInput
{
	float3 position	: POSITION; //in object space
	float2 uv		: TEXCOORD0; //in object space
	float4 normal	: NORMAL; //in object space
	float3 T 		: TANGENT; //in object space
	float3 B 		: BINORMAL; //in object space
};




// Defines the output data of our vertex shader
// - At a minimum, you'll need an SV_POSITION
// - Should match the pixel shader's input
struct VertexToPixel
{
	float4 position		: SV_POSITION;	// System Value Position - Has specific meaning to the pipeline!
	float3 normal		: TEXCOORD0;
	float2 uv		    : TEXCOORD1;
	float3 viewDirection : TEXCOORD2;

	float4 HPosition	: POSITION;
	float2 texCoord0	: TEXCOORD3;
	float2 texCoord1 	: TEXCOORD4;
	float3 LightVector	: TEXCOORD5;
};


// The entry point for our vertex shader
VertexToPixel main(VertexShaderInput input)
{
	// Set up output
	VertexToPixel output;
	float4 worldPosition;
	float4 LightPos;
	float4x4 WorldIMatrix;

	// Calculate output position
	matrix worldViewProj = mul(mul(world, view), projection);
	output.position = mul(float4(input.position, 1.0f), worldViewProj);
	output.normal = normalize(mul(input.normal, (float3x3)world));
	output.uv = input.uv;
	worldPosition = mul(input.position, world);
	output.viewDirection = normalize(cameraPosition.xyz - worldPosition.xyz);

	//create the vertex out struct
	output.texCoord0 = input.uv; //pass coords for diffuse map
	output.texCoord1 = input.uv; //pass coords for normal map

	// compute the 3x3 tranform from tangent space to object space
	float3x3 objToTangentSpace;
	objToTangentSpace[0] = input.B;
	objToTangentSpace[1] = -input.T;
	objToTangentSpace[2] = input.normal;

	//put the vert position in world space
	float4 worldSpacePos = mul(input.position, world);

		//cast a ray to the light
		float4 normLightVec = LightPos - worldSpacePos;

		//transform the light vector to object space
		float3 objnormLightVec = mul(normLightVec, WorldIMatrix).xyz;
		objnormLightVec = normalize(objnormLightVec);

	// transform light vector from object space to tangent space and pass it as a color 
	output.LightVector = 0.5 * mul(objToTangentSpace, objnormLightVec) + 0.5;

	// transform position to projection space
	output.HPosition = mul(input.position, worldPosition);

	output.LightVector = 0;
	// transform position to projection space
	output.HPosition = mul(input.position, worldPosition);
		return output;
}