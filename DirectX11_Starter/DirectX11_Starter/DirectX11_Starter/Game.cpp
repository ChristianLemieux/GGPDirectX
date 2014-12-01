#include "Game.h"

Game::Game(ID3D11Device* dev, ID3D11DeviceContext* devCxt){
	device = dev;
	deviceContext = devCxt;
}

Game::~Game(void){
	ReleaseMacro(device);
	ReleaseMacro(deviceContext);
	/*if (shaderProgram){
		delete shaderProgram;
		shaderProgram = nullptr;
	}
	if (multiTex){
		delete multiTex;
		multiTex = nullptr;
	}*/
}

void Game::initGame(SamplerState *samplerStates){

	//sound effect engine
	engine = irrklang::createIrrKlangDevice();

	lighting.ambientColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	lighting.diffuseColor = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	lighting.lightDirection = XMFLOAT3(0.0f, 0.0f, 1.0f);
	lighting.specularColor = XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f);
	lighting.specularPower = 5.0f;

	notColliding = false;
	canTakeDamage = true;
	hullIntegrity = 100;
	constantBufferList.push_back(new ConstantBuffer(dataToSendToVSConstantBuffer, device)); //create matrix constant buffer
	constantBufferList.push_back(new ConstantBuffer(dataToSendToLightConstantBuffer, device));//create light constant buffer
	constantBufferList.push_back(new ConstantBuffer(dataToSendToCameraConstantBuffer, device)); //create camera constant buffer

	//create shader program-Params(vertex shader, pixel shader, device, constant buffers)
	shaderProgram = new ShaderProgram(L"VertexShader.cso", L"PixelShader.cso", device, constantBufferList[0], constantBufferList[1], constantBufferList[2]);
	ObjectLoader *asteroidObject = new ObjectLoader(device);
	asteroid = asteroidObject->LoadModel("asteroid.obj");
	ID3D11SamplerState* sample = nullptr;

	//background
	ObjectLoader *bgObject = new ObjectLoader(device);
	Mesh *bg = bgObject->LoadModel("Menu.obj");

	player = new Player(device, deviceContext, constantBufferList, samplerStates->sampler, asteroid);

	//create sampler state
	//create materials
	materials.push_back(new Material(device, deviceContext, samplerStates->sampler, L"spaceShipTexture.jpg", shaderProgram));
	materials.push_back(new Material(device, deviceContext, samplerStates->sampler, L"asteroid.jpg", shaderProgram));
	materials.push_back(new Material(device, deviceContext, samplerStates->sampler, L"background.jpg", shaderProgram));
	materials.push_back(new Material(device, deviceContext, samplerStates->sampler, L"bullet.jpg", shaderProgram));

	//two backgrounds
	gameEntities.push_back(new GameEntity(bg, materials[2]));
	gameEntities[0]->setPosition(XMFLOAT3(2.5f, 0.0f, 6.0f));
	gameEntities.push_back(new GameEntity(bg, materials[2]));
	gameEntities[1]->setPosition(XMFLOAT3(15.0f, 0.0f, 6.0f));


	//comment
	for (int i = 2; i < 31; i++)
	{
		gameEntities.push_back(new GameEntity(asteroid, materials[1]));
		gameEntities[i]->scale(XMFLOAT3(0.1f, 0.1f, 0.1f));
		gameEntities[i]->setPosition(XMFLOAT3(((rand() % 60) + 30), ((rand() % 40) - 19.0f), 0.0f));
	}

}

