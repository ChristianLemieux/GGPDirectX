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

using namespace DirectX;

#pragma comment(lib, "irrKlang.lib") // link with irrKlang.dll

class Game{
public:
	Game(ID3D11Device* dev, ID3D11DeviceContext* devCxt);
	~Game(void);
	void initGame(SamplerState *samplerStates);
	void updateGame(float dt, StateManager *stateManager);
	//void drawGame(XMFLOAT4X4 viewMatrix, XMFLOAT4X4 projectionMatrix);
	void drawGame(XMFLOAT4X4 viewMatrix, XMFLOAT4X4 projectionMatrix, XMFLOAT3 gamePos);
	void drawText(IFW1FontWrapper *pFontWrapper);
	void fireProjectile();
	int hullIntegrity;
private:
	LightBufferType lighting;

	//sound engine for the project
	irrklang::ISoundEngine* engine;
	
	wchar_t* collision;
	ID3D11Device* device;
	ID3D11DeviceContext* deviceContext;
	ShaderProgram* shaderProgram;
	ShaderProgram *multiTex;
	std::vector<GameEntity*> gameEntities;
	std::vector<SamplerState*>samplerStates;
	std::vector<Material*> materials;
	std::vector<GameEntity*> projectiles;

	Player* player;
	Mesh* asteroid;
	bool shotFired;

	ConstantBufferLayout dataToSendToVSConstantBuffer;
	LightBufferType dataToSendToLightConstantBuffer;
	CameraBufferType dataToSendToCameraConstantBuffer;

	std::vector<ConstantBuffer*> constantBufferList;

	bool notColliding;
	bool canTakeDamage;
};
#endif