#ifndef _SAMPLERSTATE_H
#define _SAMPLERSTATE_H
#include <d3d11.h>

class SamplerState{
public:
	SamplerState(ID3D11SamplerState &sample);
	~SamplerState(void);
	void createSamplerState(ID3D11Device* dev, D3D11_TEXTURE_ADDRESS_MODE mode, D3D11_FILTER filter, float MaxLOD, float MinLOD, float MipLODBias, float MaxAnisotropy);
	void createSamplerState(ID3D11Device* dev, D3D11_TEXTURE_ADDRESS_MODE mode, D3D11_FILTER filter);
	void createSamplerState(ID3D11Device* dev);
private:
	ID3D11SamplerState* sampler;
};

#endif