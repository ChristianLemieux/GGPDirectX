#include "Game.h"

Game::Game(ID3D11Device* dev, ID3D11DeviceContext* devCxt){
	device = dev;
	deviceContext = devCxt;
}

void Game::initGame(SamplerState *samplerStates){
	notColliding = false;
	canTakeDamage = true;
	hullIntegrity = 100;
	constantBufferList.push_back(new ConstantBuffer(dataToSendToVSConstantBuffer, device));
	shaderProgram = new ShaderProgram(L"VertexShader.cso", L"PixelShader.cso", device, constantBufferList[0], constantBufferList[0]);
	ObjectLoader *asteroidObject = new ObjectLoader(device);
	Mesh *asteroid = asteroidObject->LoadModel("asteroid.obj");
	ID3D11SamplerState* sample = nullptr;
	//create sampler state
	//create materials
	materials.push_back(new Material(device, deviceContext, samplerStates->sampler, L"spaceShipTexture.jpg", shaderProgram));
	materials.push_back(new Material(device, deviceContext, samplerStates->sampler, L"asteroid.jpg", shaderProgram));

	//create game entities
	gameEntities.push_back(new GameEntity(asteroid, materials[0]));
	gameEntities[0]->translate(XMFLOAT3(0.0f, 0.0f, 0.0f));
	gameEntities[0]->scale(XMFLOAT3(0.1f, 0.1f, 0.1f));

	//comment
	for (int i = 1; i < 30; i++)
	{
		gameEntities.push_back(new GameEntity(asteroid, materials[1]));
		gameEntities[i]->scale(XMFLOAT3(0.1f, 0.1f, 0.1f));
		gameEntities[i]->setPosition(XMFLOAT3(((rand() % 60) + 30), ((rand() % 40) - 19.0f), 0.0f));
	}
}

void Game::updateGame(float dt,StateManager *stateManager){
	collision = L"Not Colliding";
	notColliding = false;
	
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
					gameEntities[i]->setPosition(XMFLOAT3(30.0f, (rand() % 40) - 19.0f, 0.0f));
					//lose condition
					if (hullIntegrity <= 0)
					{
						stateManager->setState(5);
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


void Game::drawText(IFW1FontWrapper *pFontWrapper)
{
	std::wstring pi = L"Hull Integrity:" + std::to_wstring(hullIntegrity);
	const WCHAR* szName = pi.c_str();

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
		800 - 125.0f,// X position
		15.0f,// Y position
		0xff0099ff,// Text color, 0xAaBbGgRr
		0x800// Flags (currently set to "restore state" to not ruin the rest of the scene)
		);
}