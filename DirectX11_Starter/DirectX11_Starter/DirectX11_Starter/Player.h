#ifndef _PLAYER_H
#define _PLAYER_H
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

class Player{
public:
	Player(ID3D11Device* dev, ID3D11DeviceContext* devCtx, vector<ConstantBuffer*> constantBufferList, ID3D11SamplerState* samplerState, Mesh* mesh);
	~Player(void);
	void update(float dt);
	void draw(XMFLOAT4X4 viewMatrix, XMFLOAT4X4 projectionMatrix, XMFLOAT3 camPos);
	void drawText(IFW1FontWrapper *pFontWrapper);
	int returnHealth(void);
	void setHealth(int new_health);
	void takeDamage();
	void reset();
	GameEntity* getPlayer();
	GameEntity* player;
private:
	int health;
	ShaderProgram* shaderProgram;
	Material* shipMaterial;
	ID3D11SamplerState* sampler;
	ID3D11Device* device;
	ID3D11DeviceContext* deviceContext;
	LightBufferType lighting;

};

#endif