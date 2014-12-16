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
	ShaderProgram* geoShader = new ShaderProgram(L"GeometryVertexShader.cso", L"GeometryPixelShader.cso", L"GeometryShader.cso", L"GeometryShaderStreamOutput.cso", device, constantBufferList);
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
	materials.push_back(new Material(device, deviceContext, samplerStates->sampler, L"particle.png", geoShader));

	for (float i = 0; i < 50; i++){
		stars.push_back(new ParticleSystem(XMFLOAT4((i - 50.0f) / 10, -1.5f, 0, 0), XMFLOAT2(0.1f, 0.0f), XMFLOAT2(0.1f, 0.1f), device, deviceContext, materials[6], 20));
	}
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
	HPManager->update(dt);
	collManager->update(dt);

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
			BoundingBox *projectile = new BoundingBox(XMFLOAT3(projectileManager->projectiles[x]->getPosition()._41, projectileManager->projectiles[x]->getPosition()._42, projectileManager->projectiles[x]->getPosition()._43),
				XMFLOAT3(1.0f, 0.5f, 0.0f));
			for (int i = 0; i < 29; i++)
			{
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

			BoundingBox *HPbb = new BoundingBox(XMFLOAT3(HPManager->HPUp[0]->getPosition()._41, HPManager->HPUp[0]->getPosition()._42, HPManager->HPUp[0]->getPosition()._43),
				XMFLOAT3(2.0f, 2.0f, 0.0f));
			if (projectile->Intersects(*HPbb))
			{
				// Erase the projectile and move the asteroid back off the right side of the screen (more efficient to recycle then destroy and re-create)
				projectileManager->projectiles.erase(projectileManager->projectiles.begin() + x);
				HPManager->HPUp[0]->setPosition(XMFLOAT3(150.0f, (rand() % 40) - 30.0f, 0.0f));
				engine->play2D("energy.wav", false);
				hullIntegrity += 30;
				//Make sure health doesn't exceed 100
				if (hullIntegrity >= 100)
				{
					hullIntegrity = 100;
				}
				break;
			}
			for (size_t i = 0; i < 1; i++)
			{
				BoundingBox *collBB = new BoundingBox(XMFLOAT3(collManager->collectables[i]->getPosition()._41, collManager->collectables[i]->getPosition()._42, collManager->collectables[i]->getPosition()._43),
					XMFLOAT3(2.5f, 1.0f, 2.0f));
				if (projectile->Intersects(*collBB))
				{
					// Erase the projectile and move the asteroid back off the right side of the screen (more efficient to recycle then destroy and re-create)
					projectileManager->projectiles.erase(projectileManager->projectiles.begin() + x);
					collManager->collectables[i]->setPosition(XMFLOAT3(30.0f, (rand() % 40) - 19.0f, 0.0f));
					engine->play2D("coin.wav", false);
					shootingScore += 30;

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
void Game::getHealth()
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

void Game::pickUp()
{

	shootingScore += 500;
	engine->play2D("Laser_Shot.mp3", false);

}


// Method where all the actual drawing occurs
void Game::drawGame(XMFLOAT4X4 viewMatrix, XMFLOAT4X4 projectionMatrix, XMFLOAT3 camPos, float time, wchar_t* state)
{
	c_time = time;
	UINT offset = 0;
	UINT stride = sizeof(Vertex);
	float age = time - p_time;
	if (age > 1.2)
	{
		p_time = time;
	}
	projectileManager->draw(viewMatrix, projectionMatrix, camPos);

	for (int i = 0; i < stars.size(); i++){
		stars[i]->drawParticleSystem(viewMatrix, projectionMatrix, age);
	}

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

	std::wstring pi = std::to_wstring(hullIntegrity);
	const WCHAR* szName = pi.c_str();

	score = int(time / 2) + shootingScore;
	scorep = std::to_wstring(score);
	const WCHAR* szScore = scorep.c_str();

	//hs1
	score1 = std::to_wstring(highScore1);
	const WCHAR* szScore1 = score1.c_str();

	//hs2
	score2 = std::to_wstring(highScore2);
	const WCHAR* szScore2 = score2.c_str();

	spriteBatch->Begin();
	if (state == L"Game")
	{
		//Draw Sprites and fonts
		spriteFont->DrawString(spriteBatch.get(), L"Health: ", DirectX::SimpleMath::Vector2(15, 25));
		spriteFont->DrawString(spriteBatch.get(), szName, DirectX::SimpleMath::Vector2(160, 25), Colors::LawnGreen);

		spriteFont->DrawString(spriteBatch.get(), L"Score: ", DirectX::SimpleMath::Vector2(15, 55));
		spriteFont->DrawString(spriteBatch.get(), szScore, DirectX::SimpleMath::Vector2(144, 55), Colors::LawnGreen);

		if (hullIntegrity <= 30)
		{
			spriteFont->DrawString(spriteBatch.get(), szName, DirectX::SimpleMath::Vector2(160, 25), Colors::Red);
		}
	}

		if (state == L"Pause")
		{
			spriteFont->DrawString(spriteBatch.get(), L"Health: ", DirectX::SimpleMath::Vector2(15, 25));
			spriteFont->DrawString(spriteBatch.get(), szName, DirectX::SimpleMath::Vector2(160, 25));

			spriteFont->DrawString(spriteBatch.get(), L"Score: ", DirectX::SimpleMath::Vector2(15, 55));
			spriteFont->DrawString(spriteBatch.get(), szScore, DirectX::SimpleMath::Vector2(144, 55));

			spriteFont->DrawString(spriteBatch.get(), L"Pause", DirectX::SimpleMath::Vector2(800 - 150, 25));

			spriteFont->DrawString(spriteBatch.get(), L"High Scores:", DirectX::SimpleMath::Vector2(300, 25));

			if (highScore1 >= highScore2 && highScore2 >= score)
			{
				spriteFont->DrawString(spriteBatch.get(), szScore1, DirectX::SimpleMath::Vector2(400, 50));
				spriteFont->DrawString(spriteBatch.get(), szScore2, DirectX::SimpleMath::Vector2(400, 75));
				spriteFont->DrawString(spriteBatch.get(), szScore, DirectX::SimpleMath::Vector2(400, 100), Colors::Gold);
			}

			if (highScore2 >= highScore1 && highScore1 >= score)
			{
				spriteFont->DrawString(spriteBatch.get(), szScore2, DirectX::SimpleMath::Vector2(400, 50));
				spriteFont->DrawString(spriteBatch.get(), szScore1, DirectX::SimpleMath::Vector2(400, 75));
				spriteFont->DrawString(spriteBatch.get(), szScore, DirectX::SimpleMath::Vector2(400, 100), Colors::Gold);
			}

			if (highScore1 >= score && score >= highScore2)
			{
				spriteFont->DrawString(spriteBatch.get(), szScore1, DirectX::SimpleMath::Vector2(400, 50));
				spriteFont->DrawString(spriteBatch.get(), szScore, DirectX::SimpleMath::Vector2(400, 75), Colors::Gold);
				spriteFont->DrawString(spriteBatch.get(), szScore2, DirectX::SimpleMath::Vector2(400, 100));
			}

			if (highScore2 >= score && score >= highScore1)
			{
				spriteFont->DrawString(spriteBatch.get(), szScore2, DirectX::SimpleMath::Vector2(400, 50));
				spriteFont->DrawString(spriteBatch.get(), szScore, DirectX::SimpleMath::Vector2(400, 75), Colors::Gold);
				spriteFont->DrawString(spriteBatch.get(), szScore1, DirectX::SimpleMath::Vector2(400, 100));
			}

			if (score >= highScore2 && highScore2 >= highScore1)
			{
				spriteFont->DrawString(spriteBatch.get(), szScore, DirectX::SimpleMath::Vector2(400, 50), Colors::Gold);
				spriteFont->DrawString(spriteBatch.get(), szScore2, DirectX::SimpleMath::Vector2(400, 75));
				spriteFont->DrawString(spriteBatch.get(), szScore1, DirectX::SimpleMath::Vector2(400, 100));
			}

			if (score >= highScore1 && highScore1 >= highScore2)
			{
				spriteFont->DrawString(spriteBatch.get(), szScore, DirectX::SimpleMath::Vector2(400, 50), Colors::Gold);
				spriteFont->DrawString(spriteBatch.get(), szScore1, DirectX::SimpleMath::Vector2(400, 75));
				spriteFont->DrawString(spriteBatch.get(), szScore2, DirectX::SimpleMath::Vector2(400, 100));
			}

		}
	spriteBatch->End();

}

//resets the game after lose condition
void Game::reset()
{
	if (highScore2 > score)
		highScore1 = score;
	if (highScore2 < score)
		highScore2 = score;

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
	for (int i = 0; i < 1; i++)
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

