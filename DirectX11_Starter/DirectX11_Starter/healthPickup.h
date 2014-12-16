#pragma once
#ifndef _healthPickup_H
#define _healthPickup_H
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
#include "StateManager.h"
#include "SimpleMath.h"

class Game;

using namespace DirectX;
class healthPickup
{
public:
	healthPickup(ID3D11Device* dev, ID3D11DeviceContext* devCtx, vector<ConstantBuffer*> constantBufferList, ID3D11SamplerState* samplerState, Mesh* meshReference, Player* playerReference, Game* gameReferencePassed);
	~healthPickup(void);
	void update(float dt);
	void draw(XMFLOAT4X4 viewMatrix, XMFLOAT4X4 projectionMatrix, XMFLOAT3 camPos);
	GameEntity* getHPup();

	// list of HPUp present in the game
	std::vector<GameEntity*> HPUp;
private:
	Player* player;
	Mesh* mesh;
	ShaderProgram* shaderProgram;
	Material* healthMaterial;
	ID3D11SamplerState* sampler;
	ID3D11Device* device;
	ID3D11DeviceContext* deviceContext;
	LightBufferType lighting;

	//for pickup
	Game* gameReference;
};
#endif

