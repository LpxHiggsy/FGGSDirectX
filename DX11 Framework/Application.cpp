#include "Application.h"

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;

    switch (message)
    {
        case WM_PAINT:
            hdc = BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

Application::Application()
{
	_hInst = nullptr;
	_hWnd = nullptr;
	_driverType = D3D_DRIVER_TYPE_NULL;
	_featureLevel = D3D_FEATURE_LEVEL_11_0;
	_pd3dDevice = nullptr;
	_pImmediateContext = nullptr;
	_pSwapChain = nullptr;
	_pRenderTargetView = nullptr;
	_pVertexShader = nullptr;
	_pPixelShader = nullptr;
	_pVertexLayout = nullptr;
	_pVertexBuffer = nullptr;
	_pIndexBuffer = nullptr;
	_pVertexBuffer2 = nullptr;
	_pIndexBuffer2 = nullptr;
	_pVertexBuffer3 = nullptr;
	_pIndexBuffer3 = nullptr;
	_pVertexBuffer4 = nullptr;
	_pIndexBuffer4 = nullptr;
	objMesh.VertexBuffer = nullptr;
	objMesh.IndexBuffer = nullptr;
	_pConstantBuffer = nullptr;
	_depthStencilView = nullptr;
	_depthStencilBuffer = nullptr;

}

Application::~Application()
{
	Cleanup();
}

HRESULT Application::Initialise(HINSTANCE hInstance, int nCmdShow)
{
    if (FAILED(InitWindow(hInstance, nCmdShow)))
	{
        return E_FAIL;
	}

    RECT rc;
    GetClientRect(_hWnd, &rc);
    _WindowWidth = rc.right - rc.left;
    _WindowHeight = rc.bottom - rc.top;

    if (FAILED(InitDevice()))
    {
        Cleanup();

        return E_FAIL;
    }

	 //Initialize the world matrix
	
	for (int i = 0; i < WORLDCOUNT; i++)
	{
		XMStoreFloat4x4(&_worldArray[i], XMMatrixIdentity());
	}

    // Initialize the view matrix
	XMFLOAT4 Eye = XMFLOAT4(0.0f, 0.0f, -5.0f, 0.0f);
	XMFLOAT4 At = XMFLOAT4(0.0f, 0.0f, 1.0f, 0.0f);
	XMFLOAT4 Up = XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f);

	camera = new Camera(Eye, At, Up, _WindowWidth, _WindowHeight, 0.01f, 100.0f, false);
	_view = camera->GetView();
	_projection = camera->GetProjection();

	XMFLOAT4 Eye2 = XMFLOAT4(0.0f, 0.0f, 3.0f, 0.0f);
	XMFLOAT4 At2 = XMFLOAT4(0.0f, 0.0f, -1.0f, 0.0f);
	XMFLOAT4 Up2 = XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f);

	camera2 = new Camera(Eye2, At2, Up2, _WindowWidth, _WindowHeight, 0.01f, 100.0f, true);

	XMFLOAT4 Eye3 = XMFLOAT4(0.0f, 3.0f, 0.0f, 0.0f);
	XMFLOAT4 At3 = XMFLOAT4(0.0f, -1.0f, 0.0f, 0.0f);
	XMFLOAT4 Up3 = XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f);

	camera3 = new Camera(Eye3, At3, Up3, _WindowWidth, _WindowHeight, 0.01f, 100.0f, false);

	eyePostion = XMFLOAT3(Eye.x, Eye.y, Eye.z);

	// Light direction from surface (XYZ)
	lightDirection = XMFLOAT3(0.25f, 0.5f, -1.0f);
	// Diffuse material properties (RGBA)
	diffuseMaterial = XMFLOAT4(0.8f, 0.5f, 0.5f, 1.0f);
	// Diffuse light colour (RGBA)
	diffuseLight = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	// Ambient material properties (RGBA)
	ambientMaterial = XMFLOAT4(0.2f, 0.2f, 0.2f, 0.2f);
	// Ambient light colour (RGBA)
	ambientLight = XMFLOAT4(0.2f, 0.2f, 0.2f, 0.2f);
	// Specular material properties (RGBA)
	specularMaterial= XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	// Specular light colour (RGBA)
	specularLight = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	// Specular Power
	specularPower = 10.0f;
	
	objMesh = OBJLoader::Load("Hercules.obj", _pd3dDevice);
	HouseObj = OBJLoader::Load("House.obj", _pd3dDevice);
	HouseObj2 = OBJLoader::Load("House.obj", _pd3dDevice);
	HouseObj3 = OBJLoader::Load("House.obj", _pd3dDevice);
	HouseObj4 = OBJLoader::Load("House.obj", _pd3dDevice);
	//GameObject* plane = new GameObject();
	//plane->Initialise(planeObj);

	CreateDDSTextureFromFile(_pd3dDevice, L"Pine Tree.dds", nullptr, &_pTextureRV);
	CreateDDSTextureFromFile(_pd3dDevice, L"Hercules_COLOR.dds", nullptr, &_pTextureRV2);
	CreateDDSTextureFromFile(_pd3dDevice, L"Crate_COLOR.dds", nullptr, &_pTextureRV3);


	return S_OK;
}

