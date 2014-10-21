#ifndef _MATERIAL_H
#define _MATERIAL_H

#include <d3d11.h>
#include <Windows.h>
#include <DirectXMath.h>
#include "Global.h"
using namespace DirectX;

class Material{
public:
	ID3D11ShaderResourceView* resourceView;
	ID3D11SamplerState* samplerState;
	Material(ID3D11ShaderResourceView* rv, ID3D11SamplerState* sample);
	Material(ID3D11Device* dev, ID3D11DeviceContext* devCtx, ID3D11SamplerState* sampler, wchar_t* filepath);
	~Material(void);
};

#endif