#include "ConstantBuffer.h"
#include "Global.h"
ConstantBuffer::ConstantBuffer(ConstantBufferLayout c_buffer_data, ID3D11Device* dev)
{
	c_byteWidth = sizeof(ConstantBufferLayout);
	setUpConstantBuffer(dev);
}

ConstantBuffer::ConstantBuffer(LightBufferType c_buffer_data, ID3D11Device* dev)
{

	c_byteWidth = sizeof(LightBufferType);
	setUpConstantBuffer(dev);
}

ConstantBuffer::ConstantBuffer(CameraBufferType c_buffer_data, ID3D11Device* dev)
{

	c_byteWidth = sizeof(CameraBufferType);
	setUpConstantBuffer(dev);
}

ConstantBuffer::ConstantBuffer(ParticleVertexShaderConstantBufferLayout c_buffer_data, ID3D11Device* dev)
{

	c_byteWidth = sizeof(ParticleVertexShaderConstantBufferLayout);
	setUpConstantBuffer(dev);
}

void ConstantBuffer::setUpConstantBuffer(ID3D11Device* dev){
	D3D11_BUFFER_DESC cBufferDesc;
	cBufferDesc.ByteWidth = c_byteWidth;
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

ConstantBuffer::~ConstantBuffer(void)
{
	//ReleaseMacro(constantBuffer);
}