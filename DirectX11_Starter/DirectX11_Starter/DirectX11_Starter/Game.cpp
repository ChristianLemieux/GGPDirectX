#include "Game.h"

Game::Game(ID3D11Device* dev, ID3D11DeviceContext* devCxt){
	device = dev;
	deviceContext = devCxt;
}

void Game::initGame(){
	constantBufferList.push_back(new ConstantBuffer(dataToSendToVSConstantBuffer, device));
	shaderProgram = new ShaderProgram(L"VertexShader.cso", L"PixelShader.cso", device, constantBufferList[0], constantBufferList[0]);
	ObjectLoader *asteroidObject = new ObjectLoader(device);
	Mesh *asteroid = asteroidObject->LoadModel("asteroid.obj");
	ID3D11SamplerState* sample = nullptr;
	//create sampler state
	samplerStates.push_back(new SamplerState(sample));
	samplerStates[0]->createSamplerState(device);
	//create materials
	materials.push_back(new Material(device, deviceContext, samplerStates[0]->sampler, L"spaceShipTexture.jpg", shaderProgram));
	materials.push_back(new Material(device, deviceContext, samplerStates[0]->sampler, L"asteroid.jpg", shaderProgram));

	gameEntities.push_back(new GameEntity(asteroid, materials[0]));

	//comment
	for (int i = 1; i < 20; i++)
	{
		gameEntities.push_back(new GameEntity(asteroid, materials[1]));
		gameEntities[i]->scale(XMFLOAT3(0.5f, 0.5f, 0.5f));
		gameEntities[i]->translate(XMFLOAT3((((float)rand() / (float)(RAND_MAX))* 25.0f) + 10.0f, (((float)rand() / (float)(RAND_MAX))* 8.0f) - 5.0f, 0.0f));
	}
}

void Game::updateGame(float dt){
	if (GetAsyncKeyState('D') & 0x8000){
		gameEntities[0]->translate(XMFLOAT3(0.3f * dt, 0.0f, 0.0f));
	}
	//move triangle left
	if (GetAsyncKeyState('A') & 0x8000){
		gameEntities[0]->translate(XMFLOAT3(-0.3f * dt, 0.0f, 0.0f));
	}
	//move triangle up
	if (GetAsyncKeyState('W') & 0x8000){
		gameEntities[0]->translate(XMFLOAT3(0.0f, 0.3f * dt, 0.0f));
	}
	//move triangle down
	if (GetAsyncKeyState('S') & 0x8000){
		gameEntities[0]->translate(XMFLOAT3(0.0f, -0.3f * dt, 0.0f));
	}

	for (unsigned int i = 1; i < 20; i++)
	{
		gameEntities[i]->translate(XMFLOAT3(-0.85f * dt, 0.0f, 0.0f));

		if (gameEntities[i]->getPosition()._41 < -10)
		{
			gameEntities[i]->translate(XMFLOAT3((((float)rand() / (float)(RAND_MAX))* 25.0f) + 10.0f, (((float)rand() / (float)(RAND_MAX))* 8.0f) - 5.0f, 0.0f));
		}

		//gameEntities[i]->rotate(XMFLOAT3(0.0f, 0.0f, 0.001f));
	}
}

void Game::drawGame(XMFLOAT4X4 viewMatrix, XMFLOAT4X4 projectionMatrix){
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
	}
}