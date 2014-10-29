#pragma once

#include <DirectXMath.h>
#include <vector>
#include "DirectXGame.h"
#include "Global.h"
#include "Mesh.h"
#include "Material.h"
#include "GameEntity.h"
#include "SamplerState.h"
#include "Camera.h"

// Include run-time memory checking in debug builds
#if defined(DEBUG) || defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

// For DirectX Math
using namespace DirectX;


// Vertex struct for triangles
/*struct Vertex
{
	XMFLOAT3 Position;
	XMFLOAT4 Color;
};*/

// Struct to match vertex shader's constant buffer
// You update one of these locally, then push it to the corresponding
// constant buffer on the device when it needs to be updated
struct VertexShaderConstantBufferLayout
{
	XMFLOAT4X4 world;
	XMFLOAT4X4 view;
	XMFLOAT4X4 projection;
};

// Demo class which extends the base DirectXGame class
class MyDemoGame : public DirectXGame
{
public:
	MyDemoGame(HINSTANCE hInstance);
	~MyDemoGame();

	// Overrides for base level methods
	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene(); 
	void UpdateCamera();

	// For handing mouse input
	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

private:
	// Initialization for our "game" demo
	void CreateGeometryBuffers();
	void LoadShadersAndInputLayout();

private:

	
	// Buffers to hold actual geometry
	ID3D11Buffer* vertexBuffer;
	ID3D11Buffer* indexBuffer;

	//Second set of buffers to hold actual geometry
	ID3D11Buffer* vertex2Buffer;
	ID3D11Buffer* index2Buffer;

	//Basic shapes!
	Mesh* triangle;
	Mesh* square;

	//Game Entity
	std::vector<GameEntity*> gameEntities;
	//SamplerState for textures
	std::vector<SamplerState*> samplerStates;
	//Materials
	std::vector<Material*> materials;

	// Our basic shaders for this example
	ID3D11PixelShader* pixelShader;
	ID3D11VertexShader* vertexShader;

	// A few more odds and ends we'll need
	ID3D11InputLayout* inputLayout;
	ID3D11Buffer* vsConstantBuffer;
	VertexShaderConstantBufferLayout dataToSendToVSConstantBuffer;

	// The matrices to go from model space
	// to screen space
	XMFLOAT4X4 worldMatrix;
	XMFLOAT4X4 viewMatrix;
	XMFLOAT4X4 projectionMatrix;

	Camera gameCam;
	XMVECTOR cameraPosition;
	XMVECTOR cameraRotation;
	XMVECTOR cameraCrossProduct;
	XMVECTOR upDirection;

	// Keeps track of the old mouse position.  Useful for 
	// determining how far the mouse moved in a single frame.
	POINT prevMousePos;
	int moveDistanceMouseX;
	int moveDistanceMouseY;
};