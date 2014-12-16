#ifndef _SHADERPROGRAM_H
#define _SHADERPROGRAM_H
#include <d3d11.h>
#include <cstdint>
#include <d3dcompiler.h>
#include <vector>
#include "ConstantBuffer.h"
#define ReleaseMacro(x) { if(x){ x->Release(); x = 0; } }

class ShaderProgram{
public:
	ShaderProgram(wchar_t* vs_file, wchar_t* ps_file, ID3D11Device* dev, std::vector<ConstantBuffer*> constantBufferList);
	ShaderProgram(wchar_t* vs_file, wchar_t* ps_file, wchar_t* gs_file, wchar_t* so_file, ID3D11Device* dev, std::vector<ConstantBuffer*> constantBufferList);
	~ShaderProgram(void);
	HRESULT CreateInputLayoutDescFromShaderSignature(ID3DBlob* pShaderBlob, ID3D11Device* pD3DDevice, ID3D11InputLayout** pInputLayout);
	ID3D11PixelShader* pixelShader;
	ID3D11VertexShader* vertexShader;
	ID3D11GeometryShader* geometryShader;
	ID3D11GeometryShader* streamOutputShader;
	ID3D11InputLayout* vsInputLayout;
	ID3D11InputLayout* psInputLayout;
	ID3D11InputLayout* gsInputLayout;
	ID3D11InputLayout* camInputLayout;
	std::vector<ConstantBuffer*> ConstantBuffers;
};
#endif