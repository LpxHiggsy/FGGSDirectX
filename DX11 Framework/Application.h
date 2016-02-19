#pragma once

#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include <vector>
#include "resource.h"
#include "DDSTextureLoader.h"
#include "Camera.h"
#include "Structures.h"
#include "OBJLoader.h"
#include "GameObject.h"
#define WORLDCOUNT 12
using namespace DirectX;

class Application
{
private:
	HINSTANCE               _hInst;
	HWND                    _hWnd;
	D3D_DRIVER_TYPE         _driverType;
	D3D_FEATURE_LEVEL       _featureLevel;
	ID3D11Device*           _pd3dDevice;
	ID3D11DeviceContext*    _pImmediateContext;
	IDXGISwapChain*         _pSwapChain;
	ID3D11RenderTargetView* _pRenderTargetView;
	ID3D11VertexShader*     _pVertexShader;
	ID3D11PixelShader*      _pPixelShader;
	ID3D11InputLayout*      _pVertexLayout;
	ID3D11ShaderResourceView* _pTextureRV = nullptr;
	ID3D11ShaderResourceView* _pTextureRV2 = nullptr;
	ID3D11ShaderResourceView* _pTextureRV3 = nullptr;
	ID3D11SamplerState* _pSamplerLinear = nullptr;
	ID3D11BlendState*		Transparency;
	MeshData				objMesh;
	MeshData				HouseObj;
	MeshData				HouseObj2;
	MeshData				HouseObj3;
	MeshData				HouseObj4;
	ID3D11Buffer*           _pVertexBuffer;
	ID3D11Buffer*           _pIndexBuffer;
	ID3D11Buffer*           _pVertexBuffer2;
	ID3D11Buffer*           _pIndexBuffer2;
	ID3D11Buffer*           _pVertexBuffer3;
	ID3D11Buffer*           _pIndexBuffer3;
	ID3D11Buffer*           _pVertexBuffer4;
	ID3D11Buffer*           _pIndexBuffer4;
	ID3D11Buffer*           _pConstantBuffer;
	XMFLOAT4X4				_worldArray[WORLDCOUNT];
	XMFLOAT4X4              _view;
	XMFLOAT4X4              _projection;
	XMFLOAT4				diffuseMaterial;
	XMFLOAT4				diffuseLight;
	XMFLOAT4				ambientMaterial;
	XMFLOAT4				ambientLight;
	XMFLOAT4				specularMaterial;
	XMFLOAT4				specularLight;
	XMFLOAT3				lightDirection;
	float					specularPower;
	XMFLOAT3				EyePos;
	float					_time;
	XMFLOAT3				eyePostion;
	float					zPos = 0;
	float					xPos = 0;
	Camera*					camera;
	Camera*					camera2;
	Camera*					camera3;
	bool					_wireFrameTrue;
	bool					_Blending;
	GameObject*				plane;

private:
	HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);
	HRESULT InitDevice();
	void Cleanup();
	HRESULT CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);
	HRESULT InitShadersAndInputLayout();
	HRESULT InitVertexBuffer();
	HRESULT InitIndexBuffer();
	HRESULT InitVertexBufferPyramid();
	HRESULT InitIndexBufferPyramid();
	HRESULT InitVertexBufferPlane();
	HRESULT InitIndexBufferPlane();
	HRESULT InitVertexBufferPlane2();
	HRESULT InitIndexBufferPlane2();

	ID3D11DepthStencilView* _depthStencilView;
	ID3D11Texture2D* _depthStencilBuffer;

	ID3D11RasterizerState* _wireFrame;
	ID3D11RasterizerState* _solid;

	UINT _WindowHeight;
	UINT _WindowWidth;
	

public:
	Application();
	~Application();

	HRESULT Initialise(HINSTANCE hInstance, int nCmdShow);

	void Update();
	void Draw();
};

