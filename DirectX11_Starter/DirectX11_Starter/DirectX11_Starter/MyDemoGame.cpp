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
#include "SpriteBatch.h"
#include "SpriteFont.h"
#include "SimpleMath.h"

std::unique_ptr<DirectX::SpriteBatch> spriteBatch;
std::unique_ptr<DirectX::SpriteFont> spriteFont;


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
	if (pFW1Factory){
		delete pFW1Factory;
		pFW1Factory = nullptr;
	}
	if (pFontWrapper){
		delete pFontWrapper;
		pFontWrapper = nullptr;
	}
	if (state){
		//delete state;
		//state = nullptr;
	}
	if (MatrixCB){
		delete MatrixCB;
		MatrixCB = nullptr;
	}
	if (LightCB){
		delete LightCB;
		LightCB = nullptr;
	}
	if (CamCB){
		delete CamCB;
		CamCB = nullptr;
	}
	if (samplerState){
		delete samplerState;
		samplerState = nullptr;
	}
	if (game){
		delete game;
		game = nullptr;
	}
	
}

#pragma endregion

#pragma region Initialization

// Initializes the base class (including the window and D3D),
// sets up our geometry and loads the shaders (among other things)
bool MyDemoGame::Init()
{


	
	if (!DirectXGame::Init())
		return false;

	stateManager = new StateManager();
	ObjectLoader* objLoader = new ObjectLoader(device);
	Mesh* menuMesh = objLoader->LoadModel("Menu.obj");
	game = new Game(device, deviceContext);
	ID3D11SamplerState* sample = nullptr;
	samplerState = new SamplerState(sample);
	samplerState->createSamplerState(device);
	MatrixCB = new ConstantBuffer(dataToSendToVSConstantBuffer, device);
	LightCB = new ConstantBuffer(dataToSendToLightConstantBuffer, device);
	CamCB = new ConstantBuffer(dataToSendToCameraConstantBuffer, device);
	shaderProgram = new ShaderProgram(L"VertexShader.cso", L"PixelShader.cso", device, MatrixCB, LightCB, CamCB);
	gameStates.push_back(new State(device, deviceContext, sample, L"StartScreen.png", menuMesh, shaderProgram));
	gameStates.push_back(new State(device, deviceContext, sample, L"InstructionsScreen.png", menuMesh, shaderProgram));
	gameStates.push_back(new State(device, deviceContext, sample, L"gameOverScreen.png", menuMesh, shaderProgram));
	game->initGame(samplerState);

	// Set up buffers and such

	//Create SpriteBatch
	spriteBatch.reset(new DirectX::SpriteBatch(deviceContext));

	//Create Spritefont
	spriteFont.reset(new DirectX::SpriteFont(device, L"Font.spritesheet"));


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
	tempButton = { { 100, 480, 685, 560 }, 2, 0 };
	buttons.push_back(tempButton);
	tempButton = { { 187, 360, 609, 405 }, 5, 0 };
	buttons.push_back(tempButton);
	tempButton = { { 195, 300, 615, 350 }, 5, 1 };
	buttons.push_back(tempButton);

	//initialize states so that state strings can be looked up with a state index

	return true;
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
	XMFLOAT3 camPos = XMFLOAT3(gameCam.getPositionX(), gameCam.getPositionY(), gameCam.getPositionZ());
	// Clear the buffer
	deviceContext->ClearRenderTargetView(renderTargetView, color);
	deviceContext->ClearDepthStencilView(
		depthStencilView,
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.0f,
		0);
	if (state == L"Game" || state == L"Pause")
	{
		game->drawGame(viewMatrix, projectionMatrix, camPos);
	}
	else if (state == L"Menu")
	{
		gameStates[0]->draw(viewMatrix, projectionMatrix);
	}
	else if (state == L"Instructions")
	{
		gameStates[1]->draw(viewMatrix, projectionMatrix);

	}
	else if (state == L"Lose")
	{
		gameStates[2]->draw(viewMatrix, projectionMatrix);
	}


	spriteBatch->Begin();
	if (state == L"Game" || state == L"Pause" || state == L"Win")
	{

		std::wstring pi = std::to_wstring(game->hullIntegrity);
		const WCHAR* szName = pi.c_str();

		//Draw Sprites and fonts
		spriteFont->DrawString(spriteBatch.get(), L"Health: ", DirectX::SimpleMath::Vector2(15, 25));
		spriteFont->DrawString(spriteBatch.get(), szName, DirectX::SimpleMath::Vector2(225, 25), Colors::LawnGreen);

		if (game->hullIntegrity <= 30)
		{
			spriteFont->DrawString(spriteBatch.get(), szName, DirectX::SimpleMath::Vector2(225, 25), Colors::Red);
		}

		if (state == L"Pause")
		{
			spriteFont->DrawString(spriteBatch.get(), L"Pause", DirectX::SimpleMath::Vector2(viewport.Width - 225, 25));
		}
	}
	spriteBatch->End();

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

	// If this screen has buttons, send the click coordinates to the UI click handler
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

// Checks to see if the region clicked (on a screen with clickable regions) was within the bounds of an active button, if so a state change occurs
void MyDemoGame::HandleUIClick(int x, int y)
{
	for (int i = 0; i < buttons.size(); i++)
	{
		if (state == stateManager->getStateFromIndex(buttons[i].activeState) && x > buttons[i].dimensions.left && x < buttons[i].dimensions.right && y > buttons[i].dimensions.top && y < buttons[i].dimensions.bottom)
		{
			stateManager->setState(buttons[i].newState);
		}
	}
}

#pragma endregion
