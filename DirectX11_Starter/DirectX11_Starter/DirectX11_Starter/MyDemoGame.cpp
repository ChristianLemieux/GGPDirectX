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
	
	return game.Run();
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
	cameraCrossProduct = XMVector3Cross(cameraPosition, upDirection);
	return true;
}


// Creates the vertex and index buffers for a single triangle
void MyDemoGame::CreateGeometryBuffers()
{
	XMFLOAT4 red	= XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 green	= XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	XMFLOAT4 blue	= XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
	XMFLOAT4 orange = XMFLOAT4(1.0f, 0.5f, 0.0f, 1.0f);

	Vertex triangleVertices[] = {
			{ XMFLOAT3(+0.0f, +1.0f, +0.0f), red, XMFLOAT2(0.5, 0) },
			{ XMFLOAT3(-1.0f, -0.1f, +0.0f), green, XMFLOAT2(0, 1) },
			{ XMFLOAT3(+1.0f, -0.1f, +0.0f), blue, XMFLOAT2(1, 1) },
	};
	Vertex squareVertices[] = {
			{ XMFLOAT3(-1.0f, +1.0f, +0.0f), red, XMFLOAT2(0, 0) },
			{ XMFLOAT3(-1.0f, -1.0f, +0.0f), green, XMFLOAT2(0, 1) },
			{ XMFLOAT3(+1.0f, +1.0f, +0.0f), blue, XMFLOAT2(1, 0) },
			{ XMFLOAT3(+1.0f, -1.0f, +0.0f), orange, XMFLOAT2(1, 1) }
	};
	// Set up the indices
	UINT triangleIndices[] = {0, 2, 1};

	//Set up second set of the indices
	UINT squareIndices[] = { 0, 2, 1, 2, 3, 1 };

	triangle = new Mesh(triangleVertices, triangleIndices, 3, device);
	square = new Mesh(squareVertices, squareIndices, 6, device);

	ID3D11SamplerState* sample = nullptr;
	//create sampler state
	samplerStates.push_back(new SamplerState(*sample));
	samplerStates[0]->createSamplerState(device);
	//create materials
	materials.push_back(new Material(device, deviceContext, sample, L"bullet.png"));
	materials.push_back(new Material(device, deviceContext, sample, L"120322_0001.jpg"));
	//create game entities
	gameEntities.push_back(new GameEntity(triangle, materials[0]));
	gameEntities.push_back(new GameEntity(square, materials[1]));
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

	// Load Vertex Shader --------------------------------------
	ID3DBlob* vsBlob;
	D3DReadFileToBlob(L"VertexShader.cso", &vsBlob);

	// Create the shader on the device
	HR(device->CreateVertexShader(
		vsBlob->GetBufferPointer(),
		vsBlob->GetBufferSize(),
		NULL,
		&vertexShader));

	// Before cleaning up the data, create the input layout
	HR(device->CreateInputLayout(
		vertexDesc,
		ARRAYSIZE(vertexDesc),
		vsBlob->GetBufferPointer(),
		vsBlob->GetBufferSize(),
		&inputLayout));

	// Clean up
	ReleaseMacro(vsBlob);

	// Load Pixel Shader ---------------------------------------
	ID3DBlob* psBlob;
	D3DReadFileToBlob(L"PixelShader.cso", &psBlob);

	// Create the shader on the device
	HR(device->CreatePixelShader(
		psBlob->GetBufferPointer(),
		psBlob->GetBufferSize(),
		NULL,
		&pixelShader));

	// Clean up
	ReleaseMacro(psBlob);

	// Constant buffers ----------------------------------------
	D3D11_BUFFER_DESC cBufferDesc;
	cBufferDesc.ByteWidth           = sizeof(dataToSendToVSConstantBuffer);
	cBufferDesc.Usage				= D3D11_USAGE_DEFAULT;
	cBufferDesc.BindFlags			= D3D11_BIND_CONSTANT_BUFFER;
	cBufferDesc.CPUAccessFlags		= 0;
	cBufferDesc.MiscFlags			= 0;
	cBufferDesc.StructureByteStride = 0;
	HR(device->CreateBuffer(
		&cBufferDesc,
		NULL,
		&vsConstantBuffer));
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
	if (GetAsyncKeyState('L') & 0x8000){
		gameEntities[0]->translate(XMFLOAT3(0.001f, 0.0f, 0.0f));
	}
	//move triangle left
	if (GetAsyncKeyState('K') & 0x8000){
		gameEntities[0]->translate(XMFLOAT3(-0.001f, 0.0f, 0.0f));
	}
	//move triangle up
	if (GetAsyncKeyState('O') & 0x8000){
		gameEntities[0]->translate(XMFLOAT3(0.0f, 0.001f, 0.0f));
	}
	//move triangle down
	if (GetAsyncKeyState('M') & 0x8000){
		gameEntities[0]->translate(XMFLOAT3(0.0f, -0.001f, 0.0f));
	}
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
		gameEntities[1]->rotate(XMFLOAT3(0.001f, 0.0f, 0.0f));
	}
	//rotate square in negative x
	if (GetAsyncKeyState('W') & 0x8000){
		gameEntities[1]->rotate(XMFLOAT3(-0.001f, 0.0f, 0.0f));
	}
	//rotate square in positive y
	if (GetAsyncKeyState('D') & 0x8000){
		gameEntities[1]->rotate(XMFLOAT3(0.0f, 0.001f, 0.0f));
	}
	//rotate square in negative y
	if (GetAsyncKeyState('A') & 0x8000){
		gameEntities[1]->rotate(XMFLOAT3(0.0f, -0.001f, 0.0f));
	}
	//rotate square in positive z
	if (GetAsyncKeyState('Q') & 0x8000){
		gameEntities[1]->rotate(XMFLOAT3(0.0f, 0.0f, 0.001f));
	}
	//rotate square in negative z
	if (GetAsyncKeyState('F') & 0x8000){
		gameEntities[1]->rotate(XMFLOAT3(0.0f, 0.0f, -0.001f));
	}

	//Increase Scale of "x" acis
	if (GetAsyncKeyState('C') & 0x8000){
		gameEntities[0]->scale(XMFLOAT3(1.001f, 1.0f, 1.0f));
		gameEntities[1]->scale(XMFLOAT3(1.001f, 1.0f, 1.0f));
	}	
	//Decrease Scale of "x" axis
	if (GetAsyncKeyState('V') & 0x8000){
		gameEntities[0]->scale(XMFLOAT3(0.999f, 1.0f, 1.0f));
		gameEntities[1]->scale(XMFLOAT3(0.999f, 1.0f, 1.0f));
	}


	//Increase Scale of "y" axis
	if (GetAsyncKeyState('B') & 0x8000){
		gameEntities[0]->scale(XMFLOAT3(1.0f, 1.001f, 1.0f));
		gameEntities[1]->scale(XMFLOAT3(1.0f, 1.001f, 1.0f));
	}
	//Decrease Scale of "y" axis
	if (GetAsyncKeyState('N') & 0x8000){
		gameEntities[0]->scale(XMFLOAT3(1.0f, 0.999f, 1.0f));
		gameEntities[1]->scale(XMFLOAT3(1.0f, 0.999f, 1.0f));
	}

	//Increase Scale of "z" axis
	if (GetAsyncKeyState('G') & 0x8000){
		gameEntities[0]->scale(XMFLOAT3(1.0f, 1.0f, 1.001f));
		gameEntities[1]->scale(XMFLOAT3(1.0f, 1.0f, 1.001f));
	}
	//Decrease Scale of "z" axis
	if (GetAsyncKeyState('H') & 0x8000){
		gameEntities[0]->scale(XMFLOAT3(1.0f, 1.0f, 0.999f));
		gameEntities[1]->scale(XMFLOAT3(1.0f, 1.0f, 0.999f));
	}


}

