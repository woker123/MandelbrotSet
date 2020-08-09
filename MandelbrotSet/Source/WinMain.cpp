#include <Windows.h>
#include <assert.h>
#include <dxgi.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <D3DX10Math.h>
#include <wrl.h>
#include "DInput.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "d3dx10.lib")

using namespace Microsoft::WRL;

typedef struct TransMatrix
{
	D3DXMATRIX model;
	D3DXMATRIX view;
	D3DXMATRIX projection;
	D3DXMATRIX mvp;
} TransMatrix;
typedef float Vertex[3];

const unsigned int VERTEX_STRIDE = sizeof(Vertex);
const unsigned int VERTEX_OFFSET = 0;
const int SCREEN_WIDTH = 800, SCREEN_HEIGHT = 800;
HWND g_hwnd = nullptr;
D3DXVECTOR3 g_windowLocation = D3DXVECTOR3(0, 0, 0);
D3DXVECTOR3 g_windowScale = D3DXVECTOR3(2, 2, 1);
D3DXVECTOR2 g_complex = D3DXVECTOR2(0, 0);
ComPtr<ID3D11DeviceContext> g_context;
ComPtr<ID3D11Device> g_device;
ComPtr<IDXGISwapChain> g_swapChain;
ComPtr<ID3D11RenderTargetView> g_backBufferRTV;
ComPtr<ID3D11Buffer> g_vertexBuffer;
ComPtr<ID3D11Buffer> g_transMatrixBuffer;
ComPtr<ID3D11Buffer> g_complexBuffer;
ComPtr<ID3D11VertexShader> g_vs;
ComPtr<ID3D11PixelShader> g_ps;
ComPtr<ID3D10Blob> g_vsBlob;
ComPtr<ID3D10Blob> g_psBlob;
ComPtr<ID3D11InputLayout> g_inputLayout;
LRESULT CALLBACK WinProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
HWND createWindow(HINSTANCE hInstance, const char* title, int xPos, int yPos, int width, int height);
bool doMessage(HWND hwnd);
void createDeviceAndSwapchain();
void createBackBufferRTVs();
void createConstantBuffers();
void createShaders(const char* vsFile, const char* psFile);
void initVertexBuffer();
void createInputLayout();
void initD3D11();
void bindContext();
void setViewPort(int xPos, int yPos, int width, int height);
void updateTransMatrixBuffer();
void updateComplexBuffer();
void draw();

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPreInstance, PSTR lpCmdLine, INT nCmdShow)
{
	g_hwnd = createWindow(hInstance, "Mandelbrot Set", 50, 50, 800, 800);
	assert(g_hwnd);
	ShowWindow(g_hwnd, SW_SHOW);
	initD3D11();
	bindContext();

	while (doMessage(g_hwnd))
	{	
		updateTransMatrixBuffer();
		updateComplexBuffer();
		
		draw();

		g_swapChain->Present(0, 0);
	}


	return 0;
}

LRESULT CALLBACK WinProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = 0;
	switch (msg)
	{
	case WM_CLOSE:
		PostQuitMessage(0);
		result = 0;
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		result = 0;
		break;
	default:
		result = DefWindowProc(hwnd, msg, wParam, lParam);
		break;
	}
	return result;

}

HWND createWindow(HINSTANCE hInstance, const char * title, int xPos, int yPos, int width, int height)
{
	WNDCLASS wc = {};
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hInstance = hInstance;
	wc.lpfnWndProc = WinProc;
	wc.lpszClassName = "WINCLASS";
	wc.style = CS_HREDRAW | CS_VREDRAW;

	RegisterClass(&wc);

	HWND hwnd =
		CreateWindow(
			"WINCLASS",
			title,
			WS_OVERLAPPEDWINDOW,
			xPos,
			yPos,
			width,
			height,
			NULL,
			NULL,
			hInstance,
			NULL);

	return hwnd;
}

bool doMessage(HWND hwnd)
{
	MSG msg = {};
	if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		return (msg.message == WM_QUIT) ? false : true;
	}
	else
	{
		return true;
	}

}

void createDeviceAndSwapchain()
{
	DXGI_SWAP_CHAIN_DESC scd = {};
	scd.BufferCount = 2;
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scd.BufferDesc.Height = SCREEN_HEIGHT;
	scd.BufferDesc.RefreshRate.Denominator = 1;
	scd.BufferDesc.RefreshRate.Numerator = 60;
	scd.BufferDesc.Width = SCREEN_WIDTH;
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	scd.OutputWindow = g_hwnd;
	scd.SampleDesc.Count = 1;
	scd.SampleDesc.Quality = 0;
	scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	scd.Windowed = true;

	HRESULT create_device =
		D3D11CreateDeviceAndSwapChain(
			NULL,
			D3D_DRIVER_TYPE_HARDWARE,
			NULL,
			D3D11_CREATE_DEVICE_DEBUG,
			NULL, 0,
			D3D11_SDK_VERSION,
			&scd,
			&g_swapChain,
			&g_device,
			NULL,
			&g_context);

	assert(SUCCEEDED(create_device));
}

void createBackBufferRTVs()
{
	ComPtr<ID3D11Texture2D> backBuffer;
	g_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
	HRESULT create_rendertarget =
	g_device->CreateRenderTargetView(backBuffer.Get(), NULL, &g_backBufferRTV);
	assert(SUCCEEDED(create_rendertarget));

}

