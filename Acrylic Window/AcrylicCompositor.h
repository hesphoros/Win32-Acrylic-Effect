#pragma once

#ifndef UNICODE
#define UNICODE
#endif 

// Windows API 头文件
#include <Windows.h>
#include <dcomp.h>      // Direct Composition API
#include <d2d1_2.h>     // Direct2D API
#include <dwmapi.h>     // Desktop Window Manager API
#include <dxgi1_3.h>    // DirectX Graphics Infrastructure API
#include <d3d11_2.h>    // Direct3D 11 API
#include <d2d1_2helper.h>
#include <comutil.h>
#include <wrl\implements.h>
#include <winternl.h>   // 用于 RTL_OSVERSIONINFOW 和 NTSTATUS
#include <stdexcept>    // 用于异常处理

// DWM 缩略图相关的未文档化标志
#define DWM_TNP_FREEZE            0x100000     // 冻结缩略图
#define DWM_TNP_ENABLE3D          0x4000000    // 启用3D效果
#define DWM_TNP_DISABLE3D         0x8000000    // 禁用3D效果
#define DWM_TNP_FORCECVI          0x40000000   // 强制CVI
#define DWM_TNP_DISABLEFORCECVI   0x80000000   // 禁用强制CVI

// 链接必要的库
#pragma comment(lib, "dxgi")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "d2d1")
#pragma comment(lib, "dcomp")
#pragma comment(lib, "dwmapi")

using namespace Microsoft::WRL;

// 亚克力合成器类 - 用于创建和管理 Windows 亚克力效果
class AcrylicCompositor
{
	public:
		// 背景源枚举
		enum BackdropSource
		{
			BACKDROP_SOURCE_DESKTOP = 0x0,       // 桌面背景源
			BACKDROP_SOURCE_HOSTBACKDROP = 0x1   // 主机背景源（包含其他窗口）
		};

		// 亚克力效果参数结构
		struct AcrylicEffectParameter
		{
			float blurAmount;           // 模糊强度
			float saturationAmount;     // 饱和度
			D2D1_COLOR_F tintColor;     // 着色颜色
			D2D1_COLOR_F fallbackColor; // 回退颜色
		};

		// 构造函数
		AcrylicCompositor(HWND hwnd);
		
		// 同步方法 - 处理窗口消息和状态更新
		bool Sync(HWND hwnd,int msg,WPARAM wParam,LPARAM lParam,bool active);
		
		// 设置亚克力效果
		bool SetAcrylicEffect(HWND hwnd,BackdropSource source,AcrylicEffectParameter params);

	private:
		// 窗口组合属性枚举 - 用于设置窗口的组合行为
		enum WINDOWCOMPOSITIONATTRIB
		{
			WCA_UNDEFINED = 0x0,
			WCA_NCRENDERING_ENABLED = 0x1,
			WCA_NCRENDERING_POLICY = 0x2,
			WCA_TRANSITIONS_FORCEDISABLED = 0x3,
			WCA_ALLOW_NCPAINT = 0x4,
			WCA_CAPTION_BUTTON_BOUNDS = 0x5,
			WCA_NONCLIENT_RTL_LAYOUT = 0x6,
			WCA_FORCE_ICONIC_REPRESENTATION = 0x7,
			WCA_EXTENDED_FRAME_BOUNDS = 0x8,
			WCA_HAS_ICONIC_BITMAP = 0x9,
			WCA_THEME_ATTRIBUTES = 0xA,
			WCA_NCRENDERING_EXILED = 0xB,
			WCA_NCADORNMENTINFO = 0xC,
			WCA_EXCLUDED_FROM_LIVEPREVIEW = 0xD,
			WCA_VIDEO_OVERLAY_ACTIVE = 0xE,
			WCA_FORCE_ACTIVEWINDOW_APPEARANCE = 0xF,
			WCA_DISALLOW_PEEK = 0x10,
			WCA_CLOAK = 0x11,
			WCA_CLOAKED = 0x12,
			WCA_ACCENT_POLICY = 0x13,
			WCA_FREEZE_REPRESENTATION = 0x14,
			WCA_EVER_UNCLOAKED = 0x15,
			WCA_VISUAL_OWNER = 0x16,
			WCA_HOLOGRAPHIC = 0x17,
			WCA_EXCLUDED_FROM_DDA = 0x18,
			WCA_PASSIVEUPDATEMODE = 0x19,
			WCA_LAST = 0x1A,
		};
		// 窗口组合属性数据结构
		struct WINDOWCOMPOSITIONATTRIBDATA
		{
			WINDOWCOMPOSITIONATTRIB Attrib;  // 属性类型
			void* pvData;                    // 数据指针
			DWORD cbData;                    // 数据大小
		};

