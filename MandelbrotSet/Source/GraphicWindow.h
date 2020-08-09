#pragma once
#include <Windows.h>
#include <assert.h>

static LRESULT CALLBACK WinProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
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

class GraphicWindow
{
public:
	GraphicWindow(HINSTANCE hInstance, const char* title, int xPos, int yPos, int width, int height)
		:m_hInstance(hInstance), m_xPos(xPos), m_yPos(yPos),m_width(width), m_height(height)
	{
		bool init_GraphicWindow =
			initGraphicWindow(title);
		assert(init_GraphicWindow);
 	}

	virtual ~GraphicWindow()
	{
		shutdownGraphicWindow();
	}

	//deleted function
	GraphicWindow(const GraphicWindow&) = delete;
	GraphicWindow& operator=(const GraphicWindow&) = delete;

public:
	void showWindow()
	{
		::ShowWindow(m_hwnd, SW_SHOW);
	}

	bool doMessage()
	{
		MSG msg = {};
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			return msg.message == WM_QUIT ? false : true;
		}
		else
		{
			return true;
		}
	}

public:
	int getXPos() const
	{
		return m_xPos;
	}

	int getYPos() const
	{
		return m_yPos;
	}

	int getWidth() const
	{
		return m_width;
	}

	int getHeight() const
	{
		return m_height;
	}

	HWND getHWND() const
	{
		return m_hwnd;
	}

private:
	bool initGraphicWindow(const char* title)
	{
		const char* CLASS_NAME = "GWINCLASS";
		WNDCLASS wc = {};
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hInstance = m_hInstance;
		wc.lpfnWndProc = WinProc;
		wc.lpszClassName = CLASS_NAME;
		wc.style = CS_HREDRAW | CS_VREDRAW;
		
		RegisterClass(&wc);

		m_hwnd =
			CreateWindow(CLASS_NAME, title, WS_OVERLAPPEDWINDOW, m_xPos, m_yPos, m_width, m_height, NULL, NULL, m_hInstance, NULL);
		
		return (bool)m_hwnd;
	}

	void shutdownGraphicWindow()
	{
		if (m_hwnd)
			DestroyWindow(m_hwnd);
	}

private:
	HINSTANCE m_hInstance;
	int m_xPos;
	int m_yPos;
	int m_width;
	int m_height;
	HWND m_hwnd;



};