HRESULT Application::InitShadersAndInputLayout()
{
	HRESULT hr;

    // Compile the vertex shader
    ID3DBlob* pVSBlob = nullptr;
    hr = CompileShaderFromFile(L"DX11 Framework.fx", "VS", "vs_4_0", &pVSBlob);

    if (FAILED(hr))
    {
        MessageBox(nullptr,
                   L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

	// Create the vertex shader
	hr = _pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &_pVertexShader);

	if (FAILED(hr))
	{	
		pVSBlob->Release();
        return hr;
	}

	// Compile the pixel shader
	ID3DBlob* pPSBlob = nullptr;
    hr = CompileShaderFromFile(L"DX11 Framework.fx", "PS", "ps_4_0", &pPSBlob);

    if (FAILED(hr))
    {
        MessageBox(nullptr,
                   L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

	// Create the pixel shader
	hr = _pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &_pPixelShader);
	pPSBlob->Release();

    if (FAILED(hr))
        return hr;

    // Define the input layout
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	UINT numElements = ARRAYSIZE(layout);

	

    // Create the input layout
	hr = _pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(),
                                        pVSBlob->GetBufferSize(), &_pVertexLayout);
	pVSBlob->Release();

	if (FAILED(hr))
        return hr;

    // Set the input layout
    _pImmediateContext->IASetInputLayout(_pVertexLayout);

	return hr;
}

HRESULT Application::InitVertexBuffer()
{
	HRESULT hr;

    // Create vertex buffer
    SimpleVertex vertices[] =
    {
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f),		XMFLOAT3(-1.0f, 1.0f, -1.0f),	XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.0f, 1.0f, -1.0f),		XMFLOAT3(1.0f, 1.0f, -1.0f),	XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f),	XMFLOAT3(-1.0f, -1.0f, -1.0f),	XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, -1.0f),		XMFLOAT3(1.0f, -1.0f, -1.0f),	XMFLOAT2(1.0f, 1.0f) },

		{ XMFLOAT3(-1.0f, 1.0f, 1.0f),		XMFLOAT3(-1.0f, 1.0f, 1.0f),	XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 1.0f),		XMFLOAT3(1.0f, 1.0f, 1.0f),		XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, 1.0f),		XMFLOAT3(-1.0f, -1.0f, 1.0f),	XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, 1.0f),		XMFLOAT3(1.0f, -1.0f, 1.0f),	XMFLOAT2(1.0f, 1.0f) },
    };

    D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(SimpleVertex) * 8;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = vertices;

    hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pVertexBuffer);

    if (FAILED(hr))
        return hr;

	return S_OK;
}

HRESULT Application::InitVertexBufferPyramid()
{
	HRESULT hr;

	// Create vertex buffer
	SimpleVertex vertices[] =
	{
		// base
		{ XMFLOAT3(-1.0f, 0.0f, 1.0f),	XMFLOAT3(-1.0f, 0.0f, 1.0f),	XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.0f, 0.0f, 1.0f),	XMFLOAT3(1.0f, 0.0f, 1.0f),		XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, 0.0f, -1.0f), XMFLOAT3(-1.0f, 0.0f, -1.0f),	XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, 0.0f, -1.0f),	XMFLOAT3(1.0f, 0.0f, -1.0f),	XMFLOAT2(1.0f, 1.0f) },
		// top
		{ XMFLOAT3(0.0f, 4.0f, 0.0f),	XMFLOAT3(0.0f, 4.0f, 0.0f),		XMFLOAT2(0.0f, 0.0f) },
	};

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(SimpleVertex) * 5;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = vertices;

	hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pVertexBuffer2);

	if (FAILED(hr))
		return hr;

	return S_OK;
}


