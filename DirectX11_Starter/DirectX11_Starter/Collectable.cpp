#include "Collectable.h"
#include "Game.h"

//Constructor for Collectable object
Collectable::Collectable(ID3D11Device* dev, ID3D11DeviceContext* devCtx, vector<ConstantBuffer*> constantBufferList, ID3D11SamplerState* samplerState, Mesh* meshReference, Player* playerReference, Game* gameReferencePassed){

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
	collectableMaterial = new Material(device, deviceContext, sampler, L"star.png", shaderProgram);
	player = playerReference;
	mesh = meshReference;

	gameReference = gameReferencePassed;

	// populate the list of collectables, scale them to one tenth the size of the mesh and randomize their position (off the right side of the screen)
	for (int i = 0; i < 9; i++)
	{
		collectables.push_back(new GameEntity(meshReference, collectableMaterial));
		collectables[i]->scale(XMFLOAT3(0.1f, 0.1f, 0.1f));
		collectables[i]->setPosition(XMFLOAT3(((rand() % 60) + 30), ((rand() % 40) - 19.0f), 0.0f));
	}
}

Collectable::~Collectable(){
	ReleaseMacro(device);
	ReleaseMacro(deviceContext);
	if (sampler){
		delete sampler;
		sampler = nullptr;
	}
	if (collectableMaterial){
		delete collectableMaterial;
		collectableMaterial = nullptr;
	}
	if (shaderProgram){
		delete shaderProgram;
		shaderProgram = nullptr;
	}
}


//Update asteroid positions each frame
void Collectable::update(float dt, StateManager *stateManager){

	//make bounding boxes
	BoundingBox *playerbb = new BoundingBox(XMFLOAT3(player->player->getPosition()._41, player->player->getPosition()._42, player->player->getPosition()._43),
		XMFLOAT3(4.0f, 4.0f, 0.0f));

	//moves asteroids across screen (right to left) and respawns them when they leave the screen
	for (unsigned int i = 0; i < 9; i++)
	{
		BoundingBox *asteriodbb = new BoundingBox(XMFLOAT3(collectables[i]->getPosition()._41, collectables[i]->getPosition()._42, collectables[i]->getPosition()._43),
			XMFLOAT3(4.0f, 4.0f, 2.0f));
		collectables[i]->translate(XMFLOAT3(-8.0f * dt, 0.0f, 0.0f));
		collectables[i]->rotate(XMFLOAT3(0.0f, 0.0f, 0.0f));

		//._41 is the x value for the position matrix of game entities
		if (collectables[i]->getPosition()._41 < -30)
		{
			collectables[i]->setPosition(XMFLOAT3(30.0f, (rand() % 40) - 19.0f, 0.0f));

			//helps elimate overlap
			for (int g = 0; g < 9; g++)
			{
				BoundingBox *asteriodbb2 = new BoundingBox(XMFLOAT3(collectables[g]->getPosition()._41, collectables[g]->getPosition()._42, collectables[g]->getPosition()._43),
					XMFLOAT3(4.0f, 4.0f, 0.0f));
				if (asteriodbb2->Intersects(*asteriodbb))
				{
					collectables[i]->setPosition(XMFLOAT3(30.0f, (rand() % 40) - 19.0f, 0.0f));
				}
			}
		}
	}

	// Tests for collisions between the asteroids and the player, tells the game to handle the collision (for non-asteroid consequences) if one is found
	for (int i = 0; i < 9; i++)
	{
		BoundingBox *asteriodbb = new BoundingBox(XMFLOAT3(collectables[i]->getPosition()._41, collectables[i]->getPosition()._42, collectables[i]->getPosition()._43),
			XMFLOAT3(4.0f, 4.0f, 0.0f));
		//check for intersections
		if (asteriodbb->Intersects(*playerbb))
		{
				collectables[i]->setPosition(XMFLOAT3(30.0f, (rand() % 40) - 19.0f, 0.0f));
				gameReference->pickUp(stateManager);

				//elimate spawning on each other
				for (int g = 0; g < 9; g++)
				{
					BoundingBox *asteriodbb2 = new BoundingBox(XMFLOAT3(collectables[g]->getPosition()._41, collectables[g]->getPosition()._42, collectables[g]->getPosition()._43),
						XMFLOAT3(4.0f, 4.0f, 0.0f));
					if (asteriodbb2->Intersects(*asteriodbb))
					{
						collectables[i]->setPosition(XMFLOAT3(30.0f, (rand() % 40) - 19.0f, 0.0f));
					}
				}

		}

		break;
	}
}



