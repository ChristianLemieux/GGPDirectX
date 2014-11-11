// ----------------------------------------------------------------------------
//  A few notes on project settings
//
//  - The project is set to use the UNICODE character set
//    - This was changed in Project Properties > Config Properties > General > Character Set
//    - This basically adds a "#define UNICODE" to the project
//
//  - The include directories were automagically correct, since the DirectX 
//    headers and libs are part of the windows SDK
//    - For instance, $(WindowsSDK_IncludePath) is set as a project include 
//      path by default.  That's where the DirectX headers are located.
//
//  - Two libraries had to be manually added to the Linker Input Dependencies
//    - d3d11.lib
//    - d3dcompiler.lib
//    - This was changed in Project Properties > Config Properties > Linker > Input > Additional Dependencies
//
//  - The Working Directory was changed to match the actual .exe's 
//    output directory, since we need to load the compiled shader files at run time
//    - This was changed in Project Properties > Config Properties > Debugging > Working Directory
//
// ----------------------------------------------------------------------------
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG

#include <Windows.h>
#include <d3dcompiler.h>
#include "MyDemoGame.h"
#include "WICTextureLoader.h"

#pragma region Win32 Entry Point (WinMain)

// Win32 Entry Point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
				   PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.

#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	// Make the game, initialize and run
	MyDemoGame game(hInstance);
	
	if( !game.Init() )
		return 0;
	
	int toReturn = game.Run();
	
	_CrtDumpMemoryLeaks();
	return toReturn;
}

#pragma endregion

#pragma region Constructor / Destructor

MyDemoGame::MyDemoGame(HINSTANCE hInstance) : DirectXGame(hInstance)
{
	// Set up our custom caption and window size
	windowCaption = L"Graphics Programming Project";
	windowWidth = 800;
	windowHeight = 600;
}

MyDemoGame::~MyDemoGame()
{
	// Release all of the D3D stuff that's still hanging out
	ReleaseMacro(vertexBuffer);
	ReleaseMacro(indexBuffer);
	ReleaseMacro(vertexShader);
	ReleaseMacro(pixelShader);
	ReleaseMacro(vsConstantBuffer);
	ReleaseMacro(inputLayout);
}

#pragma endregion

#pragma region Initialization

// Initializes the base class (including the window and D3D),
// sets up our geometry and loads the shaders (among other things)
bool MyDemoGame::Init()
{
	if( !DirectXGame::Init() )
		return false;

	// Set up buffers and such
	constantBufferList.push_back(new ConstantBuffer(dataToSendToVSConstantBuffer, device));
	shaderProgram = new ShaderProgram(L"VertexShader.cso", L"PixelShader.cso", device, constantBufferList[0], constantBufferList[0]);
	PhongProgram = new ShaderProgram(L"Phong.cso", L"PhongPixel.cso", device, constantBufferList[0], constantBufferList[0]);
	CreateGeometryBuffers();
	LoadShadersAndInputLayout();

	// Set up view matrix (camera)
	// In an actual game, update this when the camera moves (every frame)
	XMVECTOR position	= XMVectorSet(0, 0, -5, 0);
	XMVECTOR target		= XMVectorSet(0, 0, 0, 0);
	XMVECTOR up			= XMVectorSet(0, 1, 0, 0);
	XMMATRIX V			= XMMatrixLookAtLH(position, target, up);

	XMStoreFloat4x4(&viewMatrix, XMMatrixTranspose(V));

	// Set up world matrix
	// In an actual game, update this when the object moves (every frame)
	XMMATRIX W = XMMatrixIdentity();
	XMStoreFloat4x4(&worldMatrix, XMMatrixTranspose(W));

	cameraPosition = XMVectorSet(gameCam.getPositionX(), gameCam.getPositionY(), gameCam.getPositionZ(), 0);
	cameraRotation = XMVectorSet(gameCam.getRotationX(), gameCam.getRotationY(), gameCam.getRotationZ(), 0);
	upDirection = XMVectorSet(0, 1, 0, 0);
	return true;
}

XMFLOAT3 MyDemoGame::XMFLOAT3Cross(XMFLOAT3 a, XMFLOAT3 b){
	return XMFLOAT3(a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x);
}

