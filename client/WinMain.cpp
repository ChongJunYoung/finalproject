#include <winsock2.h>
#include <windows.h>
#include <tchar.h>
//#include "main.h"
#include "../ChattingClient/resource.h"

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK Chat1Proc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam);
HINSTANCE hInst;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow)
{
	HWND hWnd;
	MSG Message;
	hInst = hInstance;
	WNDCLASS WndClass;

	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hbrBackground = (HBRUSH)COLOR_WINDOW;
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hInstance = hInstance;
	WndClass.lpfnWndProc = WndProc;
	WndClass.lpszClassName = TEXT("Hoseo Chatting");
	WndClass.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	RegisterClass(&WndClass);

	hWnd = CreateWindow(TEXT("Hoseo Chatting"),
		TEXT("Hoseo Chatting"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		500,
		500,
		NULL,
		(HMENU)NULL,
		hInstance,
		NULL);
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	SetFocus(hWnd);

	while (GetMessage(&Message, 0, 0, 0)) {
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}
	return Message.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	int answer;

	switch (iMessage) {
	case WM_CREATE:
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_LOGIN:
			MessageBox(hWnd, TEXT("�α��� �Ͻðڽ��ϱ�?"), TEXT("LOGIN"), MB_OKCANCEL);
			break;
		case ID_LOGOUT:
			MessageBox(hWnd, TEXT("�α׾ƿ� �Ͻðڽ��ϱ�?"), TEXT("LOGOUT"), MB_OKCANCEL);
			break;
		case ID_EXIT:
			answer = MessageBox(hWnd, TEXT("���� �Ͻðڽ��ϱ�?"), TEXT("EXIT"), MB_OKCANCEL);
			if (answer == IDYES || answer == IDNO)
				PostQuitMessage(0);
			break;
		}
		break;
	case WM_LBUTTONDOWN:
		DialogBox(hInst, MAKEINTRESOURCE(Chat1Proc), hWnd, Chat1Proc);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return(DefWindowProc(hWnd, iMessage, wParam, lParam));
}

BOOL CALLBACK Chat1Proc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	int num; //ä��â ���μ� ���� ����
	char result[1200]; // ���� �޽��� 
	char msg[1000]; //�Է¸޽���.
	int len;

	switch (iMsg)
	{
	case WM_INITDIALOG:
		return 1;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			EndDialog(hDlg, 0);
			break;
		case IDCANCEL:
			EndDialog(hDlg, 0);
			break;
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return(DefWindowProc(hDlg, iMsg, wParam, lParam));
}