
// Paint3Dlg.h: 头文件
//

#pragma once
#include "std.h"
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
	vector<pair<CPoint, CPoint>> Lines; // 存储线条的起点和终点
	CPoint startPoint; // 线条起点
	CPoint endPoint;   // 线条终点
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	bool isDrawing = false;
	CPoint lastPoint;
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnCbnSelchangeCombo2();
	CComboBox m_mode;
	std::vector<CRect> Ellipses;
	CRect lastDrawRect;
	bool hasLastDrawRect = false;
	vector<vector<CPoint>> Pens; // 保存自由绘制的轨迹
	vector<COLORREF> PenColors;  // 每段画笔轨迹对应颜色
	COLORREF BackgroundColor = RGB(255, 255, 255); // 根据您实际背景而定
	afx_msg void OnCbnSelchangeCombo3();
	CComboBox m_algorithm;
	void DrawLineDDA(CPoint p1, CPoint p2, CDC& dc);
	void DrawLineMidpoint(CPoint p1, CPoint p2, CDC& dc);
	void DrawLineBresenham(CPoint p1, CPoint p2, CDC& dc);
};