HRESULT Application::InitVertexBufferPlane()
{
	HRESULT hr;

	// Create vertex buffer
	SimpleVertex vertices[] =
	{
		{ XMFLOAT3(-2.0f, 2.0f, 0.0f),	XMFLOAT3(0.0f, 0.0f, -1.0f),	XMFLOAT2(0.0f, 0.0f) },		// 1
		{ XMFLOAT3(-1.0f, 2.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },		// 2
		{ XMFLOAT3(-2.0f, 1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },		// 3
		{ XMFLOAT3(-1.0f, -1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },		// 4
		{ XMFLOAT3(0.0f, 2.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },		// 5
		
		{ XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },		// 6
		{ XMFLOAT3(1.0f, 2.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },		// 7
		{ XMFLOAT3(1.0f, 1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },		// 8
		{ XMFLOAT3(2.0f, 2.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },		// 9
		{ XMFLOAT3(2.0f, 1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },		// 10
		
		{ XMFLOAT3(-2.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },		// 11
		{ XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },		// 12
		{ XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },		// 13
		{ XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },		// 14
		{ XMFLOAT3(2.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },		// 15
		
		{ XMFLOAT3(-2.0f, -1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },		// 16
		{ XMFLOAT3(-1.0f, -1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },		// 17
		{ XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },		// 18
		{ XMFLOAT3(1.0f, -1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },		// 19
		{ XMFLOAT3(2.0f, -1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },		// 20
		
		{ XMFLOAT3(-2.0f, -2.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },		// 21
		{ XMFLOAT3(-1.0f, -2.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },		// 22
		{ XMFLOAT3(0.0f, -2.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },		// 23
		{ XMFLOAT3(1.0f, -2.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(9.0f, 0.0f) },		// 24
		{ XMFLOAT3(2.0f, -2.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },		// 25
	};

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(SimpleVertex) * 25;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = vertices;

	hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pVertexBuffer3);

	if (FAILED(hr))
		return hr;

	return S_OK;
}
HRESULT Application::InitVertexBufferPlane2()
{
	HRESULT hr;

	// Create vertex buffer
	SimpleVertex vertices[] =
	{
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) },
	};

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(SimpleVertex) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = vertices;

	hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pVertexBuffer4);

	if (FAILED(hr))
		return hr;

	return S_OK;
}
HRESULT Application::InitIndexBuffer()
{
	HRESULT hr;

    // Create index buffer
    WORD indices[] =
    {
        0, 1, 2,
        2, 1, 3,
		1, 5, 3,
		3, 5, 7,
		4, 0, 6,
		6, 0, 2,
		5, 4, 7,
		7, 4, 6,
		4, 5, 0,
		0, 5, 1,
		2, 3, 6,
		6, 3, 7,
    };

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));

    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(WORD) * 36;     
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = indices;
    hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pIndexBuffer);

    if (FAILED(hr))
        return hr;

	return S_OK;
}

HRESULT Application::InitIndexBufferPyramid()
{
	HRESULT hr;

	// Create index buffer
	WORD indices[] =
	{
		0, 2, 1,    // 
		1, 2, 3,
		0, 1, 4,    
		1, 3, 4,
		3, 2, 4,
		2, 0, 4,
	};

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));

	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(WORD) * 18;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = indices;
	hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pIndexBuffer2);

	if (FAILED(hr))
		return hr;

	return S_OK;
}
HRESULT Application::InitIndexBufferPlane()
{
	HRESULT hr;

	// Create index buffer
	WORD indices[] =
	{
		0,	1,	2,
		2,	1,	3,
		1,	4,	3,
		3,	4,	5,
		4,	6,	5,
		5,	6,	7,
		6,	8,	7,
		7,	8,	9,
		2,	3,	10,
		10, 3,	11,
		3,	5,	11,
		11,	5,	12,
		5,	7,	12,
		12,	7,	13,
		7,	9,	13,
		13,	9,	14,
		10,	11,	15,
		15,	11,	16,
		11,	12,	16,
		16,	12,	17,
		12,	13,	17,
		17,	13,	18,
		13,	14,	18,
		18,	14,	19,
		15,	16,	20,
		20,	16,	21,
		16,	17,	21,
		21,	17,	22,
		17,	18,	22,
		22,	18,	23,
		18,	19,	23,
		23,	19,	24,
	};

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));

	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(WORD) * 96;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = indices;
	hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pIndexBuffer3);

	if (FAILED(hr))
		return hr;

	return S_OK;
}
HRESULT Application::InitIndexBufferPlane2()
{
	HRESULT hr;

	// Create index buffer
	WORD indices[] =
	{
		0, 1, 2,
		2, 1, 3,
	};

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));

	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(WORD) * 6;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = indices;
	hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pIndexBuffer4);

	if (FAILED(hr))
		return hr;

	return S_OK;
}
HRESULT Application::InitWindow(HINSTANCE hInstance, int nCmdShow)
{
    // Register class
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, (LPCTSTR)IDI_TUTORIAL1);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW );
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = L"TutorialWindowClass";
    wcex.hIconSm = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_TUTORIAL1);
    if (!RegisterClassEx(&wcex))
        return E_FAIL;

    // Create window
    _hInst = hInstance;
    RECT rc = {0, 0, 640, 480};
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    _hWnd = CreateWindow(L"TutorialWindowClass", L"DX11 Framework", WS_OVERLAPPEDWINDOW,
                         CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
                         nullptr);
    if (!_hWnd)
		return E_FAIL;

    ShowWindow(_hWnd, nCmdShow);

    return S_OK;
}


