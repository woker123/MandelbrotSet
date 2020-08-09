#include <Windows.h>
#include <assert.h>
#include <DirectXMath.h>
#include "D3D11System.h"
#include "D3D11Shader.h"
#include "GraphicWindow.h"

using namespace Microsoft::WRL;
using namespace DirectX;
using namespace std;
using namespace D3DShader;

typedef struct TransMatrix
{
	XMFLOAT4X4 model;
	XMFLOAT4X4 view;
	XMFLOAT4X4 projection;
	XMFLOAT4X4 mvp;
} TransMatrix;
typedef float Vertex[3];

const unsigned int VERTEX_STRIDE = sizeof(Vertex);
const unsigned int VERTEX_OFFSET = 0;
const int SCREEN_WIDTH = 800, SCREEN_HEIGHT = 800;
XMFLOAT3 g_windowLocation = XMFLOAT3(0, 0, 0);
XMFLOAT3 g_windowScale = XMFLOAT3(2, 2, 1);
XMFLOAT2 g_complex = XMFLOAT2(0, 0);

std::shared_ptr<GraphicWindow> g_window;
std::shared_ptr<VertexShader> g_vs;
std::shared_ptr<PixelShader> g_ps;
ComPtr<ID3D11Buffer> g_vertexBuffer;
ComPtr<ID3D11Buffer> g_transMatrixBuffer;
ComPtr<ID3D11Buffer> g_complexBuffer;
ComPtr<ID3D11InputLayout> g_inputLayout;

void createShaders();
void createVertexBuffer();
void createConstantBuffers();
void createInputLayout();
void bindContext();
void setViewPort(int xPos, int yPos, int width, int height);
void updateTransMatrixBuffer();
void updateComplexBuffer();
void draw();


INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPreInstance, PSTR lpCmdLine, INT nCmdShow)
{
	g_window.reset(new GraphicWindow(hInstance, "GraphicWindow", 50, 50, SCREEN_WIDTH, SCREEN_HEIGHT));
	g_window->showWindow();
	D3D11System::initD3D11System(g_window->getHWND());
	createShaders();
	createVertexBuffer();
	createConstantBuffers();
	createInputLayout();

	bindContext();

	while (g_window->doMessage())
	{	
		updateTransMatrixBuffer();
		updateComplexBuffer();
		
		D3D11System::Context()->OMSetRenderTargets(1, D3D11System::getInstance()->getBackBufferRTV().GetAddressOf(), NULL);
		draw();

		D3D11System::getInstance()->getSwapChain()->Present(0, 0);
	}


	return 0;
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
		D3D11System::Device()->CreateBuffer(&bd, NULL, &g_transMatrixBuffer);
	assert(SUCCEEDED(create_transMatrixBuffer));

	bd.ByteWidth = sizeof(XMFLOAT4);
	HRESULT create_complexBuffer =
		D3D11System::Device()->CreateBuffer(&bd, NULL, &g_complexBuffer);
	assert(SUCCEEDED(create_complexBuffer));
}

void createShaders()
{
	g_vs.reset(new VertexShader(L"./Shader/vs.hlsl", true));
	g_ps.reset(new PixelShader(L"./Shader/ps.hlsl", true));
}

void createVertexBuffer()
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
		D3D11System::Device()->CreateBuffer(&bd, &sd, &g_vertexBuffer);
	assert(SUCCEEDED(create_vertexBuffer));

}

void createInputLayout()
{
	D3D11_INPUT_ELEMENT_DESC ds = {"V_POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0};
	HRESULT create_inputLayout = 
		D3D11System::Device()->CreateInputLayout(
			&ds, 1, g_vs->getShaderBlob()->GetBufferPointer(), g_vs->getShaderBlob()->GetBufferSize(), &g_inputLayout);
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
	D3D11System::Context()->RSSetViewports(1, &vp);
}

void updateTransMatrixBuffer()
{
	XMMATRIX t, s;
	t = XMMatrixTranslation(g_windowLocation.x, g_windowLocation.y, g_windowLocation.z);
	s = XMMatrixScaling(g_windowScale.x, g_windowScale.y, g_windowScale.z);
	XMMATRIX model = t * s;

	XMMATRIX view;
	view = XMMatrixTranslation(-g_windowLocation.x, -g_windowLocation.y, 1);
	
	XMMATRIX proj;
	proj = XMMatrixOrthographicLH(g_windowScale.x * 2.0, g_windowScale.y * 2.0, -1, 1);

	XMMATRIX mvp = model * view * proj;
	XMMatrixTranspose(model);
	XMMatrixTranspose(view);
	XMMatrixTranspose(proj);
	XMMatrixTranspose(mvp);
	
	TransMatrix mat = {};
	XMStoreFloat4x4(&mat.model, model);
	XMStoreFloat4x4(&mat.view, view);
	XMStoreFloat4x4(&mat.projection, proj);
	XMStoreFloat4x4(&mat.mvp, mvp);


	D3D11_MAPPED_SUBRESOURCE ms = {};
	D3D11System::Context()->Map(g_transMatrixBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);
	*((TransMatrix*)(ms.pData)) = mat;
	D3D11System::Context()->Unmap(g_transMatrixBuffer.Get(), 0);
}

void updateComplexBuffer()
{
	XMFLOAT4 buffer = { g_complex.x, g_complex.y, 0, 0 };
	D3D11_MAPPED_SUBRESOURCE ms = {};
	D3D11System::Context()->Map(g_complexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);
	*((XMFLOAT4*)(ms.pData)) = buffer;
	D3D11System::Context()->Unmap(g_complexBuffer.Get(), 0);
}

void bindContext()
{
	D3D11System::Context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	D3D11System::Context()->IASetVertexBuffers(0, 1, g_vertexBuffer.GetAddressOf(), &VERTEX_STRIDE, &VERTEX_OFFSET);
	D3D11System::Context()->IASetInputLayout(g_inputLayout.Get());
	D3D11System::Context()->VSSetConstantBuffers(0, 1, g_transMatrixBuffer.GetAddressOf());
	D3D11System::Context()->VSSetShader(g_vs->getD3DShader().Get(), NULL, 0);
	D3D11System::Context()->PSSetConstantBuffers(0, 1, g_complexBuffer.GetAddressOf());
	D3D11System::Context()->PSSetShader(g_ps->getD3DShader().Get(), NULL, 0);
	setViewPort(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);	
}

void draw()
{
	float color[] = {0.0, 0.0, 0.0, 0.0};
	D3D11System::Context()->ClearRenderTargetView(D3D11System::getInstance()->getBackBufferRTV().Get(), color);
	D3D11System::Context()->Draw(4, 0);
}
