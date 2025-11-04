
﻿
// Paint3Dlg.cpp: 实现文件
// 
#include "pch.h"
#include "afxdialogex.h"
#include "AlgMenu.h"
#include "Clip.h"
#include "Ellipse.h"
#include "framework.h"
#include "Line.h"
#include "Paint3.h"
#include "Paint3Dlg.h"
#include "ScanLine.h"
#include "std.h"
#include <stdint.h>
#include <cstddef>
#include <cstdint>
#include <stdbool.h>
#include <tchar.h>
#include <corecrt_math_defines.h>
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
;
using namespace std;

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();
	// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CPaint3Dlg 对话框



CPaint3Dlg::CPaint3Dlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_PAINT3_DIALOG, pParent)
	, LineWidth(5)
	, LineType(0)
	, arcAngle(M_PI)
	, arcAngleDeg(180)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CPaint3Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, LineWidth);
	DDV_MinMaxInt(pDX, LineWidth, 0, 20);
	DDX_Radio(pDX, IDC_RADIO1, LineType);
	DDX_Control(pDX, IDC_COMBO1, m_fill);
	DDX_Control(pDX, IDC_COMBO2, m_mode);
	DDX_Control(pDX, IDC_COMBO3, m_algorithm);
	DDX_Text(pDX, IDC_EDIT2, arcAngleDeg);
}

BEGIN_MESSAGE_MAP(CPaint3Dlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CPaint3Dlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CPaint3Dlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_RADIO1, &CPaint3Dlg::OnBnClickedRadio1)
	ON_BN_CLICKED(IDC_RADIO2, &CPaint3Dlg::OnBnClickedRadio2)
	ON_CBN_SELCHANGE(IDC_COMBO1, &CPaint3Dlg::OnCbnSelchangeCombo1)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_CBN_SELCHANGE(IDC_COMBO2, &CPaint3Dlg::OnCbnSelchangeCombo2)
	ON_CBN_SELCHANGE(IDC_COMBO3, &CPaint3Dlg::OnCbnSelchangeCombo3)
	ON_WM_KEYDOWN()
	ON_EN_CHANGE(IDC_EDIT1, &CPaint3Dlg::OnEnChangeEdit1)
	ON_EN_CHANGE(IDC_EDIT2, &CPaint3Dlg::OnEnChangeEdit2)
END_MESSAGE_MAP()


// CPaint3Dlg 消息处理程序

BOOL CPaint3Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	LineColor = RGB(0, 0, 255);
	ShapeColor = RGB(255, 2, 0);

	LineWidth = 5;
	m_fill.AddString(_T("Filled"));
	m_fill.AddString(_T("Not Filled"));
	m_fill.SetCurSel(1);
	m_mode.AddString(_T("Pen"));
	m_mode.AddString(_T("Line"));
	m_mode.AddString(_T("Circle"));
	m_mode.AddString(_T("Arc"));
	m_mode.AddString(_T("Polygon"));
	m_mode.AddString(_T("Fill"));
	m_mode.AddString(_T("Clip"));
	m_mode.AddString(_T("Select"));
	m_mode.SetCurSel(0);
	AlgMenu::RefreshAlgorithmListForMode(*this);
	UpdateData(FALSE);
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CPaint3Dlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。
BOOL CPaint3Dlg::PreTranslateMessage(MSG* pMsg) // 捕获键盘消息
{
	if (pMsg->message == WM_KEYDOWN)
	{
		OnKeyDown((UINT)pMsg->wParam, (UINT)pMsg->lParam, 0);
	}
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN)
	{
		// 禁止回车触发关闭
		return TRUE;
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}
void CPaint3Dlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CPaint3Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

