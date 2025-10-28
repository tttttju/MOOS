# uReplay 功能说明与升级记录

## 代码位置 · Source Location
- **入口程序 / Entry Point**：`moos-ivp-extend/src/uReplay/uReplayMain.cpp` 在 `moos-ivp-extend` 树中提供二进制入口，复用 `LogViewLauncher` 逻辑并保持与 `alogview` 一致的回放流程体验。【F:moos-ivp-extend/src/uReplay/uReplayMain.cpp†L1-L99】
- **构建配置 / Build Wiring**：`moos-ivp-extend/src/uReplay/CMakeLists.txt` 将 `alogview` 相关的 GUI 源文件（`REPLAY_GUI.cpp`、`NavPlotViewer.cpp` 等）从基础 `moos-ivp` 树中拉入编译，并补齐 `FLTK`、`marineview` 等依赖以生成 `uReplay` 可执行文件。【F:moos-ivp-extend/src/uReplay/CMakeLists.txt†L1-L45】

## 功能概览 · Functional Highlights
- **回放控制 / Replay Control**：菜单项与快捷键可切换流式回放、调节步进间隔，并支持按时间跳转或步进浏览历史数据，涵盖暂停、快进、快退等操作。【F:moos-ivp/ivp/src/app_alogview/REPLAY_GUI.cpp†L189-L214】【F:moos-ivp/ivp/src/app_alogview/REPLAY_GUI.cpp†L298-L359】
- **时间倍率管理 / Time-Warp Management**：在流式模式下可通过加减速调整播放倍率，并实时计算实际执行倍率以提示用户回放是否滞后。【F:moos-ivp/ivp/src/app_alogview/REPLAY_GUI.cpp†L814-L900】
- **多视图联动 / Linked Multi-Viewers**：主界面同时维护导航视图与日志曲线视图，用户操作会同步更新时间与子窗口中的变量值显示，确保各数据视图保持一致。【F:moos-ivp/ivp/src/app_alogview/REPLAY_GUI.cpp†L724-L938】
- **数据代理整合 / Data Broker Integration**：回放界面通过 `ALogDataBroker` 动态生成日志变量、行为报告、应用日志等菜单项，并向导航与曲线视图分发同一份数据上下文。【F:moos-ivp/ivp/src/app_alogview/REPLAY_GUI.cpp†L944-L1109】

## 核心逻辑 · Core Logic
1. **日志资源准备 / Log Resource Preparation**：`ALogDataBroker` 负责登记 `.alog` 或 `.sqlite` 文件，区分不同格式，并在需要时为普通日志创建拆分器实例。【F:moos-ivp/ivp/src/lib_logutils/ALogDataBroker.cpp†L140-L157】
2. **有效性检查与缓存 / Validation & Caching**：在加载前会逐个核验日志可读性，同时应用可能的“脱离对”配置以矫正文件内容；随后触发缓存拆分输出与摘要文件，用于后续检索。【F:moos-ivp/ivp/src/lib_logutils/ALogDataBroker.cpp†L162-L231】
3. **时间轴与元数据构建 / Timeline & Metadata Construction**：解析摘要文件得到每个参与体的起止时间、类型、颜色、长度等信息，并对齐全局起始时间与时间漂移，形成统一的时间窗口。【F:moos-ivp/ivp/src/lib_logutils/ALogDataBroker.cpp†L238-L310】
4. **主索引建立 / Master Index Construction**：为每个“车辆/变量”组合分配主索引，记录变量类型与来源，同时对行为报告与应用日志建立独立索引，便于 GUI 通过单一编号访问不同数据集。【F:moos-ivp/ivp/src/lib_logutils/ALogDataBroker.cpp†L625-L688】
5. **视图初始化 / Viewer Initialization**：导航视图读取代理中各车辆的元数据与关键导航变量曲线，而日志曲线视图在选择左、右变量时同样通过代理获取对应的 `LogPlot` 数据。【F:moos-ivp/ivp/src/app_alogview/NavPlotViewer.cpp†L145-L209】【F:moos-ivp/ivp/src/app_alogview/LogPlotViewer.cpp†L134-L215】

## 处理流程 · Processing Flow
1. **加载日志 / Load Logs**：用户指定日志后，`ALogDataBroker` 逐个登记文件、检查有效性，并在必要时拆分缓存，为 GUI 提供统一的摘要入口。【F:moos-ivp/ivp/src/lib_logutils/ALogDataBroker.cpp†L140-L231】
2. **计算时间基准 / Compute Time Baselines**：代理解析摘要并对齐多日志的起止时间，记录偏移值以保证多平台数据在同一时间轴上呈现。【F:moos-ivp/ivp/src/lib_logutils/ALogDataBroker.cpp†L238-L310】
3. **生成菜单与索引 / Generate Menus & Indices**：GUI 初始化时根据代理提供的主索引构建回放菜单（变量、行为、应用日志等），并配置导航/曲线视图所需的数据指针。【F:moos-ivp/ivp/src/app_alogview/REPLAY_GUI.cpp†L944-L1109】
4. **同步播放 / Synchronized Playback**：用户通过菜单或快捷键驱动流式播放，时间步进会同时更新导航视图与各子窗口；如果启用自动播放，系统会依据时间倍率判定是否推进下一帧。【F:moos-ivp/ivp/src/app_alogview/REPLAY_GUI.cpp†L724-L812】【F:moos-ivp/ivp/src/app_alogview/REPLAY_GUI.cpp†L1192-L1256】
5. **显示刷新 / Display Refresh**：每次时间更新后，界面刷新当前时间、变量读数与播放倍率提示，确保用户实时了解播放状态。【F:moos-ivp/ivp/src/app_alogview/REPLAY_GUI.cpp†L905-L938】

## 升级记录 · Upgrade Journal
| 日期 Date | 版本 Version | 变更说明 Change Notes |
| --- | --- | --- |
| 2025-03-09 | v1.0 | 初始文档，概述 uReplay 功能、逻辑与流程。Initial document covering uReplay features, logic, and flow. |

> 后续若有新功能、界面或数据流程改动，请在上表追加条目并更新相关章节描述。
