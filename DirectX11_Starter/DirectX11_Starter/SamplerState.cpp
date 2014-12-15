#include "SamplerState.h"
#include "Global.h"
SamplerState::SamplerState(ID3D11SamplerState *&sample){
	sampler = sample;
	test = 0;
}

SamplerState::~SamplerState(void){
	ReleaseMacro(sampler);
}

/**
*Create a Sampler State for textures
*D3D11_TEXTURE_ADDRESS_MODE : mode for texture coordinates
*D3D11_FILTER : filter mode of sampling texture
*MinLOD: Lower end of mipmap range
*MaxLOD: Upper end of mipmap range
*MipLODBias: Offset from the calculated mipmap level
*MaxAnisotropy: Clamping value used (if filter is D3D11_FILTER_ANISOTROPIC or D3D11_FILTER_COMPARISON_ANISOTROPIC)
**/
void SamplerState::createSamplerState(ID3D11Device* dev, D3D11_TEXTURE_ADDRESS_MODE mode, D3D11_FILTER filter, float MaxLOD, float MinLOD, float MipLODBias, UINT MaxAnisotropy){
	D3D11_SAMPLER_DESC desc;
	desc.AddressU = mode;
	desc.AddressV = mode;
	desc.AddressW = mode;
	desc.Filter = filter;
	desc.MaxLOD = MaxLOD;
	desc.MinLOD = MinLOD;
	desc.MipLODBias = MipLODBias;
	desc.MaxAnisotropy = MaxAnisotropy;
	dev->CreateSamplerState(&desc, &sampler);
}

/**
*Create a Sample State for textures
*Overload with default float values
**/
void SamplerState::createSamplerState(ID3D11Device* dev, D3D11_TEXTURE_ADDRESS_MODE mode, D3D11_FILTER filter){
	createSamplerState(dev, mode, filter, 0, 0, 0, 0);
}

/**
*Create a Sample State for textures
*Overload with default float values and default filter and texture mode settings
**/
void SamplerState::createSamplerState(ID3D11Device* dev){
	createSamplerState(dev, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_FILTER_ANISOTROPIC, 0, 0, 0, 0);
}

ID3D11SamplerState* SamplerState::getSamplerState()
{
	return sampler;
}

int SamplerState::getTest()
{
	return test;
}