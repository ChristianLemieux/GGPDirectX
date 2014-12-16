#include "Mesh.h"
#include <d3dcompiler.h>
#include "Global.h"
#include <typeinfo>

Mesh::Mesh(Vertex* vertices, UINT* indices, int size, ID3D11Device* device){
	m_size = size;
	m_vertices = vertices;
	m_indices = indices;
	m_device = device;

	sizeofvertex = sizeof(Vertex);

	createVertexBuffer();
	createIndexBuffer();
}

//model mesh constructor
Mesh::Mesh(Vertex2* vertices, UINT* indices, int size, ID3D11Device* device){
	m_size = size;
	m_vertices = vertices;
	m_indices = indices;
	m_device = device;

	sizeofvertex = sizeof(Vertex2);

	createVertexBuffer();
	createIndexBuffer();
}

Mesh::Mesh(Phong* vertices, UINT* indices, int size, ID3D11Device* device){
	m_size = size;
	m_vertices = vertices;
	m_indices = indices;
	m_device = device;

	sizeofvertex = sizeof(Phong);

	createVertexBuffer();
	createIndexBuffer();
}

Mesh::Mesh(Particle* vertices, UINT* indices, int size, ID3D11Device* device){
	m_size = size;
	m_vertices = vertices;
	m_indices = indices;
	m_device = device;

	sizeofvertex = sizeof(Particle);

	D3D11_BUFFER_DESC sobd;
	sobd.Usage = D3D11_USAGE_DEFAULT;
	sobd.ByteWidth = 640;
	sobd.BindFlags = D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_STREAM_OUTPUT;
	sobd.CPUAccessFlags = 0;
	sobd.MiscFlags = 0;
	sobd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA initialVertexData;
	initialVertexData.pSysMem = m_vertices;
	m_device->CreateBuffer(&sobd, &initialVertexData, &so_buffer);

	createVertexBuffer();
	createIndexBuffer();
	createInitBuffer();
}




Mesh::~Mesh(void){
	ReleaseMacro(v_buffer);
	ReleaseMacro(i_buffer);
	ReleaseMacro(m_device);
	if (m_indices){
		delete m_indices;
		m_indices = nullptr;
	}
	if (m_vertices){
		delete m_vertices;
		m_vertices = nullptr;
	}
}

void Mesh::createVertexBuffer(){
	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeofvertex * m_size;
	// Number of vertices in the "model" you want to draw
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA initialVertexData;
	initialVertexData.pSysMem = m_vertices;
	m_device->CreateBuffer(&vbd, &initialVertexData, &v_buffer);

}

void Mesh::createInitBuffer(){
	D3D11_BUFFER_DESC initbd;
	initbd.Usage = D3D11_USAGE_IMMUTABLE;
	initbd.ByteWidth = sizeofvertex * m_size;
	// Number of vertices in the "model" you want to draw
	initbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	initbd.CPUAccessFlags = 0;
	initbd.MiscFlags = 0;
	initbd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA initialVertexData;
	initialVertexData.pSysMem = m_vertices;
	m_device->CreateBuffer(&initbd, &initialVertexData, &init_buffer);
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

