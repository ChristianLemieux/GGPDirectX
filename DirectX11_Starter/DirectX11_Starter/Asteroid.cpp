#include "Asteroid.h"
#include "Game.h"

//Constructor for Asteroid object
Asteroid::Asteroid(ID3D11Device* dev, ID3D11DeviceContext* devCtx, vector<ConstantBuffer*> constantBufferList, ID3D11SamplerState* samplerState, Mesh* meshReference, Player* playerReference, Game* gameReferencePassed){

	//set up the lighting parameters
	lighting.ambientColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	lighting.diffuseColor = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	lighting.lightDirection = XMFLOAT3(0.0f, 0.0f, 1.0f);
	lighting.specularColor = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	lighting.specularPower = 2.0f;

	device = dev;
	deviceContext = devCtx;
	sampler = samplerState;
	shaderProgram = new ShaderProgram(L"NewNormalVertexShader.cso", L"NewNormalPixelShader.cso", device, constantBufferList);
	asteroidMaterial = new Material(device, deviceContext, sampler, L"asteroid.jpg", L"asteroid_norm.jpg", shaderProgram);
	player = playerReference;
	mesh = meshReference;

	gameReference = gameReferencePassed;

	// populate the list of asteroids, scale them to one tenth the size of the mesh and randomize their position (off the right side of the screen)
	for (int i = 0; i < 29; i++)
	{
		asteroids.push_back(new GameEntity(meshReference, asteroidMaterial));
		asteroids[i]->scale(XMFLOAT3(0.1f, 0.1f, 0.1f));
		asteroids[i]->setPosition(XMFLOAT3(((rand() % 60) + 30), ((rand() % 40) - 19.0f), 0.0f));
	}
}

Asteroid::~Asteroid(){
	ReleaseMacro(device);
	ReleaseMacro(deviceContext);
	if (sampler){
		delete sampler;
		sampler = nullptr;
	}
	if (asteroidMaterial){
		delete asteroidMaterial;
		asteroidMaterial = nullptr;
	}
	if (shaderProgram){
		delete shaderProgram;
		shaderProgram = nullptr;
	}
}


//Update asteroid positions each frame
void Asteroid::update(float dt, StateManager *stateManager){

	//make bounding boxes
	BoundingBox *playerbb = new BoundingBox(XMFLOAT3(player->player->getPosition()._41, player->player->getPosition()._42, player->player->getPosition()._43),
		XMFLOAT3(2.8f, 1.0f, 0.0f));

	// double boolean system used to ensure the same collision isn't registered multiple times.
	collision = L"Not Colliding";
	notColliding = false;

	//moves asteroids across screen (right to left) and respawns them when they leave the screen
	for (unsigned int i = 0; i < 29; i++)
	{
		BoundingBox *asteriodbb = new BoundingBox(XMFLOAT3(asteroids[i]->getPosition()._41, asteroids[i]->getPosition()._42, asteroids[i]->getPosition()._43),
			XMFLOAT3(2.8f, 1.0f, 2.0f));
		asteroids[i]->translate(XMFLOAT3(-8.0f * dt, 0.0f, 0.0f));
		asteroids[i]->rotate(XMFLOAT3(0.0f, 0.0f, 0.0f));

		//._41 is the x value for the position matrix of game entities
		if (asteroids[i]->getPosition()._41 < -30)
		{
			asteroids[i]->setPosition(XMFLOAT3(30.0f, (rand() % 40) - 19.0f, 0.0f));

			//helps elimate overlap
			for (int g = 0; g < 29; g++)
			{
				BoundingBox *asteriodbb2 = new BoundingBox(XMFLOAT3(asteroids[g]->getPosition()._41, asteroids[g]->getPosition()._42, asteroids[g]->getPosition()._43),
					XMFLOAT3(4.0f, 4.0f, 0.0f));
				if (asteriodbb2->Intersects(*asteriodbb))
				{
					asteroids[i]->setPosition(XMFLOAT3(30.0f, (rand() % 40) - 19.0f, 0.0f));
				}
			}
		}
	}

	// Tests for collisions between the asteroids and the player, tells the game to handle the collision (for non-asteroid consequences) if one is found
	for (int i = 0; i < 29; i++)
	{
		BoundingBox *asteriodbb = new BoundingBox(XMFLOAT3(asteroids[i]->getPosition()._41, asteroids[i]->getPosition()._42, asteroids[i]->getPosition()._43),
			XMFLOAT3(2.6f, 1.0f, 0.0f));
		//check for intersections
		if (asteriodbb->Intersects(*playerbb))
		{
			notColliding = true;
			if (canTakeDamage && notColliding)
			{
				asteroids[i]->setPosition(XMFLOAT3(30.0f, (rand() % 40) - 19.0f, 0.0f));
				canTakeDamage = false;
				gameReference->handleCollision(stateManager);

				//elimate spawning on each other
				for (int g = 0; g < 29; g++)
				{
					BoundingBox *asteriodbb2 = new BoundingBox(XMFLOAT3(asteroids[g]->getPosition()._41, asteroids[g]->getPosition()._42, asteroids[g]->getPosition()._43),
						XMFLOAT3(4.0f, 4.0f, 0.0f));
					if (asteriodbb2->Intersects(*asteriodbb))
					{
						asteroids[i]->setPosition(XMFLOAT3(30.0f, (rand() % 40) - 19.0f, 0.0f));
					}
				}

			}

			collision = L"Colliding";
			break;
		}
	}

	if (!notColliding){
		canTakeDamage = true;
	}
}



