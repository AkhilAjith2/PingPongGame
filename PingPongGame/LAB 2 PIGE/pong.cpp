#include "pong.h"
#include <stdexcept>
#include <dwmapi.h>
#include <Windows.h>
#include <Windowsx.h>
#include "resource.h"
#include <shlobj.h> 
std::wstring const pong::s_class_name{ L"Pong Window" };

const int MAX_TRAIL_SIZE = 50;
POINT trail[MAX_TRAIL_SIZE] = { 0 };
int trailSize = 5;

//sample

pong::pong(HINSTANCE instance) : m_instance{ instance }
{
	register_class();
	DWORD main_style = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX;
	DWORD popup_style = WS_OVERLAPPED | WS_CAPTION;
	m_main = create_window(main_style);
	m_popup = create_window(popup_style, m_main);
	SetLayeredWindowAttributes(m_popup, 0, 255, LWA_ALPHA);
}

//sample task from tutorial

bool pong::register_class()
{
	WNDCLASSEXW desc{};
	if (GetClassInfoExW(m_instance, s_class_name.c_str(), &desc) != 0)
		return true;
	desc = {
	.cbSize = sizeof(WNDCLASSEXW),
	.style = CS_HREDRAW | CS_VREDRAW,
	.lpfnWndProc = window_proc_static,
	.hInstance = m_instance,
	.hIcon = LoadIcon(m_instance, MAKEINTRESOURCE(IDI_TUTORIAL)),
	.hCursor = LoadCursorW(nullptr, L"IDC_ARROW"),
	.hbrBackground = CreateSolidBrush(RGB(92,243, 0)),
	.lpszMenuName = MAKEINTRESOURCEW(IDC_TUTORIAL),
	.lpszClassName = s_class_name.c_str(),
	.hIconSm = LoadIcon(m_instance, MAKEINTRESOURCE(IDI_SMALL))
	};
	return RegisterClassExW(&desc) != 0;
}

//sample task from tutorial
//https://social.msdn.microsoft.com/Forums/vstudio/en-US/54f7f131-5d15-4b2d-863b-bef329e89a32/how-to-display-the-window-center-win32-api?forum=vcgeneral
//https://www.codeproject.com/Tips/250672/CenterWindow-in-WIN32

HWND pong::create_window(DWORD style, HWND parent, DWORD ex_style)
{
	RECT windowRect;
	GetWindowRect(parent, &windowRect);
	int xPosition = (GetSystemMetrics(SM_CXSCREEN) - (windowRect.right - windowRect.left)) / 2;//screen width - window width
	int yPosition = (GetSystemMetrics(SM_CYSCREEN) - (windowRect.bottom - windowRect.top)) / 2;//screen height - window height
	SetWindowPos(parent, NULL, xPosition, yPosition, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

	update_transparency();
	window = CreateWindowExW(
		ex_style,
		s_class_name.c_str(),
		L"Pong",
		style,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		500,
		350,
		parent,
		nullptr,
		m_instance,
		this);

	RECT childRect;
	GetClientRect(window, &childRect);
	int x = (childRect.right - childWidth);
	int y = (childRect.bottom - childHeight) / 2;

	paddle = CreateWindowExW(
		0,
		L"STATIC",
		nullptr,
		WS_CHILD | WS_VISIBLE,
		x,
		y,
		childWidth,
		childHeight,
		window,
		nullptr,
		m_instance,
		nullptr);

	return window;
}

//sample task from tutorial

LRESULT pong::window_proc_static(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	pong* app = nullptr;
	if (message == WM_NCCREATE)
	{
		app = static_cast<pong*>(reinterpret_cast<LPCREATESTRUCTW>(lparam)->lpCreateParams);
		SetWindowLongPtrW(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(app));
	}
	else
		app = reinterpret_cast<pong*>(
			GetWindowLongPtrW(window, GWLP_USERDATA));
	LRESULT res = app ?
		app->window_proc(window, message, wparam, lparam) :
		DefWindowProcW(window, message, wparam, lparam);
	if (message == WM_NCDESTROY)
		SetWindowLongPtrW(window, GWLP_USERDATA, 0);
	return res;
}


INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
		{
	case WM_INITDIALOG:
		return static_cast <INT_PTR>(TRUE);
		
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
			
			 {
			EndDialog(hDlg, LOWORD(wParam));
			return static_cast <INT_PTR>(TRUE);
			 }
		 break;
		 }
	 return static_cast <INT_PTR>(FALSE);
}


