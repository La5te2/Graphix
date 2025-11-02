
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
	
	bool IsFill = false; // false不填充 true填充
	int Mode = 0; // 0画笔 1直线 2圆
	int Algorithm = 0; 
	// 0 Default 1 DDA 2 Bresenham 3 Midpoint 
	// 4 Default Circle 5 Midpoint Circle 6 Bresenham Circle
	struct LineObject {
		CPoint start;
		CPoint end;
		int lineWidth;
		int lineType;
		COLORREF color;
		bool selected;
		int algorithm;
	};
public:
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	int LineWidth;
	afx_msg void OnEnChangeEdit1();
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
	CComboBox m_mode;
	vector<vector<CPoint>> Pens; // 保存自由绘制的轨迹
	vector<COLORREF> PenColors;  // 每段画笔轨迹对应颜色
	COLORREF BackgroundColor = RGB(255, 255, 255); // 根据您实际背景而定
	afx_msg void OnCbnSelchangeCombo3();
	CComboBox m_algorithm;
	void EraseLastPreview(CDC& dc);
	// 画线相关算法
	vector<LineObject> Lines; // 存储线条的起点和终点
	CPoint startPoint; // 线条起点
	CPoint endPoint;   // 线条终点
	bool isDragging = false;
	CPoint lastMouse;
	void DrawLineDefault(CPoint p1, CPoint p2, CDC& dc);
	void DrawLineDDAFM(CPoint p1, CPoint p2, CDC& dc, COLORREF color, int lineWidth, int lineType);
	void DrawLineDDA(CPoint p1, CPoint p2, CDC& dc);
	void DrawLineMidpointFM(CPoint p1, CPoint p2, CDC& dc, COLORREF color, int lineWidth, int lineType);
	void DrawLineMidpoint(CPoint p1, CPoint p2, CDC& dc);
	void DrawLineBresenhamFM(CPoint p1, CPoint p2, CDC& dc, COLORREF color, int lineWidth, int lineType);
	void DrawLineBresenham(CPoint p1, CPoint p2, CDC& dc);
	// 画圆相关算法
	std::vector<CRect> Ellipses;
	CRect lastDrawRect;
	bool hasLastDrawRect = false;
	void DrawEllipseMidpoint(CDC& dc, const CRect& rect);
	void DrawEllipseBresenham(CDC& dc, const CRect& rect);
	// 圆弧相关
	vector<pair<CPoint, CPoint>> Arcs; // 存储弧线的起点和终点
	bool hasLastDrawArc = false;
	float arcAngleDeg;
	float arcAngle; // 圆弧角度
	float lastArcAngle;
	bool lastArcDirection;
	CPoint lastArcStart;
	CPoint lastArcEnd;
	void DrawArc(float angle, bool direction, CPoint p1, CPoint p2, CDC& dc);
	void DrawArcPreview(float angle, bool direction, CPoint p1, CPoint p2, CDC& dc);
	// 多边形相关
	void ScanConvertPolygonOutline(CDC& dc, const std::vector<CPoint>& poly, bool Clipper);
	afx_msg void OnEnChangeEdit2();
	vector<vector<CPoint>> Polygons;
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
	bool IsPointNearLine(const CPoint& p, const LineObject& line);
};
