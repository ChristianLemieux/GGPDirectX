#ifndef _GLOBAL_H
#define _GLOBAL_H
#define ReleaseMacro(x) { if(x){ x->Release(); x = 0; } }

#include <DirectXMath.h>

using namespace DirectX;

// Vertex struct for triangles
struct Vertex
{
	XMFLOAT3 Position;
	XMFLOAT4 Color;
	XMFLOAT2 uv;
};

//Vector struct for our camera class
struct Vector
{
	float x;
	float y;
	float z;
};

struct ConstantBufferLayout
{
	XMFLOAT4X4 world;
	XMFLOAT4X4 view;
	XMFLOAT4X4 projection;
};

#endif