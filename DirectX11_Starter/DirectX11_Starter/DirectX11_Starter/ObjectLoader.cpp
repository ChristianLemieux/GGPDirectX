#include "ObjectLoader.h"


ObjectLoader::ObjectLoader()
{
}


ObjectLoader::~ObjectLoader()
{
}

bool ObjectLoader::CompareVertices(Vertex a, Vertex b){

	return a.Position.x == b.Position.x &&
		a.Position.y == b.Position.y &&
		a.Position.z == b.Position.z &&
		a.uv.x == b.uv.x &&
		a.uv.y == b.uv.y;
}

UINT ObjectLoader::GetIndex(Vertex v){
	for (unsigned int i = 0; i < vertices.size(); i++){
		if (CompareVertices(vertices[i], v)){
			return i;
		}
	}
	return vertices.size();
}

Vertex* ObjectLoader::VecToArray(){

	Vertex* v = &vertices[0];
	return v;

}

void ObjectLoader::LoadModel(std::string file){

	std::ifstream ss;

	ss.open(file);

	char cmd[256] = {};

	while (ss){

		ss >> cmd;

		if (cmd == "#"){}
		else if (cmd == "v"){
			float x, y, z;
			ss >> x >> y >> z;
			Positions.push_back(XMFLOAT3(x, y, z));
		}
		else if (cmd == "vn"){
			float x, y, z;
			ss >> x >> y >> z;
			Normals.push_back(XMFLOAT3(x, y, z));
		}
		else if (cmd == "vt"){
			float u, v, w;
			ss >> u >> v >> w;
			UVs.push_back(XMFLOAT2(u, v));
		}
		else if (cmd == "f"){
			UINT u;
			Vertex v;
			for (int i = 0; i < 3; i++){
				ss >> u;
				v.Position = Positions[u - 1];
				ss.ignore();

				ss >> u;
				v.uv = UVs[u - 1];
				ss.ignore();

				ss >> u;

				indices.push_back(GetIndex(v));
				vertices.push_back(v);
			}
		}
	}

	std::cout << "model loaded" << std::endl;
}