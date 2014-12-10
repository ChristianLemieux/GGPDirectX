#include "Game.h"

Game::Game(ID3D11Device* dev, ID3D11DeviceContext* devCxt){
	device = dev;
	deviceContext = devCxt;
}

Game::~Game(void){
	ReleaseMacro(device);
	ReleaseMacro(deviceContext);
}

void Game::initGame(SamplerState *samplerStates){

	//sound effect engine
	engine = irrklang::createIrrKlangDevice();

	// Set up lighting parameters
	lighting.ambientColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	lighting.diffuseColor = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	lighting.lightDirection = XMFLOAT3(0.0f, 0.0f, 1.0f);
	lighting.specularColor = XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f);
	lighting.specularPower = 5.0f;
	
	// Set initial hull integrity to full (100%)
	hullIntegrity = 100;

	constantBufferList.push_back(new ConstantBuffer(dataToSendToVSConstantBuffer, device)); //create matrix constant buffer
	constantBufferList.push_back(new ConstantBuffer(dataToSendToLightConstantBuffer, device));//create light constant buffer
	constantBufferList.push_back(new ConstantBuffer(dataToSendToCameraConstantBuffer, device)); //create camera constant buffer
	constantBufferList.push_back(new ConstantBuffer(dataToSendToGSConstantBuffer, device)); //create geometry constant buffer
	

	//create shader program-Params(vertex shader, pixel shader, device, constant buffers)
	shaderProgram = new ShaderProgram(L"NormalVertexShader.cso", L"NormalPixelShader.cso", device, constantBufferList);
	ShaderProgram* geoShader = new ShaderProgram(L"GeometryVertexShader.cso", L"GeometryPixelShader.cso", L"GeometryShader.cso", device, constantBufferList);
	ObjectLoader *asteroidObject = new ObjectLoader(device);
	asteroid = asteroidObject->LoadModel("asteroid.obj");
	ID3D11SamplerState* sample = nullptr;

	// Set up the object loader and load the main menu
	ObjectLoader *bgObject = new ObjectLoader(device);
	Mesh *bg = bgObject->LoadModel("Menu.obj");

	player = new Player(device, deviceContext, constantBufferList, samplerStates->sampler, asteroid);

	//Create the matierials used by the myriad game entities
	materials.push_back(new Material(device, deviceContext, samplerStates->sampler, L"spaceShipTexture.jpg", shaderProgram));
	materials.push_back(new Material(device, deviceContext, samplerStates->sampler, L"asteroid.jpg", shaderProgram));
	materials.push_back(new Material(device, deviceContext, samplerStates->sampler, L"background.jpg", geoShader));
	materials.push_back(new Material(device, deviceContext, samplerStates->sampler, L"bullet.jpg", shaderProgram));
	materials.push_back(new Material(device, deviceContext, samplerStates->sampler, L"asteroid.jpg", geoShader));

	Particle point[] = { XMFLOAT3(0, 0, 0), XMFLOAT3(0, 0, 1), XMFLOAT2(0, 0), XMFLOAT2(1, 0), XMFLOAT2(0,0) };
	testGeo = new GameEntity(new Mesh(point, 0, 1, device), materials[4]);

	//Set up the two backgrounds
	gameEntities.push_back(new GameEntity(bg, materials[2]));
	gameEntities[0]->setPosition(XMFLOAT3(2.5f, 0.0f, 6.0f));
	gameEntities.push_back(new GameEntity(bg, materials[2]));
	gameEntities[1]->setPosition(XMFLOAT3(15.0f, 0.0f, 6.0f));

	// Create the particle and asteroid managers
	projectileManager = new Projectile(device, deviceContext, constantBufferList, samplerStates->sampler, asteroid, player);
	asteroidManager = new Asteroid(device, deviceContext, constantBufferList, samplerStates->sampler, asteroid, player, this);

}

// Main update function for the game
void Game::updateGame(float dt, StateManager *stateManager){

		// Call the different entity manager's update functions
		player->update(dt);
		projectileManager->update(dt);
		asteroidManager->update(dt, stateManager);

		//Parralax 
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

		

		// distance used for determining whether a collision is triggered
		float distance = 2.0f;

		// Run through the list of projectiles and check if any of them are colliding with an asteroid
		if (projectileManager->projectiles.size() > 0)
		{
			for (int x = projectileManager->projectiles.size() - 1; x >= 0; x--)
			{
				for (int i = 0; i < 29; i++)
				{
					float testDistX = pow(projectileManager->projectiles[x]->getPosition()._41 - asteroidManager->asteroids[i]->getPosition()._41, 2);
					float testDistY = pow(projectileManager->projectiles[x]->getPosition()._42 - asteroidManager->asteroids[i]->getPosition()._42, 2);

					if (distance >= testDistX + testDistY)
					{
						// Erase the projectile and move the asteroid back off the right side of the screen (more efficient to recycle then destroy and re-create)
						projectileManager->projectiles.erase(projectileManager->projectiles.begin() + x);
						asteroidManager->asteroids[i]->setPosition(XMFLOAT3(30.0f, (rand() % 40) - 19.0f, 0.0f));
						break;
					}
				}
			}
		}
}

