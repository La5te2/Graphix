# 用户操作手册

> 本程序是《计算机图形学》课程实验 1~4（含挑战）在同一框架中的实现，支持**基本图元绘制、区域填充、线段/多边形裁剪、二维几何变换、Bezier 与 B-样条曲线**等功能。程序以 MFC 对话框为界面，通过“模式（Mode）+ 算法（Algorithm）+ 参数（颜色/线宽/线型）”的方式进行交互。
>
> 作者：lzh，cqh，sjh（拼音缩写），from Shanghai University

---

## 1. 界面总览

- 上方**模式（Mode）**下拉框：选择当前工作模式（Pen/Line/Circle/Arc/Polygon/Fill/Clip/Select/Connect/Bezier/B-Spline）。  
- 右侧**算法（Algorithm）**下拉框：随模式动态更新
- 线宽输入框（`LineWidth`）、线型单选（实线/虚线）、两枚**取色按钮**：分别设置画线颜色 `LineColor` 与填充颜色 `ShapeColor`。  
- **填充（Filled/Not Filled）**：控制多边形/椭圆是否在创建时填充。  
- 主绘图区采用**双缓冲**重绘，减少闪烁；同时禁止默认擦背景（`OnEraseBkgnd` 返回 TRUE）。

---

## 2. 模式与算法

程序内部定义了稳定的**模式枚举**与**算法枚举**（仅列出常用）：  
- 模式：`PEN(0), LINE(1), CIRCLE(2), ARC(3), POLYGON(4), FILL(5), CLIP(6), SELECT(7), CONNECT(8), BEZIER(9), BSPLINE(10)`。  
- 算法：直线 `DEFAULT/DDA/MIDPOINT/BRESENHAM`；圆/椭圆 `DEFAULT/MIDPOINT/BRESENHAM`；圆弧 `BRESENHAM`；裁剪 `RECT/POLY`；变换 `TRANSLATION/SCALING/ROTATION`；曲线 `BEZIER_CASTELJAU`、`BSPLINE_DEBOOR`。

**算法下拉内容**会根据模式自动填充：  

- Line：Default、DDA、Midpoint、Bresenham（算法ID 0~3）。  
- Circle：Default、Midpoint、Bresenham（ID 4~6）。  
- Arc：Bresenham Arc（ID 7）。  
- Polygon：Default Polygon（ID 8）。  
- Fill：Scanline Fill（ID 9）。  
- Clip：Rect Clip（9）、Polygon Clip（10）。  
- Select：Translation(11) / Scaling(12) / Rotation(13) 。

---

## 3. 基本绘制操作

### 3.1 画笔（Pen）
- 选择 **Pen**，按住左键拖动即可绘制自由曲线；颜色取自 `LineColor`。

### 3.2 直线（Line）
1. 模式选 **Line**，算法可选 DDA/Midpoint/Bresenham 等。  
2. 左键按下为起点、移动显示预览、松开即生成线段（可叠加裁剪窗口裁剪）。  
3. 直线算法均支持线宽(EDIT 1)/虚线。

### 3.3 圆/椭圆（Circle）
1. 模式选 **Circle**，按下设置对角点并拖拽。按住 **Shift** 可强制为正圆。  
2. 算法支持默认 GDI、Midpoint、Bresenham，均支持线宽/虚线。

### 3.4 圆弧（Arc）
1. 模式选 **Arc**，左键按下设置起点、移动设置终点；**角度**在 `Edit2` 输入（1~180°）。  
2. 预览：按住 **Shift** 切换顺/逆时针（默认逆时针）。松开生成弧段。

### 3.5 多边形（Polygon）
1. 模式选 **Polygon**，逐点左键点击输入顶点。  
2. **Ctrl** 结束并生成多边形；如已设置裁剪窗口，则在提交前执行 Sutherland-Hodgman 裁剪。

---

## 4. 填充与裁剪

### 4.1 区域填充（Fill）
- 模式选 **Fill**，在目标区域内**单击种子点**，使用扫描线填充（颜色取 `ShapeColor`）。

### 4.2 线段/多边形裁剪（Clip）
- 模式选 **Clip**：  
  - **Rect Clip Window**：拖出矩形，即刻成为当前裁剪窗口（以灰细虚线显示）。  
  - **Polygon Clip Window**：逐点输入顶点，按 **Ctrl** 结束设置。  
- 之后绘制的**线段**会先做 Cohen-Sutherland（矩形）或 Cyrus-Beck（凸多边形）裁剪；**多边形**提交前做 Sutherland-Hodgman 裁剪。

