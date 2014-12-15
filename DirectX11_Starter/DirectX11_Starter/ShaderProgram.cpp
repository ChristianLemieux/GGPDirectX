#include "ShaderProgram.h"
#include "Global.h"

ShaderProgram::ShaderProgram(wchar_t* vs_file, wchar_t* ps_file, ID3D11Device* dev, std::vector<ConstantBuffer*> constantBufferList){
	ID3DBlob* vsBlob;
	D3DReadFileToBlob(vs_file, &vsBlob);
	HRESULT hr_vertex = this->CreateInputLayoutDescFromShaderSignature(vsBlob, dev, &vsInputLayout);
	dev->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), NULL, &vertexShader);
	ReleaseMacro(vsBlob);

	ID3DBlob* psBlob;
	D3DReadFileToBlob(ps_file, &psBlob);
	HRESULT hr_pixel = this->CreateInputLayoutDescFromShaderSignature(psBlob, dev, &psInputLayout);
	dev->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), NULL, &pixelShader);
	ReleaseMacro(psBlob);

	ConstantBuffers = constantBufferList;
}

ShaderProgram::ShaderProgram(wchar_t* vs_file, wchar_t* ps_file, wchar_t* gs_file, ID3D11Device* dev, std::vector<ConstantBuffer*> constantBufferList){
	ID3DBlob* vsBlob;
	D3DReadFileToBlob(vs_file, &vsBlob);
	HRESULT hr_vertex = this->CreateInputLayoutDescFromShaderSignature(vsBlob, dev, &vsInputLayout);
	dev->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), NULL, &vertexShader);
	ReleaseMacro(vsBlob);

	ID3DBlob* psBlob;
	D3DReadFileToBlob(ps_file, &psBlob);
	HRESULT hr_pixel = this->CreateInputLayoutDescFromShaderSignature(psBlob, dev, &psInputLayout);
	dev->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), NULL, &pixelShader);
	ReleaseMacro(psBlob);

	D3D11_SO_DECLARATION_ENTRY desc[] =
	{
		{ 0, "SV_POSITION", 0, 0, 4, 0 }
	};

	ID3DBlob* gsBlob;
	D3DReadFileToBlob(gs_file, &gsBlob);
	HRESULT hr_geometry = this->CreateInputLayoutDescFromShaderSignature(gsBlob, dev, &gsInputLayout);
	HRESULT hr_geometryShader = dev->CreateGeometryShaderWithStreamOutput(gsBlob->GetBufferPointer(), gsBlob->GetBufferSize(), desc, 1, NULL, 0, 0, NULL, &geometryShader);
	if (FAILED(hr_geometryShader)){ 
//		dev->CreateGeometryShader(gsBlob->GetBufferPointer(), gsBlob->GetBufferSize(), NULL, &geometryShader); 
	}
	ReleaseMacro(gsBlob);

	ConstantBuffers = constantBufferList;
}

ShaderProgram::~ShaderProgram(void)
{
	ReleaseMacro(pixelShader);
	ReleaseMacro(vertexShader);
	ReleaseMacro(geometryShader);
	ReleaseMacro(vsInputLayout);
	ReleaseMacro(psInputLayout);
	ReleaseMacro(gsInputLayout);
	ReleaseMacro(camInputLayout);
}


/**
*http://takinginitiative.wordpress.com/2011/12/11/directx-1011-basic-shader-reflection-automatic-input-layout-creation/
**/
HRESULT ShaderProgram::CreateInputLayoutDescFromShaderSignature(ID3DBlob* pShaderBlob, ID3D11Device* pD3DDevice, ID3D11InputLayout** pInputLayout)
{
	// Reflect shader info
	ID3D11ShaderReflection* pVertexShaderReflection = NULL;
	if (FAILED(D3DReflect(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&pVertexShaderReflection)))
	{
		return S_FALSE;
	}

	// Get shader info
	D3D11_SHADER_DESC shaderDesc;
	pVertexShaderReflection->GetDesc(&shaderDesc);

	// Read input layout description from shader info
	std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayoutDesc;
	for (uint32_t i = 0; i< shaderDesc.InputParameters; i++)
	{
		D3D11_SIGNATURE_PARAMETER_DESC paramDesc;
		pVertexShaderReflection->GetInputParameterDesc(i, &paramDesc);

		// fill out input element desc
		D3D11_INPUT_ELEMENT_DESC elementDesc;
		elementDesc.SemanticName = paramDesc.SemanticName;
		elementDesc.SemanticIndex = paramDesc.SemanticIndex;
		elementDesc.InputSlot = 0;
		elementDesc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		elementDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		elementDesc.InstanceDataStepRate = 0;

		// determine DXGI format
		if (paramDesc.Mask == 1)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32_FLOAT;
		}
		else if (paramDesc.Mask <= 3)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
		}
		else if (paramDesc.Mask <= 7)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
		}
		else if (paramDesc.Mask <= 15)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		}

		//save element desc
		inputLayoutDesc.push_back(elementDesc);
	}

	// Try to create Input Layout
	HRESULT hr = pD3DDevice->CreateInputLayout(&inputLayoutDesc[0], inputLayoutDesc.size(), pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), pInputLayout);

	//Free allocation shader reflection memory
	pVertexShaderReflection->Release();
	return hr;
}