// Handles collisions between the player and an asteroid
void Game::handleCollision(StateManager *stateManager)
{
	// Drop the hull integrity by 10% due to the collision
	hullIntegrity -= 10;

	// start the sound engine with default parameters
	engine->play2D("Explosion.wav", false);
	
	//Trigger a game loss
	if (hullIntegrity <= 0)
	{
		// State 5 is the "game loss" state that triggers the game over screen to show
		stateManager->setState(5);
		reset();
	}
}


// Method where all the actual drawing occurs
void Game::drawGame(XMFLOAT4X4 viewMatrix, XMFLOAT4X4 projectionMatrix, XMFLOAT3 camPos, float time){
	UINT offset = 0;
	UINT stride = sizeof(Vertex);
	for (unsigned int i = 0; i < gameEntities.size(); i++){
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

	stride = testGeo->g_mesh->sizeofvertex;

	projectileManager->draw(viewMatrix, projectionMatrix, camPos);
	player->draw(viewMatrix, projectionMatrix, camPos);

	deviceContext->IASetInputLayout(testGeo->g_mat->shaderProgram->vsInputLayout);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);


	testGeo->g_mat->shaderProgram->ConstantBuffers[0]->dataToSendToConstantBuffer.world = testGeo->getWorld();
	testGeo->g_mat->shaderProgram->ConstantBuffers[0]->dataToSendToConstantBuffer.view = viewMatrix;
	testGeo->g_mat->shaderProgram->ConstantBuffers[0]->dataToSendToConstantBuffer.projection = projectionMatrix;
	testGeo->g_mat->shaderProgram->ConstantBuffers[3]->dataToSendToGSBuffer.age = time;


	//matrix constant buffer
	deviceContext->UpdateSubresource(
		testGeo->g_mat->shaderProgram->ConstantBuffers[0]->constantBuffer,
		0,
		NULL,
		&testGeo->g_mat->shaderProgram->ConstantBuffers[0]->dataToSendToConstantBuffer,
		0,
		0);

	deviceContext->UpdateSubresource(
		testGeo->g_mat->shaderProgram->ConstantBuffers[3]->constantBuffer,
		0,
		NULL,
		&testGeo->g_mat->shaderProgram->ConstantBuffers[3]->dataToSendToGSBuffer,
		0,
		0);

	deviceContext->IASetVertexBuffers(0, 1, &testGeo->g_mesh->v_buffer, &stride, &offset);
	deviceContext->IASetIndexBuffer(testGeo->g_mesh->i_buffer, DXGI_FORMAT_R32_UINT, 0);

	deviceContext->PSSetSamplers(0, 1, &testGeo->g_mat->samplerState);
	deviceContext->PSSetShaderResources(0, 1, &testGeo->g_mat->resourceView);

	deviceContext->VSSetShader(testGeo->g_mat->shaderProgram->vertexShader, NULL, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &testGeo->g_mat->shaderProgram->ConstantBuffers[0]->constantBuffer); //set first constant vertex buffer-matrix
	deviceContext->VSSetConstantBuffers(1, 1, &testGeo->g_mat->shaderProgram->ConstantBuffers[3]->constantBuffer);
	deviceContext->PSSetShader(testGeo->g_mat->shaderProgram->pixelShader, NULL, 0);
	deviceContext->GSSetShader(testGeo->g_mat->shaderProgram->geometryShader, NULL, 0);

	deviceContext->DrawIndexed(
		testGeo->g_mesh->m_size,	// The number of indices we're using in this draw
		0,
		0);

	deviceContext->GSSetShader(NULL, NULL, 0);
	// Call the corresponding draw methods for the different entity managers
	projectileManager->draw(viewMatrix, projectionMatrix, camPos);
	player->draw(viewMatrix, projectionMatrix, camPos);
	asteroidManager->draw(viewMatrix, projectionMatrix, camPos);
}

//resets the game after lose condition
void Game::reset()
{
	// reset hull integrity to full
	hullIntegrity = 100;
	
	//reset player 
	player->reset();

	//reset asteroids to a random area off the right side of the screen
	for (int i = 0; i < 29; i++)
	{
		asteroidManager->asteroids[i]->setPosition(XMFLOAT3(((rand() % 60) + 30), ((rand() % 40) - 19.0f), 0.0f));
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

