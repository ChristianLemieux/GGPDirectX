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
	shaderProgram = new ShaderProgram(L"NormalVertexShader.cso", L"NormalPixelShader.cso", device, constantBufferList);
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

	projectileManager = new Projectile(device, deviceContext, constantBufferList, samplerStates->sampler, asteroid, player);
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
	notColliding = false;
	
		player->update(dt);
		projectileManager->update(dt);

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
						reset();
						
					}
					canTakeDamage = false;
				}

				collision = L"Colliding";
				break;
			}
		}

		if (projectileManager->projectiles.size() > 0)
		{
			for (int x = projectileManager->projectiles.size() - 1; x >= 0; x--)
			{
				for (int i = 3; i < 31; i++)
				{
					float testDistX = pow(projectileManager->projectiles[x]->getPosition()._41 - gameEntities[i]->getPosition()._41, 2);
					float testDistY = pow(projectileManager->projectiles[x]->getPosition()._42 - gameEntities[i]->getPosition()._42, 2);

					if (distance >= testDistX + testDistY)
					{
						projectileManager->projectiles.erase(projectileManager->projectiles.begin() + x);
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
		
		gameEntities[i]->g_mat->shaderProgram->ConstantBuffers[0]->dataToSendToConstantBuffer.world = gameEntities[i]->getWorld();
		gameEntities[i]->g_mat->shaderProgram->ConstantBuffers[0]->dataToSendToConstantBuffer.view = viewMatrix;
		gameEntities[i]->g_mat->shaderProgram->ConstantBuffers[0]->dataToSendToConstantBuffer.projection = projectionMatrix;
		
		//set values that get passed to lighting constant buffer
		gameEntities[i]->g_mat->shaderProgram->ConstantBuffers[1]->dataToSendToLightBuffer.ambientColor = lighting.ambientColor;
		gameEntities[i]->g_mat->shaderProgram->ConstantBuffers[1]->dataToSendToLightBuffer.diffuseColor = lighting.diffuseColor;
		gameEntities[i]->g_mat->shaderProgram->ConstantBuffers[1]->dataToSendToLightBuffer.lightDirection = lighting.lightDirection;
		gameEntities[i]->g_mat->shaderProgram->ConstantBuffers[1]->dataToSendToLightBuffer.specularColor = lighting.specularColor;
		gameEntities[i]->g_mat->shaderProgram->ConstantBuffers[1]->dataToSendToLightBuffer.specularPower = lighting.specularPower;
		//set values that get passed to camera constant buffer
		gameEntities[i]->g_mat->shaderProgram->ConstantBuffers[2]->dataToSendToCameraBuffer.cameraPosition = camPos;
		gameEntities[i]->g_mat->shaderProgram->ConstantBuffers[2]->dataToSendToCameraBuffer.padding = 1.0f;

		

		//matrix constant buffer
		deviceContext->UpdateSubresource(
			gameEntities[i]->g_mat->shaderProgram->ConstantBuffers[0]->constantBuffer,
			0,
			NULL,
			&gameEntities[i]->g_mat->shaderProgram->ConstantBuffers[0]->dataToSendToConstantBuffer,
			0,
			0);

		//camera constant buffer 
		deviceContext->UpdateSubresource(
			gameEntities[i]->g_mat->shaderProgram->ConstantBuffers[2]->constantBuffer,
			0,
			NULL,
			&gameEntities[i]->g_mat->shaderProgram->ConstantBuffers[2]->dataToSendToCameraBuffer,
			0,
			0);
		//light constant buffer
		deviceContext->UpdateSubresource(
			gameEntities[i]->g_mat->shaderProgram->ConstantBuffers[1]->constantBuffer,
			0,
			NULL,
			&gameEntities[i]->g_mat->shaderProgram->ConstantBuffers[1]->dataToSendToLightBuffer,
			0,
			0);

		deviceContext->IASetVertexBuffers(0, 1, &gameEntities[i]->g_mesh->v_buffer, &stride, &offset);
		deviceContext->IASetIndexBuffer(gameEntities[i]->g_mesh->i_buffer, DXGI_FORMAT_R32_UINT, 0);

		deviceContext->PSSetSamplers(0, 1, &gameEntities[i]->g_mat->samplerState);
		deviceContext->PSSetShaderResources(0, 1, &gameEntities[i]->g_mat->resourceView);



		// Set the current vertex and pixel shaders, as well the constant buffer for the vert shader
		deviceContext->VSSetShader(gameEntities[i]->g_mat->shaderProgram->vertexShader, NULL, 0);
		deviceContext->VSSetConstantBuffers(0, 1, &gameEntities[i]->g_mat->shaderProgram->ConstantBuffers[0]->constantBuffer); //set first constant vertex buffer-matrix
		deviceContext->VSSetConstantBuffers(1, 1, &gameEntities[i]->g_mat->shaderProgram->ConstantBuffers[2]->constantBuffer); //set second constant vertex buffer-camera
		deviceContext->PSSetShader(gameEntities[i]->g_mat->shaderProgram->pixelShader, NULL, 0);
		deviceContext->PSSetConstantBuffers(0, 1, &gameEntities[i]->g_mat->shaderProgram->ConstantBuffers[1]->constantBuffer); //set pixel constant buffer-light

		// Finally do the actual drawing
		deviceContext->DrawIndexed(
			gameEntities.at(i)->g_mesh->m_size,	// The number of indices we're using in this draw
			0,
			0);
	}

	projectileManager->draw(viewMatrix, projectionMatrix, camPos);
	player->draw(viewMatrix, projectionMatrix, camPos);
}

//resets after lose condition
void Game::reset()
{
	hullIntegrity = 100;
	
	//reset player 
	player->reset();

	//reset asteroids
	for (int i = 2; i < 31; i++)
	{
		gameEntities[i]->setPosition(XMFLOAT3(((rand() % 60) + 30), ((rand() % 40) - 19.0f), 0.0f));
	}

	//reset projectiles
	if (projectileManager->projectiles.size() > 0)
	{
		for (int x = projectileManager->projectiles.size() - 1; x >= 0; x--)
		{
			projectileManager->projectiles.erase(projectileManager->projectiles.begin() + x);
		}
	}

}

//method that will handle all post processing
void Game::drawPostProcessing()
{

}
