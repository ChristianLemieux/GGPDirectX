struct VSOutput
{
	float4 position		: SV_POSITION;
	float3 normal		: TEXCOORD0;
	float2 uv			: TEXCOORD1;
};

struct GSOutput
{
	float4 position		: SV_POSITION;
	float3 normal		: TEXCOORD0;
	float2 uv			: TEXCOORD1;
};



[maxvertexcount(4)]
void main(
	point VSOutput dataIn[1],
	inout TriangleStream< GSOutput > output
	)
{
	float4 v[4];

	v[0] = dataIn[0].position + float4(0.1f, -0.1f, 0.0f, 0);
	v[1] = dataIn[0].position + float4(-0.1f, -0.1f, 0.0f, 0);
	v[2] = dataIn[0].position + float4(0.1f, 0.1f, 0.0f, 0);
	v[3] = dataIn[0].position + float4(-0.1f, 0.1f, 0.0f, 0);

	float2 quadUVs[4] = {
		float2(1, 1),
		float2(1, 0),
		float2(0, 1),
		float2(0, 0)
	};

	GSOutput element;
	[unroll]
	for (uint i = 0; i < 4; i++)
	{
		element.position = v[i];
		element.normal = dataIn[0].normal;
		element.uv = quadUVs[i];
		output.Append(element);
	}
}