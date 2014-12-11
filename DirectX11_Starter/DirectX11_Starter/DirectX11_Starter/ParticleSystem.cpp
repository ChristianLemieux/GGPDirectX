#include "ParticleSystem.h"

ParticleSystem::ParticleSystem(XMFLOAT3 position, XMFLOAT2 velocity, XMFLOAT2 acceleration, ID3D11Device* dev, ID3D11DeviceContext* devCtx, Material* mat, int num_particles)
{
	device = dev;
	deviceContext = devCtx;

	Particle* point = new Particle;
	for (int i = 0; i < num_particles; i++)
	{
		point->Position = XMFLOAT3(position.x + (float)i , position.y + (float)i, position.z);
		point->velocity = velocity;
		point->acceleration = acceleration;
		particles.push_back(new GameEntity(new Mesh(point, 0, 1, device), mat));
	}
}


void ParticleSystem::drawParticleSystem(XMFLOAT4X4 viewMatrix, XMFLOAT4X4 projectionMatrix, float time)
{
	for (int i = 0; i < particles.size(); i++){
		UINT offset = 0;
		UINT stride = particles[i]->g_mesh->sizeofvertex;

		deviceContext->IASetInputLayout(particles[i]->g_mat->shaderProgram->vsInputLayout);
		deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);


		particles[i]->g_mat->shaderProgram->ConstantBuffers[0]->dataToSendToConstantBuffer.world = particles[i]->getWorld();
		particles[i]->g_mat->shaderProgram->ConstantBuffers[0]->dataToSendToConstantBuffer.view = viewMatrix;
		particles[i]->g_mat->shaderProgram->ConstantBuffers[0]->dataToSendToConstantBuffer.projection = projectionMatrix;
		particles[i]->g_mat->shaderProgram->ConstantBuffers[3]->dataToSendToGSBuffer.age = time;


		//matrix constant buffer
		deviceContext->UpdateSubresource(
			particles[i]->g_mat->shaderProgram->ConstantBuffers[0]->constantBuffer,
			0,
			NULL,
			&particles[i]->g_mat->shaderProgram->ConstantBuffers[0]->dataToSendToConstantBuffer,
			0,
			0);

		deviceContext->UpdateSubresource(
			particles[i]->g_mat->shaderProgram->ConstantBuffers[3]->constantBuffer,
			0,
			NULL,
			&particles[i]->g_mat->shaderProgram->ConstantBuffers[3]->dataToSendToGSBuffer,
			0,
			0);

		deviceContext->IASetVertexBuffers(0, 1, &particles[i]->g_mesh->v_buffer, &stride, &offset);
		deviceContext->IASetIndexBuffer(particles[i]->g_mesh->i_buffer, DXGI_FORMAT_R32_UINT, 0);
		//deviceContext->SOSetTargets(1, &particles[i]->g_mesh->so_buffer, 0);

		deviceContext->PSSetSamplers(0, 1, &particles[i]->g_mat->samplerState);
		deviceContext->PSSetShaderResources(0, 1, &particles[i]->g_mat->resourceView);
		deviceContext->PSSetShaderResources(1, 1, &particles[i]->g_mat->resourceView2);

		deviceContext->VSSetShader(particles[i]->g_mat->shaderProgram->vertexShader, NULL, 0);
		deviceContext->VSSetConstantBuffers(0, 1, &particles[i]->g_mat->shaderProgram->ConstantBuffers[0]->constantBuffer); //set first constant vertex buffer-matrix
		deviceContext->VSSetConstantBuffers(1, 1, &particles[i]->g_mat->shaderProgram->ConstantBuffers[3]->constantBuffer);
		deviceContext->PSSetShader(particles[i]->g_mat->shaderProgram->pixelShader, NULL, 0);
		deviceContext->GSSetShader(particles[i]->g_mat->shaderProgram->geometryShader, NULL, 0);

		deviceContext->DrawIndexed(
			particles[i]->g_mesh->m_size,	// The number of indices we're using in this draw
			0,
			0);

		deviceContext->GSSetShader(NULL, NULL, 0);
	}
}