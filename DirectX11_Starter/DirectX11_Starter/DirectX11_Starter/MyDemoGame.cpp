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

	game = new Game(device, deviceContext);
	ID3D11SamplerState* sample = nullptr;
	//create sampler state
	samplerStates.push_back(new SamplerState(sample));
	samplerStates[0]->createSamplerState(device);

	game->initGame(samplerStates[0]);

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

	// create the buttons for the menus
	Button tempButton = { { 165, 320, 625, 375 }, 0, 1 };
	buttons.push_back(tempButton);
	tempButton = { { 145, 420, 660, 475 }, 0, 2 };
	buttons.push_back(tempButton);
	tempButton = { { 35, 460, 195, 560 }, 2, 0 };
	buttons.push_back(tempButton);
	tempButton = { { 235, 465, 550, 515 }, 5, 0 };
	buttons.push_back(tempButton);

	//initialize states so that state strings can be looked up with a state index
	states[0] = L"Menu";
	states[1] = L"Game";
	states[2] = L"Instructions";
	states[3] = L"Pause";
	states[4] = L"Win";
	states[5] = L"Lose";

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


	ObjectLoader *obj = new ObjectLoader(device);
	ObjectLoader *menuObject = new ObjectLoader(device);

	Mesh *menu = menuObject->LoadModel("Menu.obj");


	//create materials
	materials.push_back(new Material(device, deviceContext, samplerStates[0]->sampler, L"spaceShipTexture.jpg", shaderProgram));
	materials.push_back(new Material(device, deviceContext, samplerStates[0]->sampler, L"asteroid.jpg", shaderProgram));
	materials.push_back(new Material(device, deviceContext, samplerStates[0]->sampler, L"StartScreen.png", shaderProgram));
	materials.push_back(new Material(device, deviceContext, samplerStates[0]->sampler, L"InstructionsScreen.png", shaderProgram));
	materials.push_back(new Material(device, deviceContext, samplerStates[0]->sampler, L"gameOverScreen.png", shaderProgram));

	//create game entities

	//create menu entities
	menuEntities.push_back(new GameEntity(menu, materials[2]));
	menuEntities[0]->scale(XMFLOAT3(0.3f, 0.41f, 0.0f));
	menuEntities.push_back(new GameEntity(menu, materials[3]));
	menuEntities[1]->scale(XMFLOAT3(0.3f, 0.41f, 0.0f));
	menuEntities.push_back(new GameEntity(menu, materials[4]));
	menuEntities[2]->scale(XMFLOAT3(0.3f, 0.41f, 0.0f));
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
		game->updateGame(dt, stateManager);
	}
}

