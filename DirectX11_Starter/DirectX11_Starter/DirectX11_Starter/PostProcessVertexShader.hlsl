struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float2 uv		    : TEXCOORD0;
};

struct VertexInput
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float2 uv : TEXCOORD0;
};

VertexToPixel main(VertexInput input)
{
	VertexToPixel output;

	output.position = float4(input.position, 1);
	output.uv = input.uv;

	return output;
}