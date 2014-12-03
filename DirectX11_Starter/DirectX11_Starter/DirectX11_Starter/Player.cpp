#include "Player.h"

//Constructor for player object
//Params(device, deviceContext, vector of constantbuffers, sampler state, mesh)
Player::Player(ID3D11Device* dev, ID3D11DeviceContext* devCtx, vector<ConstantBuffer*> constantBufferList, ID3D11SamplerState* samplerState, Mesh* mesh){
	lighting.ambientColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	lighting.diffuseColor = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	lighting.lightDirection = XMFLOAT3(0.0f, 0.0f, 1.0f);
	lighting.specularColor = XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f);
	lighting.specularPower = 5.0f;

	device = dev;
	deviceContext = devCtx;
	sampler = samplerState;
	health = 10;
	shaderProgram = new ShaderProgram(L"MultiTexVertexShader.cso", L"MultiTexPixelShader.cso", device, constantBufferList);
	shipMaterial = new Material(device, deviceContext, sampler, L"spaceShipTexture.jpg", L"night.jpg", L"alpha_map.png", shaderProgram);

	player = new GameEntity(mesh, shipMaterial);
	player->translate(XMFLOAT3(0.0f, 0.0f, 0.0f));
	player->scale(XMFLOAT3(0.1f, 0.1f, 0.1f));
}

Player::~Player(){
	ReleaseMacro(device);
	ReleaseMacro(deviceContext);
	if (player){
		delete player;
		player = nullptr;
	}
	if (sampler){
		delete sampler;
		sampler = nullptr;
	}
	if (shipMaterial){
		delete shipMaterial;
		shipMaterial = nullptr;
	}
	if (shaderProgram){
		delete shaderProgram;
		shaderProgram = nullptr;
	}
}

//Decrement Player's health
void Player::takeDamage(){
	if (health > 0){
		health--;
	}
}

//return ten times player's health
int Player::returnHealth(){
	return health * 10;
}


//Set Player's health ->Percent goes in, player's health goes from 1-10
//Ex: new_health is 100, health will set to 10.
void Player::setHealth(int new_health){
	int tempH = new_health / 10;
	health = max(0, min(tempH, 10));
}

//Update player position based on user input
void Player::update(float dt){
	if (GetAsyncKeyState('D') & 0x8000){
		player->translate(XMFLOAT3(5.0f * dt, 0.0f, 0.0f));
	}
	if (GetAsyncKeyState('A') & 0x8000){
		player->translate(XMFLOAT3(-5.0f * dt, 0.0f, 0.0f));
	}
	if (GetAsyncKeyState('W') & 0x8000){
		player->translate(XMFLOAT3(0.0f, 5.0f * dt, 0.0f));
	}
	if (GetAsyncKeyState('S') & 0x8000){
		player->translate(XMFLOAT3(0.0f, -5.0f * dt, 0.0f));
	}
}

//draw player game entity
void Player::draw(XMFLOAT4X4 viewMatrix, XMFLOAT4X4 projectionMatrix, XMFLOAT3 camPos){
	UINT offset = 0;
	UINT stride = player->g_mesh->sizeofvertex;
	deviceContext->IASetInputLayout(player->g_mat->shaderProgram->vsInputLayout);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//set values that get passed to matrix constant buffer
	player->g_mat->shaderProgram->ConstantBuffers[0]->dataToSendToConstantBuffer.world = player->getWorld();
	player->g_mat->shaderProgram->ConstantBuffers[0]->dataToSendToConstantBuffer.view = viewMatrix;
	player->g_mat->shaderProgram->ConstantBuffers[0]->dataToSendToConstantBuffer.projection = projectionMatrix;

	//set values that get passed to lighting constant buffer
	player->g_mat->shaderProgram->ConstantBuffers[1]->dataToSendToLightBuffer.ambientColor = lighting.ambientColor;
	player->g_mat->shaderProgram->ConstantBuffers[1]->dataToSendToLightBuffer.diffuseColor = lighting.diffuseColor;
	player->g_mat->shaderProgram->ConstantBuffers[1]->dataToSendToLightBuffer.lightDirection = lighting.lightDirection;
	player->g_mat->shaderProgram->ConstantBuffers[1]->dataToSendToLightBuffer.specularColor = lighting.specularColor;
	player->g_mat->shaderProgram->ConstantBuffers[1]->dataToSendToLightBuffer.specularPower = lighting.specularPower;
	//set values that get passed to camera constant buffer
	player->g_mat->shaderProgram->ConstantBuffers[2]->dataToSendToCameraBuffer.cameraPosition = camPos;
	player->g_mat->shaderProgram->ConstantBuffers[2]->dataToSendToCameraBuffer.padding = 1.0f;

	//matrix constant buffer
	deviceContext->UpdateSubresource(
		player->g_mat->shaderProgram->ConstantBuffers[0]->constantBuffer,
		0,
		NULL,
		&player->g_mat->shaderProgram->ConstantBuffers[0]->dataToSendToConstantBuffer,
		0,
		0);

	//camera constant buffer 
	deviceContext->UpdateSubresource(
		player->g_mat->shaderProgram->ConstantBuffers[2]->constantBuffer,
		0,
		NULL,
		&player->g_mat->shaderProgram->ConstantBuffers[2]->dataToSendToCameraBuffer,
		0,
		0);
	//light constant buffer
	deviceContext->UpdateSubresource(
		player->g_mat->shaderProgram->ConstantBuffers[1]->constantBuffer,
		0,
		NULL,
		&player->g_mat->shaderProgram->ConstantBuffers[1]->dataToSendToLightBuffer,
		0,
		0);

	deviceContext->IASetVertexBuffers(0, 1, &player->g_mesh->v_buffer, &stride, &offset);
	deviceContext->IASetIndexBuffer(player->g_mesh->i_buffer, DXGI_FORMAT_R32_UINT, 0);

	deviceContext->PSSetSamplers(0, 1, &player->g_mat->samplerState);
	deviceContext->PSSetShaderResources(0, 1, &player->g_mat->resourceView);
	deviceContext->PSSetShaderResources(1, 1, &player->g_mat->resourceView2);
	deviceContext->PSSetShaderResources(2, 1, &player->g_mat->resourceView3);


	// Set the current vertex and pixel shaders, as well the constant buffer for the vert shader
	deviceContext->VSSetShader(player->g_mat->shaderProgram->vertexShader, NULL, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &player->g_mat->shaderProgram->ConstantBuffers[0]->constantBuffer); //set first constant vertex buffer-matrix
	deviceContext->VSSetConstantBuffers(1, 1, &player->g_mat->shaderProgram->ConstantBuffers[2]->constantBuffer); //set second constant vertex buffer-camera
	deviceContext->PSSetShader(player->g_mat->shaderProgram->pixelShader, NULL, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &player->g_mat->shaderProgram->ConstantBuffers[1]->constantBuffer); //set pixel constant buffer-light

	// Finally do the actual drawing
	deviceContext->DrawIndexed(
		player->g_mesh->m_size,	// The number of indices we're using in this draw
		0,
		0);
}

//simple reset
void Player::reset()
{
	player->setPosition(XMFLOAT3(0.0f, 0.0f, 0.0f));
}

void Player::drawText(IFW1FontWrapper *pFontWrapper){

}