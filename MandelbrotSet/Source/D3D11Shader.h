#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include <wrl.h>
#include <assert.h>
#include "D3D11System.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

using namespace Microsoft::WRL;
static const char* profiles[] = { "vs_4_0", "ps_4_0" };

namespace D3DShader
{
	template<class D3DShaderType, typename FuncType, FuncType func, int profileIndex>
	class ShaderClass
	{
	public:
		ShaderClass(const wchar_t* fileName, bool debug)
		{
			Func = func;
			bool init_shader =
				initVertexShader(fileName, debug);
			assert(init_shader);
		}
		virtual ~ShaderClass() {}

		//deleted function
		ShaderClass(const ShaderClass&) = delete;
		ShaderClass& operator=(const ShaderClass&) = delete;

	public:
		ComPtr<D3DShaderType> getD3DShader()
		{
			return m_shader;
		}

		ComPtr<ID3D10Blob> getShaderBlob()
		{
			return m_shaderBlob;
		}

	private:
		bool initVertexShader(const wchar_t* fileName, bool debug)
		{
			UINT flag = debug ? D3DCOMPILE_DEBUG : 0;
			HRESULT result = 
			D3DCompileFromFile(fileName, NULL, NULL, "main", profiles[profileIndex], flag, 0, &m_shaderBlob, NULL);
			if (FAILED(result)) return false;
			ID3D11Device* device = D3D11System::getInstance()->getDevice().Get();
			result = (device->*Func)(
				m_shaderBlob->GetBufferPointer(), m_shaderBlob->GetBufferSize(), NULL, &m_shader);
			return SUCCEEDED(result);
		}

	private:
		ComPtr<D3DShaderType> m_shader;
		ComPtr<ID3D10Blob> m_shaderBlob;
		FuncType Func;
	};

	typedef ShaderClass<ID3D11VertexShader, decltype(&ID3D11Device::CreateVertexShader), &ID3D11Device::CreateVertexShader, 0> VertexShader;
	typedef ShaderClass<ID3D11PixelShader, decltype(&ID3D11Device::CreatePixelShader), &ID3D11Device::CreatePixelShader, 1> PixelShader;
}