// Creates the vertex and index buffers for a single triangle
void MyDemoGame::CreateGeometryBuffers()
{
	light.dir = XMFLOAT3(0.25f, 0.5f, -1.0f);
	light.ambient = XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f);
	light.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	XMFLOAT4 red = XMFLOAT4(1.0f * light.ambient.x, 0.0f * light.ambient.y, 0.0f * light.ambient.z, 1.0f * light.ambient.w);
	XMFLOAT4 green = XMFLOAT4(0.0f * light.ambient.x, 1.0f * light.ambient.y, 0.0f * light.ambient.z, 1.0f * light.ambient.w);
	XMFLOAT4 blue = XMFLOAT4(0.0f * light.ambient.x, 0.0f * light.ambient.y, 1.0f * light.ambient.z, 1.0f * light.ambient.w);
	XMFLOAT4 orange = XMFLOAT4(1.0f * light.ambient.x, 0.5f * light.ambient.y, 0.0f * light.ambient.z, 1.0f * light.ambient.w);
	XMFLOAT4 brown = XMFLOAT4(0.65f * light.ambient.x, 0.185f * light.ambient.y, 0.165f * light.ambient.z, 1.0f * light.ambient.w);


	XMFLOAT3 normal = XMFLOAT3(+0.0f, +0.0f, +1.0f);

	Phong triangleVertices[] = { {XMFLOAT3(+0.0f, +0.0f, +0.0f), red, XMFLOAT2(+0.5f, +0.0f), normal, light.dir },
											{XMFLOAT3(-.5f, -1.0f, +0.0f), blue, XMFLOAT2(+0.0f, +1.0f), normal, light.dir },
											{XMFLOAT3(+.5f, -1.0f, +0.0f), green, XMFLOAT2(+1.0f, +1.0f), normal, light.dir }
	};

	Vertex squareVertices[] = {
			{ XMFLOAT3(-1.0f, +1.0f, +0.2f), red, XMFLOAT2(0, 0)},
			{ XMFLOAT3(-1.0f, -1.0f, +0.2f), red, XMFLOAT2(0, 1)},
			{ XMFLOAT3(+1.0f, +1.0f, +0.2f), red, XMFLOAT2(1, 0)},
			{ XMFLOAT3(+1.0f, -1.0f, +0.2f), red, XMFLOAT2(1, 1)}
	};

	Phong asteroidVertices[] = {
			{ XMFLOAT3(+0.7f, +1.0f, +0.0f), brown, XMFLOAT2(0.7f, 1.0f), normal, light.dir },
			{ XMFLOAT3(+0.7f, +1.0f, -1.0f), brown, XMFLOAT2(0.0f, 1.0f), normal, light.dir },
			{ XMFLOAT3(+0.3f, +1.0f, +0.0f), brown, XMFLOAT2(0.3f, 1.0f), normal, light.dir },
			{ XMFLOAT3(+0.3f, +1.0f, -1.0f), brown, XMFLOAT2(1.0f, 1.0f), normal, light.dir },
			{ XMFLOAT3(+0.0f, +0.7f, +0.0f), brown, XMFLOAT2(0.0f, 0.7f), normal, light.dir },
			{ XMFLOAT3(+0.0f, +0.7f, -1.0f), brown, XMFLOAT2(0.0f, 1.0f), normal, light.dir },
			{ XMFLOAT3(+0.0f, +0.3f, +0.0f), brown, XMFLOAT2(0.0f, 0.3f), normal, light.dir },
			{ XMFLOAT3(+0.0f, +0.3f, -1.0f), brown, XMFLOAT2(1.0f, 1.0f), normal, light.dir },
			{ XMFLOAT3(+0.3f, +0.0f, +0.0f), brown, XMFLOAT2(0.3f, 0.0f), normal, light.dir },
			{ XMFLOAT3(+0.3f, +0.0f, -1.0f), brown, XMFLOAT2(0.0f, 1.0f), normal, light.dir },
			{ XMFLOAT3(+0.7f, +0.0f, +0.0f), brown, XMFLOAT2(0.7f, 0.0f), normal, light.dir },
			{ XMFLOAT3(+0.7f, +0.0f, -1.0f), brown, XMFLOAT2(1.0f, 1.0f), normal, light.dir },
			{ XMFLOAT3(+1.0f, +0.3f, +0.0f), brown, XMFLOAT2(1.0f, 0.3f), normal, light.dir },
			{ XMFLOAT3(+1.0f, +0.3f, -1.0f), brown, XMFLOAT2(0.0f, 1.0f), normal, light.dir },
			{ XMFLOAT3(+1.0f, +0.7f, +0.0f), brown, XMFLOAT2(1.0f, 0.7f), normal, light.dir },
			{ XMFLOAT3(+1.0f, +0.7f, -1.0f), brown, XMFLOAT2(1.0f, 1.0f), normal, light.dir }
	};
	// Set up the indices
	UINT triangleIndices[] = {0, 2, 1};

	//Set up second set of the indices
	//UINT squareIndices[] = { 0, 2, 1, 2, 3, 1 };

	//Set up set of asteroid indices
	UINT asteroidIndices[] = { 0,12,6,2,10,8,2,0,10,2,8,4,0,14,12,8,6,4,2,12,10 };
	triangle = new Mesh(triangleVertices, triangleIndices, 3, device);
	//square = new Mesh(squareVertices, squareIndices, 6, device);
	asteroid = new Mesh(asteroidVertices, asteroidIndices, 21, device);

	ID3D11SamplerState* sample = nullptr;
	//create sampler state
	samplerStates.push_back(new SamplerState(sample));
	samplerStates[0]->createSamplerState(device);

	//create materials
	materials.push_back(new Material(device, deviceContext, samplerStates[0]->sampler, L"bullet.png", PhongProgram));
	//materials.push_back(new Material(device, deviceContext, samplerStates[0]->sampler, L"120322_0001.jpg", shaderProgram));
	materials.push_back(new Material(device, deviceContext, samplerStates[0]->sampler, L"asteroid.jpg", PhongProgram));

	//create game entities
	gameEntities.push_back(new GameEntity(triangle, materials[0]));
	//gameEntities.push_back(new GameEntity(square, materials[1]));
	for (int i = 1; i < 20; i++)
	{
		gameEntities.push_back(new GameEntity(asteroid, materials[1]));
		gameEntities[i]->scale(XMFLOAT3(0.5f, 0.5f, 0.0f));
		gameEntities[i]->translate(XMFLOAT3((((float)rand() / (float)(RAND_MAX))* 25.0f) + 10.0f, (((float)rand() / (float)(RAND_MAX))* 10.0f) - 5.0f, 0.0f));
	}
}

