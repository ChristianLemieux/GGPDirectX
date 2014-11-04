#ifndef _GAMEENTITY_H
#define _GAMEENTITY_H

#include "Mesh.h"
#include "Material.h"
#include "ConstantBuffer.h"

class GameEntity{
public:
	Mesh* g_mesh;
	Material* g_mat;
private:
	XMFLOAT4X4 worldMatrix;
	XMFLOAT4X4 rotationMatrix;
	XMFLOAT4X4 positionMatrix;
	XMFLOAT4X4 scaleMatrix;
public:
	GameEntity(Mesh* mesh, Material* mat);
	~GameEntity(void);
	void clearTransforms(void);
	XMFLOAT4X4 getWorld(void);
	XMFLOAT4X4 getRotation(void);
	XMFLOAT4X4 getPosition(void);
    XMFLOAT4X4 getScale(void);
	void setScale(XMFLOAT4X4 scale);
	void scale(XMFLOAT3 scale);
	void translate(XMFLOAT3 translate);
	void rotate(XMFLOAT3 rotate);
};
#endif