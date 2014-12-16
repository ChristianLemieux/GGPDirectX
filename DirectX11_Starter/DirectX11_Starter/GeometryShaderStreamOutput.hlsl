struct GSOutput
{
	float4 pos : SV_POSITION;
	float2 velocity : TEXCOORD0;
	float2 acceleration : TEXCOORD1;
};

[maxvertexcount(50)]
void main(
	point GSOutput input[1] : SV_POSITION, 
	inout TriangleStream< GSOutput > output
)
{
	float4 v[3];
	v[0] = input[0].pos + float4(0.5f, -0.1f, 1.0f, 0);
	v[1] = input[0].pos + float4(0.1f, 0.1f, -1.0f, 0);
	v[2] = input[0].pos + float4(0.1f, -0.1f, 0.5f, 0);

	for (uint i = 0; i < 50; i++)
	{
		GSOutput element;
		element.pos = input[0].pos + float4(0.1f, 0.05f, 0.0f, 0.0f) * i;
		element.velocity = input[0].velocity;
		element.acceleration = input[0].acceleration;
		output.Append(element);
	}
	output.Append(input[0]);
	
}