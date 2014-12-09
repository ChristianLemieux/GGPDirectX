#ifndef _PROJECTILE_H
#define _PROJECTILE_H
#include <DirectXMath.h>
#include <vector>
#include "ConstantBuffer.h"
#include "ObjectLoader.h"
#include "Mesh.h"
#include "GameEntity.h"
#include "Material.h"
#include "ShaderProgram.h"
#include "SamplerState.h"
#include "Global.h"
#include "Camera.h"
#include "StateManager.h"
#include "FW1FontWrapper.h"
#include "Player.h"

class Projectile{
public:
	Projectile(ID3D11Device* dev, ID3D11DeviceContext* devCtx, vector<ConstantBuffer*> constantBufferList, ID3D11SamplerState* samplerState, Mesh* meshReference, Player* playerReference);
	~Projectile(void);
	void update(float dt);
	void draw(XMFLOAT4X4 viewMatrix, XMFLOAT4X4 projectionMatrix, XMFLOAT3 camPos);
	GameEntity* getProjectile();

	// list of projectiles present in the game
	std::vector<GameEntity*> projectiles;

	// fires a projectile in response to user input (current the 'q' key)
	void fireProjectile();
private:
	Player* player;
	Mesh* mesh;
	ShaderProgram* shaderProgram;
	Material* projectileMaterial;
	ID3D11SamplerState* sampler;
	ID3D11Device* device;
	ID3D11DeviceContext* deviceContext;
	LightBufferType lighting;

};

#endif