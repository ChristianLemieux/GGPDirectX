//Referenced http://www.braynzarsoft.net/Code/index.php?p=VC&code=Obj-Model-Loader while coding this class
#pragma once
#include "Mesh.h"
#include "Global.h"
#include <vector>
#include <fstream>
#include <iostream>

using namespace DirectX;

class ObjectLoader{

public:
	std::vector<Vertex> vertices;
	std::vector<UINT> indices;
	std::vector<XMFLOAT3> Positions;
	std::vector<XMFLOAT2> UVs;
	std::vector<XMFLOAT3> Normals;
	ObjectLoader();
	~ObjectLoader();
	bool CompareVertices(Vertex a, Vertex b);
	UINT GetIndex(Vertex v);
	void LoadModel(std::string file);
	Vertex* VecToArray();
};