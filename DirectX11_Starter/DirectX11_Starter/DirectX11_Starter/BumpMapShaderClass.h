//////////////////////////////////////////////////////////////////////////////////
//// Filename: bumpmapshaderclass.h
//////////////////////////////////////////////////////////////////////////////////
//#ifndef _BUMPMAPSHADERCLASS_H_
//#define _BUMPMAPSHADERCLASS_H_
//
////////////////
//// INCLUDES //
////////////////
//#include <DirectXMath.h>
//#include <d3d11.h>
//#include <d3dcompiler.h>
//#include <d3dcompiler.inl>
//#include <fstream>
//using namespace std;
//using namespace DirectX;
//
//
//////////////////////////////////////////////////////////////////////////////////
//// Class name: BumpMapShaderClass
//////////////////////////////////////////////////////////////////////////////////
//class BumpMapShaderClass
//{
//private:
//	struct MatrixBufferType
//	{
//		XMMATRIX world;
//		XMMATRIX view;
//		XMMATRIX projection;
//	};
//
//	struct LightBufferType
//	{
//		XMFLOAT4 diffuseColor;
//		XMFLOAT3 lightDirection;
//		float padding;
//	};
//
//public:
//	BumpMapShaderClass();
//	BumpMapShaderClass(const BumpMapShaderClass&);
//	~BumpMapShaderClass();
//
//	bool Initialize(ID3D11Device*, HWND);
//	void Shutdown();
//	bool Render(ID3D11DeviceContext*, int, XMMATRIX, XMMATRIX, XMMATRIX, ID3D11ShaderResourceView**, XMFLOAT3,
//		XMFLOAT4);
//
//private:
//	bool InitializeShader(ID3D11Device*, HWND, WCHAR*, WCHAR*);
//	void ShutdownShader();
//	void OutputShaderErrorMessage(ID3D10Blob*, HWND, WCHAR*);
//
//	bool SetShaderParameters(ID3D11DeviceContext*, XMMATRIX, XMMATRIX, XMMATRIX, ID3D11ShaderResourceView**, XMFLOAT3,
//		XMFLOAT4);
//	void RenderShader(ID3D11DeviceContext*, int);
//
//private:
//	ID3D11VertexShader* m_vertexShader;
//	ID3D11PixelShader* m_pixelShader;
//	ID3D11InputLayout* m_layout;
//	ID3D11Buffer* m_matrixBuffer;
//	ID3D11SamplerState* m_sampleState;
//
////The bump map shader will require a constant buffer to interface with the light direction and light color.
//
//	ID3D11Buffer* m_lightBuffer;
//};
//
//#endif