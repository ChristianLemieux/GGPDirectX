#include "ObjectLoader.h"


ObjectLoader::ObjectLoader(ID3D11Device* device)
{
	m_device = device;
}


ObjectLoader::~ObjectLoader()
{
	ReleaseMacro(m_device);
}

bool ObjectLoader::CompareVertices(Vertex2 a, Vertex2 b){

	return a.Position.x == b.Position.x &&
		a.Position.y == b.Position.y &&
		a.Position.z == b.Position.z &&
		a.UVs.x == b.UVs.x &&
		a.UVs.y == b.UVs.y;
}

UINT ObjectLoader::GetIndex(Vertex2 v){
	for (unsigned int i = 0; i < vertices.size(); i++){
		if (CompareVertices(vertices[i], v)){
			return i;
		}
	}
	return vertices.size();
}

Vertex2* ObjectLoader::VecToArray(){

	Vertex2* v = &vertices[0];
	return v;

}

Mesh* ObjectLoader::LoadModel(std::string file){

	std::ifstream ss(file);
	std::string in = "";
	//Loop until the stream reaches end of file
	while (ss.peek() != EOF)
	{
		ss >> in;
		//Skip comments
		if (in == "#"){}
		//add vertices to the position list
		else if (in == "v")
		{
			float x, y, z;
			ss >> x >> y >> z;
			Positions.push_back(XMFLOAT3(x, y, z));
		}
		//Add texture coordinates to the texture position list
		else if (in == "vt")
		{
			float u, v;
			ss >> u >> v;
			UVs.push_back(XMFLOAT2(u, v));
		}
		//add normals to the normal list
		else if (in == "vn")
		{
			float x, y, z;
			ss >> x >> y >> z;
			Normals.push_back(XMFLOAT3(x, y, z));
		}
		//faces make 3 vertices (position and uv) and add them to the vertices and indices lists
		else if (in == "f")
		{
			std::string Value;
			Vertex2 vertex;
			for (int iFace = 0; iFace < 3; iFace++)
			{
				ss >> Value;
				if (Value == ""){
					continue;
				}
				std::vector<UINT> values = ObjectLoader::splitString(Value);
				UINT pos = values[0];
				vertex.Position = Positions[pos - 1];
				UINT uv = values[1];
				vertex.UVs = UVs[uv - 1];
				UINT norm = values[2];
				vertex.Normal = Normals[norm - 1];
				vertices.push_back(vertex);
				indices.push_back(GetIndex(vertex));
			}
		}
	}

	Mesh* m = new Mesh(&vertices[0], &indices[0], vertices.size(), m_device);

	return m;
}

std::vector<UINT> ObjectLoader::splitString(std::string in){
	std::vector<UINT> out = std::vector<UINT>();
	std::string temp;
	std::stringstream ss(in);
	std::getline(ss, temp, '/');
	out.push_back(atoi(temp.c_str()));
	std::getline(ss, temp, '/');
	out.push_back(atoi(temp.c_str()));
	std::getline(ss, temp, '/');
	out.push_back(atoi(temp.c_str()));
	return out;
}