//sample task from tutorial

LRESULT pong::window_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	switch (message)
	{
	case WM_COMMAND:
	{
		int wmId = LOWORD(wparam);
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(m_instance, MAKEINTRESOURCE(IDD_ABOUTBOX), window, About);
			break;
		case IDM_EXIT:
			DestroyWindow(window);
			break;
		case IDM_RESET:
			ResetGame();
			break;
			
		case ID_BACKGROUND_COLOUR:
		{
			//https://cpp.hotexamples.com/examples/-/-/ChooseColor/cpp-choosecolor-function-examples.html
			CHOOSECOLOR cc;
			static COLORREF acrCustClr[16];
			static DWORD rgbCurrent;

			ZeroMemory(&cc, sizeof(cc));
			cc.lStructSize = sizeof(cc);
			cc.hwndOwner = window;
			cc.lpCustColors = (LPDWORD)acrCustClr;
			cc.rgbResult = rgbCurrent;
			cc.Flags = CC_FULLOPEN | CC_RGBINIT;

			if (ChooseColor(&cc))
			{
				HBRUSH hBrush = CreateSolidBrush(cc.rgbResult);
				bgColor = cc.rgbResult;
				SetClassLongPtr(window, (-10), (LONG_PTR)hBrush);
				InvalidateRect(window, nullptr, TRUE);
			}
			break;
		}
		case ID_BACKGROUND_BITMAP:
		{
			//https://stackoverflow.com/questions/16791953/openfilename-open-dialog
			OPENFILENAME ofn;     
			ZeroMemory(&ofn, sizeof(ofn));
			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.hwndOwner = window;
			ofn.lpstrFile = fileName;
			ofn.nMaxFile = MAX_PATH;
			ofn.nFilterIndex = 1;
			ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

			//https://stackoverflow.com/questions/57945118/c-win32-loading-multiple-bitmaps-in-wm-create-wont-load

			if (GetOpenFileName(&ofn) == TRUE)
			{
				bitmap = (HBITMAP)LoadImage(NULL,fileName, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
				if (bitmap == NULL)
				{
					MessageBox(window, L"Unable to load bitmap file.", L"Error", MB_OK | MB_ICONERROR);
				}
			}
		}
		default:
			return DefWindowProc(window, message, wparam, lparam);
		}
	}
	break;
	//old tutorial task
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(window, &ps);

		HDC hdc1 = GetDC(window);
		HDC hdcMem = CreateCompatibleDC(hdc1);
		BITMAP bm;
		HBITMAP oldBitmap = (HBITMAP)SelectObject(hdcMem, bitmap);
		GetObject(bitmap, sizeof(BITMAP), &bm);
		//StretchBlt(hdc1, 0, 0, 500, 350, hdcMem, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
		BitBlt(hdc1, 0, 0, bm.bmWidth, bm.bmHeight, hdcMem, 0, 0, SRCCOPY);

		HBRUSH hBrush = CreateSolidBrush(RGB(255, 0, 0));
		HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
		Ellipse(hdc, m_ball_x - m_ball_r, m_ball_y - m_ball_r, m_ball_x + m_ball_r, m_ball_y + m_ball_r);
		SelectObject(hdc, hOldBrush);
		DeleteObject(hBrush);

		int trailRadius = m_ball_r / 1.2;
		int numTrails = 5;
		int spacing = trailRadius / 15;
		int temp = trailRadius;

		for (int i = 0; i < numTrails; i++)
		{
			int x = m_ball_x - (ball_vel_x * (i + 1) * 2);
			int y = m_ball_y - (ball_vel_y * (i + 1) * 2);
			HBRUSH trailBrush = CreateSolidBrush(RGB(255, 0, 0));
			HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, trailBrush);
			Ellipse(hdc, x - temp, y - temp, x + temp, y + temp);
			SelectObject(hdc, oldBrush);
			DeleteObject(trailBrush);
			temp -= trailRadius / numTrails;
		}

		P1Counter(hdc);
		P2Counter(hdc);

		SelectObject(hdcMem, oldBitmap);
		DeleteDC(hdcMem);
		ReleaseDC(window, hdc1);

		HBRUSH brush = CreateSolidBrush(RGB(255, 0, 0));
		HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);
		Ellipse(hdc, m_ball_x - m_ball_r, m_ball_y - m_ball_r, m_ball_x + m_ball_r, m_ball_y + m_ball_r);
		SelectObject(hdc, oldBrush);
		DeleteObject(brush);

		EndPaint(window, &ps);
		break;
	}
	break;
	case WM_TIMER:
	{
		UpdateBallPosition(window);
		break;
	}
	//https://learn.microsoft.com/en-us/windows/win32/inputdev/wm-mousemove
	case WM_MOUSEMOVE:
	{
		int paddleY = HIWORD(lparam) - (childHeight / 2);
		paddle_movement(paddleY);
		break;
	}
	break;
	case WM_CLOSE:
		DestroyWindow(m_main);
		return 0;
	case WM_DESTROY:
		DeleteObject(bitmap);
		if (window == m_main)
			PostQuitMessage(EXIT_SUCCESS);
		return 0;
	}
	return DefWindowProcW(window, message, wparam, lparam);
}