HRESULT Application::CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
    HRESULT hr = S_OK;

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

    ID3DBlob* pErrorBlob;
    hr = D3DCompileFromFile(szFileName, nullptr, nullptr, szEntryPoint, szShaderModel, 
        dwShaderFlags, 0, ppBlobOut, &pErrorBlob);

    if (FAILED(hr))
    {
        if (pErrorBlob != nullptr)
            OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());

        if (pErrorBlob) pErrorBlob->Release();

        return hr;
    }

    if (pErrorBlob) pErrorBlob->Release();

    return S_OK;
}

HRESULT Application::InitDevice()
{
    HRESULT hr = S_OK;

    UINT createDeviceFlags = 0;

#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_DRIVER_TYPE driverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };

    UINT numDriverTypes = ARRAYSIZE(driverTypes);

    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };

	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 1;
    sd.BufferDesc.Width = _WindowWidth;
    sd.BufferDesc.Height = _WindowHeight;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = _hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;

    for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
    {
        _driverType = driverTypes[driverTypeIndex];
        hr = D3D11CreateDeviceAndSwapChain(nullptr, _driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
                                           D3D11_SDK_VERSION, &sd, &_pSwapChain, &_pd3dDevice, &_featureLevel, &_pImmediateContext);
        if (SUCCEEDED(hr))
            break;
    }

    if (FAILED(hr))
        return hr;

    // Create a render target view
    ID3D11Texture2D* pBackBuffer = nullptr;
    hr = _pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

    if (FAILED(hr))
        return hr;

    hr = _pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &_pRenderTargetView);
    pBackBuffer->Release();

    if (FAILED(hr))
        return hr;

	D3D11_TEXTURE2D_DESC depthStencilDesc;

	depthStencilDesc.Width = _WindowWidth;
	depthStencilDesc.Height = _WindowHeight;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	_pd3dDevice->CreateTexture2D(&depthStencilDesc, nullptr, &_depthStencilBuffer);
	_pd3dDevice->CreateDepthStencilView(_depthStencilBuffer, nullptr, &_depthStencilView);

	_pImmediateContext->OMSetRenderTargets(1, &_pRenderTargetView, _depthStencilView);

    // Setup the viewport
    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)_WindowWidth;
    vp.Height = (FLOAT)_WindowHeight;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    _pImmediateContext->RSSetViewports(1, &vp);

	InitShadersAndInputLayout();

	InitVertexBuffer();
	InitVertexBufferPyramid();
	InitVertexBufferPlane();
	InitVertexBufferPlane2();

	InitIndexBuffer();
	InitIndexBufferPyramid();
	InitIndexBufferPlane();
	InitIndexBufferPlane2();

    // Set primitive topology
    _pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Create the constant buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
    hr = _pd3dDevice->CreateBuffer(&bd, nullptr, &_pConstantBuffer);
	
	//Draw in wireframe
	D3D11_RASTERIZER_DESC wfdesc;
	ZeroMemory(&wfdesc, sizeof(D3D11_RASTERIZER_DESC));
	wfdesc.FillMode = D3D11_FILL_WIREFRAME;
	wfdesc.CullMode = D3D11_CULL_NONE;
	hr = _pd3dDevice->CreateRasterizerState(&wfdesc, &_wireFrame);

	D3D11_RASTERIZER_DESC wsdesc;
	ZeroMemory(&wsdesc, sizeof(D3D11_RASTERIZER_DESC));
	wsdesc.FillMode = D3D11_FILL_SOLID;
	wsdesc.CullMode = D3D11_CULL_NONE;
	hr = _pd3dDevice->CreateRasterizerState(&wsdesc, &_solid);
	

	// Create the sample state
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	_pd3dDevice->CreateSamplerState(&sampDesc, &_pSamplerLinear);

	
	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(blendDesc));

	D3D11_RENDER_TARGET_BLEND_DESC rtbd;
	ZeroMemory(&rtbd, sizeof(rtbd));

	rtbd.BlendEnable = true;
	rtbd.SrcBlend = D3D11_BLEND_SRC_COLOR;
	rtbd.DestBlend = D3D11_BLEND_BLEND_FACTOR;
	rtbd.BlendOp = D3D11_BLEND_OP_ADD;
	rtbd.SrcBlendAlpha = D3D11_BLEND_ONE;
	rtbd.DestBlendAlpha = D3D11_BLEND_ZERO;
	rtbd.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	rtbd.RenderTargetWriteMask = D3D10_COLOR_WRITE_ENABLE_ALL;

	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.RenderTarget[0] = rtbd;

	_pd3dDevice->CreateBlendState(&blendDesc, &Transparency);

    if (FAILED(hr))
        return hr;

    return S_OK;
}

