#include "healthPickup.h"
#include "Game.h"


healthPickup::healthPickup(ID3D11Device* dev, ID3D11DeviceContext* devCtx, vector<ConstantBuffer*> constantBufferList, ID3D11SamplerState* samplerState, Mesh* meshReference, Player* playerReference, Game* gameReferencePassed){
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
	healthMaterial = new Material(device, deviceContext, sampler, L"energy.png", shaderProgram);
	player = playerReference;
	mesh = meshReference;

	gameReference = gameReferencePassed;

	// populate the list of HPUp, scale them to one tenth the size of the mesh and randomize their position (off the right side of the screen)
	for (int i = 0; i < 1; i++)
	{
		HPUp.push_back(new GameEntity(meshReference, healthMaterial));
		HPUp[i]->scale(XMFLOAT3(0.09f, 0.09f, 0.09f));
		HPUp[i]->setPosition(XMFLOAT3(((rand() % 60) + 150), ((rand() % 40) - 30.0f), 0.0f));
	}
}


healthPickup::~healthPickup()
{
	ReleaseMacro(device);
	ReleaseMacro(deviceContext);
	if (sampler){
		delete sampler;
		sampler = nullptr;
	}
	if (healthMaterial){
		delete healthMaterial;
		healthMaterial = nullptr;
	}
	if (shaderProgram){
		delete shaderProgram;
		shaderProgram = nullptr;
	}
}

//Update HPUp positions each frame
void healthPickup::update(float dt){

	//make bounding boxes
	BoundingBox *playerbb = new BoundingBox(XMFLOAT3(player->player->getPosition()._41, player->player->getPosition()._42, player->player->getPosition()._43),
		XMFLOAT3(2.0f, 2.0f, 0.0f));

	//moves HPUp across screen (right to left) and respawns them when they leave the screen
	for (unsigned int i = 0; i < 1; i++){
	HPUp[i]->translate(XMFLOAT3(-15.0f * dt, 0.0f, 0.0f));

		//._41 is the x value for the position matrix of game entities
	if (HPUp[i]->getPosition()._41 < -300)
			HPUp[i]->setPosition(XMFLOAT3(150.0f, (rand() % 40) - 30.0f, 0.0f));
	}
	// Tests for collisions between the HUup and the player, tells the game to give health the if one is found
	for (int i = 0; i < 1; i++)
	{
		BoundingBox *healthbb = new BoundingBox(XMFLOAT3(HPUp[i]->getPosition()._41, HPUp[i]->getPosition()._42, HPUp[i]->getPosition()._43),
			XMFLOAT3(2.0f, 2.0f, 0.0f));
		//check for intersections
		if (healthbb->Intersects(*playerbb))
		{
				HPUp[i]->setPosition(XMFLOAT3(150.0f, (rand() % 40) - 30.0f, 0.0f));
				gameReference->getHealth();
				break;
		}
	}
}



//draw HPUp
void healthPickup::draw(XMFLOAT4X4 viewMatrix, XMFLOAT4X4 projectionMatrix, XMFLOAT3 camPos){
	UINT offset = 0;
	UINT stride = sizeof(Vertex);

	for (unsigned int i = 0; i < HPUp.size(); i++){
		//UINT offset = 0;
		stride = HPUp[i]->g_mesh->sizeofvertex;
		// Set up the input assembler
		deviceContext->IASetInputLayout(HPUp[i]->g_mat->shaderProgram->vsInputLayout);
		deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//set values that get passed to matrix constant buffer

		HPUp[i]->g_mat->shaderProgram->ConstantBuffers[0]->dataToSendToConstantBuffer.world = HPUp[i]->getWorld();
		HPUp[i]->g_mat->shaderProgram->ConstantBuffers[0]->dataToSendToConstantBuffer.view = viewMatrix;
		HPUp[i]->g_mat->shaderProgram->ConstantBuffers[0]->dataToSendToConstantBuffer.projection = projectionMatrix;

		//set values that get passed to lighting constant buffer
		HPUp[i]->g_mat->shaderProgram->ConstantBuffers[1]->dataToSendToLightBuffer.ambientColor = lighting.ambientColor;
		HPUp[i]->g_mat->shaderProgram->ConstantBuffers[1]->dataToSendToLightBuffer.diffuseColor = lighting.diffuseColor;
		HPUp[i]->g_mat->shaderProgram->ConstantBuffers[1]->dataToSendToLightBuffer.lightDirection = lighting.lightDirection;
		HPUp[i]->g_mat->shaderProgram->ConstantBuffers[1]->dataToSendToLightBuffer.specularColor = lighting.specularColor;
		HPUp[i]->g_mat->shaderProgram->ConstantBuffers[1]->dataToSendToLightBuffer.specularPower = lighting.specularPower;
		//set values that get passed to camera constant buffer
		HPUp[i]->g_mat->shaderProgram->ConstantBuffers[2]->dataToSendToCameraBuffer.cameraPosition = camPos;
		HPUp[i]->g_mat->shaderProgram->ConstantBuffers[2]->dataToSendToCameraBuffer.padding = 1.0f;



		//matrix constant buffer
		deviceContext->UpdateSubresource(
			HPUp[i]->g_mat->shaderProgram->ConstantBuffers[0]->constantBuffer,
			0,
			NULL,
			&HPUp[i]->g_mat->shaderProgram->ConstantBuffers[0]->dataToSendToConstantBuffer,
			0,
			0);

		//camera constant buffer 
		deviceContext->UpdateSubresource(
			HPUp[i]->g_mat->shaderProgram->ConstantBuffers[2]->constantBuffer,
			0,
			NULL,
			&HPUp[i]->g_mat->shaderProgram->ConstantBuffers[2]->dataToSendToCameraBuffer,
			0,
			0);
		//light constant buffer
		deviceContext->UpdateSubresource(
			HPUp[i]->g_mat->shaderProgram->ConstantBuffers[1]->constantBuffer,
			0,
			NULL,
			&HPUp[i]->g_mat->shaderProgram->ConstantBuffers[1]->dataToSendToLightBuffer,
			0,
			0);

		deviceContext->IASetVertexBuffers(0, 1, &HPUp[i]->g_mesh->v_buffer, &stride, &offset);
		deviceContext->IASetIndexBuffer(HPUp[i]->g_mesh->i_buffer, DXGI_FORMAT_R32_UINT, 0);

		deviceContext->PSSetSamplers(0, 1, &HPUp[i]->g_mat->samplerState);
		deviceContext->PSSetShaderResources(0, 1, &HPUp[i]->g_mat->resourceView);



		// Set the current vertex and pixel shaders, as well the constant buffer for the vert shader
		deviceContext->VSSetShader(HPUp[i]->g_mat->shaderProgram->vertexShader, NULL, 0);
		deviceContext->VSSetConstantBuffers(0, 1, &HPUp[i]->g_mat->shaderProgram->ConstantBuffers[0]->constantBuffer); //set first constant vertex buffer-matrix
		deviceContext->VSSetConstantBuffers(1, 1, &HPUp[i]->g_mat->shaderProgram->ConstantBuffers[2]->constantBuffer); //set second constant vertex buffer-camera
		deviceContext->PSSetShader(HPUp[i]->g_mat->shaderProgram->pixelShader, NULL, 0);
		deviceContext->PSSetConstantBuffers(0, 1, &HPUp[i]->g_mat->shaderProgram->ConstantBuffers[1]->constantBuffer); //set pixel constant buffer-light

		// Finally do the actual drawing
		deviceContext->DrawIndexed(
			HPUp.at(i)->g_mesh->m_size,	// The number of indices we're using in this draw
			0,
			0);
	}
}

