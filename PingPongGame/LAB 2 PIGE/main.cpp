#define NOMINMAX
#include <windows.h>
#include "pong.h"
//sample task from tutorial
int WINAPI wWinMain(HINSTANCE instance,HINSTANCE hInstance ,LPWSTR ,int show_command)
{
	pong app{ instance };
	return app.run(show_command);
}