//draw asteroids
void Asteroid::draw(XMFLOAT4X4 viewMatrix, XMFLOAT4X4 projectionMatrix, XMFLOAT3 camPos){
	UINT offset = 0;
	UINT stride = sizeof(Vertex);

	for (unsigned int i = 0; i < asteroids.size(); i++){
		//UINT offset = 0;
		stride = asteroids[i]->g_mesh->sizeofvertex;
		// Set up the input assembler
		deviceContext->IASetInputLayout(asteroids[i]->g_mat->shaderProgram->vsInputLayout);
		deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//set values that get passed to matrix constant buffer

		asteroids[i]->g_mat->shaderProgram->ConstantBuffers[0]->dataToSendToConstantBuffer.world = asteroids[i]->getWorld();
		asteroids[i]->g_mat->shaderProgram->ConstantBuffers[0]->dataToSendToConstantBuffer.view = viewMatrix;
		asteroids[i]->g_mat->shaderProgram->ConstantBuffers[0]->dataToSendToConstantBuffer.projection = projectionMatrix;

		//set values that get passed to lighting constant buffer
		asteroids[i]->g_mat->shaderProgram->ConstantBuffers[1]->dataToSendToLightBuffer.ambientColor = lighting.ambientColor;
		asteroids[i]->g_mat->shaderProgram->ConstantBuffers[1]->dataToSendToLightBuffer.diffuseColor = lighting.diffuseColor;
		asteroids[i]->g_mat->shaderProgram->ConstantBuffers[1]->dataToSendToLightBuffer.lightDirection = lighting.lightDirection;
		asteroids[i]->g_mat->shaderProgram->ConstantBuffers[1]->dataToSendToLightBuffer.specularColor = lighting.specularColor;
		asteroids[i]->g_mat->shaderProgram->ConstantBuffers[1]->dataToSendToLightBuffer.specularPower = lighting.specularPower;
		//set values that get passed to camera constant buffer
		asteroids[i]->g_mat->shaderProgram->ConstantBuffers[2]->dataToSendToCameraBuffer.cameraPosition = camPos;
		asteroids[i]->g_mat->shaderProgram->ConstantBuffers[2]->dataToSendToCameraBuffer.padding = 1.0f;



		//matrix constant buffer
		deviceContext->UpdateSubresource(
			asteroids[i]->g_mat->shaderProgram->ConstantBuffers[0]->constantBuffer,
			0,
			NULL,
			&asteroids[i]->g_mat->shaderProgram->ConstantBuffers[0]->dataToSendToConstantBuffer,
			0,
			0);

		//camera constant buffer 
		deviceContext->UpdateSubresource(
			asteroids[i]->g_mat->shaderProgram->ConstantBuffers[2]->constantBuffer,
			0,
			NULL,
			&asteroids[i]->g_mat->shaderProgram->ConstantBuffers[2]->dataToSendToCameraBuffer,
			0,
			0);
		//light constant buffer
		deviceContext->UpdateSubresource(
			asteroids[i]->g_mat->shaderProgram->ConstantBuffers[1]->constantBuffer,
			0,
			NULL,
			&asteroids[i]->g_mat->shaderProgram->ConstantBuffers[1]->dataToSendToLightBuffer,
			0,
			0);

		deviceContext->IASetVertexBuffers(0, 1, &asteroids[i]->g_mesh->v_buffer, &stride, &offset);
		deviceContext->IASetIndexBuffer(asteroids[i]->g_mesh->i_buffer, DXGI_FORMAT_R32_UINT, 0);

		deviceContext->PSSetSamplers(0, 1, &asteroids[i]->g_mat->samplerState);
		deviceContext->PSSetShaderResources(0, 1, &asteroids[i]->g_mat->resourceView);
		deviceContext->PSSetShaderResources(1, 1, &asteroids[1]->g_mat->resourceView2);



		// Set the current vertex and pixel shaders, as well the constant buffer for the vert shader
		deviceContext->VSSetShader(asteroids[i]->g_mat->shaderProgram->vertexShader, NULL, 0);
		deviceContext->VSSetConstantBuffers(0, 1, &asteroids[i]->g_mat->shaderProgram->ConstantBuffers[0]->constantBuffer); //set first constant vertex buffer-matrix
		deviceContext->VSSetConstantBuffers(1, 1, &asteroids[i]->g_mat->shaderProgram->ConstantBuffers[2]->constantBuffer); //set second constant vertex buffer-camera
		deviceContext->PSSetShader(asteroids[i]->g_mat->shaderProgram->pixelShader, NULL, 0);
		deviceContext->PSSetConstantBuffers(0, 1, &asteroids[i]->g_mat->shaderProgram->ConstantBuffers[1]->constantBuffer); //set pixel constant buffer-light

		// Finally do the actual drawing
		deviceContext->DrawIndexed(
			asteroids.at(i)->g_mesh->m_size,	// The number of indices we're using in this draw
			0,
			0);
	}
}
