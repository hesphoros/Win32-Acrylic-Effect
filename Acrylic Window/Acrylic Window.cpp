// 确保使用 Unicode 字符集
#ifndef UNICODE
#define UNICODE
#endif 

#include "Acrylic Window.h"

// Windows 应用程序入口点
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	// 定义窗口类名
	const wchar_t CLASS_NAME[] = L"Acrylic Window";

	// 注册窗口类
	WNDCLASS wc = { };
	wc.lpfnWndProc = WindowProc;      // 窗口过程函数
	wc.hInstance = hInstance;         // 应用程序实例句柄
	wc.lpszClassName = CLASS_NAME;    // 窗口类名
	RegisterClass(&wc);

	// 创建窗口 - 使用 WS_EX_NOREDIRECTIONBITMAP 扩展样式以支持 Direct Composition
	HWND hwnd = CreateWindowEx(WS_EX_NOREDIRECTIONBITMAP, CLASS_NAME, L"Acrylic Window using Direct Composition", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);

	if (hwnd == NULL)
	{
		return 0;
	}

	// 显示窗口
	ShowWindow(hwnd, nCmdShow);

	// 创建亚克力合成器实例
	compositor.reset(new AcrylicCompositor(hwnd));

	// 设置亚克力效果参数
	AcrylicCompositor::AcrylicEffectParameter param;
	param.blurAmount = 40;           // 模糊程度
	param.saturationAmount = 2;      // 饱和度
	param.tintColor = D2D1::ColorF(0.0f, 0.0f, 0.0f, .30f);        // 着色颜色（黑色，30% 透明度）
	param.fallbackColor = D2D1::ColorF(0.10f,0.10f,0.10f,1.0f);    // 回退颜色（深灰色）

	// 应用亚克力效果，使用主机背景源
	// 应用亚克力效果，使用主机背景源
	compositor->SetAcrylicEffect(hwnd, AcrylicCompositor::BACKDROP_SOURCE_HOSTBACKDROP, param);

	// 消息循环
	MSG msg = { };
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);  // 转换键盘消息
		DispatchMessage(&msg);   // 分发消息到窗口过程
	}

	return 0;
}

// 窗口过程函数 - 处理窗口消息
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (compositor)
	{
		// 处理窗口激活状态变化
		if (uMsg == WM_ACTIVATE)
		{
			if (LOWORD(wParam) == WA_ACTIVE || LOWORD(wParam)==WA_CLICKACTIVE)
			{
				active = true;   // 窗口激活
			}
			else if (LOWORD(wParam) == WA_INACTIVE)
			{
				active = false;  // 窗口失活
			}
		}
		// 同步合成器状态
		compositor->Sync(hwnd, uMsg, wParam, lParam,active);
	}

	switch (uMsg)
	{
		case WM_DESTROY:
			PostQuitMessage(0);  // 发送退出消息
			return 0;
		default:
			break;
	}

	// 调用默认窗口过程处理其他消息
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
