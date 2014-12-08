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

	return float4(avg, avg, avg, 1);

}