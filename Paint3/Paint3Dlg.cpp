﻿// Paint3Dlg.cpp: 实现文件
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

// byd类型枚举
namespace {
	enum : int {
		PEN = 0,
		LINE = 1,
		CIRCLE = 2,
		ARC = 3,
		POLYGON = 4,
		FILL = 5,
		CLIP = 6,
		SELECT = 7,
		CONNECT = 8,
		BEZIER = 9,
		BSPLINE = 10,
	};
	enum : int {
		// 直线算法
		ALG_LINE_DEFAULT = 0,
		ALG_LINE_DDA = 1,
		ALG_LINE_MIDPOINT = 2,
		ALG_LINE_BRESENHAM = 3,

		// 椭圆/圆算法
		ALG_CIRCLE_DEFAULT = 4,
		ALG_CIRCLE_MIDPOINT = 5,
		ALG_CIRCLE_BRESENHAM = 6,

		// 圆弧
		ALG_ARC_BRESENHAM = 7,

		// 多边形（绘制/扫描转换入口）
		ALG_POLY_DEFAULT = 8,

		// 裁剪窗口
		ALG_CLIP_RECT = 9,
		ALG_CLIP_POLY = 10,

		// 选择/变换
		ALG_TRANS_TRANSLATION = 11,
		ALG_TRANS_SCALING = 12,
		ALG_TRANS_ROTATION = 13,

		// 填充（给 Fill 专用一个不冲突的编号）
		ALG_FILL_SCANLINE = 20,
		// 贝塞尔
		ALG_BEZIER_CASTELJAU = 21,
		ALG_BSPLINE_DEBOOR = 22,
	};
}
// --------- 角度/向量小工具 ----------
static inline double wrap_ccw(double a) { // wrap to [0, 2π)
	const double TWO = 2.0 * M_PI;
	a = fmod(a, TWO);
	if (a < 0) a += TWO;
	return a;
}
// CCW 方向上 a→b 的有向角（返回 [0,2π)）
static inline double delta_ccw(double a, double b) {
	a = wrap_ccw(a); b = wrap_ccw(b);
	double d = b - a;
	if (d < 0) d += 2.0 * M_PI;
	return d;
}
static inline double length(double x, double y) { return hypot(x, y); }

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
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_RBUTTONDOWN()
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
	m_mode.AddString(_T("Connect"));
	m_mode.AddString(_T("Bezier"));
	m_mode.AddString(_T("B-Spline"));
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
	if (IsIconic()) {
		CPaintDC dc(this);
		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect; GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;
		dc.DrawIcon(x, y, m_hIcon);
		return;
	}

	CPaintDC dcPaint(this);

	// --- 双缓冲 ---
	CRect rc; GetClientRect(&rc);
	CDC memDC; memDC.CreateCompatibleDC(&dcPaint);
	CBitmap bmp; bmp.CreateCompatibleBitmap(&dcPaint, rc.Width(), rc.Height());
	CBitmap* pOldBmp = memDC.SelectObject(&bmp);

	// 背景
	CBrush bkBrush(BackgroundColor);
	memDC.FillRect(&rc, &bkBrush);

	// 核心：一次性把所有对象画到 memDC
	RedrawAll(memDC, /*highlightSelection=*/1);

	// 推到屏幕
	dcPaint.BitBlt(0, 0, rc.Width(), rc.Height(), &memDC, 0, 0, SRCCOPY);
	memDC.SelectObject(pOldBmp);
}

