
Texture2D myTexture : register(t0);
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
	float3 reflection = reflect(-lightDirection, input.normal);
	float4 textureColor = myTexture.Sample(mySampler, input.uv);
	float4 specular = pow(saturate(dot(reflection, -input.viewDirection)), specularPower) * specularColor;
	float4 diffuse = lerp(diffuseColor, textureColor, 0.85f) * saturate(dot(input.normal, -lightDirection)) * 0.8f;
	float4 color = saturate(diffuse + ambientColor + specular);
	return color;
}

