#include "Projectile.h"

//Constructor for Projectile object
Projectile::Projectile(ID3D11Device* dev, ID3D11DeviceContext* devCtx, vector<ConstantBuffer*> constantBufferList, ID3D11SamplerState* samplerState, Mesh* meshReference, Player* playerReference){
	// set up the lighting
	lighting.ambientColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	lighting.diffuseColor = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	lighting.lightDirection = XMFLOAT3(0.0f, 0.0f, 1.0f);
	lighting.specularColor = XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f);
	lighting.specularPower = 5.0f;

	device = dev;
	deviceContext = devCtx;
	sampler = samplerState;
	shaderProgram = new ShaderProgram(L"MultiTexVertexShader.cso", L"MultiTexPixelShader.cso", device, constantBufferList);
	projectileMaterial = new Material(device, deviceContext, sampler, L"bullet.jpg", shaderProgram);
	player = playerReference;
	mesh = meshReference;
}

Projectile::~Projectile(){
	ReleaseMacro(device);
	ReleaseMacro(deviceContext);
	if (sampler){
		delete sampler;
		sampler = nullptr;
	}
	if (projectileMaterial){
		delete projectileMaterial;
		projectileMaterial = nullptr;
	}
	if (shaderProgram){
		delete shaderProgram;
		shaderProgram = nullptr;
	}
}


//Update projectile position each frame, check for user input that fires a projectile
void Projectile::update(float dt){
	
	// fires a projectile in response to the 'q' key
	if (GetAsyncKeyState('Q') & 0x8000){
		if (projectiles.size() < 1)
		fireProjectile();
	}

	// moves the projectile, cleans it up if it moves off screen
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

}

// Creates a new projectile at the player's position, scales it to 1/10th the original mesh size, and adds it to the list of active projectiles.
void Projectile::fireProjectile()
{
		float playerX = player->player->getPosition()._41;
		float playerY = player->player->getPosition()._42;
		
		projectiles.push_back(new GameEntity(mesh, projectileMaterial));
		projectiles[projectiles.size() - 1]->scale(XMFLOAT3(0.1f, 0.1f, 0.1f));
		projectiles[projectiles.size() - 1]->setPosition(XMFLOAT3(playerX, playerY, 0.0f));

}

