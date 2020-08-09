#pragma once
#include <Windows.h>
#include <dxgi.h>
#include <d3d11.h>
#include <wrl.h>
#include <memory>

using namespace Microsoft::WRL;

class D3D11System
{
public:
	static void initD3D11System(HWND hwnd);
	static D3D11System* getInstance();
	static std::shared_ptr<D3D11System> sm_instance;
	//helper function
	static ComPtr<ID3D11Device> Device()
	{
		return getInstance()->getDevice();
	}

	static ComPtr<ID3D11DeviceContext> Context()
	{
		return getInstance()->getContext();
	}

public:
	D3D11System(HWND hwnd);
	virtual ~D3D11System();

	//deleted function
	D3D11System(const D3D11System& other) = delete;
	D3D11System& operator=(const D3D11System& other) = delete;

public:
	ComPtr<ID3D11Device> getDevice()
	{
		return m_device;
	}

	ComPtr<ID3D11DeviceContext> getContext()
	{
		return m_context;
	}

	ComPtr<IDXGISwapChain> getSwapChain()
	{
		return m_swapChain;
	}

	ComPtr<ID3D11RenderTargetView> getBackBufferRTV()
	{
		return m_backBufferRTV;
	}

	ComPtr<ID3D11DepthStencilView> getBaseDSV()
	{
		return m_baseDSV;
	}

	void setViewport(int xPos, int yPos, int width, int height);

	void clearRTVAndDSV(float r, float g, float b, float a, float depth, unsigned char stencil);

private:
	bool createDeviceAndSwapChain();
	bool createBackBufferRTVs();
	bool createBaseDSV();
	bool initD3D11System();

private:
	HWND m_hwnd;
	ComPtr<ID3D11Device> m_device;
	ComPtr<ID3D11DeviceContext> m_context;
	ComPtr<IDXGISwapChain> m_swapChain;
	ComPtr<ID3D11RenderTargetView> m_backBufferRTV;
	ComPtr<ID3D11DepthStencilView> m_baseDSV;


};


