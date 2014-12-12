#pragma once
#ifndef _GAME_H
#define _GAME_H
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
#include <Audio.h>
#include "include/irrKlang.h"
#include <iostream>
#include "Projectile.h"
#include "GameTimer.h"
#include "Asteroid.h"
#include "ParticleSystem.h"

using namespace DirectX;

#pragma comment(lib, "irrKlang.lib") // link with irrKlang.dll

class Game{
public:
	Game(ID3D11Device* dev, ID3D11DeviceContext* devCxt);
	~Game(void);
	void initGame(SamplerState *samplerStates); // sets up the default parameters for the game
	void updateGame(float dt, StateManager *stateManager); // main update method for the game
	void drawGame(XMFLOAT4X4 viewMatrix, XMFLOAT4X4 projectionMatrix, XMFLOAT3 gamePos, float time, wchar_t* state); // Main drawing method for the game
	void drawText(IFW1FontWrapper *pFontWrapper); // handles text rendering
	void DrawUI(float time, wchar_t* state);
	void reset(); // resets the game to the default state
	void handleCollision(StateManager *stateManager); // handles collisions between the player and an asteroid
	int hullIntegrity; // the current hull integrity (out of 100)
private:
	LightBufferType lighting;

	//sound engine for the project
	irrklang::ISoundEngine* engine;

	ID3D11Device* device;
	ID3D11DeviceContext* deviceContext;
	ShaderProgram* shaderProgram;
	std::vector<GameEntity*> gameEntities; // Game entities that are not covered by the player, projectile, and asteroid managers
	std::vector<SamplerState*>samplerStates;
	std::vector<Material*> materials; // The list of materials utilized by game entities

	Player* player; // the player character/ship
	Mesh* asteroid; // basic mesh used for the majority of in-game entities
	Mesh* playerm;
	Mesh* bulletm;

	ConstantBufferLayout dataToSendToVSConstantBuffer;
	ParticleVertexShaderConstantBufferLayout dataToSendToGSConstantBuffer;
	LightBufferType dataToSendToLightConstantBuffer;
	CameraBufferType dataToSendToCameraConstantBuffer;

	std::vector<ConstantBuffer*> constantBufferList;
	ParticleSystem *particle;
	ParticleSystem *stars;

	// The projectile and asteroid managers
	Projectile* projectileManager;
	Asteroid* asteroidManager;
	int shootingScore;

	std::vector<Particle*> particles; // Container for all the particles currently in the game
};
#endif