# Win32 亚克力效果 - 关键API总结

本文档总结了在实现 Win32 应用程序亚克力效果时使用的关键 Windows API。

## 核心 API 分类

### 1. Direct Composition API

Direct Composition 是 Windows 提供的高性能合成引擎，用于创建复杂的视觉效果。

#### 主要接口：
- **IDCompositionDesktopDevice** - Direct Composition 桌面设备
  - 用于创建和管理合成对象
  - 提供硬件加速的视觉合成功能

- **IDCompositionDevice3** - Direct Composition 设备 v3
  - 支持更高级的合成功能
  - 用于创建效果和视觉对象

- **IDCompositionTarget** - 合成目标
  - 将视觉内容渲染到指定的窗口
  - 连接合成树和实际窗口

- **IDCompositionVisual2** - 视觉对象 v2
  - 表示可以被渲染的视觉元素
  - 支持变换、效果和裁剪

#### 效果接口：
- **IDCompositionGaussianBlurEffect** - 高斯模糊效果
  - 实现亚克力效果的核心模糊功能
  - 可设置模糊半径和边界模式

- **IDCompositionSaturationEffect** - 饱和度效果
  - 调整图像的色彩饱和度
  - 增强亚克力效果的视觉表现

#### 变换和裁剪：
- **IDCompositionTranslateTransform** - 平移变换
  - 用于调整视觉对象的位置
  - 实现动态位置同步

- **IDCompositionRectangleClip** - 矩形裁剪
  - 限制视觉效果的显示区域
  - 防止效果溢出窗口边界

### 2. DWM (Desktop Window Manager) 私有 API

这些是未文档化的 DWM API，用于访问桌面合成功能。

#### 缩略图视觉创建：
```cpp
// 创建共享缩略图视觉对象
HRESULT DwmpCreateSharedThumbnailVisual(
    HWND hwndDestination,                    // 目标窗口
    HWND hwndSource,                        // 源窗口
    DWORD dwThumbnailFlags,                 // 缩略图标志
    DWM_THUMBNAIL_PROPERTIES* pThumbnailProperties, // 缩略图属性
    VOID* pDCompDevice,                     // Direct Composition 设备
    VOID** ppVisual,                        // 输出视觉对象
    PHTHUMBNAIL phThumbnailId               // 输出缩略图句柄
);
```

#### 虚拟桌面视觉：
```cpp
// 创建共享虚拟桌面视觉对象
HRESULT DwmpCreateSharedVirtualDesktopVisual(
    HWND hwndDestination,                   // 目标窗口
    VOID* pDCompDevice,                     // Direct Composition 设备
    VOID** ppVisual,                        // 输出视觉对象
    PHTHUMBNAIL phThumbnailId               // 输出缩略图句柄
);

// 更新共享虚拟桌面视觉对象
HRESULT DwmpUpdateSharedVirtualDesktopVisual(
    HTHUMBNAIL hThumbnailId,                // 缩略图句柄
    HWND* phwndsInclude,                    // 包含的窗口列表
    DWORD chwndsInclude,                    // 包含窗口数量
    HWND* phwndsExclude,                    // 排除的窗口列表
    DWORD chwndsExclude,                    // 排除窗口数量
    RECT* prcSource,                        // 源矩形
    SIZE* pDestinationSize                  // 目标大小
);
```

#### 窗口组合属性：
```cpp
// 设置窗口组合属性
BOOL SetWindowCompositionAttribute(
    HWND hwnd,                              // 窗口句柄
    WINDOWCOMPOSITIONATTRIBDATA* pwcad      // 组合属性数据
);
```

### 3. Direct2D API

用于创建和管理 2D 图形资源。

#### 主要接口：
- **ID2D1Factory2** - Direct2D 工厂 v2
  - 创建 Direct2D 资源的工厂接口

- **ID2D1Device1** - Direct2D 设备 v1
  - 表示 Direct2D 设备的抽象

- **ID2D1DeviceContext** - Direct2D 设备上下文
  - 用于执行绘图操作

