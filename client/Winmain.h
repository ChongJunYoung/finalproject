#ifndef _winmain_
#define _winmain_

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
extern HINSTANCE g_hInst;
extern LPCTSTR lpszClass;

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow);
LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

#endif