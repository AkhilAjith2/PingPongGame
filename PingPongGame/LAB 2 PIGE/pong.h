#pragma once
#include "windows.h"
#include <string>

//sample task from tutorial
class pong
{
private:
	bool register_class();
	static std::wstring const s_class_name;
	static LRESULT CALLBACK window_proc_static(HWND window,UINT message,WPARAM wparam,LPARAM lparam);
	LRESULT window_proc(HWND window,UINT message,WPARAM wparam,LPARAM lparam);
	HWND create_window(DWORD style, HWND parent = nullptr, DWORD ex_style = 0);
	HWND window;
	HINSTANCE m_instance;
	HWND m_main, m_popup;
	HWND paddle;
	HBITMAP bitmap;
	WCHAR fileName[MAX_PATH];

	int childWidth = 15;
	int childHeight = 60;
	int m_ball_x = 10;
	int m_ball_y = 10;
	POINT ballPos = { m_ball_x,m_ball_y };
	int m_ball_r = 7;
	int ball_vel_x = 5;
	int ball_vel_y = 5;

	int Player1Score = 0;
	int Player2Score = 0;

	COLORREF bgColor = RGB(92, 243, 0);
	void UpdateBallPosition(HWND hwnd);

	void update_transparency();
	void collision();
	void paddle_movement(int y);
	void ResetGame();
	void P1Counter(HDC hdc);
	void P2Counter(HDC hdc);

public:
	pong(HINSTANCE instance);
	int run(int show_command);
};
