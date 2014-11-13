#pragma once

#include <DirectXMath.h>
#include <vector>
#include "DirectXGame.h"
#include "Global.h"
#include "LightHelper.h"
#include "Mesh.h"
#include "Material.h"
#include "GameEntity.h"
#include "SamplerState.h"
#include "ShaderProgram.h"
#include "ConstantBuffer.h"
#include "Camera.h"
#include "FW1FontWrapper.h"
#include "ObjectLoader.h"
#include "StateManager.h"
#include <iostream>
#include <string>
#include <cstdio>
#include <ctime>

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
	XMFLOAT3 XMFLOAT3Cross(XMFLOAT3 a, XMFLOAT3 b);

	struct Light
	{
		Light()
		{
			ZeroMemory(this, sizeof(Light));
		}
		XMFLOAT3 dir;
		float pad;
		XMFLOAT4 ambient;
		XMFLOAT4 diffuse;
	};

	Light light;

	IFW1Factory *pFW1Factory;

	IFW1FontWrapper *pFontWrapper;

	BOOL uiInitialized = false;

private:
	// Initialization for our "game" demo
	void CreateGeometryBuffers();
	void LoadShadersAndInputLayout();
	void DrawUserInterface(UINT32);

private:

	StateManager* stateManager;
	wchar_t* state;

	// Buffers to hold actual geometry
	ID3D11Buffer* vertexBuffer;
	ID3D11Buffer* indexBuffer;
	ID3D11Buffer* cbPerFrameBuffer;

	//Second set of buffers to hold actual geometry
	ID3D11Buffer* vertex2Buffer;
	ID3D11Buffer* index2Buffer;

	//Basic shapes!
	Mesh* triangle;
	Mesh* square;
	Mesh* asteroid;
	ShaderProgram* shaderProgram;
	ShaderProgram* PhongProgram;
	

	//Game Entity
	std::vector<GameEntity*> gameEntities;
	//Menu Entities
	std::vector<GameEntity*> menuEntities;
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
	ConstantBufferLayout dataToSendToVSConstantBuffer;

	std::vector<ConstantBuffer*> constantBufferList;
	// The matrices to go from model space
	// to screen space
	XMFLOAT4X4 worldMatrix;
	XMFLOAT4X4 viewMatrix;
	XMFLOAT4X4 projectionMatrix;

	Camera gameCam;
	XMVECTOR cameraPosition;
	XMVECTOR cameraRotation;
	XMVECTOR upDirection;

	// Keeps track of the old mouse position.  Useful for 
	// determining how far the mouse moved in a single frame.
	POINT prevMousePos;
	int moveDistanceMouseX;
	int moveDistanceMouseY;

	int hullIntegrity;
	bool notColliding;

};