bool CPaint3Dlg::IsPointNearLine(const CPoint& p, const LineObject& line)
{
	double x0 = p.x, y0 = p.y;
	double x1 = line.start.x, y1 = line.start.y;
	double x2 = line.end.x, y2 = line.end.y;

	double dx = x2 - x1, dy = y2 - y1;
	double len2 = dx * dx + dy * dy;
	if (len2 == 0) return false; // 退化成点

	double t = ((x0 - x1) * dx + (y0 - y1) * dy) / len2;
	t = max(0.0, min(1.0, t));
	double projX = x1 + t * dx;
	double projY = y1 + t * dy;

	double dist = sqrt((x0 - projX) * (x0 - projX) + (y0 - projY) * (y0 - projY));
	return dist <= (line.lineWidth / 2.0 + 3.0); // 允许微小容差
}
bool CPaint3Dlg::IsPointNearEllipse(const CPoint& p, const EllipseObject& e)
{
	// 计算椭圆中心与半轴
	double xc = (e.rect.left + e.rect.right) / 2.0;
	double yc = (e.rect.top + e.rect.bottom) / 2.0;
	double a = fabs(e.rect.right - e.rect.left) / 2.0;
	double b = fabs(e.rect.bottom - e.rect.top) / 2.0;
	if (a < 1e-6 || b < 1e-6) return false;

	// 计算归一化后的椭圆方程值
	double dx = (p.x - xc);
	double dy = (p.y - yc);
	double value = (dx * dx) / (a * a) + (dy * dy) / (b * b);

	// 判断距离边界的误差（允许一定容差）
	// 理想椭圆上 value = 1，<1 在内部，>1 在外部
	double tol = (e.lineWidth / 2.0 + 3.0) / max(a, b);
	return fabs(value - 1.0) <= tol;
}


bool CPaint3Dlg::IsPointNearPolygon(const CPoint& p, const PolygonObject& poly)
{
	int n = (int)poly.points.size();
	if (n < 2) return false;
	for (int i = 0; i < n; ++i)
	{
		CPoint a = poly.points[i];
		CPoint b = poly.points[(i + 1) % n];
		double x0 = p.x, y0 = p.y;
		double x1 = a.x, y1 = a.y;
		double x2 = b.x, y2 = b.y;
		double dx = x2 - x1, dy = y2 - y1;
		double len2 = dx * dx + dy * dy;
		if (len2 == 0) continue;

		double t = ((x0 - x1) * dx + (y0 - y1) * dy) / len2;
		t = max(0.0, min(1.0, t));
		double projX = x1 + t * dx;
		double projY = y1 + t * dy;
		double dist = sqrt((x0 - projX) * (x0 - projX) + (y0 - projY) * (y0 - projY));
		if (dist <= (poly.lineWidth / 2.0 + 3.0))
			return true;
	}
	if (poly.isfilled)
	{
		bool inside = false;
		for (int i = 0, j = n - 1; i < n; j = i++)
		{
			double xi = poly.points[i].x, yi = poly.points[i].y;
			double xj = poly.points[j].x, yj = poly.points[j].y;
			bool intersect = ((yi > p.y) != (yj > p.y)) &&
				(p.x < (xj - xi) * (p.y - yi) / (yj - yi + 1e-9) + xi);
			if (intersect)
				inside = !inside;
		}
		return inside;
	}

	return false;
}

