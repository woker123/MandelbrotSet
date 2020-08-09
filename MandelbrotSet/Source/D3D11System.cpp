#include "D3D11System.h"
#include <assert.h>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")

std::shared_ptr<D3D11System> D3D11System::sm_instance;

void D3D11System::initD3D11System(HWND hwnd)
{
	sm_instance.reset(new D3D11System(hwnd));
}

D3D11System * D3D11System::getInstance()
{
	return &(*sm_instance);
}

D3D11System::D3D11System(HWND hwnd)
	:m_hwnd(hwnd)
{
	bool init_D3D11System =
		initD3D11System();
	assert(init_D3D11System);
}

D3D11System::~D3D11System()
{
}

void D3D11System::setViewport(int xPos, int yPos, int width, int height)
{
	D3D11_VIEWPORT vp = {};
	vp.Height = height;
	vp.MaxDepth = 1.0;
	vp.MinDepth = 0.0;
	vp.TopLeftX = xPos;
	vp.TopLeftY = yPos;
	vp.Width = width;
}

void D3D11System::clearRTVAndDSV(float r, float g, float b, float a, float depth, unsigned char stencil)
{
	float color[] = { r, g, b, a };	
	getContext()->ClearRenderTargetView(getBackBufferRTV().Get(), color);
	getContext()->ClearDepthStencilView(getBaseDSV().Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, depth, stencil);
}

bool D3D11System::createDeviceAndSwapChain()
{
	DXGI_SWAP_CHAIN_DESC scd = {};
	scd.BufferCount = 2;
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scd.BufferDesc.RefreshRate.Denominator = 1;
	scd.BufferDesc.RefreshRate.Numerator = 60;
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	scd.OutputWindow = m_hwnd;
	scd.SampleDesc.Count = 1;
	scd.SampleDesc.Quality = 0;
	scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	scd.Windowed = true;

	HRESULT result =
		D3D11CreateDeviceAndSwapChain(
			NULL,
			D3D_DRIVER_TYPE_HARDWARE,
			NULL, D3D11_CREATE_DEVICE_DEBUG, NULL,
			0,
			D3D11_SDK_VERSION,
			&scd,
			&m_swapChain,
			&m_device,
			NULL, 
			&m_context);
	return SUCCEEDED(result);
}

bool D3D11System::createBackBufferRTVs()
{
	ComPtr<ID3D11Texture2D> backBuffer;
	m_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
	HRESULT result =
	m_device->CreateRenderTargetView(backBuffer.Get(), NULL, &m_backBufferRTV);
	return SUCCEEDED(result);
}

bool D3D11System::createBaseDSV()
{
	DXGI_SWAP_CHAIN_DESC scd = {};
	m_swapChain->GetDesc(&scd);
	D3D11_TEXTURE2D_DESC td = {};
	td.ArraySize = 1;
	td.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	td.CPUAccessFlags = 0;
	td.Format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
	td.Height = scd.BufferDesc.Height;
	td.MipLevels = 1;
	td.SampleDesc.Count = scd.SampleDesc.Count;
	td.SampleDesc.Quality = scd.SampleDesc.Quality;
	td.Usage = D3D11_USAGE_DEFAULT;
	td.Width = scd.BufferDesc.Width;

	ComPtr<ID3D11Texture2D> depthStencilBuffer;
	m_device->CreateTexture2D(&td, NULL, &depthStencilBuffer);
	HRESULT result =
		m_device->CreateDepthStencilView(depthStencilBuffer.Get(), NULL, &m_baseDSV);
	return SUCCEEDED(result);

}

bool D3D11System::initD3D11System()
{
	if (!createDeviceAndSwapChain())
		return false;
	if (!createBackBufferRTVs())
		return false;
	if (!createBaseDSV())
		return false;

	return true;
}
