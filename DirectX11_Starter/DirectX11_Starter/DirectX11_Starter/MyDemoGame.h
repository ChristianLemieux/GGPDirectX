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
#include "SpriteFont.h"
#include "ConstantBuffer.h"
#include "Camera.h"
#include "FW1FontWrapper.h"
#include "ObjectLoader.h"
#include "StateManager.h"
#include <iostream>
#include <string>
#include <cstdio>
#include <ctime>
#include "Game.h"
#include "State.h"

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

struct Button
{
	RECT dimensions; // the dimensions covered by the button
	int activeState; // The state the button is active in
	int newState; // The state the button changes the game to
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
	LightBufferType lighting;


	IFW1Factory *pFW1Factory;

	IFW1FontWrapper *pFontWrapper;

	BOOL uiInitialized = false;

private:
	// Initialization for our "game" demo
	void CreateGeometryBuffers();
	void LoadShadersAndInputLayout();
	void DrawUserInterface(UINT32);
	void HandleUIClick(int x, int y);


private:
	Game* game;
	StateManager* stateManager;
	wchar_t* states[6];
	wchar_t* state;


	ShaderProgram* shaderProgram;



	std::vector<Button> buttons;
	std::vector<State*> gameStates;

	ConstantBufferLayout dataToSendToVSConstantBuffer;
	LightBufferType dataToSendToLightConstantBuffer;
	CameraBufferType dataToSendToCameraConstantBuffer;

	ConstantBuffer* MatrixCB;
	ConstantBuffer* LightCB;
	ConstantBuffer* CamCB;

	SamplerState* samplerState;
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
};
