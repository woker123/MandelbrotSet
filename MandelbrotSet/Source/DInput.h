#pragma once
#include <dinput.h>
#include <wrl.h>
#include <assert.h>
#include <array>

#pragma comment(lib, "dinput8.lib")
#define MSW Microsoft::WRL

class DInput
{
public:
	DInput(HINSTANCE hInstance)
	{
		bool init_direct_input =
			InitDInput(hInstance);
		assert(init_direct_input);
	}
	~DInput()
	{}

public:
	bool getKeyState() const
	{
	
	}

	bool getMouseButtonState() const
	{


	}

	std::array<float, 2> getMouseCursorPos() const
	{

	}

	std::array<float, 2> getMouseCursorSpeed() const
	{

	}

	bool getMouseScrollState() const
	{

	}

	void updateDeviceState()
	{

	}

private:
	bool InitDInput(HINSTANCE hInstance);

private:
	MSW::ComPtr<IDirectInput> m_directInput;
	MSW::ComPtr<IDirectInputDevice8> m_keyDevice;
	MSW::ComPtr<IDirectInputDevice8> m_mouseDevice;



};