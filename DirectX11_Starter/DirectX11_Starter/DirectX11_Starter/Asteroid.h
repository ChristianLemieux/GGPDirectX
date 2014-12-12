#pragma once
#ifndef _ASTEROID_H
#define _ASTEROID_H
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

class Game;

using namespace DirectX;

class Asteroid{
public:
	Asteroid(ID3D11Device* dev, ID3D11DeviceContext* devCtx, vector<ConstantBuffer*> constantBufferList, ID3D11SamplerState* samplerState, Mesh* meshReference, Player* playerReference, Game* gameReferencePassed);
	~Asteroid(void);
	void update(float dt, StateManager *stateManager);
	void draw(XMFLOAT4X4 viewMatrix, XMFLOAT4X4 projectionMatrix, XMFLOAT3 camPos);
	bool BoundingBoxCollision(XMVECTOR& firstObjBoundingBoxMinVertex, XMVECTOR& firstObjBoundingBoxMaxVertex,
		XMVECTOR& secondObjBoundingBoxMinVertex, XMVECTOR& secondObjBoundingBoxMaxVertex);
	GameEntity* getAsteroid();

	// list of asteroids present in the game
	std::vector<GameEntity*> asteroids;
private:
	Player* player;
	Mesh* mesh;
	ShaderProgram* shaderProgram;
	Material* asteroidMaterial;
	ID3D11SamplerState* sampler;
	ID3D11Device* device;
	ID3D11DeviceContext* deviceContext;
	LightBufferType lighting;

	// reference to the game necesary for handling the non-asteroid results of a player-asteroid collision
	Game* gameReference;

	bool collided;
	wchar_t* collision;
	bool notColliding;
	bool canTakeDamage;
};

#endif