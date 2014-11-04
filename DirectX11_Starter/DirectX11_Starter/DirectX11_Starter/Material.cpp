#include "Material.h"
#include "WICTextureLoader.h"
#include "Global.h"
/**
*Material Constructor for materials read/created
*rv: shader resource view of material
*sample: sampler state of material
**/
Material::Material(ID3D11ShaderResourceView* rv, ID3D11SamplerState* sample, ShaderProgram* s_program)
{
	samplerState = sample;
	shaderProgram = s_program;
	resourceView = rv;
}

/**
*Material Contructor that creates a texture from an image file
*dev: Device object
*devCtx: deviceContext object
*sampler: sampler state object
*filepath: address of image file
**/
Material::Material(ID3D11Device* dev, ID3D11DeviceContext* devCtx, ID3D11SamplerState* sampler, wchar_t* filepath, ShaderProgram* s_program){
	samplerState = sampler;
	shaderProgram = s_program;
	CreateWICTextureFromFile(dev, devCtx, filepath, 0, &resourceView, 0);
}

Material::~Material(void){
	ReleaseMacro(samplerState);
	ReleaseMacro(resourceView);
	ReleaseMacro(vsConstantBuffer);
	ReleaseMacro(psConstantBuffer);
	if (shaderProgram){
		delete shaderProgram;
		shaderProgram = nullptr;
	}
}