// Loads shaders from compiled shader object (.cso) files, and uses the
// vertex shader to create an input layout which is needed when sending
// vertex data to the device
void MyDemoGame::LoadShadersAndInputLayout()
{
	// Set up the vertex layout description
	// This has to match the vertex input layout in the vertex shader
	// We can't set up the input layout yet since we need the actual vert shader
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,	0, 0,	D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12,	D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};
}

#pragma endregion

#pragma region Window Resizing

// Handles resizing the window and updating our projection matrix to match
void MyDemoGame::OnResize()
{
	// Handle base-level DX resize stuff
	DirectXGame::OnResize();

	// Update our projection matrix since the window size changed
	XMMATRIX P = XMMatrixPerspectiveFovLH(
		0.25f * 3.1415926535f,
		AspectRatio(),
		0.1f,
		100.0f);
	XMStoreFloat4x4(&projectionMatrix, XMMatrixTranspose(P));
}
#pragma endregion

#pragma region Game Loop

// Updates the local constant buffer and 
// push it to the buffer on the device
void MyDemoGame::UpdateScene(float dt)
{
	UpdateCamera();

	//move triangle right
	if (GetAsyncKeyState('D') & 0x8000){
		gameEntities[0]->translate(XMFLOAT3(0.001f, 0.0f, 0.0f));
	}
	//move triangle left
	if (GetAsyncKeyState('A') & 0x8000){
		gameEntities[0]->translate(XMFLOAT3(-0.001f, 0.0f, 0.0f));
	}
	//move triangle up
	if (GetAsyncKeyState('W') & 0x8000){
		gameEntities[0]->translate(XMFLOAT3(0.0f, 0.001f, 0.0f));
	}
	//move triangle down
	if (GetAsyncKeyState('S') & 0x8000){
		gameEntities[0]->translate(XMFLOAT3(0.0f, -0.001f, 0.0f));
	}
	/*
	//move triangle forward
	if (GetAsyncKeyState('U') & 0x8000){	
		gameEntities[0]->translate(XMFLOAT3(0.0f, 0.0f, 0.001f));
	}
	//move triangle back
	if (GetAsyncKeyState('P') & 0x8000){
		gameEntities[0]->translate(XMFLOAT3(0.0f, 0.0f, -0.001f));
	}

	//rotate square in positive x
	if (GetAsyncKeyState('S') & 0x8000){
		for (unsigned int i = 0; i < gameEntities.size(); i++)
		{
			gameEntities[i]->rotate(XMFLOAT3(0.001f, 0.0f, 0.0f));
		}
	}
	//rotate square in negative x
	if (GetAsyncKeyState('W') & 0x8000){
		for (unsigned int i = 0; i < gameEntities.size(); i++)
		{
			gameEntities[i]->rotate(XMFLOAT3(-0.001f, 0.0f, 0.0f));
		}
	}
	//rotate square in positive y
	if (GetAsyncKeyState('D') & 0x8000){
		for (unsigned int i = 0; i < gameEntities.size(); i++)
		{
			gameEntities[i]->rotate(XMFLOAT3(0.0f, 0.001f, 0.0f));
		}
	}
	//rotate square in negative y
	if (GetAsyncKeyState('A') & 0x8000){
		for (unsigned int i = 0; i < gameEntities.size(); i++)
		{
			gameEntities[i]->rotate(XMFLOAT3(0.0f, -0.001f, 0.0f));
		}
	}
	//rotate square in positive z
	if (GetAsyncKeyState('Q') & 0x8000){
		for (unsigned int i = 0; i < gameEntities.size(); i++)
		{
			gameEntities[i]->rotate(XMFLOAT3(0.0f, 0.0f, 0.001f));
		}
	}
	//rotate square in negative z
	if (GetAsyncKeyState('F') & 0x8000){
		for (unsigned int i = 0; i < gameEntities.size(); i++)
		{
			gameEntities[i]->rotate(XMFLOAT3(0.0f, 0.0f, -0.001f));
		}
	}

	//Increase Scale of "x" acis
	if (GetAsyncKeyState('C') & 0x8000){
		for (unsigned int i = 0; i < gameEntities.size(); i++)
		{
			gameEntities[i]->scale(XMFLOAT3(1.001f, 1.0f, 1.0f));
		}
	}	
	//Decrease Scale of "x" axis
	if (GetAsyncKeyState('V') & 0x8000){
		for (unsigned int i = 0; i < gameEntities.size(); i++)
		{
			gameEntities[i]->scale(XMFLOAT3(0.999f, 1.0f, 1.0f));
		}
	}


	//Increase Scale of "y" axis
	if (GetAsyncKeyState('B') & 0x8000){
		for (unsigned int i = 0; i < gameEntities.size(); i++)
		{
			gameEntities[i]->scale(XMFLOAT3(1.0f, 1.001f, 1.0f));
		}
	}
	//Decrease Scale of "y" axis
	if (GetAsyncKeyState('N') & 0x8000){
		for (unsigned int i = 0; i < gameEntities.size(); i++)
		{
			gameEntities[i]->scale(XMFLOAT3(1.0f, 0.999f, 1.0f));
		}
	}

	//Increase Scale of "z" axis
	if (GetAsyncKeyState('G') & 0x8000){
		for (unsigned int i = 0; i < gameEntities.size(); i++)
		{
			gameEntities[i]->scale(XMFLOAT3(1.0f, 1.0f, 1.001f));
		}
	}
	//Decrease Scale of "z" axis
	if (GetAsyncKeyState('H') & 0x8000){
		for (unsigned int i = 0; i < gameEntities.size(); i++)
		{
			gameEntities[i]->scale(XMFLOAT3(1.0f, 1.0f, 0.999f));
		}
	}
	*/
	for (unsigned int i = 2; i < 20; i++)
	{
		gameEntities[i]->translate(XMFLOAT3(-0.75f * dt, 0.0f, 0.0f));
		if (gameEntities[i]->getPosition()._41 < -10)
		{
			//gameEntities[i]->translate(XMFLOAT3((((float)rand() / (float)(RAND_MAX))* 25.0f) + 20.0f, (((float)rand() / (float)(RAND_MAX))* 10.0f) - 5.0f, 0.0f));
		}
		
		//gameEntities[i]->rotate(XMFLOAT3(0.0f, 0.0f, 0.001f));
	}
}

