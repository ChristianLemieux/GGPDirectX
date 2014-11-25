Texture2D myTexture: register(t0);
SamplerState mySampler: register(s0);

//for normals
Texture2D shaderTextures[2];
SamplerState SampleType;

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
	
    return color;
}

