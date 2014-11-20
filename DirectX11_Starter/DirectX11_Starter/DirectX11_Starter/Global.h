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
	XMFLOAT2 UVs;

};

struct Vertex2{
	XMFLOAT3 Position;
	XMFLOAT3 Normal;
	XMFLOAT2 UVs;
};

struct Phong{
	XMFLOAT3 Position;
	XMFLOAT4 Color;
	XMFLOAT2 uv;
	XMFLOAT3 Normal;
	XMFLOAT3 lightPos;
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

//Camera Constant Buffer Data Layout
struct CameraBufferType{
	XMFLOAT3 cameraPosition;
	float padding;
};

//Light Constant Buffer Data Layout
struct LightBufferType{
	XMFLOAT4 ambientColor;
	XMFLOAT4 diffuseColor;
	XMFLOAT3 lightDirection;
	float specularPower;
	XMFLOAT4 specularColor;
};
#endif