//Updates our viewMatrix based on the camera's position
void MyDemoGame::UpdateCamera()
{
	//Left ad right arrow keys alter X position
	if (GetAsyncKeyState(VK_LEFT) & 0x8000)
	{
		gameCam.setDistanceX(-.001);
		gameCam.setPosition(gameCam.getDistanceX(), gameCam.getDistanceY(), gameCam.getDistanceZ());
		cameraPosition = XMVectorSet(gameCam.getPositionX(), gameCam.getPositionY(), gameCam.getPositionZ(), 0);
		cameraRotation = XMVectorSet(gameCam.getRotationX(), gameCam.getRotationY(), gameCam.getRotationZ(), 0);
		cameraCrossProduct = XMVector3Cross(upDirection, cameraPosition);

	}
	if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
	{
		gameCam.setDistanceX(+.001);
		gameCam.setPosition(gameCam.getDistanceX(), gameCam.getDistanceY(), gameCam.getDistanceZ());
		cameraPosition = XMVectorSet(gameCam.getPositionX(), gameCam.getPositionY(), gameCam.getPositionZ(), 0);
		cameraRotation = XMVectorSet(gameCam.getRotationX(), gameCam.getRotationY(), gameCam.getRotationZ(), 0);
		cameraCrossProduct = XMVector3Cross(upDirection, cameraPosition);
	}

	//Up/Down arrow keys alter Y position
	if (GetAsyncKeyState(VK_DOWN) & 0x8000)
	{
		gameCam.setDistanceY(-.001);
		gameCam.setPosition(gameCam.getDistanceX(), gameCam.getDistanceY(), gameCam.getDistanceZ());
		cameraPosition = XMVectorSet(gameCam.getPositionX(), gameCam.getPositionY(), gameCam.getPositionZ(), 0);
		cameraRotation = XMVectorSet(gameCam.getRotationX(), gameCam.getRotationY(), gameCam.getRotationZ(), 0);
		cameraCrossProduct = XMVector3Cross(upDirection, cameraPosition);
	}
	if (GetAsyncKeyState(VK_UP) & 0x8000)
	{
		gameCam.setDistanceY(+.001);
		gameCam.setPosition(gameCam.getDistanceX(), gameCam.getDistanceY(), gameCam.getDistanceZ());
		cameraPosition = XMVectorSet(gameCam.getPositionX(), gameCam.getPositionY(), gameCam.getPositionZ(), 0);
		cameraRotation = XMVectorSet(gameCam.getRotationX(), gameCam.getRotationY(), gameCam.getRotationZ(), 0);
		cameraCrossProduct = XMVector3Cross(upDirection, cameraPosition);
	}

	//5 and 0 on the numpad alter the Z position
	if (GetAsyncKeyState(VK_NUMPAD0) & 0x8000)
	{
		gameCam.setDistanceZ(-.001);
		gameCam.setPosition(gameCam.getDistanceX(), gameCam.getDistanceY(), gameCam.getDistanceZ());
		cameraPosition = XMVectorSet(gameCam.getPositionX(), gameCam.getPositionY(), gameCam.getPositionZ(), 0);
		cameraRotation = XMVectorSet(gameCam.getRotationX(), gameCam.getRotationY(), gameCam.getRotationZ(), 0);
		cameraCrossProduct = XMVector3Cross(upDirection, cameraPosition);
	}
	if (GetAsyncKeyState(VK_NUMPAD5) & 0x8000)
	{
		gameCam.setDistanceZ(+.001);
		gameCam.setPosition(gameCam.getDistanceX(), gameCam.getDistanceY(), gameCam.getDistanceZ());
		cameraPosition = XMVectorSet(gameCam.getPositionX(), gameCam.getPositionY(), gameCam.getPositionZ(), 0);
		cameraRotation = XMVectorSet(gameCam.getRotationX(), gameCam.getRotationY(), gameCam.getRotationZ(), 0);
		cameraCrossProduct = XMVector3Cross(upDirection, cameraPosition);
	}

	//4 and 6 on the numpad will rotate along the X axis
	if (GetAsyncKeyState(VK_NUMPAD4) & 0x8000)
	{
		gameCam.setRotationDistanceX(-.01);
		gameCam.setRotation(gameCam.getRotationDistanceX(), gameCam.getRotationDistanceY(), gameCam.getRotationDistanceZ());
		cameraRotation = XMVectorSet(gameCam.getRotationX(), gameCam.getRotationY(), gameCam.getRotationZ(), 0);
	}
	if (GetAsyncKeyState(VK_NUMPAD6) & 0x8000)
	{
		gameCam.setRotationDistanceX(+.01);
		gameCam.setRotation(gameCam.getRotationDistanceX(), gameCam.getRotationDistanceY(), gameCam.getRotationDistanceZ());
		cameraRotation = XMVectorSet(gameCam.getRotationX(), gameCam.getRotationY(), gameCam.getRotationZ(), 0);
	}

	//8 ad 2 on the unmpad will rotate along the y axis
	if (GetAsyncKeyState(VK_NUMPAD8) & 0x8000)
	{
		gameCam.setRotationDistanceY(+.01);
		gameCam.setRotation(gameCam.getRotationDistanceX(), gameCam.getRotationDistanceY(), gameCam.getRotationDistanceZ());
		cameraRotation = XMVectorSet(gameCam.getRotationX(), gameCam.getRotationY(), gameCam.getRotationZ(), 0);
	}
	if (GetAsyncKeyState(VK_NUMPAD2) & 0x8000)
	{
		gameCam.setRotationDistanceY(-.01);
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

	// Set up the input assembler
	deviceContext->IASetInputLayout(inputLayout);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// Set buffers in the input assembler
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	for (int i = 0; i < gameEntities.size(); i++){
		dataToSendToVSConstantBuffer.world = gameEntities[i]->getWorld();
		dataToSendToVSConstantBuffer.view = viewMatrix;
		dataToSendToVSConstantBuffer.projection = projectionMatrix;

		deviceContext->UpdateSubresource(
			vsConstantBuffer,
			0,
			NULL,
			&dataToSendToVSConstantBuffer,
			0,
			0);
		deviceContext->IASetVertexBuffers(0, 1, &gameEntities[i]->g_mesh->v_buffer, &stride, &offset);
		deviceContext->IASetIndexBuffer(gameEntities[i]->g_mesh->i_buffer, DXGI_FORMAT_R32_UINT, 0);

		deviceContext->PSSetSamplers(0, 1, &gameEntities[i]->g_mat->samplerState);
		deviceContext->PSSetShaderResources(0, 1, &gameEntities[i]->g_mat->resourceView);

		

		// Set the current vertex and pixel shaders, as well the constant buffer for the vert shader
		deviceContext->VSSetShader(vertexShader, NULL, 0);
		deviceContext->VSSetConstantBuffers(
			0,	// Corresponds to the constant buffer's register in the vertex shader
			1,
			&vsConstantBuffer);
		deviceContext->PSSetShader(pixelShader, NULL, 0);

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