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
	ConstantBufferLayout dataToSendToConstantBuffer;
	ID3D11Buffer* constantBuffer;
	ConstantBuffer(ConstantBufferLayout c_buffer_data, ID3D11Device* dev);
	~ConstantBuffer(void);
	void InitializeConstantData(XMFLOAT4X4 world, XMFLOAT4X4 view, XMFLOAT4X4 projection);

};
#endif