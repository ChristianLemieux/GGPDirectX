Texture2D myTexture: register(t0);
Texture2D myTexture2: register(t1);
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
	reflection = reflect(-lightDirection, input.normal);
	float4 textureColor2 = myTexture2.Sample(mySampler, reflection);
	float4 specular = pow(saturate(dot(reflection, -input.viewDirection)), specularPower) * specularColor;
	float4 diffuse = lerp(diffuseColor, textureColor, 0.85f) * saturate(dot(input.normal, -lightDirection)) * 0.2f;
		//float4 diffuse = diffuseColor * saturate(dot(input.normal, -lightDirection)*0.2f);
	float4 color = saturate(diffuse + ambientColor + specular);
	return textureColor2 * 0.3f + color;
}

