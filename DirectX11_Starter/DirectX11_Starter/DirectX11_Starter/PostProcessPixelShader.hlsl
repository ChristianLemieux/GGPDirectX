Texture2D myTexture: register(t0);
SamplerState mySampler: register(s0);

// Defines the input to this pixel shader
// - Should match the output of our corresponding vertex shader
struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float2 uv			: TEXCOORD0;
};

// Entry point for this pixel shader
float4 main(VertexToPixel input) : SV_TARGET
{

	// Just return the input color
	// - Note that this color (like all values that pass through the rasterizer)
	//   is interpolated for each pixel between the corresponding 
	//   vertices of the triangle

	float4 textureColor = myTexture.Sample(mySampler, input.uv);
	float avg = textureColor.r + textureColor.g + textureColor.b;
	avg /= 3.0f;
	float4 blackAndWhite = float4(avg, avg, avg, 1);
	float4 sepia = float4((textureColor.r * 0.393) + (textureColor.g * 0.769) + (textureColor.b * 0.189), (textureColor.r * 0.349) + (textureColor.g * 0.686) + (textureColor.b * 0.168), (textureColor.r * 0.272) + (textureColor.g * 0.534) + (textureColor.b * 0.131), 1);
	float4 inverse = 1.0f - myTexture.Sample(mySampler, input.uv);
	
	float4 effects[3];
	effects[0] = blackAndWhite;
	effects[1] = sepia;
	effects[2] = inverse;

	return effects[1];

}

