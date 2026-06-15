#include <Windows.h>

// 関数のプロトタイプ宣言
// ウィンドウプロシージャのプロトタイプ宣言
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

// グローバル変数
namespace
{
	// ウィンドウのクラス名とタイトル
	const wchar_t* WinClassName = L"SampleGame";
	const wchar_t* WinGameName = L"Game";

	// ウィンドウの幅と高さ
	constexpr UINT WinWidth = 640;
	constexpr UINT WinHeight = 480;
}

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR lpCmdLine, _In_ int nCmdShow) 
{
	// ウィンドウの登録と生成
	// ウィンドウクラス構造体を設定
	WNDCLASSEXW wc = {};
	wc.cbSize = sizeof(WNDCLASSEXW);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIconW(nullptr, L"IDI_ICON");
	wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszClassName = WinClassName;
	wc.hIconSm = LoadIconW(nullptr, L"IDI_ICON");

	// ウィンドウクラスの登録
	if (!RegisterClassExW(&wc)) return 1;

	// ウィンドウの生成と表示
	RECT rc = {};
	rc.right = (LONG)WinWidth;
	rc.bottom = (LONG)WinHeight;

	HWND hwnd;
	DWORD style = WS_SYSMENU | WS_DLGFRAME | WS_MINIMIZEBOX;
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW | style, FALSE);

	hwnd = CreateWindowExW(0, WinClassName, WinGameName, WS_OVERLAPPED | style,
		CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom -rc.top,
		nullptr, nullptr, hInstance, nullptr);
	if (!hwnd) return 1;

	ShowWindow(hwnd, SW_SHOWNORMAL);

	// メッセージループ
	MSG msg = {};
	while (GetMessageW(&msg, nullptr, 0, 0) > 0)
	{
		if (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
		if (msg.message == WM_QUIT) break;

		// ToDo (ゲームループ処理）

		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}

	// ウィンドウの登録を解除
	UnregisterClassW(wc.lpszClassName, wc.hInstance);
	return 0;
}

// 関数の定義

// ウィンドウプロシージャ
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_KEYDOWN:
		switch (wparam)
		{
		case VK_ESCAPE:
			DestroyWindow(hwnd);
			return 0;
		}
		break;
	}

	return DefWindowProcW(hwnd, msg, wparam, lparam);
}