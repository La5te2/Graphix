
// Paint3Dlg.h: 头文件
//
#pragma once
#include "std.h"
#include "afxdialogex.h"
#include <corecrt_math_defines.h>
using namespace std;

// CPaint3Dlg 对话框
class CPaint3Dlg : public CDialogEx
{
// 构造
public:
	CPaint3Dlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PAINT3_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

// 实现
protected:
	HICON m_hIcon;
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

	COLORREF LineColor = RGB(255, 0, 0);
	COLORREF ShapeColor = RGB(0, 255, 0);
	
	struct LineObject {
		int id;
		CPoint start;
		CPoint end;
		int lineWidth;
		int lineType;
		COLORREF color;
		bool selected;
		int algorithm;
		bool visible = true;
		bool operator < (const LineObject& other) const {
			return id > other.id;
		}
	};
	struct EllipseObject {
		int id;
		CRect rect;
		int lineWidth;
		int lineType;
		COLORREF color;
		bool isfilled;
		COLORREF fillColor;
		bool selected;
		int algorithm;
		bool visible = true;
		bool operator < (const EllipseObject& other) const {
			return id > other.id;
		}
	};
	struct ArcObject {
		int id;
		CPoint start;
		CPoint end;
		float angle;
		bool direction; // true顺时针 false逆时针
		int lineWidth;
		int lineType;
		COLORREF color;
		bool selected;
		int algorithm;
		bool visible = true;
		bool operator < (const ArcObject& other) const {
			return id > other.id;
		}
	};
	struct PolygonObject {
		int id;
		vector<CPoint> points;
		int lineWidth;
		int lineType;
		COLORREF color;
		bool isfilled;
		COLORREF fillColor;
		bool selected;
		int algorithm;
		bool visible = true;
		bool operator < (const PolygonObject& other) const {
			return id > other.id;
		}
	};
	struct BezierObject {
		int id;
		std::vector<CPoint> ctrl;   // 控制点
		int lineWidth;
		int lineType;               // 实线/虚线
		COLORREF color;
		bool selected = false;
		int algorithm;              // 例如 ALG_BEZIER_CASTELJAU
		bool visible = true;
		// 可选缓存：采样后的折线点，避免频繁重算
		std::vector<CPoint> cachedPolyline;
		bool cacheValid = false;

