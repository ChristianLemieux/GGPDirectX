#ifndef _STATE_H
#define _STATE_H

#include "GameEntity.h"
#include "Material.h"
#include "Mesh.h"
#include "ShaderProgram.h"
#include "ConstantBuffer.h"
#include "Global.h"
#include "ObjectLoader.h"

class State{
public:
	State(ID3D11Device* dev, ID3D11DeviceContext* devCtx, ID3D11SamplerState* sample, wchar_t* textureFile, Mesh* menuMesh, ShaderProgram* shaderProgram);
	~State(void);
	void update();
	void draw(XMFLOAT4X4 viewMatrix, XMFLOAT4X4 projectionMatrix);
	GameEntity* gameState;
	ID3D11Device* device;
	ID3D11DeviceContext* deviceContext;

	

	ConstantBufferLayout dataToSendToVSConstantBuffer;
	LightBufferType dataToSendToLightConstantBuffer;
	CameraBufferType dataToSendToCameraConstantBuffer;
	LightBufferType lighting;
};
#endif