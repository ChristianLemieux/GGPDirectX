Texture2D myTexture: register(t0);
SamplerState mySampler: register(s0);

// Defines the input to this pixel shader
// - Should match the output of our corresponding vertex shader
struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float3 normal		: TEXCOORD0;
	float2 uv			: TEXCOORD1;
	float3 viewDirection : TEXCOORD2;
};

cbuffer LightBuffer
{
	float4 ambientColor;
	float4 diffuseColor;
	float3 lightDirection;
	float specularPower;
	float4 specularColor;
};

// Entry point for this pixel shader
float4 main(VertexToPixel input) : SV_TARGET
{
	float3 reflection;
	float4 diff;
	// Just return the input color
	// - Note that this color (like all values that pass through the rasterizer)
	//   is interpolated for each pixel between the corresponding 
	//   vertices of the triangle
	float4 textureColor = myTexture.Sample(mySampler, input.uv);
		float4 color = ambientColor;
		float3 lightDir = -lightDirection;
		float4 specular = specularColor;
		float lightIntensity = saturate(dot(input.normal, lightDir));

	if (lightIntensity > 0.0f)
	{
		color += (diffuseColor * lightIntensity);
		color = saturate(color);
		reflection = normalize(2 * lightIntensity * input.normal - lightDir);
		specular = pow(saturate(dot(reflection, input.viewDirection)), specularPower);

	}
	color = color * textureColor;
	color = saturate(color + specular);

	return color;
}