void CPaint3Dlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	SetCapture();  // 捕获鼠标
	isDrawing = true;
	startPoint = lastPoint = point;
	CDialogEx::OnLButtonDown(nFlags, point);
	if (Mode == 0) // Pen Mode
	{
		vector<CPoint> newStroke;
		newStroke.push_back(point);
		Pens.push_back(newStroke);
		PenColors.push_back(LineColor);
	}
	if (Mode == 7) // Select
	{
		bool hit = false;
		int cnt = 0;
		for (auto& line : Lines)
		{
			if (line.visible && IsPointNearLine(point, line))
			{
				++cnt;
				line.selected = true;
				hit = true;
			}
		}
		for (auto& ell : Ellipses)
		{
			if (ell.visible && IsPointNearEllipse(point, ell))
			{
				++cnt;
				ell.selected = true;
				hit = true;
			}
		}
		for (auto& poly : Polygons)
		{
			if (poly.visible && IsPointNearPolygon(point, poly))
			{
				++cnt;
				poly.selected = true;
				hit = true;
			}
		}
		if (!hit)
		{
			AfxMessageBox(_T("未选中任何图形。"));
		}
		else
		{
			CClientDC dc(this);
			dc.SetROP2(R2_COPYPEN);
			Invalidate(1);
			UpdateWindow();

			// 重新绘制所有可见图形（保持选中状态）
			for (auto& line : Lines)
			{
				if (!line.visible || line.selected) continue;
				int penStyle = line.lineType ? PS_DASH : PS_SOLID;
				LOGBRUSH logBrush = { BS_SOLID, line.color, 0 };
				CPen pen(penStyle | PS_GEOMETRIC | PS_ENDCAP_ROUND, line.lineWidth, &logBrush);
				CPen* oldPen = dc.SelectObject(&pen);
				DrawLineA(line.start, line.end, dc, line.algorithm, line.color, line.lineWidth, line.lineType);
				dc.SelectObject(oldPen);
			}

			for (auto& ell : Ellipses)
			{
				if (ell.selected) continue;
				int penStyle = ell.lineType ? PS_DASH : PS_SOLID;
				LOGBRUSH logBrush = { BS_SOLID, ell.color, 0 };
				CPen pen(penStyle | PS_GEOMETRIC | PS_ENDCAP_ROUND, ell.lineWidth, &logBrush);
				CPen* oldPen = dc.SelectObject(&pen);
				CBrush* pNullBrush = CBrush::FromHandle((HBRUSH)GetStockObject(NULL_BRUSH));
				CBrush* oldBrush = dc.SelectObject(pNullBrush);
				DrawEllipseA(dc, ell.rect, ell.color, ell.lineWidth, ell.lineType, ell.algorithm);
				dc.SelectObject(oldBrush);
				dc.SelectObject(oldPen);
			}

			for (auto& poly : Polygons)
			{
				if (!poly.visible || poly.selected) continue;
				int penStyle = poly.lineType ? PS_DASH : PS_SOLID;
				LOGBRUSH logBrush = { BS_SOLID, poly.color, 0 };
				CPen pen(penStyle | PS_GEOMETRIC | PS_ENDCAP_ROUND, poly.lineWidth, &logBrush);
				CPen* oldPen = dc.SelectObject(&pen);
				DrawPolygonFM(dc, poly.points, poly.isfilled, poly.fillColor, false);
				dc.SelectObject(oldPen);
			}
		}
		if (Algorithm == 11)
		{
			isDragging = true;
			dragStart = point;
		}
		else if (Algorithm == 12)
		{
			isScaling = true;
			scaleStart = point;
		}
		else if (Algorithm == 13)
		{
			isRotating = true;
			rotateStart = point;
		}
	}
}
void CPaint3Dlg::OnMouseMove(UINT nFlags, CPoint point)
{
	if (isDrawing)
	{
		CClientDC dc(this);
		dc.SetROP2(R2_NOTXORPEN);

		/*CPen pen(PS_SOLID, 1, LineColor);
		CPen* oldPen = dc.SelectObject(&pen);*/

		if (Mode == 0)
		{
			int penStyle = LineType ? PS_DASH : PS_SOLID;
			LOGBRUSH logBrush = { BS_SOLID, LineColor, 0 };
			CPen pen(penStyle | PS_GEOMETRIC | PS_ENDCAP_ROUND, LineWidth, &logBrush);
			CPen* oldPen = dc.SelectObject(&pen);
			dc.SetROP2(R2_COPYPEN);
			Pens.back().push_back(point);
			dc.MoveTo(lastPoint);
			dc.LineTo(point);
			dc.SelectObject(oldPen);
		}

		else if (Mode == 1) // Line
		{
			int penStyle = LineType ? PS_DASH : PS_SOLID;
			LOGBRUSH logBrush = { BS_SOLID, LineColor, 0 };
			CPen pen(penStyle | PS_GEOMETRIC | PS_ENDCAP_ROUND, LineWidth, &logBrush);
			CPen* oldPen = dc.SelectObject(&pen);
			dc.MoveTo(startPoint);
			dc.LineTo(lastPoint);

			dc.MoveTo(startPoint);
			dc.LineTo(point);
			dc.SelectObject(oldPen);
		}
		else if (Mode == 2)
		{
			int penStyle = LineType ? PS_DASH : PS_SOLID;
			LOGBRUSH logBrush = { BS_SOLID, LineColor, 0 };
			CPen pen(penStyle | PS_GEOMETRIC | PS_ENDCAP_ROUND, LineWidth, &logBrush);
			CPen* oldPen = dc.SelectObject(&pen);
			// 擦除旧图形
			if (hasLastDrawRect)
				dc.Ellipse(lastDrawRect);

			// 构造新的矩形
			CRect newRect(startPoint, point);
			newRect.NormalizeRect();

			// Shift 正圆支持
			if (GetKeyState(VK_SHIFT) & 0x8000)
			{
				int len = min(newRect.Width(), newRect.Height());
				if (point.x < startPoint.x) newRect.left = startPoint.x - len;
				else newRect.right = startPoint.x + len;
				if (point.y < startPoint.y) newRect.top = startPoint.y - len;
				else newRect.bottom = startPoint.y + len;
			}

			// 绘制新的临时图形
			dc.Ellipse(newRect);

			// 保存本次真实绘制形状
			lastDrawRect = newRect;
			hasLastDrawRect = true;
			dc.SelectObject(oldPen);
		}

		else if (Mode == 3) // Arc Preview
		{
			int penStyle = LineType ? PS_DASH : PS_SOLID;
			LOGBRUSH logBrush = { BS_SOLID, LineColor, 0 };
			CPen pen(penStyle | PS_GEOMETRIC | PS_ENDCAP_ROUND, LineWidth, &logBrush);
			CPen* oldPen = dc.SelectObject(&pen);
			// 擦除旧圆弧（XOR）
			if (hasLastDrawArc)
			{
				DrawArcPreview(lastArcAngle, lastArcDirection, lastArcStart, lastArcEnd, dc);
			}
			// 当前终点
			arcAngle = arcAngleDeg * M_PI / 180.0;
			CPoint endPoint = point;
			// 判断方向（Shift 控制）
			bool direction = (GetKeyState(VK_SHIFT) & 0x8000) == 0; // true=逆时针, false=顺时针
			// 绘制新的圆弧预览
			DrawArcPreview((float)arcAngle, direction, startPoint, endPoint, dc);
			// 保存状态供擦除
			hasLastDrawArc = true;
			lastArcAngle = (float)arcAngle;
			lastArcDirection = direction;
			lastArcStart = startPoint;
			lastArcEnd = endPoint;
			dc.SelectObject(oldPen);
		}
		else if (Mode == 7)
		{
			dc.SetROP2(R2_NOTXORPEN);
			if (Algorithm == 11)
			{
				if (isDragging && (nFlags & MK_LBUTTON))
				{
					// 先擦掉上一次预览（再次画一次相同线段）
					if (hasDrawSelected)
					{
						for (auto& line : Lines)
						{
							if (line.selected)
							{
								int penStyle = line.lineType ? PS_DASH : PS_SOLID;
								LOGBRUSH logBrush = { BS_SOLID, line.color, 0 };
								CPen pen(penStyle | PS_GEOMETRIC | PS_ENDCAP_ROUND, line.lineWidth, &logBrush);
								CPen* oldPen = dc.SelectObject(&pen);
								DrawLineA(line.start, line.end, dc, 0, line.color, line.lineWidth, line.lineType);
								dc.SelectObject(oldPen);
							}
						}
						for (auto& ell : Ellipses)
						{
							if (!ell.selected) continue;
							int penStyle = ell.lineType ? PS_DASH : PS_SOLID;
							LOGBRUSH logBrush = { BS_SOLID, ell.color, 0 };
							CPen pen(penStyle | PS_GEOMETRIC | PS_ENDCAP_ROUND, ell.lineWidth, &logBrush);
							CPen* oldPen = dc.SelectObject(&pen);
							CBrush* pNullBrush = CBrush::FromHandle((HBRUSH)GetStockObject(NULL_BRUSH));
							CBrush* oldBrush = dc.SelectObject(pNullBrush);
							DrawEllipseA(dc, ell.rect, ell.color, ell.lineWidth, ell.lineType, 4);
							dc.SelectObject(oldBrush);
							dc.SelectObject(oldPen);
						}

						for (auto& poly : Polygons)
						{
							if (!poly.visible || !poly.selected) continue;
							int penStyle = poly.lineType ? PS_DASH : PS_SOLID;
							LOGBRUSH logBrush = { BS_SOLID, poly.color, 0 };
							CPen pen(penStyle | PS_GEOMETRIC | PS_ENDCAP_ROUND, poly.lineWidth, &logBrush);
							CPen* oldPen = dc.SelectObject(&pen);
							DrawPolygonFM(dc, poly.points, poly.isfilled, poly.fillColor, false);
							dc.SelectObject(oldPen);
						}
					}

					// 计算平移偏移
					int dx = point.x - lastPoint.x;
					int dy = point.y - lastPoint.y;

					// 临时平移选中线段
					for (auto& line : Lines)
					{
						if (line.selected)
						{
							line.start = line.start + CPoint(dx, dy);
							line.end = line.end + CPoint(dx, dy);
						}
					}
					for (auto& ell : Ellipses)
					{
						if (ell.selected)
						{
							ell.rect.OffsetRect(dx, dy);
						}
					}
					for (auto& poly : Polygons)
					{
						if (poly.selected)
						{
							for (auto& pt : poly.points)
							{
								pt = pt + CPoint(dx, dy);
							}
						}
					}
					// 绘制新的预览
					for (auto& line : Lines)
					{
						if (!line.selected) continue;
						int penStyle = line.lineType ? PS_DASH : PS_SOLID;
						LOGBRUSH logBrush = { BS_SOLID, line.color, 0 };
						CPen pen(penStyle | PS_GEOMETRIC | PS_ENDCAP_ROUND, line.lineWidth, &logBrush);
						CPen* oldPen = dc.SelectObject(&pen);
						DrawLineA(line.start, line.end, dc, 0, line.color, line.lineWidth, line.lineType);
						dc.SelectObject(oldPen);
					}
					for (auto& ell : Ellipses)
					{
						if (!ell.selected) continue;
						int penStyle = ell.lineType ? PS_DASH : PS_SOLID;
						LOGBRUSH logBrush = { BS_SOLID, ell.color, 0 };
						CPen pen(penStyle | PS_GEOMETRIC | PS_ENDCAP_ROUND, ell.lineWidth, &logBrush);
						CPen* oldPen = dc.SelectObject(&pen);
						CBrush* pNullBrush = CBrush::FromHandle((HBRUSH)GetStockObject(NULL_BRUSH));
						CBrush* oldBrush = dc.SelectObject(pNullBrush);
						DrawEllipseA(dc, ell.rect, ell.color, ell.lineWidth, ell.lineType, 4);
						dc.SelectObject(oldBrush);
						dc.SelectObject(oldPen);
					}

					for (auto& poly : Polygons)
					{
						if (!poly.visible || !poly.selected) continue;
						int penStyle = poly.lineType ? PS_DASH : PS_SOLID;
						LOGBRUSH logBrush = { BS_SOLID, poly.color, 0 };
						CPen pen(penStyle | PS_GEOMETRIC | PS_ENDCAP_ROUND, poly.lineWidth, &logBrush);
						CPen* oldPen = dc.SelectObject(&pen);
						DrawPolygonFM(dc, poly.points, poly.isfilled, poly.fillColor, false);
						dc.SelectObject(oldPen);
					}
					hasDrawSelected = true;
				}
			}
			else if (Algorithm == 12)
			{

			}
			else if (Algorithm == 13)
			{

			}
		}
		lastPoint = point;
	}
}
void CPaint3Dlg::EraseLastPreview(CDC& dc)
{
	dc.SetROP2(R2_NOTXORPEN);
	int penStyle = LineType ? PS_DASH : PS_SOLID;
	LOGBRUSH logBrush = { BS_SOLID, LineColor, 0 };
	CPen pen(penStyle | PS_GEOMETRIC | PS_ENDCAP_ROUND, LineWidth, &logBrush);
	CPen* oldPen = dc.SelectObject(&pen);
	switch (Mode)
	{
	case 1: // Line
		dc.MoveTo(startPoint);
		dc.LineTo(lastPoint);
		break;
	case 2: // Ellipse
		if (hasLastDrawRect)
			dc.Ellipse(lastDrawRect);
		break;
	case 3: // Arc
		if (hasLastDrawArc)
			DrawArcPreview(lastArcAngle, lastArcDirection, lastArcStart, lastArcEnd, dc);
		break;
	case 6: // Clip Line
		break;
	}

	dc.SelectObject(oldPen);
}

