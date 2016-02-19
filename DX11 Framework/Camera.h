#pragma once

#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include "resource.h"
using namespace DirectX;
class Camera
{
private:
	XMFLOAT4 _eye;
	XMFLOAT4 _at;
	XMFLOAT4 _up;

	FLOAT _windowWidth;
	FLOAT _windowHeight;
	FLOAT _nearDepth;
	FLOAT _farDepth;

	bool _lookTo;

	XMFLOAT4X4 _view;
	XMFLOAT4X4 _projection;

public:
	Camera(XMFLOAT4 eye, XMFLOAT4 at, XMFLOAT4 up, FLOAT windowWidth, FLOAT windowHeight, FLOAT nearDepth, FLOAT farDepth, bool LookTo);
	~Camera();

	XMFLOAT4X4 GetView();
	XMFLOAT4X4 GetProjection();

	XMFLOAT4 GetEye();
	XMFLOAT4 GetAt();
	XMFLOAT4 GetUp();

	void SetEye(XMFLOAT4 eye);
	void SetAt(XMFLOAT4 at);
	void SetUp(XMFLOAT4 up);

};