//Updates our viewMatrix based on the camera's position
void MyDemoGame::UpdateCamera()
{
	//Left ad right arrow keys alter X position
	if (GetAsyncKeyState(VK_LEFT) & 0x8000)
	{
		gameCam.setDistanceX(-.001f);
		gameCam.setPosition(gameCam.getDistanceX(), gameCam.getDistanceY(), gameCam.getDistanceZ());
		cameraPosition = XMVectorSet(gameCam.getPositionX(), gameCam.getPositionY(), gameCam.getPositionZ(), 0);
		cameraRotation = XMVectorSet(gameCam.getRotationX(), gameCam.getRotationY(), gameCam.getRotationZ(), 0);
	}
	if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
	{
		gameCam.setDistanceX(+.001f);
		gameCam.setPosition(gameCam.getDistanceX(), gameCam.getDistanceY(), gameCam.getDistanceZ());
		cameraPosition = XMVectorSet(gameCam.getPositionX(), gameCam.getPositionY(), gameCam.getPositionZ(), 0);
		cameraRotation = XMVectorSet(gameCam.getRotationX(), gameCam.getRotationY(), gameCam.getRotationZ(), 0);
	}

	//Up/Down arrow keys alter Y position
	if (GetAsyncKeyState(VK_DOWN) & 0x8000)
	{
		gameCam.setDistanceY(-.001f);
		gameCam.setPosition(gameCam.getDistanceX(), gameCam.getDistanceY(), gameCam.getDistanceZ());
		cameraPosition = XMVectorSet(gameCam.getPositionX(), gameCam.getPositionY(), gameCam.getPositionZ(), 0);
		cameraRotation = XMVectorSet(gameCam.getRotationX(), gameCam.getRotationY(), gameCam.getRotationZ(), 0);
	}
	if (GetAsyncKeyState(VK_UP) & 0x8000)
	{
		gameCam.setDistanceY(+.001f);
		gameCam.setPosition(gameCam.getDistanceX(), gameCam.getDistanceY(), gameCam.getDistanceZ());
		cameraPosition = XMVectorSet(gameCam.getPositionX(), gameCam.getPositionY(), gameCam.getPositionZ(), 0);
		cameraRotation = XMVectorSet(gameCam.getRotationX(), gameCam.getRotationY(), gameCam.getRotationZ(), 0);
	}

	//5 and 0 on the numpad alter the Z position
	if (GetAsyncKeyState(VK_NUMPAD0) & 0x8000)
	{
		gameCam.setDistanceZ(-.001f);
		gameCam.setPosition(gameCam.getDistanceX(), gameCam.getDistanceY(), gameCam.getDistanceZ());
		cameraPosition = XMVectorSet(gameCam.getPositionX(), gameCam.getPositionY(), gameCam.getPositionZ(), 0);
		cameraRotation = XMVectorSet(gameCam.getRotationX(), gameCam.getRotationY(), gameCam.getRotationZ(), 0);
	}
	if (GetAsyncKeyState(VK_NUMPAD5) & 0x8000)
	{
		gameCam.setDistanceZ(+.001f);
		gameCam.setPosition(gameCam.getDistanceX(), gameCam.getDistanceY(), gameCam.getDistanceZ());
		cameraPosition = XMVectorSet(gameCam.getPositionX(), gameCam.getPositionY(), gameCam.getPositionZ(), 0);
		cameraRotation = XMVectorSet(gameCam.getRotationX(), gameCam.getRotationY(), gameCam.getRotationZ(), 0);
	}

	//4 and 6 on the numpad will rotate along the X axis
	if (GetAsyncKeyState(VK_NUMPAD4) & 0x8000)
	{
		gameCam.setRotationDistanceX(-.01f);
		gameCam.setRotation(gameCam.getRotationDistanceX(), gameCam.getRotationDistanceY(), gameCam.getRotationDistanceZ());
		cameraRotation = XMVectorSet(gameCam.getRotationX(), gameCam.getRotationY(), gameCam.getRotationZ(), 0);
	}
	if (GetAsyncKeyState(VK_NUMPAD6) & 0x8000)
	{
		gameCam.setRotationDistanceX(+.01f);
		gameCam.setRotation(gameCam.getRotationDistanceX(), gameCam.getRotationDistanceY(), gameCam.getRotationDistanceZ());
		cameraRotation = XMVectorSet(gameCam.getRotationX(), gameCam.getRotationY(), gameCam.getRotationZ(), 0);
	}

	//8 ad 2 on the unmpad will rotate along the y axis
	if (GetAsyncKeyState(VK_NUMPAD8) & 0x8000)
	{
		gameCam.setRotationDistanceY(+.01f);
		gameCam.setRotation(gameCam.getRotationDistanceX(), gameCam.getRotationDistanceY(), gameCam.getRotationDistanceZ());
		cameraRotation = XMVectorSet(gameCam.getRotationX(), gameCam.getRotationY(), gameCam.getRotationZ(), 0);
	}
	if (GetAsyncKeyState(VK_NUMPAD2) & 0x8000)
	{
		gameCam.setRotationDistanceY(-.01f);
		gameCam.setRotation(gameCam.getRotationDistanceX(), gameCam.getRotationDistanceY(), gameCam.getRotationDistanceZ());
		cameraRotation = XMVectorSet(gameCam.getRotationX(), gameCam.getRotationY(), gameCam.getRotationZ(), 0);
	}

	//reset camera back to original position
	if (GetAsyncKeyState('R') & 0x8000)
	{
		gameCam.reset();
		cameraPosition = XMVectorSet(gameCam.getPositionX(), gameCam.getPositionY(), gameCam.getPositionZ(), 0);
		cameraRotation = XMVectorSet(gameCam.getRotationX(), gameCam.getRotationY(), gameCam.getRotationZ(), 0);
	}

	XMMATRIX V = XMMatrixLookToLH(cameraPosition, cameraRotation, upDirection);
	XMStoreFloat4x4(&viewMatrix, XMMatrixTranspose(V));

}
// Clear the screen, redraw everything, present
void MyDemoGame::DrawScene()
{
	const float color[4] = {0.0f, 0.0f, 0.0f, 0.0f};

	// Clear the buffer
	deviceContext->ClearRenderTargetView(renderTargetView, color);
	deviceContext->ClearDepthStencilView(
		depthStencilView, 
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.0f,
		0);

	// Set buffers in the input assembler
	//UINT stride = sizeof(PhongBlinnVertex);
	UINT offset = 0;
	UINT stride = sizeof(Vertex);
	for (unsigned int i = 0; i < gameEntities.size(); i++){
		//UINT offset = 0;
		stride = gameEntities[i]->g_mesh->sizeofvertex;
		// Set up the input assembler
		deviceContext->IASetInputLayout(gameEntities[i]->g_mat->shaderProgram->vsInputLayout);
		deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		gameEntities[i]->g_mat->shaderProgram->vsConstantBuffer->dataToSendToConstantBuffer.world = gameEntities[i]->getWorld();
		gameEntities[i]->g_mat->shaderProgram->vsConstantBuffer->dataToSendToConstantBuffer.view = viewMatrix;
		gameEntities[i]->g_mat->shaderProgram->vsConstantBuffer->dataToSendToConstantBuffer.projection = projectionMatrix;

		deviceContext->UpdateSubresource(
			gameEntities[i]->g_mat->shaderProgram->vsConstantBuffer->constantBuffer,
			0,
			NULL,
			&gameEntities[i]->g_mat->shaderProgram->vsConstantBuffer->dataToSendToConstantBuffer,
			0,
			0);

		deviceContext->IASetVertexBuffers(0, 1, &gameEntities[i]->g_mesh->v_buffer, &stride, &offset);
		deviceContext->IASetIndexBuffer(gameEntities[i]->g_mesh->i_buffer, DXGI_FORMAT_R32_UINT, 0);

		deviceContext->PSSetSamplers(0, 1, &gameEntities[i]->g_mat->samplerState);
		deviceContext->PSSetShaderResources(0, 1, &gameEntities[i]->g_mat->resourceView);



		// Set the current vertex and pixel shaders, as well the constant buffer for the vert shader
		deviceContext->VSSetShader(gameEntities[i]->g_mat->shaderProgram->vertexShader, NULL, 0);
		deviceContext->VSSetConstantBuffers(0, 1, &gameEntities[i]->g_mat->shaderProgram->vsConstantBuffer->constantBuffer);
		deviceContext->PSSetShader(gameEntities[i]->g_mat->shaderProgram->pixelShader, NULL, 0);

		// Finally do the actual drawing
		deviceContext->DrawIndexed(
			gameEntities.at(i)->g_mesh->m_size,	// The number of indices we're using in this draw
			0,
			0);
	};

	// Present the buffer
	HR(swapChain->Present(0, 0));
}

#pragma endregion

#pragma region Mouse Input

// These methods don't do much currently, but can be used for mouse-related input
void MyDemoGame::OnMouseDown(WPARAM btnState, int x, int y)
{
	prevMousePos.x = x;
	prevMousePos.y = y;

	SetCapture(hMainWnd);
}

void MyDemoGame::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void MyDemoGame::OnMouseMove(WPARAM btnState, int x, int y)
{
	prevMousePos.x = x;
	prevMousePos.y = y;
}
#pragma endregion
