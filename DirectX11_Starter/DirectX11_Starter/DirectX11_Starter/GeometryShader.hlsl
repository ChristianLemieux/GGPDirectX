
struct GSOutput
{
	float4 pos : SV_POSITION;
};

[maxvertexcount(4)]
void main(point float4 input[1] : SV_POSITION, inout TriangleStream< GSOutput > output)
{
	for (uint i = 0; i < 3; i++)
	{
		GSOutput element;
		element.pos = input[i];
		output.Append(element);
	}
}