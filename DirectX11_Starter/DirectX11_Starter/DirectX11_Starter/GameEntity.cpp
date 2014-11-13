#include "GameEntity.h"
#include "Global.h"

GameEntity::GameEntity(Mesh* mesh, Material* mat){
	g_mesh = mesh;
	clearTransforms();
	worldMatrix = getWorld();
	g_mat = mat;
}

GameEntity::~GameEntity(void){
	if (g_mesh){
		delete g_mesh;
		g_mesh = nullptr;
	}
	if (g_mat){
		delete g_mat;
		g_mat = nullptr;
	}
}

/**
*Reset all matrices to identity matrices
**/
void GameEntity::clearTransforms(void){
	XMMATRIX identity = XMMatrixIdentity();
	XMStoreFloat4x4(&scaleMatrix, identity);
	XMStoreFloat4x4(&rotationMatrix, identity);
	XMStoreFloat4x4(&positionMatrix, identity);
}

/**
* Return position matrix
**/
XMFLOAT4X4 GameEntity::getPosition(void){
	return positionMatrix;
}

/**
* Return rotation matrix
**/
XMFLOAT4X4 GameEntity::getRotation(void){
	return rotationMatrix;
}

/**
* Return scale matrix
**/
XMFLOAT4X4 GameEntity::getScale(void){
	return scaleMatrix;
}

/**
*Calculate and return world matrix based on current transforms
**/
XMFLOAT4X4 GameEntity::getWorld(void){
	XMMATRIX pos = XMLoadFloat4x4(&positionMatrix);
	XMMATRIX rot = XMLoadFloat4x4(&rotationMatrix);
	XMMATRIX sca = XMLoadFloat4x4(&scaleMatrix);
	XMMATRIX world = pos * rot * sca;
	XMStoreFloat4x4(&worldMatrix, XMMatrixTranspose(world));
	return worldMatrix;
}

/**
*Apply a scale transform to game object
**/
void GameEntity::scale(XMFLOAT3 s){
	XMMATRIX scale = { 
		s.x, 0.0f, 0.0f, 0.0f,
		0.0f, s.y, 0.0f, 0.0f,
		0.0f, 0.0f, s.z, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f };
	XMMATRIX currentScale = XMLoadFloat4x4(&scaleMatrix);
	currentScale *= scale;
	XMStoreFloat4x4(&scaleMatrix, currentScale);
}

/**
*Change position of game object
**/
void GameEntity::translate(XMFLOAT3 t){

	XMMATRIX translate = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		t.x, t.y, t.z, 1.0f };
	XMMATRIX currentTranslation = XMLoadFloat4x4(&positionMatrix);
	currentTranslation *= translate;
	XMStoreFloat4x4(&positionMatrix, currentTranslation);
}

/**
*Apply a rotation to game object
**/
void GameEntity::rotate(XMFLOAT3 r){
	XMMATRIX rotX = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, cos(r.x), sin(r.x), 0.0f,
		0.0f, -sin(r.x), cos(r.x), 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f	};

	XMMATRIX rotY = {
		cos(r.y), 0.0f, -sin(r.y), 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		sin(r.y), 0.0f, cos(r.y), 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f };

	XMMATRIX rotZ = {
		cos(r.z), sin(r.z), 0.0f, 0.0f,
		-sin(r.z), cos(r.z), 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f };

	XMMATRIX rotation = rotX * rotY * rotZ;
	XMMATRIX currentRotation = XMLoadFloat4x4(&rotationMatrix);
	currentRotation *= rotation;
	XMStoreFloat4x4(&rotationMatrix, currentRotation);
}

void GameEntity::setPosition(XMFLOAT3 pos){
	XMMATRIX position = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		pos.x, pos.y, pos.z, 1.0f
	};

	XMStoreFloat4x4(&positionMatrix, position);
}