# Win32 亚克力效果
使用 Direct Composition 和 DWM 私有 API 在 C++ Win32 应用程序上演示亚克力效果。

<details open="open">
  <summary>目录</summary>
  <ol>
    <li>
      <a href="#概述">概述</a>
      <ul>
        <li type="disc">简史</li>
        <li type="disc">关于此项目</li>
      </ul>
    </li>
    <li>
      <a href="#可能的实现方法">可能的实现方法</a>
      <ul>
        <li type="disc">SetWindowCompositionAttribute()</li>
        <li type="disc">Windows.UI.Composition - 互操作</li>
        <li type="disc">Windows 放大镜 API</li>
        <li type="disc">桌面复制 API</li>
        <li type="disc">DWM 私有 API</li>
      </ul>
    </li>
    <li>
      <a href="#背景源">背景源</a>
      <ul>
        <li type="disc">桌面背景</li>
        <li type="disc">主机背景</li>
      </ul>
    </li>
    <li>
      <a href="#功能特性">功能特性</a>
    </li>
    <li>
      <a href="#已知限制">已知限制</a>
    </li>
    <li>
      <a href="#支持的版本">支持的版本</a>
    </li>
  </ol>
</details>

## 概述

![示例](https://user-images.githubusercontent.com/72641365/121798817-2d04d300-cc46-11eb-845b-0f2863d7cfde.png)

### 简史
Windows 10 的亚克力效果最初在 2017 年 Windows Fall Creators Update 之后引入到 UWP 应用程序中。从那时起，开发者们一直在尝试在普通 Windows 应用程序（如 WPF、Win32 等）上实现亚克力效果。最常用的方法是 `SetWindowCompositionAttribute()`，但它的有效期并不长，从 Windows 10 1903 开始，窗口在拖拽时会变得卡顿 *（这个问题似乎在 Windows Insider 预览版中得到了修复，但在任何稳定版本的 Windows 中都没有修复）*。

### 关于此项目
此项目只是一个演示，展示如何在 Win32 应用程序上使用 DWM API 和 Direct Composition 实现（或可能的实现方式）自定义亚克力效果。这只是一个基本实现，仍有很大的改进空间，比如噪声叠加和其他混合效果。我们还将讨论实现亚克力效果的可能方法。

## 可能的实现方法
有不同的方法可以在 Windows 10 上实现亚克力效果，其中一些可能也能在 Windows 7、8 上工作 *（我还没有测试过）*。

### SetWindowCompositionAttribute()

`SetWindowCompositionAttribute()` 是 Windows 中一个未文档化的 API，它允许我们为窗口启用亚克力效果、DWM 模糊 *（与 `DwmEnableBlurBehindWindow()` 相同）* 和透明度。由于它是一个未文档化的 API，这个 API 可能在未来版本的 Windows 中被更改或移除。正如我之前提到的，使用这个 API 创建亚克力效果会在拖拽或调整窗口大小时造成卡顿，而且模糊效果在最大化期间会消失，直到窗口完全最大化为止。

### Windows.UI.Composition - 互操作

Microsoft 仓库中的 <a href="https://github.com/microsoft/Windows.UI.Composition-Win32-Samples">Windows.UI.Composition-Win32-Samples</a> 实际上提供了一种在 Windows Forms 和 WPF 中实现亚克力效果的方法。这是在 WPF 中实现亚克力效果最接近的方法之一。但是 WPF 使用不同的渲染技术，亚克力效果使用直接组合渲染，所以总是会导致 <a href="https://github.com/dotnet/wpf/issues/152">空域问题</a>。只有两种可能的方法来克服这个问题：

#### 1. 重叠窗口：
您可以创建多个 WPF 窗口，一个用于渲染 WPF 内容，一个用于亚克力效果。这些窗口应该是透明的，并且必须相互通信以同步窗口调整大小、窗口拖拽和其他事件，这可以通过重写 `WndProc` 来实现。但是这种方法会在调整大小时造成闪烁，并且在最大化动画期间也会影响亚克力效果。*（这个方法之前已经测试过）*

#### 2. WPF 交换链破解：
这是另一种我还没有测试过的可能方法，但理论上它可能会起作用。这可以通过破解 WPF 交换链来获取其后台缓冲区，并将其复制到 ID2D1Bitmap 或类似对象中，然后使用类似 <a href="https://docs.microsoft.com/en-us/windows/uwp/composition/composition-native-interop">Composition Native Interoperation</a> 的东西将其传递给 Windows.UI.Composition.Visual。通过这种方式，我们可能能够克服 WPF 亚克力效果的空域问题。*（:sweat_smile: 事实是目前我只能从 WPF 获取交换链后台缓冲区）*

### Windows 放大镜 API
Windows 放大镜 API 也可以通过使用 `MagImageScalingCallback()` 提供窗口后面的视觉效果 *（背景）*，但它在 Windows 7 之后已被弃用。通过将从 `MagImageScalingCallback()` 获得的位图传递给 D2D1Effects（如高斯模糊），可能有机会在 Windows 7 上创建模糊效果。如果源矩形坐标超出桌面坐标，`MagImageScalingCallback()` 也会抛出异常。

### 桌面复制 API
Windows 桌面复制 API 也是创建亚克力效果的一种可能方法。桌面复制 API 可用于捕获整个桌面（包括窗口本身），但在 Windows 10 2004 版本中，Microsoft 在 `SetWindowDisplayAffinity()` 函数中添加了一个新参数 `WDA_EXCLUDEFROMCAPTURE`，它将帮助我们捕获整个桌面但排除应用了 `WDA_EXCLUDEFROMCAPTURE` 的窗口。但一切都是有代价的，这样做您将无法通过截图或屏幕捕获来捕获该窗口，而且早期版本的 Windows 不支持 `WDA_EXCLUDEFROMCAPTURE`。

### DWM 私有 API
`dwmapi.dll` 包含一些私有 API，可用于将每个窗口或一组窗口捕获到 `IDCompositionVisual` 中。如果您使用一些反编译器（如 <a href="https://hex-rays.com/ida-free/">HexRays 的 IDA</a>）反编译 dwmapi.dll 并探索 dll 函数，您将能够找到 `DwmpCreateSharedThumbnailVisual()` 函数，它帮助我们将窗口捕获到 `IDCompositionVisual` 中。在这个示例中，我们主要使用两个函数 `DwmpCreateSharedThumbnailVisual()` 和 `DwmpCreateSharedVirtualDesktopVisual()` 来创建视觉效果，并使用 `DwmpUpdateSharedVirtualDesktopVisual()` 和 `DwmpUpdateDesktopThumbnail()` 来更新视觉效果。

<b>特别感谢 <a href="https://github.com/ADeltaX">ADeltaX</a> 的实现。*（<a href="https://gist.github.com/ADeltaX/aea6aac248604d0cb7d423a61b06e247">DWM 缩略图/虚拟桌面 IDCompositionVisual 示例</a>）*</b>

获得视觉效果后，将高斯模糊、饱和度等图形效果应用于视觉效果，最后使用 Direct Composition 将其渲染回窗口。这种方法也有几个限制，比如与其他平台（如 WPF）一起使用时的空域问题。

## 背景源
在这个示例中，我们为亚克力效果定义了两个背景源：

#### 1. 桌面背景
桌面背景使用桌面视觉效果作为亚克力效果的源。因此，主机窗口下的其他窗口不会被捕获进行模糊处理，它将是普通的桌面壁纸。这是通过使用 `DwmpCreateSharedThumbnailVisual()` 完成的。

#### 2. 主机背景
主机背景使用桌面背景作为背景视觉效果，并且还捕获主机窗口下的所有窗口进行模糊处理。这是通过使用 `DwmpCreateSharedVirtualDesktopVisual()` 函数完成的。

您可以在此示例中使用 `BackdropSource` 枚举调整背景源。

```C++
// 对于主机背景
compositor->SetAcrylicEffect(hwnd, AcrylicCompositor::BACKDROP_SOURCE_HOSTBACKDROP, param); 
// 对于桌面背景
compositor->SetAcrylicEffect(hwnd, AcrylicCompositor::BACKDROP_SOURCE_DESKTOP , param);     
```

## 功能特性
  <ul>
    <li>减少调整大小或拖拽时的闪烁</li>
    <li>多种背景源</li>
    <li>您可以从亚克力效果中排除特定窗口，就像主机窗口对自己所做的那样。</li>
  </ul>

## 已知限制
<ul>
  <li>有时桌面图标在亚克力效果中会丢失。</li>
  <li>没有直接的方法在 WPF 或 Win UI3 中实现这个效果。</li>
  <li>空域问题：目前您只能使用 Direct Composition 在亚克力效果之上绘制内容。</li>
  <li>不支持实时亚克力，这意味着主机窗口只有在激活时才重绘亚克力，所以使用了回退颜色。</li>
  <li>未添加排斥混合和噪声效果（可以实现）</li>
</ul>

## 支持的版本
此示例目前已在以下版本上测试：

| 操作系统 | 版本 | 构建版本 |
| ------------- | ------------- | ------------- |
| Windows 10 Pro  | 20H2  | 19042.1052|

<hr/>

## 加入我们 :triangular_flag_on_post:

您可以加入我们的 <a href="https://discord.gg/PEqkwGcEtu">Discord 频道</a> 与我们联系。您可以分享您的发现、想法和关于改进/实现普通应用程序上亚克力效果的想法。