//draw projectiles
void Projectile::draw(XMFLOAT4X4 viewMatrix, XMFLOAT4X4 projectionMatrix, XMFLOAT3 camPos){
	for (unsigned int i = 0; i < projectiles.size(); i++){
		// projectiles are scaled to 50% size so that they are not as large as the player ship
		projectiles[i]->scale(XMFLOAT3(0.5f, 0.5f, 0.5f));
		// The projectile is moved to compensate for the scaling operation (that would change its location)
		projectiles[i]->setPosition(XMFLOAT3(projectiles[i]->getPosition()._41 * 2, projectiles[i]->getPosition()._42 * 2, 0.0f));
		UINT offset = 0;
		UINT stride = projectiles[i]->g_mesh->sizeofvertex;
		deviceContext->IASetInputLayout(projectiles[i]->g_mat->shaderProgram->vsInputLayout);
		deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//set values that get passed to matrix constant buffer
		projectiles[i]->g_mat->shaderProgram->ConstantBuffers[0]->dataToSendToConstantBuffer.world = projectiles[i]->getWorld();
		projectiles[i]->g_mat->shaderProgram->ConstantBuffers[0]->dataToSendToConstantBuffer.view = viewMatrix;
		projectiles[i]->g_mat->shaderProgram->ConstantBuffers[0]->dataToSendToConstantBuffer.projection = projectionMatrix;

		//set values that get passed to lighting constant buffer
		projectiles[i]->g_mat->shaderProgram->ConstantBuffers[1]->dataToSendToLightBuffer.ambientColor = lighting.ambientColor;
		projectiles[i]->g_mat->shaderProgram->ConstantBuffers[1]->dataToSendToLightBuffer.diffuseColor = lighting.diffuseColor;
		projectiles[i]->g_mat->shaderProgram->ConstantBuffers[1]->dataToSendToLightBuffer.lightDirection = lighting.lightDirection;
		projectiles[i]->g_mat->shaderProgram->ConstantBuffers[1]->dataToSendToLightBuffer.specularColor = lighting.specularColor;
		projectiles[i]->g_mat->shaderProgram->ConstantBuffers[1]->dataToSendToLightBuffer.specularPower = lighting.specularPower;
		//set values that get passed to camera constant buffer
		projectiles[i]->g_mat->shaderProgram->ConstantBuffers[2]->dataToSendToCameraBuffer.cameraPosition = camPos;
		projectiles[i]->g_mat->shaderProgram->ConstantBuffers[2]->dataToSendToCameraBuffer.padding = 1.0f;

		//matrix constant buffer
		deviceContext->UpdateSubresource(
			projectiles[i]->g_mat->shaderProgram->ConstantBuffers[0]->constantBuffer,
			0,
			NULL,
			&projectiles[i]->g_mat->shaderProgram->ConstantBuffers[0]->dataToSendToConstantBuffer,
			0,
			0);

		//camera constant buffer 
		deviceContext->UpdateSubresource(
			projectiles[i]->g_mat->shaderProgram->ConstantBuffers[2]->constantBuffer,
			0,
			NULL,
			&projectiles[i]->g_mat->shaderProgram->ConstantBuffers[2]->dataToSendToCameraBuffer,
			0,
			0);
		//light constant buffer
		deviceContext->UpdateSubresource(
			projectiles[i]->g_mat->shaderProgram->ConstantBuffers[1]->constantBuffer,
			0,
			NULL,
			&projectiles[i]->g_mat->shaderProgram->ConstantBuffers[1]->dataToSendToLightBuffer,
			0,
			0);

		deviceContext->IASetVertexBuffers(0, 1, &projectiles[i]->g_mesh->v_buffer, &stride, &offset);
		deviceContext->IASetIndexBuffer(projectiles[i]->g_mesh->i_buffer, DXGI_FORMAT_R32_UINT, 0);

		deviceContext->PSSetSamplers(0, 1, &projectiles[i]->g_mat->samplerState);
		deviceContext->PSSetShaderResources(0, 1, &projectiles[i]->g_mat->resourceView);
		deviceContext->PSSetShaderResources(1, 1, &projectiles[i]->g_mat->resourceView2);



		// Set the current vertex and pixel shaders, as well the constant buffer for the vert shader
		deviceContext->VSSetShader(projectiles[i]->g_mat->shaderProgram->vertexShader, NULL, 0);
		deviceContext->VSSetConstantBuffers(0, 1, &projectiles[i]->g_mat->shaderProgram->ConstantBuffers[0]->constantBuffer); //set first constant vertex buffer-matrix
		deviceContext->VSSetConstantBuffers(1, 1, &projectiles[i]->g_mat->shaderProgram->ConstantBuffers[2]->constantBuffer); //set second constant vertex buffer-camera
		deviceContext->PSSetShader(projectiles[i]->g_mat->shaderProgram->pixelShader, NULL, 0);
		deviceContext->PSSetConstantBuffers(0, 1, &projectiles[i]->g_mat->shaderProgram->ConstantBuffers[1]->constantBuffer); //set pixel constant buffer-light

		// Finally do the actual drawing
		deviceContext->DrawIndexed(
			projectiles[i]->g_mesh->m_size,	// The number of indices we're using in this draw
			0,
			0);
		// Move the projectile back to the position it was at before we compensated for the scaling operation.
		projectiles[i]->setPosition(XMFLOAT3(projectiles[i]->getPosition()._41 / 2, projectiles[i]->getPosition()._42 / 2, 0.0f));
		// Scale the projectile back to the typical mesh size so that collision is handled properly.
		projectiles[i]->scale(XMFLOAT3(2.0f, 2.0f, 2.0f));
	}
}