//draw asteroids
void Collectable::draw(XMFLOAT4X4 viewMatrix, XMFLOAT4X4 projectionMatrix, XMFLOAT3 camPos){
	UINT offset = 0;
	UINT stride = sizeof(Vertex);

	for (unsigned int i = 0; i < collectables.size(); i++){
		//UINT offset = 0;
		stride = collectables[i]->g_mesh->sizeofvertex;
		// Set up the input assembler
		deviceContext->IASetInputLayout(collectables[i]->g_mat->shaderProgram->vsInputLayout);
		deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//set values that get passed to matrix constant buffer

		collectables[i]->g_mat->shaderProgram->ConstantBuffers[0]->dataToSendToConstantBuffer.world = collectables[i]->getWorld();
		collectables[i]->g_mat->shaderProgram->ConstantBuffers[0]->dataToSendToConstantBuffer.view = viewMatrix;
		collectables[i]->g_mat->shaderProgram->ConstantBuffers[0]->dataToSendToConstantBuffer.projection = projectionMatrix;

		//set values that get passed to lighting constant buffer
		collectables[i]->g_mat->shaderProgram->ConstantBuffers[1]->dataToSendToLightBuffer.ambientColor = lighting.ambientColor;
		collectables[i]->g_mat->shaderProgram->ConstantBuffers[1]->dataToSendToLightBuffer.diffuseColor = lighting.diffuseColor;
		collectables[i]->g_mat->shaderProgram->ConstantBuffers[1]->dataToSendToLightBuffer.lightDirection = lighting.lightDirection;
		collectables[i]->g_mat->shaderProgram->ConstantBuffers[1]->dataToSendToLightBuffer.specularColor = lighting.specularColor;
		collectables[i]->g_mat->shaderProgram->ConstantBuffers[1]->dataToSendToLightBuffer.specularPower = lighting.specularPower;
		//set values that get passed to camera constant buffer
		collectables[i]->g_mat->shaderProgram->ConstantBuffers[2]->dataToSendToCameraBuffer.cameraPosition = camPos;
		collectables[i]->g_mat->shaderProgram->ConstantBuffers[2]->dataToSendToCameraBuffer.padding = 1.0f;



		//matrix constant buffer
		deviceContext->UpdateSubresource(
			collectables[i]->g_mat->shaderProgram->ConstantBuffers[0]->constantBuffer,
			0,
			NULL,
			&collectables[i]->g_mat->shaderProgram->ConstantBuffers[0]->dataToSendToConstantBuffer,
			0,
			0);

		//camera constant buffer 
		deviceContext->UpdateSubresource(
			collectables[i]->g_mat->shaderProgram->ConstantBuffers[2]->constantBuffer,
			0,
			NULL,
			&collectables[i]->g_mat->shaderProgram->ConstantBuffers[2]->dataToSendToCameraBuffer,
			0,
			0);
		//light constant buffer
		deviceContext->UpdateSubresource(
			collectables[i]->g_mat->shaderProgram->ConstantBuffers[1]->constantBuffer,
			0,
			NULL,
			&collectables[i]->g_mat->shaderProgram->ConstantBuffers[1]->dataToSendToLightBuffer,
			0,
			0);

		deviceContext->IASetVertexBuffers(0, 1, &collectables[i]->g_mesh->v_buffer, &stride, &offset);
		deviceContext->IASetIndexBuffer(collectables[i]->g_mesh->i_buffer, DXGI_FORMAT_R32_UINT, 0);

		deviceContext->PSSetSamplers(0, 1, &collectables[i]->g_mat->samplerState);
		deviceContext->PSSetShaderResources(0, 1, &collectables[i]->g_mat->resourceView);
		deviceContext->PSSetShaderResources(1, 1, &collectables[1]->g_mat->resourceView2);



		// Set the current vertex and pixel shaders, as well the constant buffer for the vert shader
		deviceContext->VSSetShader(collectables[i]->g_mat->shaderProgram->vertexShader, NULL, 0);
		deviceContext->VSSetConstantBuffers(0, 1, &collectables[i]->g_mat->shaderProgram->ConstantBuffers[0]->constantBuffer); //set first constant vertex buffer-matrix
		deviceContext->VSSetConstantBuffers(1, 1, &collectables[i]->g_mat->shaderProgram->ConstantBuffers[2]->constantBuffer); //set second constant vertex buffer-camera
		deviceContext->PSSetShader(collectables[i]->g_mat->shaderProgram->pixelShader, NULL, 0);
		deviceContext->PSSetConstantBuffers(0, 1, &collectables[i]->g_mat->shaderProgram->ConstantBuffers[1]->constantBuffer); //set pixel constant buffer-light

		// Finally do the actual drawing
		deviceContext->DrawIndexed(
			collectables.at(i)->g_mesh->m_size,	// The number of indices we're using in this draw
			0,
			0);
	}
}