		bool operator<(const BezierObject& other) const { return id > other.id; }
	};
	std::vector<BezierObject> Beziers;
	int idBezier = 0;

public:
	bool IsFill = false; // false不填充 true填充
	int Mode = 0; // 0画笔 1直线 2圆
	int Algorithm = 0; 
	// 0 Default 1 DDA 2 Bresenham 3 Midpoint 
	// 4 Default Circle 5 Midpoint Circle 6 Bresenham Circle
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	int LineWidth;
	int LineType;
	afx_msg void OnBnClickedRadio1();
	afx_msg void OnBnClickedRadio2();
	CComboBox m_fill;
	afx_msg void OnCbnSelchangeCombo1();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	bool isDrawing = false;
	CPoint lastPoint;
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnCbnSelchangeCombo2();
	afx_msg void OnEnChangeEdit1();
	afx_msg void OnEnChangeEdit2();
	CComboBox m_mode;
	vector<vector<CPoint>> Pens; // 保存自由绘制的轨迹
	vector<COLORREF> PenColors;  // 每段画笔轨迹对应颜色
	COLORREF BackgroundColor = RGB(255, 255, 255); // 根据您实际背景而定
	afx_msg void OnCbnSelchangeCombo3();
	CComboBox m_algorithm;
	void EraseLastPreview(CDC& dc);
	// 画线相关算法
	int idLine = 0;
	vector<LineObject> Lines; // 存储线条的起点和终点
	CPoint startPoint; // 线条起点
	CPoint endPoint;   // 线条终点
	void DrawLineA(CPoint p1, CPoint p2, CDC& dc, int alg, COLORREF color, int lineWidth, int lineType);
	void DrawLineDefault(CPoint p1, CPoint p2, CDC& dc);
	void DrawLineDDAFM(CPoint p1, CPoint p2, CDC& dc, COLORREF color, int lineWidth, int lineType);
	void DrawLineDDA(CPoint p1, CPoint p2, CDC& dc);
	void DrawLineMidpointFM(CPoint p1, CPoint p2, CDC& dc, COLORREF color, int lineWidth, int lineType);
	void DrawLineMidpoint(CPoint p1, CPoint p2, CDC& dc);
	void DrawLineBresenhamFM(CPoint p1, CPoint p2, CDC& dc, COLORREF color, int lineWidth, int lineType);
	void DrawLineBresenham(CPoint p1, CPoint p2, CDC& dc);
	// 画圆相关算法
	int idEllipse = 0;
	std::vector<EllipseObject> Ellipses;
	CRect lastDrawRect;
	bool hasLastDrawRect = false;
	void DrawEllipseA(CDC& dc, const CRect& rect, COLORREF color, int lineWidth, int lineType, int alg);
	void DrawEllipseDefault(CDC& dc, const CRect& rect);
	void DrawEllipseMidpointFM(CDC& dc, const CRect& rect, COLORREF color, int lineWidth, int lineType);
	void DrawEllipseMidpoint(CDC& dc, const CRect& rect);
	void DrawEllipseBresenhamFM(CDC& dc, const CRect& rect, COLORREF color, int lineWidth, int lineType);
	void DrawEllipseBresenham(CDC& dc, const CRect& rect);
	// 圆弧相关
	int idArc = 0;
	vector<ArcObject> Arcs; // 存储弧线的起点和终点
	bool hasLastDrawArc = false;
	float arcAngleDeg;
	float arcAngle; // 圆弧角度
	float lastArcAngle;
	bool lastArcDirection;
	CPoint lastArcStart;
	CPoint lastArcEnd;
	void DrawArcFM(float angle, bool direction, CPoint p1, CPoint p2, CDC& dc,
		COLORREF color, int lineWidth, int lineType);
	void DrawArc(float angle, bool direction, CPoint p1, CPoint p2, CDC& dc);
	void DrawArcPreview(float angle, bool direction, CPoint p1, CPoint p2, CDC& dc);
	// 多边形相关
	int idPolygon = 0;
	void DrawPolygonFM(CDC& dc, const std::vector<CPoint>& poly, bool isfilled, COLORREF fillColor, bool Clipper);
	vector<PolygonObject> Polygons;
	vector<CPoint> currentPolygon;
	bool isDrawingPolygon = false;
	// 填充相关
	void ScanlineFill(CDC& dc, CPoint seed, COLORREF fillColor, COLORREF borderColor);
	void ScanlineFillFM(CDC& dc, CPoint seed, COLORREF fillColor, COLORREF borderColor);
	// 裁剪相关
	CRect clipRect;
	bool ClipLineCohenSutherland(CPoint& p1, CPoint& p2, CRect clip);
	bool ClipLineMidpoint(CPoint &p1, CPoint &p2, CRect clip);
	bool CyrusBeckClipLine(CPoint& p1, CPoint& p2, const std::vector<CPoint>& clipPoly);
	std::vector<CPoint> SutherlandHodgmanClipPolygon(const std::vector<CPoint>& subject, const std::vector<CPoint>& clipPoly);
	std::vector<std::vector<CPoint>> CPolygons;
	std::vector<CPoint> clipPolygon;   // 裁剪多边形的顶点
	bool isDefiningClipRect = false;    // 是否正在定义矩形裁剪窗口
	bool isDefiningClipPoly = false;   // 是否正在定义多边形裁剪窗口
	bool DefinedClipRect = false; // 是否已经定义了矩形裁剪窗口
	bool DefinedClipPoly = false; // 是否已经定义了多边形裁剪窗口
	// 变换相关
	int selCnt = 0;
	bool hasDrawSelected;
	bool hasDrawScaling;
	bool hasDrawRotating;
	std::vector<CPoint> RB;
	bool IsPointNearLine(const CPoint& p, const LineObject& line);
	bool IsPointNearEllipse(const CPoint& p, const EllipseObject& line);
	bool IsPointNearArc(const CPoint& p, const ArcObject& arc); // 你在 .cpp 中实现了它，但在 .h 中缺失了
	bool IsPointNearPolygon(const CPoint& p, const PolygonObject& line);
	bool isDragging = false;
	bool isScaling = false;
	bool isRotating = false;
	CPoint dragStart; // 拖拽起点 (你已经有了)
	CPoint scaleStart; // 缩放起点 (你已经有了)
	CPoint rotateStart; // 旋转起点 (你已经有了)
	CPoint m_refPoint; // 变换参考点 (重心或自定义点)
	bool m_bUseCustomRefPoint = false; // 是否使用自定义参考点

