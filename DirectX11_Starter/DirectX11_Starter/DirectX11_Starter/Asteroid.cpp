#include "Asteroid.h"
#include "Game.h"

//Constructor for Asteroid object
Asteroid::Asteroid(ID3D11Device* dev, ID3D11DeviceContext* devCtx, vector<ConstantBuffer*> constantBufferList, ID3D11SamplerState* samplerState, Mesh* meshReference, Player* playerReference, Game* gameReferencePassed){

	// set up the lighting parameters
	lighting.ambientColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	lighting.diffuseColor = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	lighting.lightDirection = XMFLOAT3(0.0f, 0.0f, 1.0f);
	lighting.specularColor = XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f);
	lighting.specularPower = 5.0f;

	device = dev;
	deviceContext = devCtx;
	sampler = samplerState;
	shaderProgram = new ShaderProgram(L"NormalVertexShader.cso", L"NormalPixelShader.cso", device, constantBufferList);
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

bool Asteroid::BoundingBoxCollision(XMVECTOR& firstObjBoundingBoxMinVertex, XMVECTOR& firstObjBoundingBoxMaxVertex,
	XMVECTOR& secondObjBoundingBoxMinVertex, XMVECTOR& secondObjBoundingBoxMaxVertex)
{
	for (unsigned int i = 0; i < 29; i++)
	{
		//change the info in memory and save it
		float numberMin = asteroids[i]->getPosition()._41 - 6.0f;
		float* numMin = &numberMin;

		float numberMax = asteroids[i]->getPosition()._41 + 6.0f;
		float* numMax = &numberMax;

		//store it into a vector
		XMStoreFloat(numMin, firstObjBoundingBoxMinVertex);
		XMStoreFloat(numMax, firstObjBoundingBoxMaxVertex);

		//Is obj1's max X greater than obj2's min X? If not, obj1 is to the LEFT of obj2
		if (XMVectorGetX(firstObjBoundingBoxMaxVertex) > XMVectorGetX(secondObjBoundingBoxMinVertex))

			//Is obj1's min X less than obj2's max X? If not, obj1 is to the RIGHT of obj2
			if (XMVectorGetX(firstObjBoundingBoxMinVertex) < XMVectorGetX(secondObjBoundingBoxMaxVertex))

				//Is obj1's max Y greater than obj2's min Y? If not, obj1 is UNDER obj2
				if (XMVectorGetY(firstObjBoundingBoxMaxVertex) > XMVectorGetY(secondObjBoundingBoxMinVertex))

					//Is obj1's min Y less than obj2's max Y? If not, obj1 is ABOVE obj2
					if (XMVectorGetY(firstObjBoundingBoxMinVertex) < XMVectorGetY(secondObjBoundingBoxMaxVertex))

						//Is obj1's max Z greater than obj2's min Z? If not, obj1 is IN FRONT OF obj2
						if (XMVectorGetZ(firstObjBoundingBoxMaxVertex) > XMVectorGetZ(secondObjBoundingBoxMinVertex))

							//Is obj1's min Z less than obj2's max Z? If not, obj1 is BEHIND obj2
							if (XMVectorGetZ(firstObjBoundingBoxMinVertex) < XMVectorGetZ(secondObjBoundingBoxMaxVertex))

								//If we've made it this far, then the two bounding boxes are colliding
								collided = true;
		return collided;

		//If the two bounding boxes are not colliding, then return false


		collided = false;
		return collided;
	}
}


//Update asteroid positions each frame
void Asteroid::update(float dt, StateManager *stateManager){

	// double boolean system used to ensure the same collision isn't registered multiple times.
	collision = L"Not Colliding";
	notColliding = false;

	//moves asteroids across screen (right to left) and respawns them when they leave the screen
	for (unsigned int i = 0; i < 29; i++)
	{
		asteroids[i]->translate(XMFLOAT3(-8.0f * dt, 0.0f, 0.0f));

		//._41 is the x value for the position matrix of game entities
		if (asteroids[i]->getPosition()._41 < -30)
		{
			asteroids[i]->setPosition(XMFLOAT3(30.0f, (rand() % 40) - 19.0f, 0.0f));
		}
	}

	// Tests for collisions between the asteroids and the player, tells the game to handle the collision (for non-asteroid consequences) if one is found
	float distance = 12.0f;
	float playerX = player->player->getPosition()._41;
	float playerY = player->player->getPosition()._42;
	for (int i = 0; i < 29; i++)
	{
		float testDistX = pow(playerX - asteroids[i]->getPosition()._41, 2);
		float testDistY = pow(playerY - asteroids[i]->getPosition()._42, 2);

		if (distance >= testDistX + testDistY)
		{
			notColliding = true;
			if (canTakeDamage && notColliding)
			{
				asteroids[i]->setPosition(XMFLOAT3(30.0f, (rand() % 40) - 19.0f, 0.0f));
				canTakeDamage = false;
				gameReference->handleCollision(stateManager);
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