void CPaint3Dlg::RedrawAll(CDC& dc, bool highlightSelection)
{
	dc.SetROP2(R2_COPYPEN);

	// 1) 画笔轨迹 Pens 
	for (size_t i = 0; i < Pens.size(); ++i) {
		const auto& stroke = Pens[i];
		if (stroke.size() < 2) continue;
		int penStyle = PS_SOLID; // 自由画笔一般用实线
		LOGBRUSH lb{ BS_SOLID, PenColors[i], 0 };
		CPen pen(penStyle | PS_GEOMETRIC | PS_ENDCAP_ROUND, LineWidth, &lb);
		CPen* oldPen = dc.SelectObject(&pen);
		for (size_t k = 1; k < stroke.size(); ++k) {
			dc.MoveTo(stroke[k - 1]);
			dc.LineTo(stroke[k]);
		}
		dc.SelectObject(oldPen);
	}

	// 2) 直线
	for (const auto& line : Lines) {
		if (!line.visible) continue;
		COLORREF c = line.color;
		int lw = line.lineWidth;
		int penStyle = line.lineType ? PS_DASH : PS_SOLID;

		// 若需要高亮选中，可改笔样式或加一层外描边
		if (highlightSelection && line.selected) {
			// 简单做法：换色或加粗
			c = RGB(255, 128, 0);
		}
		LOGBRUSH lb{ BS_SOLID, c, 0 };
		CPen pen(penStyle | PS_GEOMETRIC | PS_ENDCAP_ROUND, lw, &lb);
		CPen* oldPen = dc.SelectObject(&pen);
		DrawLineA(line.start, line.end, dc, line.algorithm, c, lw, line.lineType);
		dc.SelectObject(oldPen);
	}

	// 3) 椭圆
	for (const auto& ell : Ellipses) {
		if (!ell.visible) continue;
		int penStyle = ell.lineType ? PS_DASH : PS_SOLID;
		COLORREF edge = ell.color;
		if (highlightSelection && ell.selected) edge = RGB(255, 128, 0);
		LOGBRUSH lb{ BS_SOLID, edge, 0 };
		CPen pen(penStyle | PS_GEOMETRIC | PS_ENDCAP_ROUND, ell.lineWidth, &lb);
		CPen* oldPen = dc.SelectObject(&pen);

		CBrush* pNull = CBrush::FromHandle((HBRUSH)GetStockObject(NULL_BRUSH));
		CBrush* oldBrush = dc.SelectObject(ell.isfilled ? new CBrush(ell.fillColor) : pNull);
		DrawEllipseA(dc, ell.rect, edge, ell.lineWidth, ell.lineType, ell.algorithm);
		dc.SelectObject(oldBrush);
		dc.SelectObject(oldPen);
	}

	// 4) 多边形
	for (const auto& poly : Polygons) {
		if (!poly.visible) continue;
		COLORREF c = poly.color;
		if (highlightSelection && poly.selected) c = RGB(255, 128, 0);
		LOGBRUSH lb{ BS_SOLID, c, 0 };
		int penStyle = poly.lineType ? PS_DASH : PS_SOLID;
		CPen pen(penStyle | PS_GEOMETRIC | PS_ENDCAP_ROUND, poly.lineWidth, &lb);
		CPen* oldPen = dc.SelectObject(&pen);
		DrawPolygonFM(dc, poly.points, poly.isfilled, poly.fillColor, /*Clipper*/false);
		dc.SelectObject(oldPen);
	}

	// 5) 圆弧
	for (const auto& arc : Arcs) {
	if (!arc.visible) continue;
	COLORREF c = arc.color;
	if (highlightSelection && arc.selected) {
		c = RGB(255, 128, 0); // 高亮颜色
	}
	LOGBRUSH lb{ BS_SOLID, c, 0 };
	int penStyle = arc.lineType ? PS_DASH : PS_SOLID;
	CPen pen(penStyle | PS_GEOMETRIC | PS_ENDCAP_ROUND, arc.lineWidth, &lb);
	CPen* oldPen = dc.SelectObject(&pen);

	// ArcObject.angle 存储的是角度值 (arcAngleDeg)
	// DrawArcFM 需要弧度值
	float angle_rad = arc.angle * (float)M_PI / 180.0f;
	DrawArcFM(angle_rad, arc.direction, arc.start, arc.end, dc, c, arc.lineWidth, arc.lineType);

	dc.SelectObject(oldPen);
	}

	// 6) 绘制裁剪窗口 (如果已定义)
	if ((DefinedClipRect || DefinedClipPoly) && !CPolygons.empty())
	{
		// 最后一个 CPolygon 总是当前的裁剪窗口
		const auto& clipPoly = CPolygons.back();

		// 裁剪窗口使用固定的细虚线样式绘制，以便区分
		COLORREF clipColor = RGB(100, 100, 100); // 灰色
		LOGBRUSH lb{ BS_SOLID, clipColor, 0 };
		// 1像素虚线
		CPen pen(PS_DASH | PS_GEOMETRIC | PS_ENDCAP_ROUND, 1, &lb);
		CPen* oldPen = dc.SelectObject(&pen);

		// 裁剪窗口本身不填充，并标记为 Clipper
		// 确保 DrawPolygonFM 函数能正确处理 isfilled=false 和 Clipper=true 的情况
		DrawPolygonFM(dc, clipPoly, false, 0, /*Clipper*/true);

		dc.SelectObject(oldPen);
	}
	// —— 在“选择/变换模式”下显示参考点标记
	if (Mode == SELECT && HasAnySelection()) {
		// 如果当前不是自定义参考点，就用最新的选中集平均重心来显示
		CPoint ref = m_bUseCustomRefPoint ? m_refPoint : GetAverageCentroidOfSelection();
		DrawRefPoint(dc, ref, m_bUseCustomRefPoint);
	}
	// X) Bezier
	for (auto& bz : Beziers) {
		if (!bz.visible) continue;
		COLORREF c = bz.color;
		if (highlightSelection && bz.selected) c = RGB(255, 128, 0);
		LOGBRUSH lb{ BS_SOLID, c, 0 };
		CPen pen((bz.lineType ? PS_DASH : PS_SOLID) | PS_GEOMETRIC | PS_ENDCAP_ROUND,
			max(1, bz.lineWidth), &lb);
		CPen* old = dc.SelectObject(&pen);
		DrawBezierFM(dc, bz);
		dc.SelectObject(old);

		// 若被选中，画控制折线与控制点（帮助编辑）
		if (highlightSelection && bz.selected) {
			// 控制折线
			LOGBRUSH lb2{ BS_SOLID, RGB(120,120,120), 0 };
			CPen pen2(PS_DOT | PS_GEOMETRIC, 1, &lb2);
			CPen* old2 = dc.SelectObject(&pen2);
			for (size_t i = 1; i < bz.ctrl.size(); ++i) { dc.MoveTo(bz.ctrl[i - 1]); dc.LineTo(bz.ctrl[i]); }
			dc.SelectObject(old2);
			// 控制点
			CBrush b(RGB(50, 50, 50)); CBrush* ob = dc.SelectObject(&b);
			for (auto& q : bz.ctrl) dc.Ellipse(q.x - 3, q.y - 3, q.x + 3, q.y + 3);
			dc.SelectObject(ob);
		}
	}
	for (auto& sp : BSplines) {
		if (!sp.visible) continue;

		// 1) 先画曲线（折线近似，已在 DrawBSplineFM 内部完成缓存与采样）
		DrawBSplineFM(dc, sp);

		// 2) 若被选中：画控制折线 + 控制点把手（高亮）
		if (sp.selected) {
			// 控制折线：灰色/虚线
			{
				LOGBRUSH lb{ BS_SOLID, RGB(160,160,160), 0 };
				CPen pen(PS_DASH | PS_GEOMETRIC | PS_ENDCAP_ROUND, 1, &lb);
				CPen* old = dc.SelectObject(&pen);
				for (size_t i = 1; i < sp.ctrl.size(); ++i) {
					dc.MoveTo(sp.ctrl[i - 1]);
					dc.LineTo(sp.ctrl[i]);
				}
				dc.SelectObject(old);
			}
			// 控制点把手：小圆点
			const int r = 3;
			HBRUSH hbr = ::CreateSolidBrush(RGB(255, 128, 0)); // 橙色点
			HBRUSH oldBr = (HBRUSH)dc.SelectObject(CBrush::FromHandle(hbr));
			CPen  penPt(PS_SOLID, 1, RGB(80, 40, 0));
			CPen* oldPen = dc.SelectObject(&penPt);
			for (auto& p : sp.ctrl) {
				dc.Ellipse(p.x - r, p.y - r, p.x + r, p.y + r);
			}
			dc.SelectObject(oldPen);
			dc.SelectObject(oldBr);
			::DeleteObject(hbr);
		}
	}

	// 正在录入（BEZIER 模式）临时预览
	if (Mode == 9 && !currentBezierCtrl.empty()) {
		// 控制折线
		LOGBRUSH lb{ BS_SOLID, RGB(80,80,80), 0 };
		CPen pen(PS_DOT | PS_GEOMETRIC, 1, &lb);
		CPen* old = dc.SelectObject(&pen);
		for (size_t i = 1; i < currentBezierCtrl.size(); ++i) {
			dc.MoveTo(currentBezierCtrl[i - 1]);
			dc.LineTo(currentBezierCtrl[i]);
		}
		dc.SelectObject(old);
		// 控制点
		CBrush b(RGB(30, 30, 30)); CBrush* ob = dc.SelectObject(&b);
		for (auto& q : currentBezierCtrl) dc.Ellipse(q.x - 3, q.y - 3, q.x + 3, q.y + 3);
		dc.SelectObject(ob);
		// 预览曲线（≥3个点）
		if (currentBezierCtrl.size() >= 3) {
			BezierObject tmp{};
			tmp.ctrl = currentBezierCtrl;
			tmp.lineWidth = LineWidth;
			tmp.lineType = LineType;
			tmp.color = LineColor;
			tmp.cacheValid = false;
			DrawBezierFM(dc, tmp);
		}
	}
	// ===== B-Spline：正在录入时的预览（Mode==10）=====
	if (Mode == 10 && !currentBSplineCtrl.empty()) {
		// 1) 控制折线（灰色）
		{
			LOGBRUSH lb{ BS_SOLID, RGB(180,180,180), 0 };
			CPen pen(PS_SOLID | PS_GEOMETRIC | PS_ENDCAP_ROUND, 1, &lb);
			CPen* old = dc.SelectObject(&pen);
			for (size_t i = 1; i < currentBSplineCtrl.size(); ++i) {
				dc.MoveTo(currentBSplineCtrl[i - 1]);
				dc.LineTo(currentBSplineCtrl[i]);
			}
			dc.SelectObject(old);
		}
		// 2) 控制点把手
		{
			const int r = 3;
			HBRUSH hbr = ::CreateSolidBrush(RGB(120, 120, 120));
			HBRUSH oldBr = (HBRUSH)dc.SelectObject(CBrush::FromHandle(hbr));
			CPen  penPt(PS_SOLID, 1, RGB(60, 60, 60));
			CPen* oldPen = dc.SelectObject(&penPt);
			for (auto& p : currentBSplineCtrl) {
				dc.Ellipse(p.x - r, p.y - r, p.x + r, p.y + r);
			}
			dc.SelectObject(oldPen);
			dc.SelectObject(oldBr);
			::DeleteObject(hbr);
		}

		// 3) 曲线预览（当点数≥p+1 时才生成临时曲线）
		const int p = max(1, defaultBSplineDegree);
		if ((int)currentBSplineCtrl.size() >= p + 1) {
			BSplineObject tmp{};
			tmp.ctrl = currentBSplineCtrl;
			tmp.degree = p;
			tmp.knots = {};            // 置空：EnsureBSplineCache 内会自动生成 clamped uniform
			tmp.lineWidth = LineWidth;
			tmp.lineType = LineType;
			tmp.color = LineColor;
			tmp.visible = true;
			tmp.selected = false;
			tmp.algorithm = ALG_BSPLINE_DEBOOR;
			tmp.cacheValid = false;

			DrawBSplineFM(dc, tmp);
		}
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
bool CPaint3Dlg::IsPointNearArc(const CPoint& p, const ArcObject& arc)
{
	// 1) 基本健壮性检查
	double x1 = arc.start.x, y1 = arc.start.y;
	double x2 = arc.end.x, y2 = arc.end.y;
	double vx = x2 - x1, vy = y2 - y1;
	double chord = length(vx, vy);      // 弦长
	if (chord < 1e-6) return false;

	double phi = abs(arc.angle) * M_PI / 180.0; // 圆心角(弧度)
	// 保护：避免 sin(phi/2) ≈ 0 导致半径爆炸
	const double EPS = 1e-6;
	if (phi < EPS || phi >= 2.0 * M_PI - EPS) return false;

	// 2) 由弦长与圆心角求圆半径 r 与弓高 h（从弦中点到圆心的距离）
	double r = chord / (2.0 * sin(phi * 0.5));
	double h = sqrt(max(0.0, (double)r * r - (chord * 0.5) * (chord * 0.5)));

	// 3) 计算弦中点 M、以及单位法线（有左右两解）
	double mx = (x1 + x2) * 0.5;
	double my = (y1 + y2) * 0.5;
	double inv = (chord > EPS) ? (1.0 / chord) : 0.0;
	// 以 S->E 的“左法线”为 nL（CCW 旋转 90°）
	double nLx = -vy * inv, nLy = vx * inv;
	// 右法线 nR = -nL
	double nRx = -nLx, nRy = -nLy;

	// 4) 由于方向不同，圆心在弦两侧之一。我们尝试两侧，选择与 arc.direction + phi 最匹配的那一个。
	auto pick_center = [&](double nx, double ny)->tuple<double, double, bool> {
		double cx = mx + nx * h, cy = my + ny * h;  // 候选圆心
		double angS = atan2(y1 - cy, x1 - cx);
		double angE = atan2(y2 - cy, x2 - cx);
		double ccwSE = delta_ccw(angS, angE);       // 以 CCW 计算 S→E 的有向角

		// 目标角度：若 direction==true 表示逆时针，则期望 ccwSE ≈ phi；
		// 若你的 ArcObject 里 direction==true 表示“顺时针”，只需把 want = (2π - phi)。
		double want = arc.direction ? phi : (2.0 * M_PI - phi);

		double err = abs(ccwSE - want);
		// 允许 2π-wrap 的等价
		err = min(err, abs(ccwSE + 2.0 * M_PI - want));
		err = min(err, abs(ccwSE - 2.0 * M_PI - want));
		bool ok = (err < 1e-3); // 误差阈值可适当放宽
		return { cx, cy, ok };
		};

	double cx = 0, cy = 0;
	bool okL = false, okR = false;
	{
		double cxl, cyl, cxr, cyr;
		bool   ol, orr;

		tie(cxl, cyl, ol) = pick_center(nLx, nLy);
		tie(cxr, cyr, orr) = pick_center(nRx, nRy);
		// 优先选择匹配方向的那侧；若都不“严格匹配”，选左侧做近似也行
		if (ol) { cx = cxl; cy = cyl; okL = true; }
		else if (orr) { cx = cxr; cy = cyr; okR = true; }
		else { cx = cxl; cy = cyl; } // 容错：退而求其次
	}

	// 5) 先做“半径贴近”判定（允许线宽 + 微容差）
	double px = p.x - cx, py = p.y - cy;
	double rp = length(px, py);
	double tol = (arc.lineWidth * 0.5 + 3.0); // 与其他函数一致的容差
	if (abs(rp - r) > tol) return false;

	// 6) 再做“角度落在弧段范围内”的判定
	double angS = atan2(y1 - cy, x1 - cx);
	double angE = atan2(y2 - cy, x2 - cx);
	double angP = atan2(p.y - cy, p.x - cx);

	if (arc.direction) {
		// 逆时针：要求 0 <= CCW(S->P) <= phi
		double ccwSP = delta_ccw(angS, angP);
		return (ccwSP >= -1e-6 && ccwSP <= phi + 1e-6);
	}
	else {
		// 顺时针：等价于要求 0 <= CW(S->P) <= phi
		// 利用 CCW：CW(S->P) = 2π - CCW(S->P)
		double ccwSP = delta_ccw(angS, angP);
		double cwSP = (2.0 * M_PI - ccwSP);
		return (cwSP >= -1e-6 && cwSP <= phi + 1e-6);
	}
}


void CPaint3Dlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	SetCapture();  // 捕获鼠标
	isDrawing = true;
	startPoint = lastPoint = point;
	if (Mode == 0) // Pen Mode
	{
		vector<CPoint> newStroke;
		newStroke.push_back(point);
		Pens.push_back(newStroke);
		PenColors.push_back(LineColor);
	}
	if (Mode == SELECT) // Select
	{
		// Shift 点击，否则清空重新选
		if (!(nFlags & MK_SHIFT)) {
			DeselectAll(); // (!!! 调用新函数清空选择 !!!)
		}
		for (auto& line : Lines)
		{
			if (line.visible && IsPointNearLine(point, line))
			{
				++selCnt;
				line.selected = !line.selected;
			}
		}
		for (auto& ell : Ellipses)
		{
			if (ell.visible && IsPointNearEllipse(point, ell))
			{
				++selCnt;
				ell.selected = !ell.selected;
			}
		}
		for (auto& poly : Polygons)
		{
			if (poly.visible && IsPointNearPolygon(point, poly))
			{
				++selCnt;
				poly.selected = !poly.selected;
			}
		}
		for (auto& arc : Arcs)
		{
			if (arc.visible && IsPointNearArc(point, arc)) 
			{
				arc.selected = !arc.selected;
				++selCnt;
			}
		}
		for (auto& bz : Beziers) {
			if (bz.visible && IsPointNearBezier(point, bz)) {
				bz.selected = !bz.selected;
				++selCnt;
			}
		}
		for (auto& sp : BSplines) {
			if (sp.visible && IsPointNearBSpline(point, sp)) {
				sp.selected = !sp.selected;
				++selCnt;
			}
		}


		if (!selCnt)
		{
			AfxMessageBox(_T("未选中任何图形。"));
		}
		else
		{
			CClientDC dc(this);
			dc.SetROP2(R2_COPYPEN);
			Invalidate(FALSE); // 不要擦背景，减少闪烁
			UpdateWindow();

			//RedrawAll(dc, /*highlightSelection=*/true);
		}

		// —— 选中后设定默认参考点为“选中集平均重心”，并默认不是自定义点
		m_refPoint = GetAverageCentroidOfSelection();
		m_bUseCustomRefPoint = false;
		Invalidate(FALSE);


		// 实验三的变换
		if (Algorithm == ALG_TRANS_TRANSLATION)
		{
			isDragging = true;
			dragStart = point;
		}
		else if (Algorithm == ALG_TRANS_SCALING)
		{
			isScaling = true;
			scaleStart = point;
		}
		else if (Algorithm == ALG_TRANS_ROTATION)
		{
			isRotating = true;
			rotateStart = point;
		}
	}
	if (Mode == BEZIER)
	{
		// 先看看是否命中已有控制点（便于拖动编辑）
		const int R = 8; // 命中半径
		bezierDragIdx = -1;
		for (int i = (int)currentBezierCtrl.size() - 1; i >= 0; --i) {
			if (std::hypot(currentBezierCtrl[i].x - point.x, currentBezierCtrl[i].y - point.y) <= R) {
				bezierDragIdx = i;
				break;
			}
		}
		// 没命中则新增控制点
		if (bezierDragIdx < 0) {
			currentBezierCtrl.push_back(point);
		}
		isDrawing = true;
		Invalidate(FALSE);
		CDialogEx::OnLButtonDown(nFlags, point);
		return;
	}
	if (Mode == 10) { // BSPLINE
		const int R = 8;
		bsplineDragIdx = -1;
		for (int i = (int)currentBSplineCtrl.size() - 1; i >= 0; --i) {
			if (std::hypot(currentBSplineCtrl[i].x - point.x, currentBSplineCtrl[i].y - point.y) <= R) {
				bsplineDragIdx = i; break;
			}
		}
		if (bsplineDragIdx < 0) currentBSplineCtrl.push_back(point);
		isDrawing = true;
		Invalidate(FALSE);
		CDialogEx::OnLButtonDown(nFlags, point);
		return;
	}

	CDialogEx::OnLButtonDown(nFlags, point);
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
		else if (Mode == SELECT)
		{
			if (isDragging) // 1. 平移
			{
				int tx = point.x - lastPoint.x;
				int ty = point.y - lastPoint.y;
				ApplyTranslationToSelection(tx, ty); // (!!! 应用增量平移 !!!)

				// 平移时，参考点也必须一起移动！
				if (m_bUseCustomRefPoint) {
					m_refPoint.x += tx;
					m_refPoint.y += ty;
				}
				else {
					m_refPoint = GetAverageCentroidOfSelection();
				}

				Invalidate(FALSE); // 请求重绘
			}
			else if (isScaling) // 2. 缩放
			{
				// (!!! 计算缩放因子：基于到参考点距离的变化 !!!)
				double dist_last = max(1.0, hypot(lastPoint.x - m_refPoint.x, lastPoint.y - m_refPoint.y));
				double dist_now = hypot(point.x - m_refPoint.x, point.y - m_refPoint.y);
				double scale = dist_now / dist_last;

				// (!!! 按住 Shift 键 = 保持 X/Y 独立缩放，否则 = 统一缩放 !!!)
				double sx = scale, sy = scale;
				if (nFlags & MK_SHIFT) {
					double dx_last = max(1.0, (double)abs(lastPoint.x - m_refPoint.x));
					double dx_now = (double)abs(point.x - m_refPoint.x);
					double dy_last = max(1.0, (double)abs(lastPoint.y - m_refPoint.y));
					double dy_now = (double)abs(point.y - m_refPoint.y);
					sx = dx_now / dx_last;
					sy = dy_now / dy_last;
				}

				ApplyScalingToSelection(m_refPoint, sx, sy);
				Invalidate(FALSE);
			}
			else if (isRotating) // 3. 旋转
			{
				// (!!! 计算旋转角度：基于参考点与鼠标连线的角度变化 !!!)
				double angle_last = atan2(lastPoint.y - m_refPoint.y, lastPoint.x - m_refPoint.x);
				double angle_now = atan2(point.y - m_refPoint.y, point.x - m_refPoint.x);
				double deltaAngle = angle_now - angle_last;

				ApplyRotationToSelection(m_refPoint, deltaAngle);
				Invalidate(FALSE);
			}
		}
		else if (Mode == 9) { // BEZIER 拖动最近控制点
			if (isDrawing && bezierDragIdx >= 0) {
				currentBezierCtrl[bezierDragIdx] = point;
				Invalidate(FALSE);
			}
		}
		else if (Mode == 10) {
			if (isDrawing && bsplineDragIdx >= 0) {
				currentBSplineCtrl[bsplineDragIdx] = point;
				Invalidate(FALSE);
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
		else if (Mode == SELECT)
		{
			if (isDragging || isScaling || isRotating)
			{
				isDragging = false;
				isScaling = false;
				isRotating = false;
				m_bUseCustomRefPoint = false;
				Invalidate(FALSE); // 刷新最终状态
			}
		}
		else if (Mode == CONNECT)
		{
			CClientDC dc(this);
			if (m_connectStage == 0) {
				if (TryHitExisting(point, m_anchor)) {
					DrawAnchorDotXor(dc, m_anchor.hitPoint);
					m_anchorDraw = m_anchor.hitPoint;
					m_hasAnchorDraw = true;
					m_connectStage = 1;
				}
				else {
					AfxMessageBox(_T("请先点在第一个图元的轮廓/顶点上。"));
				}
				CDialogEx::OnLButtonDown(nFlags, point);
				return;
			}
			else {
				if (TryHitExisting(point, m_target)) {
					if (m_hasAnchorDraw) { DrawAnchorDotXor(dc, m_anchorDraw); m_hasAnchorDraw = false; }
					const int tx = m_anchor.hitPoint.x - m_target.hitPoint.x;
					const int ty = m_anchor.hitPoint.y - m_target.hitPoint.y;
					TranslateOne(m_target, tx, ty);
					m_connectStage = 0;
					Invalidate(FALSE);
				}
				else {
					AfxMessageBox(_T("第二次也需要点在另一个图元的轮廓/顶点上。"));
				}
				CDialogEx::OnLButtonDown(nFlags, point);
				return;
			}
		}
		else if (Mode == 9) 
		{
			// 只是结束拖动，真正“落盘”在 Ctrl 或右键
			bezierDragIdx = -1;
			Invalidate(FALSE);
		}
		else if (Mode == 10)
		{
			bsplineDragIdx = -1;
			Invalidate(FALSE);
		}

		dc.SelectObject(oldPen);
	}

	CDialogEx::OnLButtonUp(nFlags, point);
}
void CPaint3Dlg::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (nChar == VK_ESCAPE)
	{
		DeselectAll();
		Invalidate(FALSE);
	}
	// BEZIER 结束输入：Ctrl
	if (Mode == 9 && (nChar == VK_CONTROL)) {
		if (currentBezierCtrl.size() >= 2) {
			BezierObject bz{};
			bz.id = ++idBezier;
			bz.ctrl = currentBezierCtrl;
			bz.lineWidth = LineWidth;
			bz.lineType = LineType;
			bz.color = LineColor;
			bz.algorithm = ALG_BEZIER_CASTELJAU;
			bz.selected = false;
			bz.visible = true;
			bz.cacheValid = false;
			Beziers.push_back(std::move(bz));
		}
		currentBezierCtrl.clear();
		bezierDragIdx = -1;
		Invalidate(FALSE);
	}
	// BSPLINE 结束输入：Ctrl
	if (Mode == 10 && (nChar == VK_CONTROL)) {
		const int p = max(1, defaultBSplineDegree);
		if ((int)currentBSplineCtrl.size() >= p + 1) {
			BSplineObject sp{};
			sp.id = ++idBSpline;
			sp.ctrl = currentBSplineCtrl;
			sp.degree = p;
			sp.knots = {}; // 置空：由 EnsureBSplineCache 自动生成 clamped uniform
			sp.lineWidth = LineWidth;
			sp.lineType = LineType;
			sp.color = LineColor;
			sp.algorithm = ALG_BSPLINE_DEBOOR;
			sp.selected = false;
			sp.visible = true;
			sp.cacheValid = false;
			BSplines.push_back(std::move(sp));
		}
		else {
			AfxMessageBox(_T("控制点不足，至少需要 p+1 个。"));
		}
		currentBSplineCtrl.clear();
		bsplineDragIdx = -1;
		Invalidate(FALSE);
	}

	if (Mode == 4 && nChar == VK_CONTROL) // Ctrl 键
	{
		CClientDC dc(this);
		dc.SetROP2(R2_COPYPEN);
		int penStyle = LineType ? PS_DASH : PS_SOLID;
		LOGBRUSH logBrush = { BS_SOLID, LineColor, 0 };
		CPen pen(penStyle | PS_GEOMETRIC | PS_ENDCAP_ROUND, LineWidth, &logBrush);
		CPen* oldPen = dc.SelectObject(&pen);
		vector<CPoint> clippedPoly;
		if (DefinedClipPoly || DefinedClipRect)
		{
			//AfxMessageBox(CString(to_wstring(CPolygons.size()).c_str()));
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

	// 切换模式时取消所有变换
	isDragging = false;
	isScaling = false;
	isRotating = false;
	DeselectAll();
	Invalidate(FALSE);

	AlgMenu::RefreshAlgorithmListForMode(*this);
}
void CPaint3Dlg::OnCbnSelchangeCombo3()
{
	int sel = m_algorithm.GetCurSel();
	if (sel >= 0) {
		Algorithm = static_cast<int>(m_algorithm.GetItemData(sel));
	}

	// 切换模式取消所有变换
	isDragging = false;
	isScaling = false;
	isRotating = false;
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

void CPaint3Dlg::OnRButtonDown(UINT nFlags, CPoint point)
{
	if (Mode != SELECT || !HasAnySelection()) {
		CDialogEx::OnRButtonDown(nFlags, point);
		return;
	}
	// 右键：在“重心参考点”<->“鼠标位置自定义参考点”之间切换
	if (!m_bUseCustomRefPoint) {
		// 切到自定义：就地取鼠标点
		m_bUseCustomRefPoint = true;
		m_refPoint = point;
	}
	else {
		// 切回重心：实时计算选中集的平均重心
		m_bUseCustomRefPoint = false;
		m_refPoint = GetAverageCentroidOfSelection(); // 你已实现的函数
	}
	Invalidate(FALSE); // 立刻刷新，把小点画出来/移走
	CDialogEx::OnRButtonDown(nFlags, point);
}


// 双缓冲
BOOL CPaint3Dlg::OnEraseBkgnd(CDC* /*pDC*/)
{
	// 交由双缓冲背景填充，避免多余擦除造成闪烁
	return TRUE;
}
bool CPaint3Dlg::HasAnySelection() const {
	for (const auto& x : Lines)     if (x.visible && x.selected) return true;
	for (const auto& x : Ellipses)  if (x.visible && x.selected) return true;
	for (const auto& x : Polygons)  if (x.visible && x.selected) return true;
	for (const auto& x : Arcs)      if (x.visible && x.selected) return true;
	for (const auto& x : Beziers) if (x.visible && x.selected) return true;
	return false;
}

void CPaint3Dlg::DrawRefPoint(CDC& dc, const CPoint& pt, bool custom) {
	// 用明显的颜色：自定义=洋红，默认重心=蓝色
	COLORREF c = custom ? RGB(220, 20, 60) : RGB(30, 144, 255);
	const int r = 4; // 半径像素
	LOGBRUSH lb{ BS_SOLID, c, 0 };
	CPen pen(PS_SOLID | PS_GEOMETRIC | PS_ENDCAP_ROUND, 1, &lb);
	CBrush brush(c);
	CPen* oldPen = dc.SelectObject(&pen);
	CBrush* oldBrush = dc.SelectObject(&brush);

	// 小圆点 + 十字，既醒目又不挡视线
	dc.Ellipse(pt.x - r, pt.y - r, pt.x + r, pt.y + r);
	dc.MoveTo(pt.x - 6, pt.y); dc.LineTo(pt.x + 6, pt.y);
	dc.MoveTo(pt.x, pt.y - 6); dc.LineTo(pt.x, pt.y + 6);

	dc.SelectObject(oldBrush);
	dc.SelectObject(oldPen);
}
void CPaint3Dlg::TranslateOne(const HitInfo& h, int tx, int ty)
{
	switch (h.kind) {
	case 1: { auto& L = Lines[h.index];  L.start += CPoint(tx, ty); L.end += CPoint(tx, ty); } break;
	case 2: { auto& E = Ellipses[h.index]; E.rect.OffsetRect(tx, ty); } break;
	case 3: { auto& A = Arcs[h.index]; A.start += CPoint(tx, ty); A.end += CPoint(tx, ty); } break;
	case 4: { auto& P = Polygons[h.index]; for (auto& p : P.points) p += CPoint(tx, ty); } break;
	default: break;
	}
}

// ====== 简单的 XOR 参考点（可见即所得，点击第二点后自动擦除） ======
void CPaint3Dlg::DrawAnchorDotXor(CDC& dc, const CPoint& p)
{
	dc.SetROP2(R2_NOTXORPEN);
	CPen pen(PS_SOLID, 1, RGB(0, 0, 0));
	CPen* old = dc.SelectObject(&pen);
	dc.MoveTo(p.x - 4, p.y); dc.LineTo(p.x + 5, p.y);
	dc.MoveTo(p.x, p.y - 4); dc.LineTo(p.x, p.y + 5);
	dc.Rectangle(p.x - 2, p.y - 2, p.x + 2, p.y + 2);
	dc.SelectObject(old);
}
// —— 内联小工具：线段投影 —— //
static inline CPoint ProjectPointToSegment(const CPoint& A, const CPoint& B, const CPoint& Q)
{
	const double ax = A.x, ay = A.y, bx = B.x, by = B.y, qx = Q.x, qy = Q.y;
	const double vx = bx - ax, vy = by - ay;
	const double len2 = vx * vx + vy * vy;
	if (len2 <= 1e-12) return A;
	double t = ((qx - ax) * vx + (qy - ay) * vy) / len2;
	t = max(0.0, min(1.0, t));
	return CPoint((long)(ax + t * vx + 0.5), (long)(ay + t * vy + 0.5));
}

// —— 内联小工具：椭圆边界的“中心射线投影”（足够稳，用于对齐） —— //
static inline CPoint RayProjectToEllipse(const CRect& rc, const CPoint& Q)
{
	const double cx = (rc.left + rc.right) * 0.5;
	const double cy = (rc.top + rc.bottom) * 0.5;
	const double a = max(1.0, fabs(rc.right - rc.left) * 0.5);
	const double b = max(1.0, fabs(rc.bottom - rc.top) * 0.5);
	double dx = Q.x - cx, dy = Q.y - cy;
	if (fabs(dx) < 1e-12 && fabs(dy) < 1e-12) return CPoint((long)cx, (long)cy);
	double scale = 1.0 / sqrt((dx * dx) / (a * a) + (dy * dy) / (b * b));
	return CPoint((long)(cx + dx * scale + 0.5), (long)(cy + dy * scale + 0.5));
}

// —— 仿照 SELECT 的遍历顺序做命中：命中即返 —— //
// 说明：与 SELECT 一致，先 Lines、再 Ellipses、再 Polygons、最后 Arcs。
//      命中判定复用 IsPointNear*，命中后组装 HitInfo。
//      命中点：Line/Poly 用线段投影，Ellipse 用中心射线投影，Arc 暂用鼠标点（已在弧上容差内）。
bool CPaint3Dlg::TryHitExisting(const CPoint& mouse, HitInfo& out)
{
	// 1) 直线
	for (int i = 0; i < (int)Lines.size(); ++i)
	{
		const auto& L = Lines[i];
		if (!L.visible) continue;
		if (IsPointNearLine(mouse, L)) // 你的现成命中函数
		{
			CPoint proj = ProjectPointToSegment(L.start, L.end, mouse);
			out = { 1, i, proj };
			return true;
		}
	}

	// 2) 椭圆/圆
	for (int i = 0; i < (int)Ellipses.size(); ++i)
	{
		const auto& E = Ellipses[i];
		if (!E.visible) continue;
		if (IsPointNearEllipse(mouse, E))
		{
			CPoint proj = RayProjectToEllipse(E.rect, mouse);
			out = { 2, i, proj };
			return true;
		}
	}

	// 3) 多边形（找距离最近的边做投影）
	for (int i = 0; i < (int)Polygons.size(); ++i)
	{
		const auto& P = Polygons[i];
		if (!P.visible) continue;
		if (IsPointNearPolygon(mouse, P))
		{
			// 以最近边作为对齐几何点（比鼠标原始点更“贴边”）
			CPoint best = mouse;
			double bestd2 = 1e100;
			const int n = (int)P.points.size();
			for (int k = 0; k < n; ++k)
			{
				CPoint a = P.points[k];
				CPoint b = P.points[(k + 1) % n];
				CPoint h = ProjectPointToSegment(a, b, mouse);
				double dx = mouse.x - h.x, dy = mouse.y - h.y;
				double d2 = dx * dx + dy * dy;
				if (d2 < bestd2) { bestd2 = d2; best = h; }
			}
			out = { 4, i, best };
			return true;
		}
	}

	// 4) 圆弧（先给鼠标点；若要“零缝”，可改成弧投影）
	for (int i = 0; i < (int)Arcs.size(); ++i)
	{
		const auto& A = Arcs[i];
		if (!A.visible) continue;
		if (IsPointNearArc(mouse, A))
		{
			out = { 3, i, mouse };
			return true;
		}
	}

	return false;
}
// 命中已选中 Bezier 的控制点：命中返回下标，未命中返回 -1
int CPaint3Dlg::HitBezierCtrlPoint(const BezierObject& bz, const CPoint& mouse, int radius)
{
	const int R2 = radius * radius;
	for (int i = 0; i < (int)bz.ctrl.size(); ++i) {
		int dx = bz.ctrl[i].x - mouse.x;
		int dy = bz.ctrl[i].y - mouse.y;
		if (dx * dx + dy * dy <= R2) return i;
	}
	return -1;
}
