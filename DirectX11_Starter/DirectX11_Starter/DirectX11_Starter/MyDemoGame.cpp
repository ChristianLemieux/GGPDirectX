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
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	// Make the game, initialize and run
	MyDemoGame game(hInstance);

	if (!game.Init())
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
	collision = L"Not Colliding";
	stateManager = new StateManager();
	notColliding = false;
	canTakeDamage = true;
	hullIntegrity = 100;

	if (!DirectXGame::Init())
		return false;

	// Set up buffers and such
	constantBufferList.push_back(new ConstantBuffer(dataToSendToVSConstantBuffer, device));
	shaderProgram = new ShaderProgram(L"VertexShader.cso", L"PixelShader.cso", device, constantBufferList[0], constantBufferList[0]);
	PhongProgram = new ShaderProgram(L"Phong.cso", L"PhongPixel.cso", device, constantBufferList[0], constantBufferList[0]);
	CreateGeometryBuffers();

	// Set up view matrix (camera)
	// In an actual game, update this when the camera moves (every frame)
	XMVECTOR position = XMVectorSet(0, 0, -5, 0);
	XMVECTOR target = XMVectorSet(0, 0, 0, 0);
	XMVECTOR up = XMVectorSet(0, 1, 0, 0);
	XMMATRIX V = XMMatrixLookAtLH(position, target, up);

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

	XMFLOAT4 red = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 green = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	XMFLOAT4 blue = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
	XMFLOAT4 orange = XMFLOAT4(1.0f, 0.5f, 0.0f, 1.0f);
	XMFLOAT4 brown = XMFLOAT4(0.65f, 0.185f, 0.165f, 1.0f);


	XMFLOAT3 normal = XMFLOAT3(+0.0f, +0.0f, +1.0f);

	/*Phong triangleVertices[] = { {XMFLOAT3(+0.0f, +0.0f, +0.0f), red, XMFLOAT2(+0.5f, +0.0f), normal, light.dir },
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
	};*/


	//Set up set of asteroid indices
	//UINT asteroidIndices[] = { 0,12,6,2,10,8,2,0,10,2,8,4,0,14,12,8,6,4,2,12,10 };
	//asteroid = new Mesh(asteroidVertices, asteroidIndices, 21, device);

	ObjectLoader *obj = new ObjectLoader(device);
	ObjectLoader *asteroidObject = new ObjectLoader(device);
	ObjectLoader *menuObject = new ObjectLoader(device);

	Mesh *ship = obj->LoadModel("asteroid.obj");
	Mesh *asteroid = asteroidObject->LoadModel("asteroid.obj");
	Mesh *menu = menuObject->LoadModel("Menu.obj");

	ID3D11SamplerState* sample = nullptr;
	//create sampler state
	samplerStates.push_back(new SamplerState(sample));
	samplerStates[0]->createSamplerState(device);

	//create materials
	materials.push_back(new Material(device, deviceContext, samplerStates[0]->sampler, L"spaceShipTexture.jpg", shaderProgram));
	materials.push_back(new Material(device, deviceContext, samplerStates[0]->sampler, L"asteroid.jpg", shaderProgram));
	materials.push_back(new Material(device, deviceContext, samplerStates[0]->sampler, L"StartScreen.png", shaderProgram));
	materials.push_back(new Material(device, deviceContext, samplerStates[0]->sampler, L"InstructionsScreen.png", shaderProgram));

	//create game entities
	gameEntities.push_back(new GameEntity(ship, materials[0]));
	gameEntities[0]->translate(XMFLOAT3(0.0f, 0.0f, 0.0f));
	gameEntities[0]->scale(XMFLOAT3(0.1f, 0.1f, 0.1f));

	//create menu entities
	menuEntities.push_back(new GameEntity(menu, materials[2]));
	menuEntities[0]->scale(XMFLOAT3(0.3f, 0.41f, 0.0f));
	menuEntities.push_back(new GameEntity(menu, materials[3]));
	menuEntities[1]->scale(XMFLOAT3(0.3f, 0.41f, 0.0f));

	for (int i = 1; i < 30; i++)
	{
		gameEntities.push_back(new GameEntity(ship, materials[1]));
		gameEntities[i]->scale(XMFLOAT3(0.1f, 0.1f, 0.1f));
		gameEntities[i]->setPosition(XMFLOAT3(((rand() % 60) + 30) , ((rand() % 40) - 19.0f), 0.0f));
	}
}

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
	collision = L"Not Colliding";
	notColliding = false;
	UpdateCamera();
	state = stateManager->changeState();
	if (state == L"Game")
	{
		//move triangle right
		if (GetAsyncKeyState('D') & 0x8000){
			gameEntities[0]->translate(XMFLOAT3(5.0f * dt, 0.0f, 0.0f));
		}
		//move triangle left
		if (GetAsyncKeyState('A') & 0x8000){
			gameEntities[0]->translate(XMFLOAT3(-5.0f * dt, 0.0f, 0.0f));
		}
		//move triangle up
		if (GetAsyncKeyState('W') & 0x8000){
			gameEntities[0]->translate(XMFLOAT3(0.0f, 5.0f * dt, 0.0f));
		}
		//move triangle down
		if (GetAsyncKeyState('S') & 0x8000){
			gameEntities[0]->translate(XMFLOAT3(0.0f, -5.0f * dt, 0.0f));
		}

		
		//moves asteroids across screen and respawns them when they leave the screen
		for (unsigned int i = 1; i < 30; i++)
		{
			gameEntities[i]->translate(XMFLOAT3(-8.0f * dt, 0.0f, 0.0f));

			//._41 is the x value for the position matrix of game entities
			if (gameEntities[i]->getPosition()._41 < -30)
			{
				gameEntities[i]->setPosition(XMFLOAT3(30.0f, (rand() % 40) - 19.0f, 0.0f));
			}

		}

		float distance = 2.0f;

		for (int i = 1; i < 30; i++)
		{
			float testDistX = pow(gameEntities[0]->getPosition()._41 - gameEntities[i]->getPosition()._41, 2);
			float testDistY = pow(gameEntities[0]->getPosition()._42 - gameEntities[i]->getPosition()._42, 2);

			if (distance >= testDistX + testDistY)
			{
				notColliding = true;
				if (canTakeDamage && notColliding){
					hullIntegrity -= 10;

					//lose condition
					if (hullIntegrity <= 0)
					{
						state = stateManager->setState(5);
						hullIntegrity = 100;
					}
					canTakeDamage = false;
				}
				
				collision = L"Colliding";
				break;
			}
		}

		if (!notColliding){
			canTakeDamage = true;
		}
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
	const float color[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	// Clear the buffer
	deviceContext->ClearRenderTargetView(renderTargetView, color);
	deviceContext->ClearDepthStencilView(
		depthStencilView,
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.0f,
		0);
	if (state == L"Game" || state == L"Pause")
	{
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
			
			//lights
			DirectionalLight mDirLight;
			
			mDirLight.Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
			mDirLight.Diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
			mDirLight.Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
			mDirLight.Direction = XMFLOAT3(0.57735f, -0.57735f, 0.57735f);
			
			// Finally do the actual drawing
			deviceContext->DrawIndexed(
				gameEntities.at(i)->g_mesh->m_size,	// The number of indices we're using in this draw
				0,
				0);
		};
	}
	else if (state == L"Menu")
	{
		UINT offset = 0;
			//UINT offset = 0;
			UINT stride = menuEntities[0]->g_mesh->sizeofvertex;
			// Set up the input assembler
			deviceContext->IASetInputLayout(menuEntities[0]->g_mat->shaderProgram->vsInputLayout);
			deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			menuEntities[0]->g_mat->shaderProgram->vsConstantBuffer->dataToSendToConstantBuffer.world = menuEntities[0]->getWorld();
			menuEntities[0]->g_mat->shaderProgram->vsConstantBuffer->dataToSendToConstantBuffer.view = viewMatrix;
			menuEntities[0]->g_mat->shaderProgram->vsConstantBuffer->dataToSendToConstantBuffer.projection = projectionMatrix;

			deviceContext->UpdateSubresource(
				menuEntities[0]->g_mat->shaderProgram->vsConstantBuffer->constantBuffer,
				0,
				NULL,
				&menuEntities[0]->g_mat->shaderProgram->vsConstantBuffer->dataToSendToConstantBuffer,
				0,
				0);

			deviceContext->IASetVertexBuffers(0, 1, &menuEntities[0]->g_mesh->v_buffer, &stride, &offset);
			deviceContext->IASetIndexBuffer(menuEntities[0]->g_mesh->i_buffer, DXGI_FORMAT_R32_UINT, 0);

			deviceContext->PSSetSamplers(0, 1, &menuEntities[0]->g_mat->samplerState);
			deviceContext->PSSetShaderResources(0, 1, &menuEntities[0]->g_mat->resourceView);



			// Set the current vertex and pixel shaders, as well the constant buffer for the vert shader
			deviceContext->VSSetShader(menuEntities[0]->g_mat->shaderProgram->vertexShader, NULL, 0);
			deviceContext->VSSetConstantBuffers(0, 1, &menuEntities[0]->g_mat->shaderProgram->vsConstantBuffer->constantBuffer);
			deviceContext->PSSetShader(menuEntities[0]->g_mat->shaderProgram->pixelShader, NULL, 0);

			// Finally do the actual drawing
			deviceContext->DrawIndexed(
				menuEntities.at(0)->g_mesh->m_size,	// The number of indices we're using in this draw
				0,
				0);
	}
	else if (state == L"Instructions")
	{
		UINT offset = 0;
		//UINT offset = 0;
		UINT stride = menuEntities[1]->g_mesh->sizeofvertex;
		// Set up the input assembler
		deviceContext->IASetInputLayout(menuEntities[1]->g_mat->shaderProgram->vsInputLayout);
		deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		menuEntities[1]->g_mat->shaderProgram->vsConstantBuffer->dataToSendToConstantBuffer.world = menuEntities[1]->getWorld();
		menuEntities[1]->g_mat->shaderProgram->vsConstantBuffer->dataToSendToConstantBuffer.view = viewMatrix;
		menuEntities[1]->g_mat->shaderProgram->vsConstantBuffer->dataToSendToConstantBuffer.projection = projectionMatrix;

		deviceContext->UpdateSubresource(
			menuEntities[1]->g_mat->shaderProgram->vsConstantBuffer->constantBuffer,
			0,
			NULL,
			&menuEntities[1]->g_mat->shaderProgram->vsConstantBuffer->dataToSendToConstantBuffer,
			0,
			0);

		deviceContext->IASetVertexBuffers(0, 1, &menuEntities[1]->g_mesh->v_buffer, &stride, &offset);
		deviceContext->IASetIndexBuffer(menuEntities[1]->g_mesh->i_buffer, DXGI_FORMAT_R32_UINT, 0);

		deviceContext->PSSetSamplers(0, 1, &menuEntities[1]->g_mat->samplerState);
		deviceContext->PSSetShaderResources(0, 1, &menuEntities[1]->g_mat->resourceView);



		// Set the current vertex and pixel shaders, as well the constant buffer for the vert shader
		deviceContext->VSSetShader(menuEntities[1]->g_mat->shaderProgram->vertexShader, NULL, 0);
		deviceContext->VSSetConstantBuffers(0, 1, &menuEntities[1]->g_mat->shaderProgram->vsConstantBuffer->constantBuffer);
		deviceContext->PSSetShader(menuEntities[1]->g_mat->shaderProgram->pixelShader, NULL, 0);

		// Finally do the actual drawing
		deviceContext->DrawIndexed(
			menuEntities.at(1)->g_mesh->m_size,	// The number of indices we're using in this draw
			0,
			0);

	}
	DrawUserInterface(0xff0099ff);

	// Present the buffer
	HR(swapChain->Present(0, 0));
}