void Application::Cleanup()
{
    if (_pImmediateContext) _pImmediateContext->ClearState();
    if (_pConstantBuffer) _pConstantBuffer->Release();
    if (_pVertexBuffer) _pVertexBuffer->Release();
    if (_pIndexBuffer) _pIndexBuffer->Release();
	if (_pVertexBuffer2) _pVertexBuffer2->Release();
	if (_pIndexBuffer2) _pIndexBuffer2->Release();
	if (_pVertexBuffer3) _pVertexBuffer3->Release();
	if (_pIndexBuffer3) _pIndexBuffer3->Release();
	if (_pVertexBuffer4) _pVertexBuffer4->Release();
	if (_pIndexBuffer4) _pIndexBuffer4->Release();
	if (objMesh.VertexBuffer) objMesh.VertexBuffer->Release();
	if (objMesh.IndexBuffer) objMesh.IndexBuffer->Release();
    if (_pVertexLayout) _pVertexLayout->Release();
    if (_pVertexShader) _pVertexShader->Release();
    if (_pPixelShader) _pPixelShader->Release();
    if (_pRenderTargetView) _pRenderTargetView->Release();
    if (_pSwapChain) _pSwapChain->Release();
    if (_pImmediateContext) _pImmediateContext->Release();
    if (_pd3dDevice) _pd3dDevice->Release();
	if (_depthStencilView) _depthStencilView->Release();
	if (_depthStencilBuffer) _depthStencilBuffer->Release();
	if (_wireFrame) _wireFrame->Release();
	if (_solid) _solid->Release();
	if (Transparency) Transparency->Release();
}


