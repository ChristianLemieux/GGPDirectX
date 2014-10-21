#include "Mesh.h"
#include <d3dcompiler.h>

Mesh::Mesh(Vertex* vertices, UINT* indices, int size, ID3D11Device* device){
	m_size = size;
	m_vertices = vertices;
	m_indices = indices;
	m_device = device;

	createVertexBuffer();
	createIndexBuffer();
}

Mesh::~Mesh(void){
	if (m_vertices){
		delete[] m_vertices;
	}
	if (m_indices){
		delete[] m_indices;
	}
	if (m_device){
		delete[] m_device;
	}
}

void Mesh::createVertexBuffer(){
	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex) * m_size; // Number of vertices in the "model" you want to draw
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA initialVertexData;
	initialVertexData.pSysMem = m_vertices;
	m_device->CreateBuffer(&vbd, &initialVertexData, &v_buffer);

}

void Mesh::createIndexBuffer(){
	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * m_size; // Number of indices in the "model" you want to draw
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA initialIndexData;
	initialIndexData.pSysMem = m_indices;
	m_device->CreateBuffer(&ibd, &initialIndexData, &i_buffer);
}