void MyDemoGame::DrawUserInterface(UINT32 textColor)
{
	if (state == L"Game" || state == L"Pause" || state == L"Win" || state == L"Lose")
	{
		std::wstring pi = L"Hull Integrity:" + std::to_wstring(hullIntegrity);
		const WCHAR* szName = pi.c_str();

		if (!uiInitialized)
		{
			// set up the font factory
			// The font wrapper used to actually draw the text
			HRESULT hResult = FW1CreateFactory(FW1_VERSION, &pFW1Factory);
			pFW1Factory->CreateFontWrapper(device, L"Arial", &pFontWrapper);
			uiInitialized = true;
		}

		// The function to draw the actual text
		pFontWrapper->DrawString(
			deviceContext,
			szName,// String
			24.0f,// Font size
			25.0f,// X position
			15.0f,// Y position
			0xff0099ff,// Text color, 0xAaBbGgRr
			0x800// Flags (currently set to "restore state" to not ruin the rest of the scene)
			);


		pFontWrapper->DrawString(
			deviceContext,
			L"Score: 0",// String
			24.0f,// Font size
			viewport.Width - 125.0f,// X position
			15.0f,// Y position
			0xff0099ff,// Text color, 0xAaBbGgRr
			0x800// Flags (currently set to "restore state" to not ruin the rest of the scene)
			);

		pFontWrapper->DrawString(
			deviceContext,
			state,// String
			24.0f,// Font size
			viewport.Width - 125.0f,// X position
			50.0f,// Y position
			0xff0099ff,// Text color, 0xAaBbGgRr
			0x800// Flags (currently set to "restore state" to not ruin the rest of the scene)
			);

		/*pFontWrapper->DrawString(
			deviceContext,
			collision,// String
			24.0f,// Font size
			viewport.Width - 200.0f,// X position
			100.0f,// Y position
			0xff0099ff,// Text color, 0xAaBbGgRr
			0x800// Flags (currently set to "restore state" to not ruin the rest of the scene)
			);*/
	}
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
