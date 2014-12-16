struct GSOutput
{
	float4 pos : SV_POSITION;
};

[maxvertexcount(3)]
void main(
	point GSOutput input[1] : SV_POSITION, 
	inout TriangleStream< GSOutput > output
)
{
	float4 v[3];
	v[0] = input[0].pos + float4(0.1f, -0.1f, 0.0f, 0);
	v[1] = input[0].pos + float4(0.1f, 0.1f, 0.0f, 0);
	v[2] = input[0].pos + float4(-0.1f, -0.1f, 0.0f, 0);

	for (uint i = 0; i < 3; i++)
	{
		GSOutput element;
		element.pos = v[i];
		output.Append(element);
	}
}