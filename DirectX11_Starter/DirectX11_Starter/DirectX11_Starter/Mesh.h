#ifndef _MESH_H
#define _MESH_H

#include "Global.h"
#include <Windows.h>
#include <d3d11.h>

class Mesh{
public:
	int m_size;
	Vertex* m_vertices;
	UINT* m_indices;
	ID3D11Device* m_device;
	ID3D11Buffer* v_buffer;
	ID3D11Buffer* i_buffer;
	Mesh(Vertex* vertices, UINT* indices, int size, ID3D11Device* device);
	~Mesh(void);
	void createVertexBuffer();
	void createIndexBuffer();
	void drawMesh(ID3D11DeviceContext* deviceContext);
};

#endif