void Game::updateGame(float dt, StateManager *stateManager){
	collision = L"Not Colliding";
	shotFired = false;
	notColliding = false;
	
		player->update(dt);
		if (GetAsyncKeyState('Q') & 0x8000)
		{
			if (projectiles.size() < 1)
			{
				fireProjectile();
			}
		}
		//parallex
		gameEntities[0]->translate(XMFLOAT3(-0.5f * dt, 0.0f, 0.0f));
		gameEntities[1]->translate(XMFLOAT3(-0.5f * dt, 0.0f, 0.0f));
		if (gameEntities[0]->getPosition()._41 < -14)
		{
			gameEntities[0]->setPosition(XMFLOAT3(15.0f, 0.0f, 6.0f));
		}
		if (gameEntities[1]->getPosition()._41 < -14)
		{
			gameEntities[1]->setPosition(XMFLOAT3(15.0f, 0.0f, 6.0f));
		}

		//moves asteroids across screen and respawns them when they leave the screen
		for (unsigned int i = 2; i < 31; i++)
		{
			gameEntities[i]->translate(XMFLOAT3(-8.0f * dt, 0.0f, 0.0f));

			//._41 is the x value for the position matrix of game entities
			if (gameEntities[i]->getPosition()._41 < -30)
			{
				gameEntities[i]->setPosition(XMFLOAT3(30.0f, (rand() % 40) - 19.0f, 0.0f));
			}

		}

		if (projectiles.size() > 0)
		{
			for (int x = projectiles.size() - 1; x >= 0; x--)
			{
				projectiles[x]->translate(XMFLOAT3(10.0f * dt, 0.0f, 0.0f));

				//._41 is the x value for the position matrix of game entities
				if (projectiles[x]->getPosition()._41 > 30)
				{
					projectiles.erase(projectiles.begin() + x);
				}

			}
		}

		float distance = 2.0f;
		float playerX = player->player->getPosition()._41;
		float playerY = player->player->getPosition()._42;
		for (int i = 2; i < 31; i++)
		{
			float testDistX = pow(playerX - gameEntities[i]->getPosition()._41, 2);
			float testDistY = pow(playerY - gameEntities[i]->getPosition()._42, 2);

			if (distance >= testDistX + testDistY)
			{
				notColliding = true;
				if (canTakeDamage && notColliding){
					hullIntegrity -= 10;
					gameEntities[i]->setPosition(XMFLOAT3(30.0f, (rand() % 40) - 19.0f, 0.0f));
					// start the sound engine with default parameters
					engine->play2D("Explosion.wav", false);
					//std::unique_ptr<SoundEffect> soundEffect(new SoundEffect(audEngine.get(), L"Explosion.wav"));
					//soundEffect->Play();
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

		if (projectiles.size() > 0)
		{
			for (int x = projectiles.size() - 1; x >= 0; x--)
			{
				for (int i = 3; i < 31; i++)
				{
					float testDistX = pow(projectiles[x]->getPosition()._41 - gameEntities[i]->getPosition()._41, 2);
					float testDistY = pow(projectiles[x]->getPosition()._42 - gameEntities[i]->getPosition()._42, 2);

					if (distance >= testDistX + testDistY)
					{
						projectiles.erase(projectiles.begin() + x);
						gameEntities[i]->setPosition(XMFLOAT3(30.0f, (rand() % 40) - 19.0f, 0.0f));
						break;
					}
				}
			}
		}

		if (!notColliding){
			canTakeDamage = true;
		}
}

void Game::fireProjectile()
{
	if (!shotFired)
	{
		float playerX = player->player->getPosition()._41;
		float playerY = player->player->getPosition()._42;
		projectiles.push_back(new GameEntity(asteroid, materials[3]));
		projectiles[projectiles.size() - 1]->scale(XMFLOAT3(0.1f, 0.1f, 0.1f));
		projectiles[projectiles.size() - 1]->setPosition(XMFLOAT3(playerX, playerY, 0.0f));
		shotFired = true;
	}
}

void Game::drawGame(XMFLOAT4X4 viewMatrix, XMFLOAT4X4 projectionMatrix, XMFLOAT3 camPos){
	UINT offset = 0;
	UINT stride = sizeof(Vertex);
	for (unsigned int i = 0; i < gameEntities.size(); i++){
		//UINT offset = 0;
		stride = gameEntities[i]->g_mesh->sizeofvertex;
		// Set up the input assembler
		deviceContext->IASetInputLayout(gameEntities[i]->g_mat->shaderProgram->vsInputLayout);
		deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//set values that get passed to matrix constant buffer
		
		gameEntities[i]->g_mat->shaderProgram->vsConstantBuffer->dataToSendToConstantBuffer.world = gameEntities[i]->getWorld();
		gameEntities[i]->g_mat->shaderProgram->vsConstantBuffer->dataToSendToConstantBuffer.view = viewMatrix;
		gameEntities[i]->g_mat->shaderProgram->vsConstantBuffer->dataToSendToConstantBuffer.projection = projectionMatrix;
		
		//set values that get passed to lighting constant buffer
		gameEntities[i]->g_mat->shaderProgram->psConstantBuffer->dataToSendToLightBuffer.ambientColor = lighting.ambientColor;
		gameEntities[i]->g_mat->shaderProgram->psConstantBuffer->dataToSendToLightBuffer.diffuseColor = lighting.diffuseColor;
		gameEntities[i]->g_mat->shaderProgram->psConstantBuffer->dataToSendToLightBuffer.lightDirection = lighting.lightDirection;
		gameEntities[i]->g_mat->shaderProgram->psConstantBuffer->dataToSendToLightBuffer.specularColor = lighting.specularColor;
		gameEntities[i]->g_mat->shaderProgram->psConstantBuffer->dataToSendToLightBuffer.specularPower = lighting.specularPower;
		//set values that get passed to camera constant buffer
		gameEntities[i]->g_mat->shaderProgram->camConstantBuffer->dataToSendToCameraBuffer.cameraPosition = camPos;
		gameEntities[i]->g_mat->shaderProgram->camConstantBuffer->dataToSendToCameraBuffer.padding = 1.0f;

		

		//matrix constant buffer
		deviceContext->UpdateSubresource(
			gameEntities[i]->g_mat->shaderProgram->vsConstantBuffer->constantBuffer,
			0,
			NULL,
			&gameEntities[i]->g_mat->shaderProgram->vsConstantBuffer->dataToSendToConstantBuffer,
			0,
			0);

		//camera constant buffer 
		deviceContext->UpdateSubresource(
			gameEntities[i]->g_mat->shaderProgram->camConstantBuffer->constantBuffer,
			0,
			NULL,
			&gameEntities[i]->g_mat->shaderProgram->camConstantBuffer->dataToSendToCameraBuffer,
			0,
			0);
		//light constant buffer
		deviceContext->UpdateSubresource(
			gameEntities[i]->g_mat->shaderProgram->psConstantBuffer->constantBuffer,
			0,
			NULL,
			&gameEntities[i]->g_mat->shaderProgram->psConstantBuffer->dataToSendToLightBuffer,
			0,
			0);

		deviceContext->IASetVertexBuffers(0, 1, &gameEntities[i]->g_mesh->v_buffer, &stride, &offset);
		deviceContext->IASetIndexBuffer(gameEntities[i]->g_mesh->i_buffer, DXGI_FORMAT_R32_UINT, 0);

		deviceContext->PSSetSamplers(0, 1, &gameEntities[i]->g_mat->samplerState);
		deviceContext->PSSetShaderResources(0, 1, &gameEntities[i]->g_mat->resourceView);



		// Set the current vertex and pixel shaders, as well the constant buffer for the vert shader
		deviceContext->VSSetShader(gameEntities[i]->g_mat->shaderProgram->vertexShader, NULL, 0);
		deviceContext->VSSetConstantBuffers(0, 1, &gameEntities[i]->g_mat->shaderProgram->vsConstantBuffer->constantBuffer); //set first constant vertex buffer-matrix
		deviceContext->VSSetConstantBuffers(1, 1, &gameEntities[i]->g_mat->shaderProgram->camConstantBuffer->constantBuffer); //set second constant vertex buffer-camera
		deviceContext->PSSetShader(gameEntities[i]->g_mat->shaderProgram->pixelShader, NULL, 0);
		deviceContext->PSSetConstantBuffers(0, 1, &gameEntities[i]->g_mat->shaderProgram->psConstantBuffer->constantBuffer); //set pixel constant buffer-light

		// Finally do the actual drawing
		deviceContext->DrawIndexed(
			gameEntities.at(i)->g_mesh->m_size,	// The number of indices we're using in this draw
			0,
			0);
	}

	for (unsigned int i = 0; i < projectiles.size(); i++){
		projectiles[i]->scale(XMFLOAT3(0.5f, 0.5f, 0.5f));
		projectiles[i]->setPosition(XMFLOAT3(projectiles[i]->getPosition()._41 * 2, projectiles[i]->getPosition()._42 * 2, 0.0f));
		//UINT offset = 0;
		stride = projectiles[i]->g_mesh->sizeofvertex;
		// Set up the input assembler
		deviceContext->IASetInputLayout(projectiles[i]->g_mat->shaderProgram->vsInputLayout);
		deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//set values that get passed to matrix constant buffer

		projectiles[i]->g_mat->shaderProgram->vsConstantBuffer->dataToSendToConstantBuffer.world = projectiles[i]->getWorld();
		projectiles[i]->g_mat->shaderProgram->vsConstantBuffer->dataToSendToConstantBuffer.view = viewMatrix;
		projectiles[i]->g_mat->shaderProgram->vsConstantBuffer->dataToSendToConstantBuffer.projection = projectionMatrix;

		//set values that get passed to lighting constant buffer
		projectiles[i]->g_mat->shaderProgram->psConstantBuffer->dataToSendToLightBuffer.ambientColor = lighting.ambientColor;
		projectiles[i]->g_mat->shaderProgram->psConstantBuffer->dataToSendToLightBuffer.diffuseColor = lighting.diffuseColor;
		projectiles[i]->g_mat->shaderProgram->psConstantBuffer->dataToSendToLightBuffer.lightDirection = lighting.lightDirection;
		projectiles[i]->g_mat->shaderProgram->psConstantBuffer->dataToSendToLightBuffer.specularColor = lighting.specularColor;
		projectiles[i]->g_mat->shaderProgram->psConstantBuffer->dataToSendToLightBuffer.specularPower = lighting.specularPower;
		//set values that get passed to camera constant buffer
		projectiles[i]->g_mat->shaderProgram->camConstantBuffer->dataToSendToCameraBuffer.cameraPosition = camPos;
		projectiles[i]->g_mat->shaderProgram->camConstantBuffer->dataToSendToCameraBuffer.padding = 1.0f;



		//matrix constant buffer
		deviceContext->UpdateSubresource(
			projectiles[i]->g_mat->shaderProgram->vsConstantBuffer->constantBuffer,
			0,
			NULL,
			&projectiles[i]->g_mat->shaderProgram->vsConstantBuffer->dataToSendToConstantBuffer,
			0,
			0);

		//camera constant buffer 
		deviceContext->UpdateSubresource(
			projectiles[i]->g_mat->shaderProgram->camConstantBuffer->constantBuffer,
			0,
			NULL,
			&projectiles[i]->g_mat->shaderProgram->camConstantBuffer->dataToSendToCameraBuffer,
			0,
			0);
		//light constant buffer
		deviceContext->UpdateSubresource(
			projectiles[i]->g_mat->shaderProgram->psConstantBuffer->constantBuffer,
			0,
			NULL,
			&projectiles[i]->g_mat->shaderProgram->psConstantBuffer->dataToSendToLightBuffer,
			0,
			0);

		deviceContext->IASetVertexBuffers(0, 1, &projectiles[i]->g_mesh->v_buffer, &stride, &offset);
		deviceContext->IASetIndexBuffer(projectiles[i]->g_mesh->i_buffer, DXGI_FORMAT_R32_UINT, 0);

		deviceContext->PSSetSamplers(0, 1, &projectiles[i]->g_mat->samplerState);
		deviceContext->PSSetShaderResources(0, 1, &projectiles[i]->g_mat->resourceView);



		// Set the current vertex and pixel shaders, as well the constant buffer for the vert shader
		deviceContext->VSSetShader(projectiles[i]->g_mat->shaderProgram->vertexShader, NULL, 0);
		deviceContext->VSSetConstantBuffers(0, 1, &projectiles[i]->g_mat->shaderProgram->vsConstantBuffer->constantBuffer); //set first constant vertex buffer-matrix
		deviceContext->VSSetConstantBuffers(1, 1, &projectiles[i]->g_mat->shaderProgram->camConstantBuffer->constantBuffer); //set second constant vertex buffer-camera
		deviceContext->PSSetShader(projectiles[i]->g_mat->shaderProgram->pixelShader, NULL, 0);
		deviceContext->PSSetConstantBuffers(0, 1, &projectiles[i]->g_mat->shaderProgram->psConstantBuffer->constantBuffer); //set pixel constant buffer-light

		// Finally do the actual drawing
		deviceContext->DrawIndexed(
			projectiles.at(i)->g_mesh->m_size,	// The number of indices we're using in this draw
			0,
			0);
		projectiles[i]->setPosition(XMFLOAT3(projectiles[i]->getPosition()._41 / 2, projectiles[i]->getPosition()._42 / 2, 0.0f));
		projectiles[i]->scale(XMFLOAT3(2.0f, 2.0f, 2.0f));

	}
	player->draw(viewMatrix, projectionMatrix, camPos);
}
