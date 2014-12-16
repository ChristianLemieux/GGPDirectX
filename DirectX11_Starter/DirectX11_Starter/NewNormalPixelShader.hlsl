Texture2D myTexture: register(t0);
SamplerState mySampler: register(s0);

Texture2D bumps: register(t1);
SamplerState samLinear
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = WRAP;
	AddressV = WRAP;
};

float3 NormalSampleToWorldSpace(float3 normalMapSample, float3 unitNormalW, float3 tangentW)
{
	// Uncompress each component from [0,1] to [-1,1].
	float3 normalT = 2.0f*normalMapSample - 1.0f;
		// Build orthonormal basis.
		float3 N = unitNormalW;
		float3 T = normalize(tangentW - dot(tangentW, N)*N);
		float3 B = cross(N, T);
		float3x3 TBN = float3x3(T, B, N);
		// Transform from tangent space to world space.
		float3 bumpedNormalW = mul(normalT, TBN);

		return bumpedNormalW;
}

// Defines the input to this pixel shader
// - Should match the output of our corresponding vertex shader
struct VertexToPixel
{
	float4 position		 : SV_POSITION;
	float3 posW          : POSITION;
	float3 normal		 : NORMAL;
	float2 uv			 : TEXCOORD1;
	float3 tangent		 : TANGENT;
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
	// Interpolating normal can unnormalize it, so normalize it.
	input.normal = normalize(input.normal);

	//Normal Mapping
	float3 normalMapSample = bumps.Sample(mySampler, input.normal).rgb;
		float3 bumpedNormalW = NormalSampleToWorldSpace(
		normalMapSample, input.normal, input.tangent);

	float3 reflection = reflect(-lightDirection, bumpedNormalW);
		float4 textureColor = myTexture.Sample(mySampler, input.uv);
		//float4 texturecolor = float4(1.0f,1.0f,1.0f,1.0f);
		float4 specular = pow(saturate(dot(reflection, -input.posW)), specularPower) * specularColor;
		float4 diffuse = lerp(diffuseColor, textureColor, 0.85f) * saturate(dot(bumpedNormalW, -lightDirection)) * 0.8f;

		float4 color = saturate(diffuse + ambientColor + specular);
		return color;
}