void CPaint3Dlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	ReleaseCapture();  // 释放捕获（在绘图区外松开鼠标也能侦测到）
	if (isDrawing)
	{
		isDrawing = false;
		CClientDC dc(this);
		EraseLastPreview(dc);
		dc.SetROP2(R2_COPYPEN);

		endPoint = point;

		int penStyle = LineType ? PS_DASH : PS_SOLID;
		LOGBRUSH logBrush = { BS_SOLID, LineColor, 0 };
		CPen pen(penStyle | PS_GEOMETRIC | PS_ENDCAP_ROUND, LineWidth, &logBrush);
		CPen* oldPen = dc.SelectObject(&pen);

		if (Mode == 0) // Pen
		{
			Pens.back().push_back(point);
			dc.MoveTo(lastPoint);
			dc.LineTo(point);
		}
		else if (Mode == 1) // Line
		{
			bool tmp;
			if (DefinedClipRect)
			{
				//AfxMessageBox(_T("Rect Clip"));
				tmp = ClipLineCohenSutherland(startPoint, endPoint, clipRect);
			}
			else if (DefinedClipPoly)
			{
				//AfxMessageBox(_T("Poly Clip"));
				tmp = CyrusBeckClipLine(startPoint, endPoint, CPolygons.back());
			}
			else tmp = true;
			if (!tmp) // 完全不可见
			{
				AfxMessageBox(_T("线段完全不可见，未绘制。"));
			}
			else
			{
				Lines.push_back({ ++idLine, startPoint, endPoint, LineWidth, LineType, LineColor, false, Algorithm, true });
				DrawLineA(startPoint, endPoint, dc, Algorithm, LineColor, LineWidth, LineType);
			}
		}
		else if (Mode == 2)
		{
			CRect rect(startPoint, endPoint);
			rect.NormalizeRect();
			hasLastDrawRect = false;

			if (GetKeyState(VK_SHIFT) & 0x8000)
			{
				int len = min(rect.Width(), rect.Height());
				if (endPoint.x < startPoint.x) rect.left = startPoint.x - len;
				else rect.right = startPoint.x + len;
				if (endPoint.y < startPoint.y) rect.top = startPoint.y - len;
				else rect.bottom = startPoint.y + len;
			}

			Ellipses.push_back({ ++idEllipse, rect, LineWidth,  LineType, LineColor, IsFill, ShapeColor, false, Algorithm, true });

			if (IsFill)
			{
				CBrush brush(ShapeColor);
				CBrush* oldBrush = dc.SelectObject(&brush);
				dc.Ellipse(rect);
				dc.SelectObject(oldBrush);
			}
			else
			{
				CBrush* pNullBrush = CBrush::FromHandle((HBRUSH)GetStockObject(NULL_BRUSH));
				CBrush* oldBrush = dc.SelectObject(pNullBrush);
				DrawEllipseA(dc, rect, LineColor, LineWidth, LineType, Algorithm);
				dc.SelectObject(oldBrush);
			}
		}
		else if (Mode == 3) // Arc
		{
			hasLastDrawArc = false;
			bool direction = true; // 默认逆时针
			if (GetKeyState(VK_SHIFT) & 0x8000)
				direction = false; // 顺时针
			if (Algorithm == 7) // Bresenham Arc
			{
				arcAngle = arcAngleDeg * M_PI / 180.0;
				Arcs.push_back({ ++idArc, startPoint, endPoint, arcAngleDeg, direction, LineWidth, LineType, LineColor, false, Algorithm, true });
				DrawArcFM(arcAngle, direction, startPoint, endPoint, dc, LineColor, LineWidth, LineType);
			}
		}
		else if (Mode == 4) // Polygon
		{
			if (Algorithm == 8)
			{
				currentPolygon.push_back(point);
			}
		}
		else if (Mode == 5) // Fill
		{
			// ScanlineFill(dc, point, ShapeColor, LineColor);
			ScanlineFillFM(dc, point, ShapeColor, LineColor);
		}
		else if (Mode == 6) // Clip Line
		{
			if (Algorithm == 9)
			{
				clipRect = CRect(startPoint, point);
				clipRect.NormalizeRect();
				CPoint topRight(clipRect.right, clipRect.top);
				CPoint bottomLeft(clipRect.left, clipRect.bottom);
				clipPolygon.clear();
				clipPolygon.push_back(topRight);
				clipPolygon.push_back(clipRect.TopLeft());
				clipPolygon.push_back(bottomLeft);
				clipPolygon.push_back(clipRect.BottomRight());
				CPolygons.push_back(clipPolygon);
				DrawPolygonFM(dc, clipPolygon, IsFill, ShapeColor, true);
				AfxMessageBox(_T("矩形裁剪窗口已设置。"));
				DefinedClipRect = true;
			}
			if (Algorithm == 10)
			{
				if (!isDefiningClipPoly)
				{
					clipPolygon.clear();
					isDefiningClipPoly = true;
				}
				clipPolygon.push_back(point);
			}
		}
		else if (Mode == 7)
		{
			if (Algorithm == 11)
			{
				if (isDragging)
				{
					isDragging = false;

					// 最终平移量
					int dx = point.x - dragStart.x;
					int dy = point.y - dragStart.y;

					for (auto& line : Lines)
					{
						line.selected = false;
					}
					for (auto& ell : Ellipses)
					{
						ell.selected = false;
					}
					for (auto& poly : Polygons)
					{
						poly.selected = false;
					}
					hasDrawSelected = false;
					Invalidate(1);
					UpdateWindow();
					for (auto& line : Lines)
					{
						if (line.selected) continue;
						int penStyle = line.lineType ? PS_DASH : PS_SOLID;
						LOGBRUSH logBrush = { BS_SOLID, line.color, 0 };
						CPen pen(penStyle | PS_GEOMETRIC | PS_ENDCAP_ROUND, line.lineWidth, &logBrush);
						CPen* oldPen = dc.SelectObject(&pen);
						DrawLineA(line.start, line.end, dc, line.algorithm, line.color, line.lineWidth, line.lineType);
						dc.SelectObject(oldPen);
					}
					for (auto& ell : Ellipses)
					{
						if (ell.selected) continue;
						int penStyle = ell.lineType ? PS_DASH : PS_SOLID;
						LOGBRUSH logBrush = { BS_SOLID, ell.color, 0 };
						CPen pen(penStyle | PS_GEOMETRIC | PS_ENDCAP_ROUND, ell.lineWidth, &logBrush);
						CPen* oldPen = dc.SelectObject(&pen);
						CBrush* pNullBrush = CBrush::FromHandle((HBRUSH)GetStockObject(NULL_BRUSH));
						CBrush* oldBrush = dc.SelectObject(pNullBrush);
						DrawEllipseA(dc, ell.rect, ell.color, ell.lineWidth, ell.lineType, ell.algorithm);
						dc.SelectObject(oldBrush);
						dc.SelectObject(oldPen);
					}

					//for (auto& arc : Arcs)
					//{
					//	if (!arc.visible || arc.selected) continue;
					//	int penStyle = arc.lineType ? PS_DASH : PS_SOLID;
					//	LOGBRUSH logBrush = { BS_SOLID, arc.color, 0 };
					//	CPen pen(penStyle | PS_GEOMETRIC | PS_ENDCAP_ROUND, arc.lineWidth, &logBrush);
					//	CPen* oldPen = dc.SelectObject(&pen);
					//	DrawArcFM(arc.angle, arc.direction, arc.start, arc.end, dc, arc.color, arc.lineWidth, arc.lineType);
					//	dc.SelectObject(oldPen);
					//}

					for (auto& poly : Polygons)
					{
						if (!poly.visible || poly.selected) continue;
						int penStyle = poly.lineType ? PS_DASH : PS_SOLID;
						LOGBRUSH logBrush = { BS_SOLID, poly.color, 0 };
						CPen pen(penStyle | PS_GEOMETRIC | PS_ENDCAP_ROUND, poly.lineWidth, &logBrush);
						CPen* oldPen = dc.SelectObject(&pen);
						DrawPolygonFM(dc, poly.points, poly.isfilled, poly.fillColor, false);
						dc.SelectObject(oldPen);
					}
				}
			}
		}
		dc.SelectObject(oldPen);
	}

	CDialogEx::OnLButtonUp(nFlags, point);
}
void CPaint3Dlg::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (Mode == 4 && nChar == VK_CONTROL) // Ctrl 键
	{
		CClientDC dc(this);
		dc.SetROP2(R2_COPYPEN);
		int penStyle = LineType ? PS_DASH : PS_SOLID;
		LOGBRUSH logBrush = { BS_SOLID, LineColor, 0 };
		CPen pen(penStyle | PS_GEOMETRIC | PS_ENDCAP_ROUND, LineWidth, &logBrush);
		CPen* oldPen = dc.SelectObject(&pen);
		std::vector<CPoint> clippedPoly;
		if (DefinedClipPoly || DefinedClipRect)
		{
			//AfxMessageBox(CString(std::to_wstring(CPolygons.size()).c_str()));
			clippedPoly = SutherlandHodgmanClipPolygon(currentPolygon, CPolygons.back());
		}
		else clippedPoly = currentPolygon;
		if (clippedPoly.size() >= 3)
		{
			DrawPolygonFM(dc, clippedPoly, IsFill, ShapeColor, false);
			Polygons.push_back({ ++idPolygon, clippedPoly, LineWidth, LineType, LineColor, IsFill, ShapeColor, false, Algorithm, true });
			currentPolygon.clear();
			dc.SelectObject(oldPen);
		}
		else
		{
			AfxMessageBox(_T("顶点数不足3，裁剪后无法构成多边形！请重新输入。"));
			currentPolygon.clear();
		}

	}
	if (Mode == 6 && Algorithm == 10 && isDefiningClipPoly)
	{
		if (GetKeyState(VK_CONTROL) & 0x8000)
		{
			if (clipPolygon.size() >= 3)
			{
				CClientDC dc(this);
				dc.SetROP2(R2_COPYPEN);
				int penStyle = LineType ? PS_DASH : PS_SOLID;
				LOGBRUSH logBrush = { BS_SOLID, LineColor, 0 };
				CPen pen(penStyle | PS_GEOMETRIC | PS_ENDCAP_ROUND, LineWidth, &logBrush);
				CPen* oldPen = dc.SelectObject(&pen);
				DrawPolygonFM(dc, clipPolygon, IsFill, ShapeColor, true);
				isDefiningClipPoly = false;
				DefinedClipPoly = true;
				CPolygons.push_back(clipPolygon);
				clipPolygon.clear();
				AfxMessageBox(_T("多边形裁剪窗口定义完成。"));
			}
			else
			{
				AfxMessageBox(_T("顶点数不足3，无法构成多边形！"));
			}
		}
	}
	CDialogEx::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CPaint3Dlg::OnBnClickedButton1()
{
	CColorDialog colorDlg(LineColor, CC_FULLOPEN);
	// LineColor = color;
	if (IDOK == colorDlg.DoModal()) {
		LineColor = colorDlg.GetColor();
	}
}

