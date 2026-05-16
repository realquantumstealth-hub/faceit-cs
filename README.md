# faceit-cs

> **Official Forum / 官方论坛**: https://discord.gg/qslab

## Languages

[English](#en) · [中文](#zh)

<a id="en"></a>
## English

### Project Overview

`faceit-cs` is a Windows native C++ project organized as a Visual Studio workspace. The project is centered on one root application and one companion `usermode/` project, with shared support code and bundled third-party sources included directly in the repository.

The codebase is arranged around these major layers:

- root desktop application code
- entity/state update code
- rendering and UI code
- system/helper utilities
- low-level wrapper code in `hypercall/`
- a separate `usermode/` project tree

### What This Project Does

At a high level, this project packages several pieces of a native Windows tooling stack into one workspace:

- a root application that owns startup, process discovery, state refresh, and drawing
- a snapshot-oriented entity layer that converts raw runtime state into application-side data models
- a rendering layer that turns those snapshots into an on-screen presentation
- a support layer for system queries, module discovery, and PE/image utilities
- a companion `usermode/` tree that organizes command-style and utility-style functionality in a separate project

In practical terms, the project is useful as a codebase for studying how a native Windows application can be split into:

- acquisition/update logic
- projection/transformation logic
- presentation logic
- helper/infrastructure code
- companion tooling

### High-Level Design

The project follows a layered data flow:

1. the root application initializes shared state and discovers the runtime context
2. the entity layer refreshes application-side snapshots on a background thread
3. the renderer consumes those snapshots on the foreground/UI side
4. helper code in `system.*` and `ext/portable_executable/` supports discovery, parsing, and utility tasks
5. wrapper code in `hypercall/` isolates a lower-level interface behind regular C++ calls

This separation gives each part of the codebase a clear role:

- `main.cpp` coordinates lifecycle
- `entities.*` owns state modeling and refresh
- `renderer.*` owns drawing
- `system.*` owns support and utility functions
- `hypercall/` owns wrapper/interface boundaries
- `usermode/` owns companion project functionality

### Root Directory Layout

The repository root contains:

- `Csgoext.sln`
- `Csgoext.vcxproj`
- `Csgoext.vcxproj.filters`
- `Csgoext.vcxproj.user`
- `main.cpp`
- `entities.cpp`
- `entities.hpp`
- `renderer.cpp`
- `renderer.hpp`
- `system.cpp`
- `system.h`
- `globals.h`
- `Offset.hpp`
- `imgui.ini`
- `orionrift64.sys`
- `README.md`

It also contains these major directories:

- `ext/`
- `hypercall/`
- `Imgui/`
- `Overlay/`
- `usermode/`
- `x64/`

### Root Application Structure

The root application is defined by `Csgoext.sln` and `Csgoext.vcxproj`.

The main root-level source files are:

- `main.cpp`: application startup and main loop entry
- `entities.cpp` / `entities.hpp`: in-memory data model and update logic
- `renderer.cpp` / `renderer.hpp`: drawing logic
- `system.cpp` / `system.h`: support layer, helpers, and utility code
- `globals.h`: shared declarations
- `Offset.hpp`: offset definitions used by the application

The root application is built as one native executable that combines:

- the main application entry
- the entity update pipeline
- the rendering pipeline
- the helper/system layer
- the low-level wrapper layer
- bundled ImGui sources
- bundled PE helper sources

### Main Execution Flow

The high-level execution path in `main.cpp` is:

1. initialize the system layer
2. wait for the target process
3. resolve process and module information
4. initialize the entity subsystem
5. launch a background update thread
6. create the overlay
7. enter the render loop

This gives the project a clear split between:

- setup and discovery
- background state refresh
- foreground drawing

### Entity Layer

`entities.hpp` defines the main runtime structures:

- `Vector3`
- `ViewMatrix`
- `ScreenPos`
- `PlayerBones`
- `Player`
- `EntitySystem`

The entity layer is responsible for:

- storing process-related state
- updating local and remote entity snapshots
- maintaining player collections
- handling coordinate conversion data
- exposing renderer-friendly state

The entity layer is implemented mainly in:

- `entities.hpp`
- `entities.cpp`

### Rendering Layer

The rendering layer is implemented in:

- `renderer.hpp`
- `renderer.cpp`

The renderer provides:

- box drawing
- health and armor bars
- label rendering
- skeleton rendering
- layout calculations based on projected coordinates

This layer uses ImGui draw lists and works with snapshot data produced by the entity layer.

### System and Helper Layer

The system/helper layer is implemented in:

- `system.h`
- `system.cpp`

This area groups together support code for:

- Windows and NT structure declarations
- process and module helpers
- system information queries
- privilege helpers
- string helpers
- filesystem helpers
- image/export helper logic

This layer is paired with the PE helper tree under `ext/portable_executable/`.

### `ext/portable_executable/`

This directory contains bundled PE helper code:

- `data_directory.*`
- `dos_header.*`
- `export_directory.*`
- `file.*`
- `file_header.hpp`
- `image.*`
- `imports_directory.*`
- `nt_headers.*`
- `optional_header.hpp`
- `relocations_directory.*`
- `section_header.*`

This subtree is part of the project itself and is compiled directly into the root application.

### `hypercall/`

The root-level `hypercall/` directory contains:

- `hypercall.h`
- `hypercall.cpp`
- `vmexit.asm`

This directory forms the wrapper boundary between higher-level application code and lower-level call handling. The split is straightforward:

- declarations and structures in `hypercall.h`
- wrapper implementations in `hypercall.cpp`
- assembly entry code in `vmexit.asm`

### `Overlay/`

The `Overlay/` directory contains:

- `overlay.h`
- `skcrypt.hpp`
- `uiaccses.h`

This subtree groups overlay-facing integration headers used by the root application.

### `Imgui/`

The `Imgui/` directory bundles the UI dependency tree directly in the repository. Visible files include:

- `imgui.cpp`
- `imgui.h`
- `imgui_demo.cpp`
- `imgui_draw.cpp`
- `imgui_impl_dx11.cpp`
- `imgui_impl_dx11.h`
- `imgui_impl_win32.cpp`
- `imgui_impl_win32.h`
- `imgui_internal.h`
- `imgui_tables.cpp`
- `imgui_widgets.cpp`
- `imconfig.h`
- `imstb_rectpack.h`
- `imstb_textedit.h`
- `imstb_truetype.h`

This directory provides the full immediate-mode UI layer used by the project.

### `usermode/` Project

`usermode/` is a separate companion project tree.

At the top level it includes:

- `usermode.vcxproj`
- `usermode.vcxproj.filters`
- `usermode.vcxproj.user`
- `vcpkg.json`
- `test_enum.cpp`
- `ext/`
- `src/`
- `vcpkg_installed/`

The `usermode/src/` subtree includes:

- `main.cpp`
- `pfn_query_demo.cpp`
- `commands/`
- `dll_loader/`
- `hook/`
- `hypercall/`
- `system/`

The `usermode/ext/` subtree contains:

- `portable_executable/`

The `usermode/` project mirrors the root project structure in a more tool-oriented layout:

- its own project file
- its own entry point
- its own wrapper/system subtrees
- its own dependency manifest
- its own local dependency installation tree

### Build Footprint

The repository also includes local build output directories:

- `x64/Debug`
- `x64/Release`
- `usermode/vcpkg_installed/x64-windows-static`

This makes the project feel like a working local development tree rather than a source-only archive.

### Project Relationships

The workspace can be read as two native project tracks plus shared support code:

1. root application track
2. `usermode/` companion project track
3. bundled support/dependency trees

The root track consumes:

- `entities.*`
- `renderer.*`
- `system.*`
- `hypercall/`
- `Overlay/`
- `Imgui/`
- `ext/portable_executable/`

The `usermode/` track consumes:

- its own `src/` tree
- its own `ext/portable_executable/`
- its own `vcpkg` manifest and local install tree

### Recommended Reading Order

Recommended reading order:

1. `Csgoext.sln`
2. `Csgoext.vcxproj`
3. `main.cpp`
4. `entities.hpp`
5. `entities.cpp`
6. `renderer.hpp`
7. `renderer.cpp`
8. `system.h`
9. `system.cpp`
10. `hypercall/`
11. `Overlay/`
12. `Imgui/`
13. `ext/portable_executable/`
14. `usermode/usermode.vcxproj`
15. `usermode/src/`

### Summary

`faceit-cs` is a Windows C++ workspace built from:

- a root Visual Studio application
- an entity/state subsystem
- a rendering/UI subsystem
- a system/helper subsystem
- a dedicated wrapper boundary in `hypercall/`
- bundled PE and ImGui support trees
- a separate `usermode/` companion project

<a id="zh"></a>
## 中文

### 项目概览

`faceit-cs` 是一个以 Visual Studio 工作区方式组织的 Windows 原生 C++ 项目。项目由一个根目录主应用工程和一个配套的 `usermode/` 工程组成，并且把支持代码与第三方源码直接放在同一仓库中。

整个代码树主要围绕以下几层展开：

- 根目录桌面应用代码
- 实体/状态更新代码
- 渲染与 UI 代码
- 系统/辅助工具代码
- `hypercall/` 中的底层封装代码
- 独立的 `usermode/` 工程树

### 项目作用

从工程层面看，这个项目把一套原生 Windows 工具链常见的几个部分放进了同一个工作区：

- 一个负责启动、进程发现、状态刷新和绘制调度的根目录主应用
- 一个把运行时原始状态整理成应用侧数据模型的快照式实体层
- 一个把这些快照转换成屏幕呈现结果的绘制层
- 一个负责系统查询、模块发现、PE/镜像辅助能力的支持层
- 一个以独立工程形式组织命令式和工具式能力的 `usermode/` 配套树

从阅读和工程拆分的角度看，这个项目适合用来理解一套原生 Windows 应用是怎样拆成以下几层的：

- 采集/更新逻辑
- 投影/转换逻辑
- 呈现逻辑
- 辅助/基础设施代码
- 配套工具工程

### 整体原理

项目整体采用分层数据流：

1. 根目录主应用负责初始化共享状态并发现运行时上下文
2. 实体层在后台线程中持续刷新应用侧快照
3. 渲染层在前台/UI 侧消费这些快照
4. `system.*` 和 `ext/portable_executable/` 提供发现、解析和辅助工具能力
5. `hypercall/` 把更底层的接口隔离在常规 C++ 包装调用之后

这种拆分让每一部分的职责都比较明确：

- `main.cpp` 负责生命周期调度
- `entities.*` 负责状态建模与刷新
- `renderer.*` 负责绘制
- `system.*` 负责支持与工具函数
- `hypercall/` 负责包装/接口边界
- `usermode/` 负责配套工程能力

### 根目录结构

仓库根目录包含：

- `Csgoext.sln`
- `Csgoext.vcxproj`
- `Csgoext.vcxproj.filters`
- `Csgoext.vcxproj.user`
- `main.cpp`
- `entities.cpp`
- `entities.hpp`
- `renderer.cpp`
- `renderer.hpp`
- `system.cpp`
- `system.h`
- `globals.h`
- `Offset.hpp`
- `imgui.ini`
- `orionrift64.sys`
- `README.md`

同时包含这些主要目录：

- `ext/`
- `hypercall/`
- `Imgui/`
- `Overlay/`
- `usermode/`
- `x64/`

### 根目录主应用结构

根目录主应用由 `Csgoext.sln` 和 `Csgoext.vcxproj` 定义。

根目录主源码文件包括：

- `main.cpp`：应用启动与主循环入口
- `entities.cpp` / `entities.hpp`：内存数据模型与快照更新逻辑
- `renderer.cpp` / `renderer.hpp`：绘制逻辑
- `system.cpp` / `system.h`：支持层、辅助函数与工具代码
- `globals.h`：共享声明
- `Offset.hpp`：主应用使用的偏移定义

根目录主应用以一个原生可执行工程的形式整合：

- 主程序入口
- 实体更新管线
- 渲染管线
- 系统/工具层
- 低层封装层
- 内置 ImGui 源码
- 内置 PE 辅助源码

### 主程序执行流程

`main.cpp` 中的高层流程是：

1. 初始化系统层
2. 等待目标进程
3. 获取进程和模块信息
4. 初始化实体子系统
5. 启动后台更新线程
6. 创建 overlay
7. 进入渲染循环

整个项目因此形成了清晰的三段式结构：

- 初始化与发现
- 后台状态刷新
- 前台绘制输出

### 实体层

`entities.hpp` 定义了主要运行时结构：

- `Vector3`
- `ViewMatrix`
- `ScreenPos`
- `PlayerBones`
- `Player`
- `EntitySystem`

实体层负责：

- 保存进程相关状态
- 更新本地与远端实体快照
- 管理玩家集合
- 维护坐标转换相关数据
- 对渲染层暴露可直接消费的状态

实体层主要由以下文件实现：

- `entities.hpp`
- `entities.cpp`

### 渲染层

渲染层由以下文件实现：

- `renderer.hpp`
- `renderer.cpp`

这一层提供：

- 框体绘制
- 血量条与护甲条
- 标签绘制
- 骨骼连线
- 基于投影坐标的布局计算

渲染层使用 ImGui draw list，并消费实体层提供的快照数据。

### 系统与辅助层

系统/辅助层由以下文件实现：

- `system.h`
- `system.cpp`

这一层汇总了：

- Windows / NT 结构声明
- 进程与模块辅助
- 系统信息查询
- 权限辅助
- 字符串辅助
- 文件系统辅助
- 镜像/导出辅助逻辑

这一层和 `ext/portable_executable/` 下的 PE 辅助树配合使用。

### `ext/portable_executable/`

这个目录包含内置的 PE 辅助代码：

- `data_directory.*`
- `dos_header.*`
- `export_directory.*`
- `file.*`
- `file_header.hpp`
- `image.*`
- `imports_directory.*`
- `nt_headers.*`
- `optional_header.hpp`
- `relocations_directory.*`
- `section_header.*`

这棵子树是项目源码本体的一部分，并且会直接参与根目录主应用的编译。

### `hypercall/`

根目录下的 `hypercall/` 目录包含：

- `hypercall.h`
- `hypercall.cpp`
- `vmexit.asm`

这一层是高层应用代码与更底层调用处理之间的包装边界：

- `hypercall.h` 放接口声明和结构
- `hypercall.cpp` 放包装实现
- `vmexit.asm` 放汇编入口

### `Overlay/`

`Overlay/` 目录包含：

- `overlay.h`
- `skcrypt.hpp`
- `uiaccses.h`

这棵子树集中放置根目录主应用使用的 overlay 集成头文件。

### `Imgui/`

`Imgui/` 目录直接把 UI 依赖树内置在仓库中。当前可见文件包括：

- `imgui.cpp`
- `imgui.h`
- `imgui_demo.cpp`
- `imgui_draw.cpp`
- `imgui_impl_dx11.cpp`
- `imgui_impl_dx11.h`
- `imgui_impl_win32.cpp`
- `imgui_impl_win32.h`
- `imgui_internal.h`
- `imgui_tables.cpp`
- `imgui_widgets.cpp`
- `imconfig.h`
- `imstb_rectpack.h`
- `imstb_textedit.h`
- `imstb_truetype.h`

这一目录提供了项目完整的即时模式 UI 层。

### `usermode/` 配套工程

`usermode/` 是一个独立的配套工程树。

其顶层包含：

- `usermode.vcxproj`
- `usermode.vcxproj.filters`
- `usermode.vcxproj.user`
- `vcpkg.json`
- `test_enum.cpp`
- `ext/`
- `src/`
- `vcpkg_installed/`

`usermode/src/` 子树包含：

- `main.cpp`
- `pfn_query_demo.cpp`
- `commands/`
- `dll_loader/`
- `hook/`
- `hypercall/`
- `system/`

`usermode/ext/` 子树包含：

- `portable_executable/`

`usermode/` 以更偏工具化的方式镜像了根目录主工程结构：

- 自己的工程文件
- 自己的入口
- 自己的 system / wrapper 子树
- 自己的依赖清单
- 自己的本地依赖安装目录

### 构建痕迹

仓库中还直接保留了本地构建输出目录：

- `x64/Debug`
- `x64/Release`
- `usermode/vcpkg_installed/x64-windows-static`

因此整个项目更像真实在本地持续开发的工作树，而不是只保留源码的压缩归档。

### 工程关系

整个工作区可以按“两套原生工程 + 支持代码”来理解：

1. 根目录主应用工程
2. `usermode/` 配套工程
3. 内置支持/依赖树

根目录主工程直接使用：

- `entities.*`
- `renderer.*`
- `system.*`
- `hypercall/`
- `Overlay/`
- `Imgui/`
- `ext/portable_executable/`

`usermode/` 工程使用：

- 自己的 `src/` 树
- 自己的 `ext/portable_executable/`
- 自己的 `vcpkg` 清单与本地依赖树

### 阅读建议

推荐按下面顺序阅读：

1. `Csgoext.sln`
2. `Csgoext.vcxproj`
3. `main.cpp`
4. `entities.hpp`
5. `entities.cpp`
6. `renderer.hpp`
7. `renderer.cpp`
8. `system.h`
9. `system.cpp`
10. `hypercall/`
11. `Overlay/`
12. `Imgui/`
13. `ext/portable_executable/`
14. `usermode/usermode.vcxproj`
15. `usermode/src/`

### 总结

`faceit-cs` 是一个由多层模块构成的 Windows C++ 工作区，主要包含：

- 根目录 Visual Studio 原生应用
- 实体/状态子系统
- 渲染/UI 子系统
- 系统与辅助工具子系统
- `hypercall/` 封装边界
- 内置 PE 与 ImGui 支持树
- 独立的 `usermode/` 配套工程
