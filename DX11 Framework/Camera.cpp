#include "Camera.h"

Camera::Camera(XMFLOAT4 eye, XMFLOAT4 at, XMFLOAT4 up, FLOAT windowWidth, FLOAT windowHeight, FLOAT nearDepth, FLOAT farDepth, bool LookTo)
{
	_eye = eye;
	_at = at;
	_up = up;
	_windowWidth = windowWidth;
	_windowHeight = windowHeight;
	_nearDepth = nearDepth;
	_farDepth = farDepth;
	_lookTo = LookTo;
	
}

Camera::~Camera()
{
}

XMFLOAT4X4 Camera::GetView()
{	
	// Initialize the view matrix
	XMVECTOR Eye = XMLoadFloat4(&_eye);
	XMVECTOR At = XMLoadFloat4(&_at);
	XMVECTOR Up = XMLoadFloat4(&_up);
		
	if (_lookTo = true)
	{
		XMStoreFloat4x4(&_view, XMMatrixLookToLH(Eye, At, Up));
	}
	else
	{
		XMStoreFloat4x4(&_view, XMMatrixLookAtLH(Eye, At, Up));
	}
	
	return _view;
}
XMFLOAT4X4 Camera::GetProjection()
{
	// Initialize the projection matrix
	XMStoreFloat4x4(&_projection, XMMatrixPerspectiveFovLH(XM_PIDIV2, _windowWidth / _windowHeight, _nearDepth, _farDepth));

	return _projection;
}
XMFLOAT4 Camera::GetEye()
{
	return _eye;
}
XMFLOAT4 Camera::GetAt()
{
	return _at;
}
XMFLOAT4 Camera::GetUp()
{
	return _up;
}
void Camera::SetEye(XMFLOAT4 eye)
{
	_eye = eye;
}
void Camera::SetAt(XMFLOAT4 at)
{
	_at = at;
}
void Camera::SetUp(XMFLOAT4 up)
{
	_up = up;
}
