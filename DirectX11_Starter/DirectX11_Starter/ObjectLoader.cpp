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

XMFLOAT3* ObjectLoader::CalculateTangent(Triangle& triangle){

	//XMFLOAT3* tan2 = new XMFLOAT3();
	XMFLOAT3 tan1;

	int a = triangle.index[0];
	int b = triangle.index[1];
	int c = triangle.index[2];

	Vertex2* vert1 = &vertices[a];
	Vertex2* vert2 = &vertices[b];
	Vertex2* vert3 = &vertices[c];

	XMFLOAT3 v1 = vert1->Position;
	XMFLOAT3 v2 = vert2->Position;
	XMFLOAT3 v3 = vert3->Position;

	XMFLOAT2 w1 = vert1->UVs;
	XMFLOAT2 w2 = vert2->UVs;
	XMFLOAT2 w3 = vert3->UVs;

	float x1 = v2.x - v1.x;
	float x2 = v3.x - v1.x;
	float y1 = v2.y - v1.y;
	float y2 = v3.y - v1.y;
	float z1 = v2.z - v1.z;
	float z2 = v3.z - v1.z;

	float s1 = w2.x - w1.x;
	float s2 = w3.x - w1.x;
	float t1 = w2.y - w1.y;
	float t2 = w3.y - w1.y;

	float r = 1.0F / (s1 * t2 - s2 * t1);

	XMFLOAT3 sdir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r,
		(t2 * z1 - t1 * z2) * r);
	XMFLOAT3 tdir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r,
		(s1 * z2 - s2 * z1) * r);

	tan1.x += sdir.x;
	tan1.y += sdir.y;
	tan1.z += sdir.z;

	/*tan2[a].x += tdir.x;
	tan2[b].x += tdir.x;
	tan2[c].x += tdir.x;

	tan2[a].y += tdir.y;
	tan2[b].y += tdir.y;
	tan2[c].y += tdir.y;

	tan2[a].z += tdir.z;
	tan2[b].z += tdir.z;
	tan2[c].z += tdir.z;*/

	XMFLOAT3 tangent1;
	XMFLOAT3 norm1 = vert1->Normal;
	XMFLOAT3 noob1 = tan1;

	XMFLOAT3 newGuy1 = { noob1.x - norm1.x, noob1.y - norm1.y, noob1.z - norm1.z };

	float dot1 = (norm1.x * noob1.x) + (norm1.y * noob1.y) + (norm1.z * noob1.z);

	tangent1.x = (newGuy1.x * dot1);
	tangent1.y = (newGuy1.y * dot1);
	tangent1.z = (newGuy1.z * dot1);


	XMFLOAT3 tangent2;
	XMFLOAT3 norm2 = vert2->Normal;
	XMFLOAT3 noob2 = tan1;

	XMFLOAT3 newGuy2 = { noob2.x - norm2.x, noob2.y - norm2.y, noob2.z - norm2.z };

	float dot2 = (norm2.x * noob2.x) + (norm2.y * noob2.y) + (norm2.z * noob2.z);

	tangent2.x = (newGuy2.x * dot2);
	tangent2.y = (newGuy2.y * dot2);
	tangent2.z = (newGuy2.z * dot2);


	XMFLOAT3 tangent3;
	XMFLOAT3 norm3 = vert3->Normal;
	XMFLOAT3 noob3 = tan1;

	XMFLOAT3 newGuy3 = { noob3.x - norm3.x, noob3.y - norm3.y, noob3.z - norm3.z };

	float dot3 = (norm3.x * noob3.x) + (norm3.y * noob3.y) + (norm3.z * noob3.z);

	tangent3.x = (newGuy3.x * dot3);
	tangent3.y = (newGuy3.y * dot3);
	tangent3.z = (newGuy3.z * dot3);


	XMFLOAT3* tanArr = new XMFLOAT3[3];
	tanArr[0] = tangent1;
	tanArr[1] = tangent2;
	tanArr[2] = tangent3;

	return tanArr;

	delete[] tanArr;

}

float ObjectLoader::XMFLOATDotProduct(XMFLOAT3 first, XMFLOAT3 second){

	float value = (first.x * second.x) + (first.y * second.y) + (first.z * second.z);
	return value;
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