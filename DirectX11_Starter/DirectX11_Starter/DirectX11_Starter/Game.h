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

using namespace DirectX;

class Game{
public:
	Game(ID3D11Device* dev, ID3D11DeviceContext* devCxt);
	void initGame();
	void updateGame(float dt);
	void drawGame(XMFLOAT4X4 viewMatrix, XMFLOAT4X4 projectionMatrix);
private:
	ID3D11Device* device;
	ID3D11DeviceContext* deviceContext;
	ShaderProgram* shaderProgram;
	std::vector<GameEntity*> gameEntities;
	std::vector<SamplerState*>samplerStates;
	std::vector<Material*> materials;

	ConstantBufferLayout dataToSendToVSConstantBuffer;

	std::vector<ConstantBuffer*> constantBufferList;
};
#endif