void createConstantBuffers()
{
	D3D11_BUFFER_DESC bd = {};
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.ByteWidth = sizeof(TransMatrix);
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bd.MiscFlags = 0;
	bd.StructureByteStride = 0;
	bd.Usage = D3D11_USAGE_DYNAMIC;

	HRESULT create_transMatrixBuffer =
		g_device->CreateBuffer(&bd, NULL, &g_transMatrixBuffer);
	assert(SUCCEEDED(create_transMatrixBuffer));

	bd.ByteWidth = sizeof(D3DXVECTOR4);
	HRESULT create_complexBuffer =
		g_device->CreateBuffer(&bd, NULL, &g_complexBuffer);
	assert(SUCCEEDED(create_complexBuffer));
}

void createShaders(const char* vsFile, const char* psFile)
{
	D3DX10CompileFromFileA(vsFile, NULL, NULL, "main", "vs_4_0", 0, 0, NULL, &g_vsBlob, NULL, NULL);
	HRESULT create_vertex_shader =
		g_device->CreateVertexShader(g_vsBlob->GetBufferPointer(), g_vsBlob->GetBufferSize(), NULL, &g_vs);
	assert(SUCCEEDED(create_vertex_shader));

	D3DX10CompileFromFileA(psFile, NULL, NULL, "main", "ps_4_0", 0, 0, NULL, &g_psBlob, NULL, NULL);
	HRESULT create_pixel_shader =
		g_device->CreatePixelShader(g_psBlob->GetBufferPointer(), g_psBlob->GetBufferSize(), NULL, &g_ps);
	assert(SUCCEEDED(create_pixel_shader));
}

void initVertexBuffer()
{
	Vertex vert[] = { {-1, -1, 0}, {-1, 1, 0}, {1, -1, 0}, {1, 1, 0} };
	D3D11_BUFFER_DESC bd = {};
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.ByteWidth = sizeof(vert);
	bd.CPUAccessFlags = 0;
	bd.StructureByteStride = 0;
	bd.Usage = D3D11_USAGE_IMMUTABLE;

	D3D11_SUBRESOURCE_DATA sd = {};
	sd.pSysMem = vert;
	sd.SysMemPitch = sizeof(vert);
	sd.SysMemSlicePitch = sizeof(vert);

	HRESULT create_vertexBuffer =
		g_device->CreateBuffer(&bd, &sd, &g_vertexBuffer);
	assert(SUCCEEDED(create_vertexBuffer));

}

void createInputLayout()
{
	D3D11_INPUT_ELEMENT_DESC ds = {"V_POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0};
	HRESULT create_inputLayout = 
		g_device->CreateInputLayout(&ds, 1, g_vsBlob->GetBufferPointer(), g_vsBlob->GetBufferSize(), &g_inputLayout);
	assert(SUCCEEDED(create_inputLayout));
}

void setViewPort(int xPos, int yPos, int width, int height)
{
	D3D11_VIEWPORT vp = {};
	vp.Height = height;
	vp.Width = width;
	vp.MaxDepth = 1.0;
	vp.MinDepth = 0.0;
	vp.TopLeftX = 0.0;
	vp.TopLeftY = 0.0;
	g_context->RSSetViewports(1, &vp);
}

void updateTransMatrixBuffer()
{
	D3DXMATRIX t, s;
	D3DXMatrixTranslation(&t, g_windowLocation.x, g_windowLocation.y, g_windowLocation.z);
	D3DXMatrixScaling(&s, g_windowScale.x, g_windowScale.y, g_windowScale.z);
	D3DXMATRIX model = t * s;

	D3DXMATRIX view;
	D3DXMatrixTranslation(&view, -g_windowLocation.x, -g_windowLocation.y, 1);
	
	D3DXMATRIX proj;
	D3DXMatrixOrthoLH(&proj, g_windowScale.x * 2.0, g_windowScale.y * 2.0, -1, 1);

	D3DXMATRIX mvp = model * view * proj;
	D3DXMatrixTranspose(&model, &model);
	D3DXMatrixTranspose(&view, &view);
	D3DXMatrixTranspose(&proj, &proj);
	D3DXMatrixTranspose(&mvp, &mvp);
	
	TransMatrix mat = {model, view, proj, mvp};
	D3D11_MAPPED_SUBRESOURCE ms = {};
	g_context->Map(g_transMatrixBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);
	*((TransMatrix*)(ms.pData)) = mat;
	g_context->Unmap(g_transMatrixBuffer.Get(), 0);
}

void updateComplexBuffer()
{
	D3DXVECTOR4 buffer = { g_complex.x, g_complex.y, 0, 0 };
	D3D11_MAPPED_SUBRESOURCE ms = {};
	g_context->Map(g_complexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);
	*((D3DXVECTOR4*)(ms.pData)) = buffer;
	g_context->Unmap(g_complexBuffer.Get(), 0);
}

void initD3D11()
{
	createDeviceAndSwapchain();
	createBackBufferRTVs();
	initVertexBuffer();
	createConstantBuffers();	
	createShaders("Shader/vs.hlsl", "Shader/ps.hlsl");
	createInputLayout();
}

void bindContext()
{
	g_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	g_context->IASetVertexBuffers(0, 1, g_vertexBuffer.GetAddressOf(), &VERTEX_STRIDE, &VERTEX_OFFSET);
	g_context->IASetInputLayout(g_inputLayout.Get());
	g_context->VSSetConstantBuffers(0, 1, g_transMatrixBuffer.GetAddressOf());
	g_context->VSSetShader(g_vs.Get(), NULL, 0);
	g_context->PSSetConstantBuffers(0, 1, g_complexBuffer.GetAddressOf());
	g_context->PSSetShader(g_ps.Get(), NULL, 0);
	setViewPort(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	g_context->OMSetRenderTargets(1, g_backBufferRTV.GetAddressOf(), NULL);
}

void draw()
{
	float color[] = {0.0, 0.0, 0.0, 0.0};
	g_context->ClearRenderTargetView(g_backBufferRTV.Get(), color);
	g_context->Draw(4, 0);
}