void Application::Update()
{
    // Update our time
    static float t = 0.0f;

    if (_driverType == D3D_DRIVER_TYPE_REFERENCE)
    {
        t += (float) XM_PI * 0.0125f;
    }
    else
    {
        static DWORD dwTimeStart = 0;
        DWORD dwTimeCur = GetTickCount();

        if (dwTimeStart == 0)
            dwTimeStart = dwTimeCur;

        t = (dwTimeCur - dwTimeStart) / 1000.0f;
    }

	if (GetAsyncKeyState(VK_NUMPAD9))
	{
		_wireFrameTrue = true;
	}
	else
	{
		_wireFrameTrue = false;
	}	
	
	if (GetAsyncKeyState(VK_NUMPAD1) & 1)
	{
		_view = camera->GetView();
		_projection = camera->GetProjection();
	}
	if (GetAsyncKeyState(VK_NUMPAD2) & 1)
	{
		_view = camera2->GetView();
		_projection = camera2->GetProjection();

	}
	if (GetAsyncKeyState(VK_NUMPAD3) & 1)
	{
		_view = camera3->GetView();
		_projection = camera3->GetProjection();
	}


		//Lookat camera controls
		if (GetAsyncKeyState(VK_NUMPAD8))
		{
			XMFLOAT4 currentPos = camera->GetEye();
			currentPos.z += 0.001;
			camera->SetEye(currentPos);
			_view = camera->GetView();
			_projection = camera->GetProjection();

		}
		if (GetAsyncKeyState(VK_NUMPAD5))
		{
			XMFLOAT4 currentPos = camera->GetEye();
			currentPos.z -= 0.001;
			camera->SetEye(currentPos);
			_view = camera->GetView();
			_projection = camera->GetProjection();

		}
		if (GetAsyncKeyState(VK_NUMPAD4))
		{
			XMFLOAT4 currentPos = camera->GetEye();
			currentPos.z += 0.001;
			currentPos.x -= 0.001;
			camera->SetEye(currentPos);
			_view = camera->GetView();
			_projection = camera->GetProjection();

		}
		if (GetAsyncKeyState(VK_NUMPAD6))
		{
			XMFLOAT4 currentPos = camera->GetEye();
			currentPos.x += 0.001;
			currentPos.z += 0.001;
			camera->SetEye(currentPos);
			_view = camera->GetView();
			_projection = camera->GetProjection();
		}
	

	//lookTo
		if (GetAsyncKeyState(VK_LEFT))
		{
			xPos += 0.001;
			XMStoreFloat4x4(&_worldArray[6], XMMatrixScaling(0.1f, 0.1f, 0.1f) * XMMatrixTranslation(xPos, -1.0f, zPos));

			XMFLOAT4 currentPos = camera2->GetEye();
			currentPos.x += 0.001;
			camera2->SetEye(currentPos);

			_view = camera2->GetView();
			_projection = camera2->GetProjection();
		}
		if (GetAsyncKeyState(VK_RIGHT))
		{
			xPos -= 0.001;
			XMStoreFloat4x4(&_worldArray[6], XMMatrixScaling(0.1f, 0.1f, 0.1f) * XMMatrixTranslation(xPos, -1.0f, zPos));
			
			XMFLOAT4 currentPos = camera2->GetEye();
			currentPos.x -= 0.001;
			camera2->SetEye(currentPos);
	
			_view = camera2->GetView();
			_projection = camera2->GetProjection();
		}
		//XMStoreFloat4x4(&_worldArray[6], XMMatrixScaling(0.1f, 0.1f, 0.1f));
		if (GetAsyncKeyState(VK_UP))
		{
			zPos -= 0.001;
			XMStoreFloat4x4(&_worldArray[6], XMMatrixScaling(0.1f, 0.1f, 0.1f)* XMMatrixTranslation(xPos, -1.0f, zPos));

			XMFLOAT4 currentPos = camera2->GetEye();
			currentPos.z -= 0.001;
			camera2->SetEye(currentPos);
			_view = camera2->GetView();
			_projection = camera2->GetProjection();
		}
		if (GetAsyncKeyState(VK_DOWN))
		{
			zPos += 0.001;
			XMStoreFloat4x4(&_worldArray[6], XMMatrixScaling(0.1f, 0.1f, 0.1f)* XMMatrixTranslation(xPos, -1.0f, zPos));

			XMFLOAT4 currentPos = camera2->GetEye();
			currentPos.z += 0.001;
			camera2->SetEye(currentPos);
			_view = camera2->GetView();
			_projection = camera2->GetProjection();
		}

		if (GetAsyncKeyState(VK_NUMPAD0) & 1)
		{
			_Blending = true;
		}

		/*plane->SetScale(0.1f, 0.01f, 0.1f);
		plane->SetTranslation(1.0f, 1.0f, 1.0f);
		plane->SetRotation(1.5708, 0, 0);
		plane->UpdateWorld();
		*/
   
    
	// Animate the objects
	XMStoreFloat4x4(&_worldArray[0], XMMatrixScaling(0.2f, 0.2f, 0.2f) * XMMatrixRotationY(t) * XMMatrixTranslation(0.0f, -2.5f, 0.0f));
	XMStoreFloat4x4(&_worldArray[1], XMMatrixScaling(0.1f, 0.1f, 0.1f) * XMMatrixRotationY(t) * XMMatrixTranslation(2.0f, -2.5f, 0.0f) * XMMatrixRotationY(t));
	XMStoreFloat4x4(&_worldArray[2], XMMatrixScaling(0.1f, 0.1f, 0.1f) * XMMatrixRotationY(t) * XMMatrixTranslation(-2.0f, -2.5f, 0.0f) * XMMatrixRotationY(t));
	XMStoreFloat4x4(&_worldArray[3], XMMatrixScaling(0.05f, 0.05f, 0.05f) * XMMatrixRotationY(t) * XMMatrixTranslation(3.0f, -2.5f, 0.0f) * XMMatrixRotationY(t));
	XMStoreFloat4x4(&_worldArray[4], XMMatrixScaling(0.05f, 0.05f, 0.05f) * XMMatrixRotationY(t) * XMMatrixTranslation(-3.0f, -2.5f, 0.0f) * XMMatrixRotationY(t));
	XMStoreFloat4x4(&_worldArray[5], XMMatrixScaling(3.5f, 2.5f, 3.5f) * XMMatrixRotationX(1.5708) * XMMatrixTranslation(0.0f, -3.0f, 0.0f));
	XMStoreFloat4x4(&_worldArray[7], XMMatrixScaling(0.5f, 0.5f, 0.5f) *   XMMatrixTranslation(0.0f, -3.0f, 0.0f));
	XMStoreFloat4x4(&_worldArray[8], XMMatrixScaling(0.2f, 0.2f, 0.2f) *  XMMatrixRotationX(4.71239) *	 XMMatrixTranslation(5.0f, -3.0f, 0.0f));
	XMStoreFloat4x4(&_worldArray[9], XMMatrixScaling(0.2f, 0.2f, 0.2f) *  XMMatrixRotationX(4.71239) *	XMMatrixRotationY(3.14159) *XMMatrixTranslation(-5.0f, -3.0f, 0.0f));
	XMStoreFloat4x4(&_worldArray[10], XMMatrixScaling(0.2f, 0.2f, 0.2f) *  XMMatrixRotationX(4.71239) *	XMMatrixRotationY(4.71239) * XMMatrixTranslation(0.0f, -3.0f, 4.0f));
	XMStoreFloat4x4(&_worldArray[11], XMMatrixScaling(0.2f, 0.2f, 0.2f) *  XMMatrixRotationX(4.71239) * XMMatrixRotationY(1.5708) * XMMatrixTranslation(0.0f, -3.0f, -4.0f));

	_time = t;


}

