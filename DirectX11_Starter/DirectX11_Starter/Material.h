#ifndef _MATERIAL_H
#define _MATERIAL_H

#include <d3d11.h>
#include <Windows.h>
#include <DirectXMath.h>
#include "Global.h"
#include <d3dcompiler.h>
#include <vector>
#include <cstdint>
#include "dxerr.h"
#include "ShaderProgram.h"

using namespace DirectX;

class Material{
public:
	ID3D11ShaderResourceView* resourceView;
	ID3D11ShaderResourceView* resourceView2;
	ID3D11ShaderResourceView* resourceView3;
	ID3D11SamplerState* samplerState;

	// Our basic shaders for this example
	ShaderProgram* shaderProgram;

	ID3D11Buffer* vsConstantBuffer;
	ID3D11Buffer* psConstantBuffer;

	Material(ID3D11ShaderResourceView* rv, ID3D11SamplerState* sample, ShaderProgram* s_program);
	Material(ID3D11Device* dev, ID3D11DeviceContext* devCtx, ID3D11SamplerState* sampler, wchar_t* filepath, ShaderProgram* s_program);
	Material(ID3D11Device* dev, ID3D11DeviceContext* devCtx, ID3D11SamplerState* sampler, wchar_t* filepath, wchar_t* filepath2, ShaderProgram* s_program);
	Material(ID3D11Device* dev, ID3D11DeviceContext* devCtx, ID3D11SamplerState* sampler, wchar_t* filepath, wchar_t* filepath2, wchar_t* filepath3, ShaderProgram* s_program);
	~Material(void);
};

#endif