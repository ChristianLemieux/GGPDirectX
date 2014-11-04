#include "ConstantBuffer.h"
#include "Global.h"
ConstantBuffer::ConstantBuffer(ConstantBufferLayout c_buffer_data, ID3D11Device* dev)
{
	D3D11_BUFFER_DESC cBufferDesc;
	cBufferDesc.ByteWidth = sizeof(dataToSendToConstantBuffer);
	cBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	cBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cBufferDesc.CPUAccessFlags = 0;
	cBufferDesc.MiscFlags = 0;
	cBufferDesc.StructureByteStride = 0;
	dev->CreateBuffer(
		&cBufferDesc,
		NULL,
		&constantBuffer);
}

void ConstantBuffer::InitializeConstantData(XMFLOAT4X4 world, XMFLOAT4X4 view, XMFLOAT4X4 projection){
	dataToSendToConstantBuffer.world = world;
	dataToSendToConstantBuffer.view = view;
	dataToSendToConstantBuffer.projection = projection;
}

ConstantBuffer::~ConstantBuffer(void)
{
	//ReleaseMacro(constantBuffer);
}