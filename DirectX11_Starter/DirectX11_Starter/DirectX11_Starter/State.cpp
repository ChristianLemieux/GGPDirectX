#include "State.h"

State::State(ID3D11Device* dev, ID3D11DeviceContext* devCtx, ID3D11SamplerState* sample, wchar_t* textureFile, Mesh* menuMesh, ShaderProgram* shaderProgram){
	device = dev;
	deviceContext = devCtx;

	Material* material = new Material(device, deviceContext, sample, textureFile, shaderProgram);
	gameState = new GameEntity(menuMesh, material);

	lighting.ambientColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	lighting.diffuseColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	lighting.lightDirection = XMFLOAT3(0.0f, 0.0f, 1.0f);
	lighting.specularColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	lighting.specularPower = 0.0f;

	gameState->scale(XMFLOAT3(0.3f, 0.41f, 0.0f));
}

void State::update(){

}

State::~State(void)
{
	ReleaseMacro(device);
	ReleaseMacro(deviceContext);
	if (gameState){
		delete gameState;
		gameState = nullptr;
	}

}

void State::draw(XMFLOAT4X4 viewMatrix, XMFLOAT4X4 projectionMatrix){
	UINT offset = 0;
	//UINT offset = 0;
	UINT stride = gameState->g_mesh->sizeofvertex;
	// Set up the input assembler
	deviceContext->IASetInputLayout(gameState->g_mat->shaderProgram->vsInputLayout);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	gameState->g_mat->shaderProgram->vsConstantBuffer->dataToSendToConstantBuffer.world = gameState->getWorld();
	gameState->g_mat->shaderProgram->vsConstantBuffer->dataToSendToConstantBuffer.view = viewMatrix;
	gameState->g_mat->shaderProgram->vsConstantBuffer->dataToSendToConstantBuffer.projection = projectionMatrix;

	gameState->g_mat->shaderProgram->psConstantBuffer->dataToSendToLightBuffer.ambientColor = lighting.ambientColor;
	gameState->g_mat->shaderProgram->psConstantBuffer->dataToSendToLightBuffer.diffuseColor = lighting.diffuseColor;
	gameState->g_mat->shaderProgram->psConstantBuffer->dataToSendToLightBuffer.lightDirection = lighting.lightDirection;
	gameState->g_mat->shaderProgram->psConstantBuffer->dataToSendToLightBuffer.specularColor = lighting.specularColor;
	gameState->g_mat->shaderProgram->psConstantBuffer->dataToSendToLightBuffer.specularPower = lighting.specularPower;

	gameState->g_mat->shaderProgram->camConstantBuffer->dataToSendToCameraBuffer.cameraPosition = XMFLOAT3(0.0f, 0.0f, -5.0f);
	gameState->g_mat->shaderProgram->camConstantBuffer->dataToSendToCameraBuffer.padding = 100.0f;


	deviceContext->UpdateSubresource(
		gameState->g_mat->shaderProgram->vsConstantBuffer->constantBuffer,
		0,
		NULL,
		&gameState->g_mat->shaderProgram->vsConstantBuffer->dataToSendToConstantBuffer,
		0,
		0);

	deviceContext->UpdateSubresource(
		gameState->g_mat->shaderProgram->camConstantBuffer->constantBuffer,
		0,
		NULL,
		&gameState->g_mat->shaderProgram->camConstantBuffer->dataToSendToCameraBuffer,
		0,
		0);

	deviceContext->UpdateSubresource(
		gameState->g_mat->shaderProgram->psConstantBuffer->constantBuffer,
		0,
		NULL,
		&gameState->g_mat->shaderProgram->psConstantBuffer->dataToSendToLightBuffer,
		0,
		0);

	deviceContext->IASetVertexBuffers(0, 1, &gameState->g_mesh->v_buffer, &stride, &offset);
	deviceContext->IASetIndexBuffer(gameState->g_mesh->i_buffer, DXGI_FORMAT_R32_UINT, 0);

	deviceContext->PSSetSamplers(0, 1, &gameState->g_mat->samplerState);
	deviceContext->PSSetShaderResources(0, 1, &gameState->g_mat->resourceView);



	// Set the current vertex and pixel shaders, as well the constant buffer for the vert shader
	deviceContext->VSSetShader(gameState->g_mat->shaderProgram->vertexShader, NULL, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &gameState->g_mat->shaderProgram->vsConstantBuffer->constantBuffer);
	deviceContext->VSSetConstantBuffers(1, 1, &gameState->g_mat->shaderProgram->camConstantBuffer->constantBuffer);
	deviceContext->PSSetShader(gameState->g_mat->shaderProgram->pixelShader, NULL, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &gameState->g_mat->shaderProgram->psConstantBuffer->constantBuffer);
	// Finally do the actual drawing
	deviceContext->DrawIndexed(
		gameState->g_mesh->m_size,	// The number of indices we're using in this draw
		0,
		0);
}