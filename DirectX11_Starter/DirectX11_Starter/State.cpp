#include "State.h"

State::State(ID3D11Device* dev, ID3D11DeviceContext* devCtx, ID3D11SamplerState* sample, wchar_t* textureFile, Mesh* menuMesh, ShaderProgram* shaderProgram){
	device = dev;
	deviceContext = devCtx;

	Material* material = new Material(device, deviceContext, sample, textureFile, shaderProgram);
	gameState = new GameEntity(menuMesh, material);

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

	gameState->g_mat->shaderProgram->ConstantBuffers[0]->dataToSendToConstantBuffer.world = gameState->getWorld();
	gameState->g_mat->shaderProgram->ConstantBuffers[0]->dataToSendToConstantBuffer.view = viewMatrix;
	gameState->g_mat->shaderProgram->ConstantBuffers[0]->dataToSendToConstantBuffer.projection = projectionMatrix;


	deviceContext->UpdateSubresource(
		gameState->g_mat->shaderProgram->ConstantBuffers[0]->constantBuffer,
		0,
		NULL,
		&gameState->g_mat->shaderProgram->ConstantBuffers[0]->dataToSendToConstantBuffer,
		0,
		0);

	deviceContext->IASetVertexBuffers(0, 1, &gameState->g_mesh->v_buffer, &stride, &offset);
	deviceContext->IASetIndexBuffer(gameState->g_mesh->i_buffer, DXGI_FORMAT_R32_UINT, 0);

	deviceContext->PSSetSamplers(0, 1, &gameState->g_mat->samplerState);
	deviceContext->PSSetShaderResources(0, 1, &gameState->g_mat->resourceView);



	// Set the current vertex and pixel shaders, as well the constant buffer for the vert shader
	deviceContext->VSSetShader(gameState->g_mat->shaderProgram->vertexShader, NULL, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &gameState->g_mat->shaderProgram->ConstantBuffers[0]->constantBuffer);
	deviceContext->PSSetShader(gameState->g_mat->shaderProgram->pixelShader, NULL, 0);
	// Finally do the actual drawing
	deviceContext->DrawIndexed(
		gameState->g_mesh->m_size,	// The number of indices we're using in this draw
		0,
		0);
}