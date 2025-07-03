#pragma once

#include <windows.h>
#include <memory>
#include "AcrylicCompositor.h"

// 全局变量
bool active;                                          // 窗口激活状态
std::unique_ptr<AcrylicCompositor> compositor{nullptr}; // 亚克力合成器智能指针

// 窗口过程函数声明
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);