---

## 5. 选择与几何变换（Select）

### 5.1 选中
- 切换到 **Select**。在图元边界附近**单击可切换选中**；按住 **Shift** 支持多选。未选中会提示信息。  
- 选中图元会以橙色高亮，当前参考点在画面上显示“小圆点+十字”（默认为选中集**平均重心**）。

### 5.2 参考点切换
- 在 Select 模式**右键**：在“平均重心”与“鼠标位置的自定义点”之间切换作为**变换参考点**。

### 5.3 平移 / 缩放 / 旋转
- 在算法下拉选择 **Translation / Scaling / Rotation**，然后**拖动鼠标**：  
  - **Translation**：直接拖动；若参考点为自定义点，它会随对象一起移动。  
  - **Scaling**：以参考点为中心，基于鼠标与参考点的距离变化计算比例；按 **Shift** 时，X/Y 轴分别独立缩放。  
  - **Rotation**：以参考点为中心，按当前与上一帧鼠标连线的夹角增量旋转。  
- **Esc** 快速清空选择。

> 注：非均匀缩放会把“圆弧”形变为“椭圆弧”；对**非正圆的椭圆**进行旋转时，程序会将其**转换为多边形近似**再旋转，属于设计上的可视化近似（也可以认为是bug）。

---

## 6. Bezier 与 B-样条（实验 4）

### 6.1 Bezier（de Casteljau）
- 模式选 **Bezier**：  
  -  **左键单击**依次添加控制点；若命中已有控制点则进入**拖动修改**。  
  - 控制折线与点有预览；≥3点时同步预览曲线。  
  -  **按 Ctrl** 结束并生成曲线对象；之后可在 Select 模式选中并变换。
- 命中判定基于“采样折线的点到线段最短距离”，绘制尊重线宽/虚线；自带采样缓存提升效率。

### 6.2 B-样条（de Boor，挑战）
- 模式选 **B-Spline**：  
  - **左键**逐点录入控制点；若命中已有控制点则拖动修改。  
  - 当点数 ≥ `p+1`（默认 `p=3`）时显示曲线预览（自动生成**均匀夹持**节点向量）。  
  -  **按 Ctrl** 结束并生成曲线对象。
- 评估使用 de Boor，支持命中检测与折线绘制。

---

## 7. 图元对接（Connect，挑战）

- 模式选 **Connect**，分两步：  
  - 第一次点击：命中**第一个**图元的轮廓/顶点，程序以 XOR 方式标注锚点；  
  - 第二次点击：命中**第二个**图元，程序将其**C0 平移对齐**到第一点上（零缝拼接）。

---

## 8. 快捷键

- **Ctrl**：结束当前多边形/Bezier/B-样条的输入并“落盘”。  
- **Shift**：  
  - Circle 模式：强制正圆；Arc 预览时切换方向；  
  - Scaling 时：X/Y 轴独立缩放。  
- **Esc**：清空选择并刷新。  
- **右键（Select 模式）**：在“重心参考点 ↔ 自定义参考点”间切换。

---

## 9. 构建与运行

- **环境**：Visual Studio（MFC），C++14。程序主对话框类为 `CPaint3Dlg`，入口初始化界面控件与默认参数。  
- **文件结构（摘）**：  
  - `Paint3Dlg.{h,cpp}`：界面、事件、对象容器与重绘调度。  
  - `AlgMenu.{h,cpp}`：根据模式刷新算法下拉框。  
  - `Line.h / Ellipse.h`：直线与圆/椭圆的多算法绘制（含 DIB 局部渲染）。  
  - `transform.cpp`：重心计算、平移/缩放/旋转实现。  
  - `Bezier.cpp / BSpline.cpp`：曲线评估、采样缓存、命中检测与绘制。  

---

## 10. 与课程实验的对应关系

- **实验一：基本图元** —— 直线 Midpoint/Bresenham、圆/椭圆 Midpoint/Bresenham；线宽/线型参数控制。  
- **实验二：填充与裁剪** —— 扫描线填充；Cohen-Sutherland/Cyrus-Beck 线段裁剪；Sutherland-Hodgman 多边形裁剪。  
- **实验三：二维变换** —— 选中集平移/缩放/旋转，支持重心/任意点为参考点。  
- **实验四：二维曲线** —— Bezier（de Casteljau）交互编辑；挑战：B-样条（de Boor）。

---