	void RedrawAll(CDC& dc, bool highlightSelection); // 重画函数

	// 变换需要声明
	CPoint GetCentroid(const LineObject& line);
	CPoint GetCentroid(const EllipseObject& ell);
	CPoint GetCentroid(const PolygonObject& poly);
	CPoint GetCentroid(const ArcObject& arc);
	CPoint CPaint3Dlg::GetCentroid(const BezierObject& bz);
	CPoint GetAverageCentroidOfSelection();

	void ApplyTranslationToSelection(int tx, int ty);
	void ApplyScalingToSelection(const CPoint& ref, double sx, double sy);
	void ApplyRotationToSelection(const CPoint& ref, double angleRad);
	void DeselectAll();

	// 双缓冲
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	// —— 参考点可视化与切换 ——
	// 右键切换：重心 <-> 自定义（鼠标处）
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);

	// 画参考点的小工具
	void DrawRefPoint(CDC& dc, const CPoint& pt, bool custom);

	// 是否有任何被选中的图形
	bool HasAnySelection() const;
	// 拼接
	struct HitInfo {
		int   kind = 0;     // 1=line, 2=ellipse, 3=arc, 4=polygon
		int   index = -1;   // 图元容器的下标
		CPoint hitPoint;    // 命中逻辑返回的“几何点”（顶点/投影点/曲线点）
	};
	int     m_connectStage = 0;  // 0等第一点 1等第二点
	HitInfo m_anchor, m_target;
	bool    m_hasAnchorDraw = false;
	CPoint  m_anchorDraw;

	bool    TryHitExisting(const CPoint& mouse, HitInfo& out);
	void    TranslateOne(const HitInfo& h, int tx, int ty);          
	void    DrawAnchorDotXor(CDC& dc, const CPoint& p);               
	// —— Bezier 相关（实验4） ——
	void EnsureBezierCache(BezierObject& bz);
	void DrawBezierFM(CDC& dc, BezierObject& bz);
	bool IsPointNearBezier(const CPoint& p, BezierObject& bz);

	// 构造中的临时状态
	std::vector<CPoint> currentBezierCtrl; // 录入中的控制点
	int  bezierDragIdx = -1;               // 正在拖的控制点下标，-1 表示没有
	// —— Bezier 命中控制点（返回控制点下标，未命中则 -1） ——
	int HitBezierCtrlPoint(const BezierObject& bz, const CPoint& mouse, int radius /*像素*/);

	// —— 编辑状态（SELECT 模式下） ——
	bool    m_bezierEditing = false;
	int     m_bezierEditIndex = -1;  // 正在编辑的 Bezier 在 Beziers[] 的下标
	int     m_bezierCtrlIndex = -1;  // 正在编辑的控制点下标
	struct BSplineObject {
		int id = 0;
		std::vector<CPoint> ctrl;        // 控制点
		int degree = 3;                  // p：默认三次B样条
		std::vector<double> knots;       // 节点向量（clamped uniform）
		int lineWidth = 1;
		int lineType = 0;
		COLORREF color = RGB(0, 0, 0);
		bool selected = false;
		int algorithm = 0;               // ALG_BSPLINE_DEBOOR
		bool visible = true;

		std::vector<CPoint> cachedPolyline;
		bool cacheValid = false;

		bool operator<(const BSplineObject& other) const { return id > other.id; }
	};

	std::vector<BSplineObject> BSplines;
	int idBSpline = 0;

	// —— B样条相关接口 —— 
	void EnsureBSplineCache(BSplineObject& sp);
	void DrawBSplineFM(CDC& dc, BSplineObject& sp);
	bool IsPointNearBSpline(const CPoint& p, BSplineObject& sp);

	// 录入临时态（和 Bezier 一样）
	std::vector<CPoint> currentBSplineCtrl;
	int  bsplineDragIdx = -1;
	int  defaultBSplineDegree = 3; // 需要的话允许外部改

};
