#ifndef _GLOBAL_H
#define _GLOBAL_H

#include <DirectXMath.h>

using namespace DirectX;

// Vertex struct for triangles
struct Vertex
{
	XMFLOAT3 Position;
	XMFLOAT4 Color;
	XMFLOAT2 uv;
};

#endif