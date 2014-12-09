#ifndef _CONSTANTBUFFER_H
#define _CONSTANTBUFFER_H
#include <d3d11.h>
#include <DirectXMath.h>
#include <unordered_map>
#include <string>
#include "Global.h"

using namespace DirectX;
class ConstantBuffer{
public:
	int c_byteWidth;
	ConstantBufferLayout dataToSendToConstantBuffer;
	ParticleVertexShaderConstantBufferLayout dataToSendToGSBuffer;
	LightBufferType dataToSendToLightBuffer;
	CameraBufferType dataToSendToCameraBuffer;
	ID3D11Buffer* constantBuffer;
	ConstantBuffer(ConstantBufferLayout c_buffer_data, ID3D11Device* dev);
	ConstantBuffer(ParticleVertexShaderConstantBufferLayout c_buffer_data, ID3D11Device* dev);
	ConstantBuffer(LightBufferType c_buffer_data, ID3D11Device* dev);
	ConstantBuffer(CameraBufferType c_buffer_data, ID3D11Device* dev);
	~ConstantBuffer(void);
private:
	void setUpConstantBuffer(ID3D11Device* dev);

};
#endif