//Updates our viewMatrix based on the camera's position
void MyDemoGame::UpdateCamera()
{
	// values used to translate and rotate the camera in response to input
	float translationScale = -0.001f;
	float rotationScale = -.01f;
	//Left ad right arrow keys alter X position

	// make all camera manipulations occur at double speed when holding spacebar
	if (GetAsyncKeyState(VK_SPACE) & 0x8000)
	{
		translationScale *= 2.0f;
		rotationScale *= 2.0f;
	}

	if (GetAsyncKeyState(VK_LEFT) & 0x8000)
	{
		gameCam.setDistanceX(translationScale);
		gameCam.setPosition(gameCam.getDistanceX(), gameCam.getDistanceY(), gameCam.getDistanceZ());
		cameraPosition = XMVectorSet(gameCam.getPositionX(), gameCam.getPositionY(), gameCam.getPositionZ(), 0);
		cameraRotation = XMVectorSet(gameCam.getRotationX(), gameCam.getRotationY(), gameCam.getRotationZ(), 0);
	}
	if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
	{
		gameCam.setDistanceX(-translationScale);
		gameCam.setPosition(gameCam.getDistanceX(), gameCam.getDistanceY(), gameCam.getDistanceZ());
		cameraPosition = XMVectorSet(gameCam.getPositionX(), gameCam.getPositionY(), gameCam.getPositionZ(), 0);
		cameraRotation = XMVectorSet(gameCam.getRotationX(), gameCam.getRotationY(), gameCam.getRotationZ(), 0);
	}

	//Up/Down arrow keys alter Y position
	if (GetAsyncKeyState(VK_DOWN) & 0x8000)
	{
		gameCam.setDistanceY(translationScale);
		gameCam.setPosition(gameCam.getDistanceX(), gameCam.getDistanceY(), gameCam.getDistanceZ());
		cameraPosition = XMVectorSet(gameCam.getPositionX(), gameCam.getPositionY(), gameCam.getPositionZ(), 0);
		cameraRotation = XMVectorSet(gameCam.getRotationX(), gameCam.getRotationY(), gameCam.getRotationZ(), 0);
	}
	if (GetAsyncKeyState(VK_UP) & 0x8000)
	{
		gameCam.setDistanceY(-translationScale);
		gameCam.setPosition(gameCam.getDistanceX(), gameCam.getDistanceY(), gameCam.getDistanceZ());
		cameraPosition = XMVectorSet(gameCam.getPositionX(), gameCam.getPositionY(), gameCam.getPositionZ(), 0);
		cameraRotation = XMVectorSet(gameCam.getRotationX(), gameCam.getRotationY(), gameCam.getRotationZ(), 0);
	}

	//5 and 0 on the numpad alter the Z position
	if (GetAsyncKeyState(VK_NUMPAD0) & 0x8000)
	{
		gameCam.setDistanceZ(translationScale);
		gameCam.setPosition(gameCam.getDistanceX(), gameCam.getDistanceY(), gameCam.getDistanceZ());
		cameraPosition = XMVectorSet(gameCam.getPositionX(), gameCam.getPositionY(), gameCam.getPositionZ(), 0);
		cameraRotation = XMVectorSet(gameCam.getRotationX(), gameCam.getRotationY(), gameCam.getRotationZ(), 0);
	}
	if (GetAsyncKeyState(VK_NUMPAD5) & 0x8000)
	{
		gameCam.setDistanceZ(-translationScale);
		gameCam.setPosition(gameCam.getDistanceX(), gameCam.getDistanceY(), gameCam.getDistanceZ());
		cameraPosition = XMVectorSet(gameCam.getPositionX(), gameCam.getPositionY(), gameCam.getPositionZ(), 0);
		cameraRotation = XMVectorSet(gameCam.getRotationX(), gameCam.getRotationY(), gameCam.getRotationZ(), 0);
	}

	//4 and 6 on the numpad will rotate along the X axis
	if (GetAsyncKeyState(VK_NUMPAD4) & 0x8000)
	{
		gameCam.setRotationDistanceX(rotationScale);
		gameCam.setRotation(gameCam.getRotationDistanceX(), gameCam.getRotationDistanceY(), gameCam.getRotationDistanceZ());
		cameraRotation = XMVectorSet(gameCam.getRotationX(), gameCam.getRotationY(), gameCam.getRotationZ(), 0);
	}
	if (GetAsyncKeyState(VK_NUMPAD6) & 0x8000)
	{
		gameCam.setRotationDistanceX(-rotationScale);
		gameCam.setRotation(gameCam.getRotationDistanceX(), gameCam.getRotationDistanceY(), gameCam.getRotationDistanceZ());
		cameraRotation = XMVectorSet(gameCam.getRotationX(), gameCam.getRotationY(), gameCam.getRotationZ(), 0);
	}

	//8 ad 2 on the unmpad will rotate along the y axis
	if (GetAsyncKeyState(VK_NUMPAD8) & 0x8000)
	{
		gameCam.setRotationDistanceY(-rotationScale);
		gameCam.setRotation(gameCam.getRotationDistanceX(), gameCam.getRotationDistanceY(), gameCam.getRotationDistanceZ());
		cameraRotation = XMVectorSet(gameCam.getRotationX(), gameCam.getRotationY(), gameCam.getRotationZ(), 0);
	}
	if (GetAsyncKeyState(VK_NUMPAD2) & 0x8000)
	{
		gameCam.setRotationDistanceY(rotationScale);
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

	// return the manipulation scales to their normal values
	translationScale = -0.001f;
	rotationScale = -.01f;

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
		game->drawGame(viewMatrix, projectionMatrix);
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
	else if (state == L"Lose")
	{
		UINT offset = 0;
		//UINT offset = 0;
		UINT stride = menuEntities[2]->g_mesh->sizeofvertex;
		// Set up the input assembler
		deviceContext->IASetInputLayout(menuEntities[2]->g_mat->shaderProgram->vsInputLayout);
		deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		menuEntities[2]->g_mat->shaderProgram->vsConstantBuffer->dataToSendToConstantBuffer.world = menuEntities[2]->getWorld();
		menuEntities[2]->g_mat->shaderProgram->vsConstantBuffer->dataToSendToConstantBuffer.view = viewMatrix;
		menuEntities[2]->g_mat->shaderProgram->vsConstantBuffer->dataToSendToConstantBuffer.projection = projectionMatrix;

		deviceContext->UpdateSubresource(
			menuEntities[2]->g_mat->shaderProgram->vsConstantBuffer->constantBuffer,
			0,
			NULL,
			&menuEntities[2]->g_mat->shaderProgram->vsConstantBuffer->dataToSendToConstantBuffer,
			0,
			0);

		deviceContext->IASetVertexBuffers(0, 1, &menuEntities[2]->g_mesh->v_buffer, &stride, &offset);
		deviceContext->IASetIndexBuffer(menuEntities[2]->g_mesh->i_buffer, DXGI_FORMAT_R32_UINT, 0);

		deviceContext->PSSetSamplers(0, 1, &menuEntities[2]->g_mat->samplerState);
		deviceContext->PSSetShaderResources(0, 1, &menuEntities[2]->g_mat->resourceView);



		// Set the current vertex and pixel shaders, as well the constant buffer for the vert shader
		deviceContext->VSSetShader(menuEntities[2]->g_mat->shaderProgram->vertexShader, NULL, 0);
		deviceContext->VSSetConstantBuffers(0, 1, &menuEntities[0]->g_mat->shaderProgram->vsConstantBuffer->constantBuffer);
		deviceContext->PSSetShader(menuEntities[2]->g_mat->shaderProgram->pixelShader, NULL, 0);

		// Finally do the actual drawing
		deviceContext->DrawIndexed(
			menuEntities.at(2)->g_mesh->m_size,	// The number of indices we're using in this draw
			0,
			0);
	}
	DrawUserInterface(0xff0099ff);

	// Present the buffer
	HR(swapChain->Present(0, 0));
}

void MyDemoGame::DrawUserInterface(UINT32 textColor)
{
	if (state == L"Game" || state == L"Pause" || state == L"Win")
	{


		if (!uiInitialized)
		{
			// set up the font factory
			// The font wrapper used to actually draw the text
			HRESULT hResult = FW1CreateFactory(FW1_VERSION, &pFW1Factory);
			pFW1Factory->CreateFontWrapper(device, L"Copperplate Gothic", &pFontWrapper);
			uiInitialized = true;
		}



		pFontWrapper->DrawString(
			deviceContext,
			state,// String
			24.0f,// Font size
			viewport.Width - 125.0f,// X position
			50.0f,// Y position
			0xff0099ff,// Text color, 0xAaBbGgRr
			0x800// Flags (currently set to "restore state" to not ruin the rest of the scene)
			);

		if (state == L"Game")
		{
			game->drawText(pFontWrapper);
		}
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

	if (state == L"Menu" || state == L"Instructions" || state == L"Lose")
	{
		HandleUIClick(x, y);
	}


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

void MyDemoGame::HandleUIClick(int x, int y)
{
	for (int i = 0; i < buttons.size(); i++)
	{
		if (state == states[buttons[i].activeState] && x > buttons[i].dimensions.left && x < buttons[i].dimensions.right && y > buttons[i].dimensions.top && y < buttons[i].dimensions.bottom)
		{
			stateManager->setState(buttons[i].newState);
		}
	}
}

#pragma endregion
