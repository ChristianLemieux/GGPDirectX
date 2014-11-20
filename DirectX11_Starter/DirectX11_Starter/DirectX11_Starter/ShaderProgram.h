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
	ShaderProgram(wchar_t* vs_file, wchar_t* ps_file, ID3D11Device* dev, ConstantBuffer *& vs_const_b, ConstantBuffer *& ps_const_b);
	ShaderProgram(wchar_t* vs_file, wchar_t* ps_file, ID3D11Device* dev, ConstantBuffer *& vs_const_b, ConstantBuffer *& light_buffer, ConstantBuffer *& camera_buffer);
	~ShaderProgram(void);
	HRESULT CreateInputLayoutDescFromShaderSignature(ID3DBlob* pShaderBlob, ID3D11Device* pD3DDevice, ID3D11InputLayout** pInputLayout);
	ID3D11PixelShader* pixelShader;
	ID3D11VertexShader* vertexShader;
	ID3D11InputLayout* vsInputLayout;
	ID3D11InputLayout* psInputLayout;
	ID3D11InputLayout* camInputLayout;
	ConstantBuffer* vsConstantBuffer;
	ConstantBuffer* psConstantBuffer;
	ConstantBuffer* camConstantBuffer;
};
#endif