		// Direct2D 和 Direct3D 相关接口指针
		ComPtr<ID2D1Device1> d2Device;              // Direct2D 设备
		ComPtr<ID3D11Device> d3d11Device;           // Direct3D 11 设备
		ComPtr<IDXGIDevice2> dxgiDevice;            // DXGI 设备
		ComPtr<IDXGIFactory2> dxgiFactory;          // DXGI 工厂
		ComPtr<ID2D1Factory2> d2dFactory2;          // Direct2D 工厂
		ComPtr<ID2D1DeviceContext> deviceContext;   // Direct2D 设备上下文

		// Direct Composition 相关接口指针
		ComPtr<IDCompositionDesktopDevice> dcompDevice;  // Direct Composition 桌面设备
		ComPtr<IDCompositionDevice3> dcompDevice3;       // Direct Composition 设备 v3
		ComPtr<IDCompositionTarget> dcompTarget;         // Direct Composition 目标

		// 视觉对象指针
		ComPtr<IDCompositionVisual2> rootVisual;         // 根视觉对象
		ComPtr<IDCompositionVisual2> fallbackVisual;     // 回退视觉对象
		ComPtr<IDCompositionVisual2> desktopWindowVisual; // 桌面窗口视觉对象
		ComPtr<IDCompositionVisual2> topLevelWindowVisual; // 顶级窗口视觉对象

		#pragma region Acrylic Essentials - 亚克力效果核心组件

		ComPtr<IDCompositionGaussianBlurEffect> blurEffect;      // 高斯模糊效果
		ComPtr<IDCompositionSaturationEffect> saturationEffect;  // 饱和度效果
		ComPtr<IDCompositionTranslateTransform> translateTransform; // 平移变换
		ComPtr<IDCompositionRectangleClip> clip;                 // 矩形裁剪

		#pragma endregion
		// Fallback Visual - 回退视觉对象相关组件
		#pragma region  

		DXGI_SWAP_CHAIN_DESC1 description = {};     // 交换链描述
		D2D1_BITMAP_PROPERTIES1 properties = {};    // 位图属性
		ComPtr<IDXGISwapChain1> swapChain;           // 交换链
		ComPtr<IDXGISurface2> fallbackSurface;       // 回退表面
		ComPtr<ID2D1Bitmap1> fallbackBitmap;         // 回退位图
		ComPtr<ID2D1SolidColorBrush> fallbackBrush;  // 回退纯色画刷
		D2D1_COLOR_F tintColor = D2D1::ColorF(0.0f, 0.0f, 0.0f, .70f);     // 着色颜色
		D2D1_COLOR_F fallbackColor = D2D1::ColorF(1.0f,1.0f,1.0f,1.0f);    // 回退颜色
		D2D1_RECT_F fallbackRect = D2D1::RectF(0, 0, (float)GetSystemMetrics(SM_CXSCREEN), (float)GetSystemMetrics(SM_CYSCREEN)); // 回退矩形

		#pragma endregion

		#pragma region Desktop Backdrop - 桌面背景相关

		HWND desktopWindow;                    // 桌面窗口句柄
		RECT desktopWindowRect;               // 桌面窗口矩形
		SIZE thumbnailSize{};                 // 缩略图大小
		DWM_THUMBNAIL_PROPERTIES thumbnail;   // DWM 缩略图属性
		HTHUMBNAIL desktopThumbnail = NULL;   // 桌面缩略图句柄

		#pragma endregion

		#pragma region Top Level Window Backdrop - 顶级窗口背景相关

