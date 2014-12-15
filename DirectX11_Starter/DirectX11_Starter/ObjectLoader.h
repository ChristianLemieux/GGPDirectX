//Referenced http://www.braynzarsoft.net/Code/index.php?p=VC&code=Obj-Model-Loader while coding this class
#pragma once
#include "Mesh.h"
#include "Global.h"
#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

using namespace DirectX;

class ObjectLoader{

public:
	std::vector<Vertex2> vertices;
	std::vector<UINT> indices;
	std::vector<XMFLOAT3> Positions;
	std::vector<XMFLOAT2> UVs;
	std::vector<XMFLOAT3> Normals;
	std::vector<UINT> splitString(std::string in);
	ID3D11Device* m_device;
	ObjectLoader(ID3D11Device* device);
	~ObjectLoader();
	bool CompareVertices(Vertex2 a, Vertex2 b);
	UINT GetIndex(Vertex2 v);
	Mesh* LoadModel(std::string file);
	Vertex2* VecToArray();
	XMFLOAT3* CalculateTangent(Triangle& triangle);
	float XMFLOATDotProduct(XMFLOAT3 first, XMFLOAT3 second);
};