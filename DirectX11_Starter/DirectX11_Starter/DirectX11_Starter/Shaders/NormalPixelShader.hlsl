Texture2D myTexture: register(t0);
SamplerState mySampler: register(s0);

//for normals
Texture2D shaderTextures[3];
SamplerState SampleType;
Texture2D texture2;
float BumpConstant = 1;
texture NormalMap;
SamplerState bumpSampler = sampler_state {
	Texture = (NormalMap);
	MinFilter = Linear;
	MagFilter = Linear;
	AddressU = Wrap;
	AddressV = Wrap;
};
SamplerState textureSampler = sampler_state {
	Texture = (ModelTexture);
	MinFilter = Linear;
	MagFilter = Linear;
	AddressU = Clamp;
	AddressV = Clamp;
};
// Defines the input to this pixel shader
// - Should match the output of our corresponding vertex shader
struct VertexToPixel
{
	float4 position		 : SV_POSITION;
	float3 normal		 : TEXCOORD0;
	float2 uv			 : TEXCOORD1;
	float3 viewDirection : TEXCOORD2;
	float3 tangent		 : TANGENT;
	float3 binormal		 : BINORMAL;
};

cbuffer LightBuffer
{
	float4 ambientColor;
	float4 diffuseColor;
	float3 lightDirection;
	float specularPower;
	float4 specularColor;
	float4 textureColor;
};

// Entry point for this pixel shader
float4 main(VertexToPixel input) : SV_TARGET
{
	float3 reflection = reflect(-lightDirection, input.normal);
	float4 textureColor = myTexture.Sample(mySampler, input.uv);
	float4 specular = pow(saturate(dot(reflection, -input.viewDirection)), specularPower) * specularColor;
	float4 diffuse = lerp(diffuseColor, textureColor, 0.85f) * saturate(dot(input.normal, -lightDirection)) * 0.8f;
	float4 color = saturate(diffuse + ambientColor + specular);

	float4 bumpMap;
	float3 bumpNormal;
	float3 lightDir;
	float lightIntensity;
	float3 DiffuseLightDirection = float3(1, 0, 0);
		float3 ViewVector = float3(1, 0, 0);
		float4x4 World;
	float SpecularIntensity = 1;
	float4 SpecularColor = float4(1, 1, 1, 1);
		float Shininess = 200;
	float4 AmbientColor = float4(1, 1, 1, 1);
		float AmbientIntensity = 0.1;

	// Sample the pixel in the bump map.
	bumpMap = shaderTextures[1].Sample(SampleType, input.normal);

	// Expand the range of the normal value from (0, +1) to (-1, +1).
	bumpMap = (bumpMap * 2.0f) - 1.0f;

	// Calculate the normal from the data in the bump map.
	bumpNormal = (bumpMap.x * input.tangent) + (bumpMap.y * input.binormal) + (bumpMap.z * input.normal);

	// Normalize the resulting bump normal.
	bumpNormal = normalize(bumpNormal);

	// Invert the light direction for calculations.
	lightDir = -lightDirection;

	// Calculate the amount of light on this pixel based on the bump map normal value.
	lightIntensity = saturate(dot(bumpNormal, lightDir));

	// Combine the final bump light color with the texture color.
	color = color * textureColor;

	// Calculate the normal, including the information in the bump map
	float4 bump = BumpConstant * (texture2.Sample(bumpSampler, input.uv)) - (0.5, 0.5, 0.5);
		bumpNormal = input.normal + (bump.x * input.tangent + bump.y * input.binormal);
	bumpNormal = normalize(bumpNormal);

	// Calculate the diffuse light component with the bump map normal
	float diffuseIntensity = dot(normalize(DiffuseLightDirection), bumpNormal);
	if (diffuseIntensity < 0)
		diffuseIntensity = 0;

	// Calculate the specular light component with the bump map normal
	float3 light = normalize(DiffuseLightDirection);
		float3 r = normalize(2 * dot(light, bumpNormal) * bumpNormal - light);
		float3 v = normalize(mul(normalize(ViewVector), lightDir));
		float dotProduct = dot(r, v);

	specular = SpecularIntensity * SpecularColor * max(pow(dotProduct, Shininess), 0) * diffuseIntensity;

	// Calculate the texture color
	textureColor = texture2.Sample(textureSampler, input.uv);
	textureColor.a = 1;

	// Combine all of these values into one (including the ambient light)
	return saturate(textureColor + AmbientColor * AmbientIntensity * specular);
}