		RECT sourceRect{0,0,GetSystemMetrics(SM_CXSCREEN),GetSystemMetrics(SM_CYSCREEN)}; // 源矩形
		SIZE destinationSize{ GetSystemMetrics(SM_CXSCREEN),GetSystemMetrics(SM_CYSCREEN) }; // 目标大小
		HTHUMBNAIL topLevelWindowThumbnail = NULL;  // 顶级窗口缩略图句柄
		HWND* hwndExclusionList;                    // 排除窗口列表

		#pragma endregion

		// 前向声明和类型定义
		typedef PVOID PRTL_OSVERSIONINFOW;
		typedef LONG NTSTATUS;

		// 函数指针类型定义 - 用于调用未文档化的 Windows API
		typedef NTSTATUS (WINAPI* RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);
		typedef BOOL (WINAPI* SetWindowCompositionAttribute)(HWND hwnd, WINDOWCOMPOSITIONATTRIBDATA* pwcad);
		typedef HRESULT (WINAPI* DwmpCreateSharedThumbnailVisual)(HWND hwndDestination, HWND hwndSource, DWORD dwThumbnailFlags, DWM_THUMBNAIL_PROPERTIES* pThumbnailProperties, VOID* pDCompDevice, VOID** ppVisual, PHTHUMBNAIL phThumbnailId);
		typedef HRESULT (WINAPI* DwmpCreateSharedMultiWindowVisual)(HWND hwndDestination, VOID* pDCompDevice, VOID** ppVisual, PHTHUMBNAIL phThumbnailId);
		typedef HRESULT (WINAPI* DwmpUpdateSharedMultiWindowVisual)(HTHUMBNAIL hThumbnailId, HWND* phwndsInclude, DWORD chwndsInclude, HWND* phwndsExclude, DWORD chwndsExclude, RECT* prcSource, SIZE* pDestinationSize, DWORD dwFlags);
		typedef HRESULT (WINAPI* DwmpCreateSharedVirtualDesktopVisual)(HWND hwndDestination, VOID* pDCompDevice, VOID** ppVisual, PHTHUMBNAIL phThumbnailId);
		typedef HRESULT (WINAPI* DwmpUpdateSharedVirtualDesktopVisual)(HTHUMBNAIL hThumbnailId, HWND* phwndsInclude, DWORD chwndsInclude, HWND* phwndsExclude, DWORD chwndsExclude, RECT* prcSource, SIZE* pDestinationSize);

		// 函数指针实例
		DwmpCreateSharedThumbnailVisual DwmCreateSharedThumbnailVisual;
		DwmpCreateSharedMultiWindowVisual DwmCreateSharedMultiWindowVisual;
		DwmpUpdateSharedMultiWindowVisual DwmUpdateSharedMultiWindowVisual;
		DwmpCreateSharedVirtualDesktopVisual DwmCreateSharedVirtualDesktopVisual;
		DwmpUpdateSharedVirtualDesktopVisual DwmUpdateSharedVirtualDesktopVisual;
		SetWindowCompositionAttribute DwmSetWindowCompositionAttribute;
		RtlGetVersionPtr GetVersionInfo;

		HRESULT hr;                    // HRESULT 错误码
		RECT hostWindowRect{};         // 主机窗口矩形

		// 私有方法声明
		bool InitLibs();                                          // 初始化库
		long GetBuildVersion();                                   // 获取构建版本
		bool CreateCompositionDevice();                           // 创建组合设备
		bool CreateFallbackVisual();                             // 创建回退视觉对象
		void SyncFallbackVisual(bool active);                    // 同步回退视觉对象
		bool CreateCompositionVisual(HWND hwnd);                 // 创建组合视觉对象
		bool CreateCompositionTarget(HWND hwnd);                 // 创建组合目标
		bool CreateBackdrop(HWND hwnd,BackdropSource source);    // 创建背景
		bool CreateEffectGraph(ComPtr<IDCompositionDevice3> dcompDevice3); // 创建效果图
		void SyncCoordinates(HWND hwnd);                         // 同步坐标
		bool Flush();                                            // 刷新
		bool Commit();                                           // 提交
};

