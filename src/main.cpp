#include <windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>
#include <cwchar>

using Microsoft::WRL::ComPtr;

namespace {
constexpr wchar_t kWindowClassName[] = L"DirectX12GameWindow";
constexpr wchar_t kWindowTitle[] = L"DirectX12 Game 1";
ComPtr<ID3D12Device> g_device;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM w_param, LPARAM l_param) {
  switch (message) {
    case WM_DESTROY:
      PostQuitMessage(0);
      return 0;
    default:
      return DefWindowProc(hwnd, message, w_param, l_param);
  }
}

bool InitializeWindow(HINSTANCE instance, int show_command, HWND& out_window) {
  WNDCLASSEX window_class = {};
  window_class.cbSize = sizeof(window_class);
  window_class.lpfnWndProc = WindowProc;
  window_class.hInstance = instance;
  window_class.lpszClassName = kWindowClassName;

  if (RegisterClassEx(&window_class) == 0) {
    if (GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
      return false;
    }
  }

  out_window = CreateWindowEx(
      0,
      kWindowClassName,
      kWindowTitle,
      WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT,
      CW_USEDEFAULT,
      1280,
      720,
      nullptr,
      nullptr,
      instance,
      nullptr);

  if (out_window == nullptr) {
    UnregisterClass(kWindowClassName, instance);
    return false;
  }

  ShowWindow(out_window, show_command);
  return true;
}

bool InitializeDirectX12() {
  HRESULT result = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&g_device));
  if (FAILED(result)) {
    wchar_t buffer[256] = {};
    swprintf_s(buffer, L"D3D12CreateDevice failed (HRESULT: 0x%08X). Ensure a DirectX 12-compatible adapter/driver is available.\n", static_cast<unsigned int>(result));
    OutputDebugString(buffer);
    return false;
  }

  return true;
}

void RunMessageLoop() {
  MSG message = {};
  while (GetMessage(&message, nullptr, 0, 0) > 0) {
    TranslateMessage(&message);
    DispatchMessage(&message);
  }
}
}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int show_command) {
  HWND window = nullptr;
  if (!InitializeWindow(instance, show_command, window)) {
    MessageBox(nullptr, L"Failed to create window.", kWindowTitle, MB_ICONERROR | MB_OK);
    return -1;
  }

  if (!InitializeDirectX12()) {
    MessageBox(window, L"Failed to initialize DirectX 12 device. Ensure your GPU/driver supports DirectX 12.", kWindowTitle, MB_ICONERROR | MB_OK);
    DestroyWindow(window);
    UnregisterClass(kWindowClassName, instance);
    return -1;
  }

  RunMessageLoop();
  g_device.Reset();
  UnregisterClass(kWindowClassName, instance);
  return 0;
}
