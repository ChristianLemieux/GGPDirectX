Texture2D myTexture: register(t0);
SamplerState mySampler: register(s0);

// Defines the input to this pixel shader
// - Should match the output of our corresponding vertex shader
struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float3 normal		: TEXCOORD0;
	float2 uv			: TEXCOORD1;
	float3 viewDirection: TEXCOORD2;
	float3 LightVector	: TEXCOORD3;
};

cbuffer LightBuffer
{
	float4 ambientColor;
	float4 diffuseColor;
	float3 lightDirection;
	float specularPower;
	float4 specularColor;
};

float4 lightColor : LIGHTCOLOR
<
int LightRef = 0;
> = { 1.0f, 1.0f, 1.0f, 0.0f };

float4 ambiColor : Ambient
<
string UIName = "Ambient Color";
> = { 0.1f, 0.1f, 0.1f, 1.0f };


//pixel output
struct PixelOut
{
	float4 col : COLOR;
};

// Entry point for this pixel shader
PixelOut main(VertexToPixel input) : SV_TARGET
{
	float4 LightColor;
	float4 AmbientColor;

	float3 reflection = reflect(-lightDirection, input.normal);
	float4 specular = pow(saturate(dot(reflection, -input.viewDirection)), specularPower) * specularColor;

	PixelOut OUT;

	//get the color from the diffuse texture
	float4 texColor = myTexture.Sample(mySampler, input.uv);

		//get the color from the normal map and convert to normal
		float3 bumpNormal = (2 * (myTexture.Sample(mySampler, input.uv) - 0.5));

		//unpack the light vector to [-1,1]
		float3 lightVector = 2 * (input.LightVector - 0.5);

		//compute the angle to the light and clamp at zero
		float bump = max(dot(bumpNormal, lightVector), 0);

	//compute final color (diffuse + ambient)
	float3 diffuse = texColor * bump * lightColor;
		float3 ambient = texColor * ambiColor*0.5;
		OUT.col.rgb = diffuse + ambient;
	OUT.col.a = 1.0;

	return OUT;

}