- **ID2D1Bitmap1** - Direct2D 位图 v1
  - 表示可以被渲染的位图资源

- **ID2D1SolidColorBrush** - 纯色画刷
  - 用于绘制纯色内容

### 4. Direct3D 11 API

提供硬件加速的 3D 图形功能。

#### 主要接口：
- **ID3D11Device** - Direct3D 11 设备
  - Direct3D 设备的主要接口
  - 用于创建资源和状态对象

### 5. DXGI API

DirectX Graphics Infrastructure，提供底层图形功能。

#### 主要接口：
- **IDXGIDevice2** - DXGI 设备 v2
  - 表示图形适配器的抽象

- **IDXGIFactory2** - DXGI 工厂 v2
  - 用于创建 DXGI 对象

- **IDXGISwapChain1** - 交换链 v1
  - 管理前后缓冲区交换

- **IDXGISurface2** - DXGI 表面 v2
  - 表示可以被渲染的表面

## API 使用流程

### 1. 初始化阶段
1. 加载 DWM 私有 API 函数指针
2. 创建 Direct3D 11 设备
3. 创建 DXGI 设备和工厂
4. 创建 Direct2D 工厂和设备
5. 创建 Direct Composition 设备

### 2. 效果创建阶段
1. 创建高斯模糊效果
2. 创建饱和度效果
3. 设置效果参数
4. 创建变换和裁剪对象

### 3. 背景捕获阶段
1. 使用 DWM 私有 API 创建桌面或窗口视觉对象
2. 设置缩略图属性
3. 更新视觉对象内容

### 4. 合成阶段
1. 创建根视觉对象
2. 构建视觉层次结构
3. 应用效果到视觉对象
4. 设置裁剪和变换
5. 提交更改

### 5. 同步阶段
1. 监听窗口消息
2. 更新视觉对象位置和大小
3. 处理窗口激活状态变化
4. 同步回退视觉对象

## 关键常量和结构

### 窗口组合属性
```cpp
enum WINDOWCOMPOSITIONATTRIB {
    WCA_EXCLUDED_FROM_LIVEPREVIEW = 0xD,  // 从实时预览中排除
    WCA_ACCENT_POLICY = 0x13,             // 重音策略
    // ... 其他属性
};
```

### DWM 缩略图标志
```cpp
#define DWM_TNP_FREEZE            0x100000     // 冻结缩略图
#define DWM_TNP_ENABLE3D          0x4000000    // 启用3D效果
#define DWM_TNP_DISABLE3D         0x8000000    // 禁用3D效果
```

### 亚克力效果参数
```cpp
struct AcrylicEffectParameter {
    float blurAmount;           // 模糊强度 (建议值: 20-60)
    float saturationAmount;     // 饱和度 (建议值: 1.5-2.5)
    D2D1_COLOR_F tintColor;     // 着色颜色 (通常是半透明的黑色或白色)
    D2D1_COLOR_F fallbackColor; // 回退颜色 (当亚克力效果不可用时使用)
};
```

## 注意事项

1. **兼容性**: DWM 私有 API 未文档化，可能在未来的 Windows 版本中发生变化
2. **性能**: 亚克力效果需要硬件加速支持，在低端设备上可能影响性能
3. **权限**: 某些 API 可能需要特定的系统权限或仅在特定版本的 Windows 上可用
4. **错误处理**: 必须妥善处理 API 调用失败的情况，提供合适的回退机制

## 参考链接

- [Microsoft Direct Composition 文档](https://docs.microsoft.com/en-us/windows/win32/directcomp/directcomposition-portal)
- [Direct2D 文档](https://docs.microsoft.com/en-us/windows/win32/direct2d/direct2d-portal)
- [DWM 文档](https://docs.microsoft.com/en-us/windows/win32/dwm/dwm-overview)
- [DXGI 文档](https://docs.microsoft.com/en-us/windows/win32/direct3ddxgi/dx-graphics-dxgi)
