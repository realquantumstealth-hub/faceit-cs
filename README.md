# faceit-cs

## Languages

[English](#en) · [中文](#zh) · [日本語](#ja) · [한국어](#ko) · [Русский](#ru) · [Українська](#uk) · [Tiếng Việt](#vi)

<a id="zh"></a>
## 中文说明

`faceit-cs` 是一个面向 Windows 平台的反作弊对抗研究工程样例，当前仓库结构包含：

- `Overlay/`：图形叠加与展示层
- `Imgui/`：UI 组件与渲染依赖
- `hypercall/`：底层通信相关逻辑
- `usermode/`：用户态控制与命令入口
- `system.*` / `entities.*` / `renderer.*`：运行时系统、实体处理与渲染组织

### 反作弊视角

从防守研究角度看，本项目可用于分析“用户态可见性”与“底层可见性”差异带来的检测盲区：

- 帮助研究者理解仅依赖用户态采样时可能遗漏的对象与行为
- 帮助反作弊工程师设计跨层一致性校验（user/kernel/hypervisor 视角对比）
- 帮助蓝队建立针对异常绘制链路、异常数据读取链路的告警规则

### 可能作用与用途（防守用途）

- 作为教学样例：演示多模块协作的工程化组织
- 作为实验基线：对比不同采集路径在完整性和时效性上的差异
- 作为检测设计输入：提炼可观测特征（线程行为、调用路径、数据访问节奏）

### 核心原理（高层）

1. 分层采集：由用户态控制层驱动，结合底层通信获取目标运行态数据
2. 数据整形：将底层采样结果映射为可渲染实体与状态
3. 可视化输出：通过 overlay/UI 管线展示结果，用于验证采集与判断逻辑
4. 反馈闭环：根据观测结果反向调整检测策略与采样策略

### 防守研究建议

- 建立跨层遥测：同一对象在不同层的状态应可相互验证
- 引入时间序列特征：关注高频、规则化、低抖动的数据访问模式
- 加强完整性监控：重点监控关键模块加载链与通信路径变更

### 研究目标

- 研究用户态与底层组件之间的接口组织
- 研究渲染与数据采集线程的协作方式
- 研究工程化拆分和调试流程

### 合规与边界

本项目仅用于安全研究与技术交流，不用于任何未授权环境。

由于部分密钥、证书、可执行链路、绕过/注入成品等属敏感信息不方便在 GitHub 上公开，需要或想交流的同伴可以联系我们官方 Discord 进行深入探讨。

---

<a id="en"></a>
## English

`faceit-cs` is a Windows anti-cheat research-oriented project sample. The repository currently includes:

- `Overlay/`: overlay and presentation layer
- `Imgui/`: UI components and rendering dependencies
- `hypercall/`: low-level communication related logic
- `usermode/`: user-mode control and command entry
- `system.*` / `entities.*` / `renderer.*`: runtime system, entity processing, and rendering flow

### Anti-Cheat Perspective

From a defensive research perspective, this project helps analyze detection blind spots caused by visibility gaps between user-mode and lower layers:

- Understand what may be missed when detection relies only on user-mode sampling
- Design cross-layer consistency checks (user/kernel/hypervisor viewpoints)
- Build blue-team alerts for abnormal rendering pipelines and abnormal data-access paths

### Potential Value and Use Cases (Defensive)

- Teaching sample for multi-module collaboration and engineering structure
- Experimental baseline for comparing collection paths by integrity and timeliness
- Detection-design input by extracting observable features (thread behavior, call paths, access cadence)

### Core Principles (High Level)

1. Layered collection driven by a user-mode control layer and low-level communication
2. Data normalization that maps low-level samples into renderable entities/states
3. Visual output through overlay/UI pipeline to validate collection and decision logic
4. Feedback loop to refine detection and sampling strategies

### Defensive Research Recommendations

- Build cross-layer telemetry for object state consistency checks
- Add time-series features for high-frequency and low-jitter access patterns
- Strengthen integrity monitoring around key module loading and communication paths

### Research Focus

- Interface organization between user-mode and low-level components
- Cooperation between rendering and data-capture threads
- Project modularization and debugging workflow

### Compliance & Boundaries

This project is for security research and technical communication only, and must not be used in any unauthorized environment.

Some keys, certificates, executable chains, and bypass/injection deliverables are sensitive and are not suitable for public release on GitHub. For deeper discussion, please contact our official Discord.

---

<a id="ja"></a>
## 日本語

`faceit-cs` は Windows 向けのアンチチート研究プロジェクトのサンプルです。現在の構成は以下の通りです。

- `Overlay/`：オーバーレイ表示レイヤー
- `Imgui/`：UI コンポーネントと描画依存
- `hypercall/`：低レベル通信ロジック
- `usermode/`：ユーザーモード制御とコマンド入口
- `system.*` / `entities.*` / `renderer.*`：実行時システム、エンティティ処理、描画フロー

### 研究目的

- ユーザーモードと低レベルコンポーネント間のインターフェース設計
- 描画スレッドとデータ取得スレッドの協調
- モジュール分割とデバッグ手順の検証

### コンプライアンス

本プロジェクトはセキュリティ研究と技術交流のみを目的とし、無許可環境での利用は禁止します。

鍵・証明書・実行チェーン・バイパス/インジェクション成果物などの機微情報は GitHub で公開しません。詳細な議論は公式 Discord へご連絡ください。

---

<a id="ko"></a>
## 한국어

`faceit-cs`는 Windows 환경을 위한 안티치트 연구용 프로젝트 샘플입니다. 현재 저장소 구성은 다음과 같습니다.

- `Overlay/`: 오버레이 표시 계층
- `Imgui/`: UI 구성요소 및 렌더링 의존성
- `hypercall/`: 저수준 통신 로직
- `usermode/`: 사용자 모드 제어 및 명령 진입점
- `system.*` / `entities.*` / `renderer.*`: 런타임 시스템, 엔티티 처리, 렌더링 흐름

### 연구 목표

- 사용자 모드와 저수준 컴포넌트 간 인터페이스 구조 연구
- 렌더링 스레드와 데이터 수집 스레드 협업 방식 연구
- 모듈 분리와 디버깅 워크플로우 연구

### 준수 및 범위

본 프로젝트는 보안 연구 및 기술 교류 목적에 한해 사용되며, 무단 환경에서의 사용은 금지됩니다.

키, 인증서, 실행 체인, 바이패스/인젝션 결과물 등 민감 정보는 GitHub에 공개하지 않습니다. 심화 논의는 공식 Discord로 문의해 주세요.

---

<a id="ru"></a>
## Русский

`faceit-cs` — это пример исследовательского проекта по anti-cheat для Windows. Текущая структура включает:

- `Overlay/`: слой оверлея и отображения
- `Imgui/`: UI-компоненты и зависимости рендеринга
- `hypercall/`: низкоуровневая коммуникация
- `usermode/`: управление в user-mode и точка входа команд
- `system.*` / `entities.*` / `renderer.*`: runtime-система, обработка сущностей и рендер-поток

### Цели исследования

- Изучение интерфейсов между user-mode и низкоуровневыми компонентами
- Изучение взаимодействия рендер-потоков и потоков сбора данных
- Изучение модульной структуры и процесса отладки

### Соответствие и ограничения

Проект предназначен только для исследований в области безопасности и технического обмена, без использования в несанкционированной среде.

Ключи, сертификаты, исполняемые цепочки и готовые bypass/injection материалы являются чувствительной информацией и не публикуются на GitHub. Для обсуждения обращайтесь в наш официальный Discord.

---

<a id="uk"></a>
## Українська

`faceit-cs` — це приклад дослідницького anti-cheat проєкту для Windows. Поточна структура містить:

- `Overlay/`: шар оверлею та відображення
- `Imgui/`: UI-компоненти та залежності рендерингу
- `hypercall/`: низькорівнева комунікація
- `usermode/`: керування у user-mode та вхід команд
- `system.*` / `entities.*` / `renderer.*`: runtime-система, обробка сутностей і рендер-потік

### Мета дослідження

- Вивчення інтерфейсів між user-mode і низькорівневими компонентами
- Вивчення взаємодії рендер-потоків і потоків збору даних
- Вивчення модульної структури та процесу налагодження

### Відповідність і межі

Проєкт призначено лише для безпекових досліджень і технічного обміну, без використання в неавторизованих середовищах.

Ключі, сертифікати, виконувані ланцюги та готові bypass/injection матеріали є чутливою інформацією і не публікуються на GitHub. Для детального обговорення звертайтеся до нашого офіційного Discord.

---

<a id="vi"></a>
## Tiếng Việt

`faceit-cs` là một dự án mẫu nghiên cứu anti-cheat cho Windows. Cấu trúc hiện tại gồm:

- `Overlay/`: lớp hiển thị overlay
- `Imgui/`: thành phần UI và phụ thuộc render
- `hypercall/`: logic giao tiếp mức thấp
- `usermode/`: điều khiển user-mode và điểm vào lệnh
- `system.*` / `entities.*` / `renderer.*`: hệ thống runtime, xử lý thực thể và luồng render

### Mục tiêu nghiên cứu

- Nghiên cứu tổ chức giao diện giữa user-mode và thành phần mức thấp
- Nghiên cứu phối hợp giữa luồng render và luồng thu thập dữ liệu
- Nghiên cứu tách mô-đun và quy trình debug

### Tuân thủ và phạm vi

Dự án chỉ phục vụ nghiên cứu bảo mật và trao đổi kỹ thuật, không dùng cho môi trường chưa được ủy quyền.

Một số khóa, chứng chỉ, chuỗi thực thi và sản phẩm bypass/injection là thông tin nhạy cảm nên không công khai trên GitHub. Nếu cần trao đổi sâu hơn, vui lòng liên hệ Discord chính thức của chúng tôi.