void pong::P1Counter(HDC hdc)
{
	//https://stackoverflow.com/questions/17347324/how-to-change-text-size-in-basic-text-window-win32-c
	COLORREF neg = RGB(255 - GetRValue(bgColor), 255 - GetGValue(bgColor), 255 - GetBValue(bgColor));
	HFONT hFont1 = CreateFont(70, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE,
		ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial");
	HFONT font1 = (HFONT)SelectObject(hdc, hFont1);
	TCHAR buffer_right[32];
	wsprintf(buffer_right, L"%d", Player1Score);
	SetBkMode(hdc, TRANSPARENT);
	SetBkColor(hdc, RGB(0, 0, 0));
	SetTextColor(hdc, neg);
	TextOutW(hdc, 350, 15, (LPCWSTR)buffer_right, wcslen(buffer_right));
	SelectObject(hdc, font1);
}

void pong::P2Counter(HDC hdc)
{
	//https://stackoverflow.com/questions/17347324/how-to-change-text-size-in-basic-text-window-win32-c
	COLORREF neg = RGB(255 - GetRValue(bgColor), 255 - GetGValue(bgColor), 255 - GetBValue(bgColor));
	HFONT hFont = CreateFont(70, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Arial"));
	HFONT font = (HFONT)SelectObject(hdc, hFont);
	TCHAR buffer_left[32];
	wsprintf(buffer_left, L"%d", Player2Score);
	SetBkMode(hdc, TRANSPARENT);
	SetBkColor(hdc, RGB(0, 0, 0));
	SetTextColor(hdc, neg);
	TextOutW(hdc, 100, 18, (LPCWSTR)buffer_left, wcslen(buffer_left));
	SelectObject(hdc, font);
}

//https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-intersectrect

void pong::collision()
{
	//https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-mapwindowpoints
	//https://groups.google.com/g/microsoft.public.win32.programmer.ui/c/WOIzUSYGHAI?pli=1
	RECT brect{ m_ball_x - m_ball_r, m_ball_y - m_ball_r, m_ball_x + m_ball_r, m_ball_y + m_ball_r };
	HWND paddle = FindWindowExW(m_main, nullptr, L"STATIC", nullptr);
	RECT prect;
	GetWindowRect(paddle, &prect);
	//https://en.cppreference.com/w/cpp/language/reinterpret_cast
	MapWindowPoints(nullptr, m_main, reinterpret_cast<POINT*>(&prect), 2);

	if (IntersectRect(&brect, &brect, &prect))  
	{
		Player1Score++;
		ball_vel_x *= -1;
		if (ball_vel_x < 0)
			m_ball_x = prect.left - m_ball_r;
		else
			m_ball_x = prect.right + m_ball_r;
	}
}

//sample task from tutorial

int pong::run(int show_command)
{
	ShowWindow(m_main, show_command);
	HACCEL hAccelTable = LoadAccelerators(m_instance, MAKEINTRESOURCE(IDC_TUTORIAL));
	MSG msg{};
	BOOL result = TRUE;
	SetTimer(m_main, 1, 50, NULL);
	while ((result = GetMessageW(&msg, nullptr, 0, 0)) != 0)
	{
		if (result == -1)
			return EXIT_FAILURE;
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
	}
		
	return EXIT_SUCCESS;
}

//https://www.codeproject.com/Questions/61633/creating-a-transparent-window

void pong::update_transparency()
{
	SetWindowLong(m_main, GWL_EXSTYLE, GetWindowLong(m_main, GWL_EXSTYLE) | WS_EX_LAYERED);
	SetLayeredWindowAttributes(m_main, 0, 255 * 80 / 100, LWA_ALPHA);
}

void pong::paddle_movement(int y)
{
	RECT mrect;
	HWND parent = FindWindowExW(m_main, nullptr, L"STATIC", nullptr);
	GetWindowRect(GetParent(parent), &mrect);

	int windowHeight = mrect.bottom - mrect.top - (childHeight);

	RECT prect;
	GetWindowRect(parent, &prect);
	//https://stackoverflow.com/questions/15734528/client-rectangle-coordinates-on-screen
	//https://groups.google.com/g/microsoft.public.win32.programmer.ui/c/WOIzUSYGHAI?pli=1
	MapWindowPoints(nullptr, m_main, reinterpret_cast<POINT*>(&prect), 2);

	int paddleHeight = prect.bottom - prect.top;

	/*RECT win;
	GetClientRect(window, &win);

	if(m_ball_x + m_ball_r > win.right) 
	{
		return;
	}*/

	//http://www.jasinskionline.com/windowsapi/ref/m/movewindow.html
	int newY = max(0, min(y, windowHeight - paddleHeight));

	MoveWindow(parent, prect.left, newY, prect.right - prect.left,prect.bottom - prect.top, true);
}

void pong::ResetGame()
{
	RECT g;
	GetClientRect(window, &g);
	int width = g.right - g.left;
	int height = g.bottom - g.top;
	m_ball_x = width / 3;
	m_ball_y = height / 3;
	ball_vel_x = abs(ball_vel_x);
	ball_vel_y = abs(ball_vel_y);

	Player1Score = 0;
	Player2Score = 0;

	InvalidateRect(window, NULL, TRUE);
}

void pong::UpdateBallPosition(HWND hwnd)
{
	m_ball_x += ball_vel_x;
	m_ball_y += ball_vel_y;

	RECT client;
	GetClientRect(window, &client);

	collision();

	if (m_ball_x - m_ball_r < client.left) {
		ball_vel_x *= -1;
	}
	if (m_ball_y - m_ball_r < client.top || m_ball_y + m_ball_r > client.bottom) {
		ball_vel_y *= -1;
	}
	if (m_ball_x + m_ball_r > client.right) {
		ball_vel_x *= -1;
		Player2Score++;
	}

	InvalidateRect(hwnd, NULL, TRUE);
}
