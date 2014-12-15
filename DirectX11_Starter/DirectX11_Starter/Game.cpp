#include "Game.h"
#include "WICTextureLoader.h"
#include "SpriteBatch.h"
#include "SpriteFont.h"
#include "SimpleMath.h"

std::unique_ptr<DirectX::SpriteBatch> spriteBatch;
std::unique_ptr<DirectX::SpriteFont> spriteFont;

Game::Game(ID3D11Device* dev, ID3D11DeviceContext* devCxt){
	device = dev;
	deviceContext = devCxt;
}

Game::~Game(void){
	ReleaseMacro(device);
	ReleaseMacro(deviceContext);
}

void Game::initGame(SamplerState *samplerStates){
	p_time = 0;
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
	shootingScore = 0;

	constantBufferList.push_back(new ConstantBuffer(dataToSendToVSConstantBuffer, device)); //create matrix constant buffer
	constantBufferList.push_back(new ConstantBuffer(dataToSendToLightConstantBuffer, device));//create light constant buffer
	constantBufferList.push_back(new ConstantBuffer(dataToSendToCameraConstantBuffer, device)); //create camera constant buffer
	constantBufferList.push_back(new ConstantBuffer(dataToSendToGSConstantBuffer, device)); //create geometry constant buffer


	//create shader program-Params(vertex shader, pixel shader, device, constant buffers)
	shaderProgram = new ShaderProgram(L"FlatVertexShader.cso", L"FlatPixelShader.cso", device, constantBufferList);
	ShaderProgram* geoShader = new ShaderProgram(L"GeometryVertexShader.cso", L"GeometryPixelShader.cso", L"GeometryShader.cso", device, constantBufferList);
	ObjectLoader *asteroidObject = new ObjectLoader(device);
	ObjectLoader *collObject = new ObjectLoader(device);
	ObjectLoader *HPObject = new ObjectLoader(device);
	ObjectLoader *playerObject = new ObjectLoader(device);
	ObjectLoader *bulletObject = new ObjectLoader(device);
	asteroid = asteroidObject->LoadModel("asteroid.obj");
	playerm = playerObject->LoadModel("ship.obj");
	bulletm = bulletObject->LoadModel("bullet.obj");
	HPm = HPObject->LoadModel("energy.obj");
	Collm = collObject->LoadModel("star.obj");
	ID3D11SamplerState* sample = nullptr;

	// Set up the object loader and load the main menu
	ObjectLoader *bgObject = new ObjectLoader(device);
	Mesh *bg = bgObject->LoadModel("Menu.obj");



	//Create the matierials used by the myriad game entities
	materials.push_back(new Material(device, deviceContext, samplerStates->sampler, L"spaceShipTexture.jpg", shaderProgram));
	materials.push_back(new Material(device, deviceContext, samplerStates->sampler, L"asteroid.jpg", shaderProgram));
	materials.push_back(new Material(device, deviceContext, samplerStates->sampler, L"star.png", shaderProgram));
	materials.push_back(new Material(device, deviceContext, samplerStates->sampler, L"energy.png", shaderProgram));
	materials.push_back(new Material(device, deviceContext, samplerStates->sampler, L"background.jpg", shaderProgram));
	materials.push_back(new Material(device, deviceContext, samplerStates->sampler, L"bullet.jpg", shaderProgram));
	materials.push_back(new Material(device, deviceContext, samplerStates->sampler, L"goldstar.png", geoShader));

	stars = new ParticleSystem(XMFLOAT3(0, 0, 0), XMFLOAT2(-0.01f, 0.0f), XMFLOAT2(-0.001f, 0.001f), device, deviceContext, materials[4], 20);
	player = new Player(device, deviceContext, constantBufferList, samplerStates->sampler, playerm);

	//Set up the two backgrounds
	gameEntities.push_back(new GameEntity(bg, materials[2]));
	gameEntities[0]->setPosition(XMFLOAT3(2.5f, 0.0f, 6.0f));
	gameEntities.push_back(new GameEntity(bg, materials[2]));
	gameEntities[1]->setPosition(XMFLOAT3(15.0f, 0.0f, 6.0f));

	// Create the managers
	projectileManager = new Projectile(device, deviceContext, constantBufferList, samplerStates->sampler, bulletm, player);
	asteroidManager = new Asteroid(device, deviceContext, constantBufferList, samplerStates->sampler, asteroid, player, this);
	HPManager = new healthPickup(device, deviceContext, constantBufferList, samplerStates->sampler, HPm, player, this);
	collManager = new Collectable(device, deviceContext, constantBufferList, samplerStates->sampler, Collm, player, this);

	spriteBatch.reset(new DirectX::SpriteBatch(deviceContext));
	spriteFont.reset(new DirectX::SpriteFont(device, L"Font.spritesheet"));

}

