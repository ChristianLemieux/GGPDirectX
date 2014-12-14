#include "ParticleSystem.h"

ParticleSystem::ParticleSystem(XMFLOAT3 position, XMFLOAT2 velocity, XMFLOAT2 acceleration, ID3D11Device* dev, ID3D11DeviceContext* devCtx, Material* mat, int num_particles)
{
	device = dev;
	deviceContext = devCtx;
	initialized = false;

	Particle point[] = {
		position,
		velocity,
		acceleration,
	};
	object = new GameEntity(new Mesh(point, 0, 1, device), mat);
}


void ParticleSystem::drawParticleSystem(XMFLOAT4X4 viewMatrix, XMFLOAT4X4 projectionMatrix, float time)
{
	ID3D11BlendState* blendState = nullptr;
	D3D11_BLEND_DESC blendDesc;
	blendDesc.AlphaToCoverageEnable = 0;
	blendDesc.IndependentBlendEnable = 0;
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	HRESULT hr_blend = device->CreateBlendState(&blendDesc, &blendState);
	ID3D11Buffer* bufferArray[1] = { 0 };


	UINT offset = 0;
	UINT stride = object->g_mesh->sizeofvertex;

	deviceContext->IASetInputLayout(object->g_mat->shaderProgram->vsInputLayout);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	object->g_mat->shaderProgram->ConstantBuffers[0]->dataToSendToConstantBuffer.world = object->getWorld();
	object->g_mat->shaderProgram->ConstantBuffers[0]->dataToSendToConstantBuffer.view = viewMatrix;
	object->g_mat->shaderProgram->ConstantBuffers[0]->dataToSendToConstantBuffer.projection = projectionMatrix;
	object->g_mat->shaderProgram->ConstantBuffers[3]->dataToSendToGSBuffer.age = time;


	//matrix constant buffer
	deviceContext->UpdateSubresource(
		object->g_mat->shaderProgram->ConstantBuffers[0]->constantBuffer,
		0,
		NULL,
		&object->g_mat->shaderProgram->ConstantBuffers[0]->dataToSendToConstantBuffer,
		0,
		0);

	deviceContext->UpdateSubresource(
		object->g_mat->shaderProgram->ConstantBuffers[3]->constantBuffer,
		0,
		NULL,
		&object->g_mat->shaderProgram->ConstantBuffers[3]->dataToSendToGSBuffer,
		0,
		0);

	deviceContext->IASetVertexBuffers(0, 1, &object->g_mesh->v_buffer, &stride, &offset);
	deviceContext->IASetIndexBuffer(object->g_mesh->i_buffer, DXGI_FORMAT_R32_UINT, 0);
	if (!initialized){
		deviceContext->SOSetTargets(1, &object->g_mesh->so_buffer, 0);
		//initialized = true;
	}
	else{
		deviceContext->IASetVertexBuffers(0, 1, &object->g_mesh->so_buffer, &stride, &offset);
	}
	deviceContext->OMSetBlendState(blendState, NULL, 0xffffffff);

	deviceContext->PSSetSamplers(0, 1, &object->g_mat->samplerState);
	deviceContext->PSSetShaderResources(0, 1, &object->g_mat->resourceView);

	deviceContext->VSSetShader(object->g_mat->shaderProgram->vertexShader, NULL, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &object->g_mat->shaderProgram->ConstantBuffers[0]->constantBuffer); //set first constant vertex buffer-matrix
	deviceContext->VSSetConstantBuffers(1, 1, &object->g_mat->shaderProgram->ConstantBuffers[3]->constantBuffer);
	deviceContext->PSSetShader(object->g_mat->shaderProgram->pixelShader, NULL, 0);
	deviceContext->GSSetShader(object->g_mat->shaderProgram->geometryShader, NULL, 0);
	
	if (initialized){
		deviceContext->DrawAuto();
		initialized = false;
	}
	else{ initialized = true; }

	deviceContext->DrawIndexed(
		object->g_mesh->m_size,	// The number of indices we're using in this draw
		0,
		0);

	deviceContext->GSSetShader(NULL, NULL, 0);
	deviceContext->SOSetTargets(1, bufferArray, 0);

}