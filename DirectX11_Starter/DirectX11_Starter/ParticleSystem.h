#ifndef _PARTICLESYSTEM_H
#define _PARTICLESTSTEM_H

#include "Global.h"
#include "GameEntity.h"
#include "Material.h"
#include "ConstantBuffer.h"
#include "Mesh.h"
#include <Windows.h>
#include <d3d11.h>
#include <vector>

class ParticleSystem{
public:
	ParticleSystem(XMFLOAT4 position, XMFLOAT2 velocity, XMFLOAT2 acceleration, ID3D11Device* dev, ID3D11DeviceContext* devCtx, Material* mat, int num_particles = 20);
	void drawParticleSystem(XMFLOAT4X4 viewMatrix, XMFLOAT4X4 projectionMatrix, float time);
private:
	ID3D11Device * device;
	ID3D11DeviceContext* deviceContext;
	std::vector<GameEntity*> particles;
	GameEntity* object;
	bool initialized;
	bool firstPass;
	int numParticles;
};
#endif