// Main update function for the game
void Game::updateGame(float dt, StateManager *stateManager)
{
	// Call the different entity manager's update functions
	player->update(dt);
	projectileManager->update(dt);
	asteroidManager->update(dt, stateManager);
	HPManager->update(dt, stateManager);
	collManager->update(dt, stateManager);

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


	// Run through the list of projectiles and check if any of them are colliding with an asteroid
	if (projectileManager->projectiles.size() > 0)
	{
		for (int x = projectileManager->projectiles.size() - 1; x >= 0; x--)
		{
			for (int i = 0; i < 29; i++)
			{
				BoundingBox *projectile = new BoundingBox(XMFLOAT3(projectileManager->projectiles[x]->getPosition()._41, projectileManager->projectiles[x]->getPosition()._42, projectileManager->projectiles[x]->getPosition()._43),
					XMFLOAT3(1.0f, 0.5f, 0.0f));
				BoundingBox *asteriodbb = new BoundingBox(XMFLOAT3(asteroidManager->asteroids[i]->getPosition()._41, asteroidManager->asteroids[i]->getPosition()._42, asteroidManager->asteroids[i]->getPosition()._43),
					XMFLOAT3(2.5f, 1.0f, 2.0f));

				if (projectile->Intersects(*asteriodbb))
				{
					// Erase the projectile and move the asteroid back off the right side of the screen (more efficient to recycle then destroy and re-create)
					projectileManager->projectiles.erase(projectileManager->projectiles.begin() + x);
					asteroidManager->asteroids[i]->setPosition(XMFLOAT3(30.0f, (rand() % 40) - 19.0f, 0.0f));
					engine->play2D("Crumble.wav", false);
					shootingScore += 100;
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

// Handles collisions between the player and an HPUp
void Game::getHealth(StateManager *stateManager)
{
	// Raise the hull intergerty by 30 due to pick up
	hullIntegrity += 30;

	// start the sound engine with default parameters
	engine->play2D("energy.wav", false);

	//MAke sure health doesn't exceed 100
	if (hullIntegrity >= 100)
	{
		hullIntegrity = 100;
	}
}

void Game::pickUp(StateManager *stateManager)
{

	shootingScore += 50;

}


// Method where all the actual drawing occurs
void Game::drawGame(XMFLOAT4X4 viewMatrix, XMFLOAT4X4 projectionMatrix, XMFLOAT3 camPos, float time, wchar_t* state)
{
	c_time = time;
	UINT offset = 0;
	UINT stride = sizeof(Vertex);
	/*for (unsigned int i = 0; i < gameEntities.size(); i++)
	{
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
	}*/
	float age = time - p_time;
	if (age > 1.2)
	{
		p_time = time;
	}
	projectileManager->draw(viewMatrix, projectionMatrix, camPos);
	//player->draw(viewMatrix, projectionMatrix, camPos);

	stars->drawParticleSystem(viewMatrix, projectionMatrix, age);

	// Call the corresponding draw methods for the different entity managers
	projectileManager->draw(viewMatrix, projectionMatrix, camPos);
	player->draw(viewMatrix, projectionMatrix, camPos);
	asteroidManager->draw(viewMatrix, projectionMatrix, camPos);
	collManager->draw(viewMatrix, projectionMatrix, camPos);
	HPManager->draw(viewMatrix, projectionMatrix, camPos);
	DrawUI(time, state);
}

void Game::DrawUI(float time, wchar_t* state)
{
	spriteBatch->Begin();
	if (state == L"Game" || state == L"Pause" || state == L"Win")
	{

		std::wstring pi = std::to_wstring(hullIntegrity);
		const WCHAR* szName = pi.c_str();

		std::wstring score = std::to_wstring(int(time / 2) + shootingScore);
		const WCHAR* szScore = score.c_str();

		//Draw Sprites and fonts
		spriteFont->DrawString(spriteBatch.get(), L"Health: ", DirectX::SimpleMath::Vector2(15, 25));
		spriteFont->DrawString(spriteBatch.get(), szName, DirectX::SimpleMath::Vector2(160, 25), Colors::LawnGreen);

		spriteFont->DrawString(spriteBatch.get(), L"Score: ", DirectX::SimpleMath::Vector2(15, 55));
		spriteFont->DrawString(spriteBatch.get(), szScore, DirectX::SimpleMath::Vector2(144, 55), Colors::LawnGreen);

		if (hullIntegrity <= 30)
		{
			spriteFont->DrawString(spriteBatch.get(), szName, DirectX::SimpleMath::Vector2(160, 25), Colors::Red);
		}

		if (state == L"Pause")
		{
			spriteFont->DrawString(spriteBatch.get(), L"Pause", DirectX::SimpleMath::Vector2(800 - 225, 25));

		}
	}
	spriteBatch->End();

}

//resets the game after lose condition
void Game::reset()
{
	// reset hull integrity to full
	hullIntegrity = 100;
	shootingScore = 0;

	//reset player 
	player->reset();

	//reset asteroids to a random area off the right side of the screen
	for (int i = 0; i < 29; i++)
	{
		asteroidManager->asteroids[i]->setPosition(XMFLOAT3(((rand() % 60) + 30), ((rand() % 40) - 19.0f), 0.0f));
	}

	//reset HP to a random area off the right side of the screen
	for (int i = 0; i < 1; i++)
	{
		HPManager->HPUp[i]->setPosition(XMFLOAT3(((rand() % 60) + 30), ((rand() % 40) - 19.0f), 0.0f));
	}

	//reset HP to a random area off the right side of the screen
	for (int i = 0; i < 9; i++)
	{
		collManager->collectables[i]->setPosition(XMFLOAT3(((rand() % 60) + 30), ((rand() % 40) - 19.0f), 0.0f));
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