void CPaint3Dlg::OnBnClickedButton2()
{
	CColorDialog colorDlg(ShapeColor, CC_FULLOPEN);
	// LineColor = color;
	if (IDOK == colorDlg.DoModal()) {
		ShapeColor = colorDlg.GetColor();
	}
}


void CPaint3Dlg::OnBnClickedRadio1()
{
	LineType = 0;
}

void CPaint3Dlg::OnBnClickedRadio2()
{
	LineType = 1;
}

void CPaint3Dlg::OnCbnSelchangeCombo1()
{
	int sel = m_fill.GetCurSel();
	IsFill = (sel == 0) ? true : false;
}
void CPaint3Dlg::OnCbnSelchangeCombo2()
{
	int sel = m_mode.GetCurSel();
	Mode = sel;
	AlgMenu::RefreshAlgorithmListForMode(*this);
}
void CPaint3Dlg::OnCbnSelchangeCombo3()
{
	int sel = m_algorithm.GetCurSel();
	if (sel >= 0) {
		Algorithm = static_cast<int>(m_algorithm.GetItemData(sel));
	}
}
void CPaint3Dlg::OnEnChangeEdit1()
{
	CString text;
	GetDlgItemText(IDC_EDIT1, text);

	// 防止空字符串或非法输入导致崩溃
	if (!text.IsEmpty())
	{
		LineWidth = atoi(CT2A(text.GetBuffer())); // 字符串转整数
		LineWidth = max(1, min(LineWidth, 20)); // 限制范围在0到20
	}
}

void CPaint3Dlg::OnEnChangeEdit2()
{
	CString text;
	GetDlgItemText(IDC_EDIT2, text);
	if (!text.IsEmpty())
	{
		arcAngleDeg = atoi(CT2A(text.GetBuffer()));
		arcAngleDeg = max(1, min(arcAngleDeg, 360)); // 限制范围在1到360
	}
}