void Application::Draw()
{
    //
    // Clear the back buffer
    //
    float ClearColor[4] = {0.0f, 0.125f, 0.3f, 1.0f}; // red,green,blue,alpha
    _pImmediateContext->ClearRenderTargetView(_pRenderTargetView, ClearColor);
	
	_pImmediateContext->ClearDepthStencilView(_depthStencilView, 
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	
	XMMATRIX view = XMLoadFloat4x4(&_view);
	XMMATRIX projection = XMLoadFloat4x4(&_projection);

	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;

	XMMATRIX worldArr[WORLDCOUNT];

	for (int i = 0; i < WORLDCOUNT; i++)
	{
		worldArr[i] = XMLoadFloat4x4(&_worldArray[i]);
	}

	//Draw in wireFrame
	if (_wireFrameTrue == true)
	{
		_pImmediateContext->RSSetState(_wireFrame);
	}
	else
	{
		_pImmediateContext->RSSetState(_solid);
	}
	

    //
    // Update variables
    //
    ConstantBuffer cb;
	cb.LightVecW = lightDirection;
	cb.DiffuseMtrl = diffuseMaterial;
	cb.DiffuseLight = diffuseLight;
	cb.AmbientMtrl = ambientMaterial;
	cb.AmbientLight = ambientLight;
	cb.SpecularMtrl = specularMaterial;
	cb.SpecularLight = specularLight;
	cb.SpecularPower = specularPower;
	cb.EyePosW = EyePos;


	for (int i = 0; i < WORLDCOUNT; i++)
	{
		cb.mWorld = XMMatrixTranspose(worldArr[i]);
		cb.mView = XMMatrixTranspose(view);
		cb.mProjection = XMMatrixTranspose(projection);
		cb.mTime = _time;
		_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);

		//
		// Renders a triangle
		//
		_pImmediateContext->VSSetShader(_pVertexShader, nullptr, 0);
		_pImmediateContext->VSSetConstantBuffers(0, 1, &_pConstantBuffer);
		_pImmediateContext->PSSetShader(_pPixelShader, nullptr, 0);
		_pImmediateContext->PSSetConstantBuffers(0, 1, &_pConstantBuffer);
		_pImmediateContext->PSSetShaderResources(0, 1, &_pTextureRV);
		_pImmediateContext->PSSetSamplers(0, 1, &_pSamplerLinear);
	
		
		if (i == 5)
		{
			_pImmediateContext->IASetVertexBuffers(0, 1, &_pVertexBuffer3, &stride, &offset);
			_pImmediateContext->IASetIndexBuffer(_pIndexBuffer3, DXGI_FORMAT_R16_UINT, 0);
		}
		else if (i == 7)
		{
			_pImmediateContext->IASetVertexBuffers(0, 1, &_pVertexBuffer4, &stride, &offset);
			_pImmediateContext->IASetIndexBuffer(_pIndexBuffer4, DXGI_FORMAT_R16_UINT, 0);
		}
		else if (i == 6)
		{
			_pImmediateContext->PSSetShaderResources(0, 1, &_pTextureRV2);
			_pImmediateContext->IASetVertexBuffers(0, 1, &objMesh.VertexBuffer, &objMesh.VBStride, &objMesh.VBOffset);
			_pImmediateContext->IASetIndexBuffer(objMesh.IndexBuffer, DXGI_FORMAT_R16_UINT, 0);
		}
		else if (i == 8)
		{
			_pImmediateContext->PSSetShaderResources(0, 1, &_pTextureRV2);
			_pImmediateContext->IASetVertexBuffers(0, 1, &HouseObj.VertexBuffer, &HouseObj.VBStride, &HouseObj.VBOffset);
			_pImmediateContext->IASetIndexBuffer(HouseObj.IndexBuffer, DXGI_FORMAT_R16_UINT, 0);
		}
		else if (i == 9)
		{
			_pImmediateContext->PSSetShaderResources(0, 1, &_pTextureRV2);
			_pImmediateContext->IASetVertexBuffers(0, 1, &HouseObj2.VertexBuffer, &HouseObj2.VBStride, &HouseObj2.VBOffset);
			_pImmediateContext->IASetIndexBuffer(HouseObj2.IndexBuffer, DXGI_FORMAT_R16_UINT, 0);
		}
		
		else if (i == 0)
		{
			_pImmediateContext->PSSetShaderResources(0, 1, &_pTextureRV2);
			_pImmediateContext->IASetVertexBuffers(0, 1, &_pVertexBuffer2, &stride, &offset);
			_pImmediateContext->IASetIndexBuffer(_pIndexBuffer2, DXGI_FORMAT_R16_UINT, 0);
		}
		else if (i == 10)
		{
			_pImmediateContext->PSSetShaderResources(0, 1, &_pTextureRV3);
			_pImmediateContext->IASetVertexBuffers(0, 1, &HouseObj3.VertexBuffer, &HouseObj3.VBStride, &HouseObj3.VBOffset);
			_pImmediateContext->IASetIndexBuffer(HouseObj3.IndexBuffer, DXGI_FORMAT_R16_UINT, 0);
		}
		else if (i == 11)
		{
			_pImmediateContext->PSSetShaderResources(0, 1, &_pTextureRV3);
			_pImmediateContext->IASetVertexBuffers(0, 1, &HouseObj4.VertexBuffer, &HouseObj4.VBStride, &HouseObj4.VBOffset);
			_pImmediateContext->IASetIndexBuffer(HouseObj4.IndexBuffer, DXGI_FORMAT_R16_UINT, 0);
		}
		else
		{
			_pImmediateContext->PSSetShaderResources(0, 1, &_pTextureRV3);
			_pImmediateContext->IASetVertexBuffers(0, 1, &_pVertexBuffer, &stride, &offset);
			_pImmediateContext->IASetIndexBuffer(_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
		}

		if (i == 0)
			_pImmediateContext->DrawIndexed(18, 0, 0);
		else if (i == 5)
			_pImmediateContext->DrawIndexed(96, 0, 0);
		else if (i == 6)
			_pImmediateContext->DrawIndexed(objMesh.IndexCount, 0, 0);
		else if (i == 8)
			_pImmediateContext->DrawIndexed(HouseObj.IndexCount, 0, 0);
		else if (i == 9)
			_pImmediateContext->DrawIndexed(HouseObj2.IndexCount, 0, 0);
		else if (i == 10)
			_pImmediateContext->DrawIndexed(HouseObj3.IndexCount, 0, 0);
		else if (i == 11)
			_pImmediateContext->DrawIndexed(HouseObj4.IndexCount, 0, 0);
		else if (i == 7)
			_pImmediateContext->DrawIndexed(6, 0, 0);
		else
			_pImmediateContext->DrawIndexed(36, 0, 0);
	}
	
	//plane->Draw(_pd3dDevice, _pImmediateContext);
	
	if (_Blending == true)
	{
		// "fine-tune" the blending equation
		float blendFactor[] = { 0.75f, 0.75f, 0.75f, 1.0f };

		// Set the default blend state (no blending) for opaque objects
		_pImmediateContext->OMSetBlendState(0, 0, 0xffffffff);

		// Render opaque objects //

		// Set the blend state for transparent objects
		_pImmediateContext->OMSetBlendState(Transparency, blendFactor, 0xffffffff);
	}
	


    //
    // Present our back buffer to our front buffer
    //
    _pSwapChain->Present(0, 0);
}
