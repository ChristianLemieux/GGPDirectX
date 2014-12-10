Texture2D myTexture: register(t0);
SamplerState mySampler: register(s0);

Texture2D bumps: register(t1);
SamplerState samLinear
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = WRAP;
	AddressV = WRAP;
};

float3 NormalSampleToWorldSpace(float3 normalMapSample,
	float3 unitNormalW,
	float3 tangentW)
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
	

	// Interpolating normal can unnormalize it, so normalize it.
	input.normal = normalize(input.normal);
	float3 toEye = lightDirection - input.viewDirection;
	// Cache the distance to the eye from this surface point. 
	float distToEye = length(toEye);
	// Normalize.
	toEye /= distToEye;

	// Default to multiplicative identity.
	float4 texColor = float4(1, 1, 1, 1);
	// Sample texture.
	texColor = bumps.Sample(samLinear, input.normal);

	//Normal Mappign
	float3 normalMapSample = bumps.Sample(samLinear, input.normal).rgb;
	float3 bumpedNormalW = NormalSampleToWorldSpace(
	normalMapSample, input.normal, input.tangent);

	float4 color = saturate(diffuse + ambientColor + specular);
	return color;
}

