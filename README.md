# faceit-cs

[中文](#中文说明) | [English](#english)

## 中文说明

`faceit-cs` 是一个面向 Windows 平台的反作弊对抗研究工程样例，当前仓库结构包含：

- `Overlay/`：图形叠加与展示层
- `Imgui/`：UI 组件与渲染依赖
- `hypercall/`：低层通信相关逻辑
- `usermode/`：用户态控制与命令入口
- `system.*` / `entities.*` / `renderer.*`：运行时系统、实体处理与渲染组织

### 研究目标

- 研究用户态与底层组件之间的接口组织
- 研究渲染与数据采集线程的协作方式
- 研究工程化拆分和调试流程

### 合规与边界

本项目仅用于安全研究与技术交流，不用于任何未授权环境。

**由于部分密钥、证书、可执行链路、绕过/注入成品等属敏感信息不方便在github上公开，需要或想交流的同伴可以联系我们官方discord进行深入探讨。**

---

## English

`faceit-cs` is a Windows anti-cheat research-oriented project sample. The repository currently includes:

- `Overlay/`: overlay and presentation layer
- `Imgui/`: UI components and rendering dependencies
- `hypercall/`: low-level communication related logic
- `usermode/`: user-mode control and command entry
- `system.*` / `entities.*` / `renderer.*`: runtime system, entity processing, and rendering flow

### Research Focus

- Interface organization between user-mode and low-level components
- Cooperation between rendering and data-capture threads
- Project modularization and debugging workflow

### Compliance & Boundaries

This project is for security research and technical communication only, and must not be used in any unauthorized environment.

**Some keys, certificates, executable chains, and bypass/injection deliverables are sensitive and are not suitable for public release on GitHub. If you need deeper discussion, please contact our official Discord.**


