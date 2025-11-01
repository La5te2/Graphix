
// Paint3Dlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "Paint3.h"
#include "Paint3Dlg.h"
#include "afxdialogex.h"
#include "std.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#include <corecrt_math_defines.h>
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
	, LineWidth(0)
	, LineType(0)
	, arcAngle(M_PI)
	, arcAngleDeg(0)
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
	ON_EN_CHANGE(IDC_EDIT1, &CPaint3Dlg::OnEnChangeEdit1)
	ON_BN_CLICKED(IDC_RADIO1, &CPaint3Dlg::OnBnClickedRadio1)
	ON_BN_CLICKED(IDC_RADIO2, &CPaint3Dlg::OnBnClickedRadio2)
	ON_CBN_SELCHANGE(IDC_COMBO1, &CPaint3Dlg::OnCbnSelchangeCombo1)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_CBN_SELCHANGE(IDC_COMBO2, &CPaint3Dlg::OnCbnSelchangeCombo2)
	ON_CBN_SELCHANGE(IDC_COMBO3, &CPaint3Dlg::OnCbnSelchangeCombo3)
	ON_EN_CHANGE(IDC_EDIT2, &CPaint3Dlg::OnEnChangeEdit2)
	ON_WM_KEYDOWN()
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
	m_mode.SetCurSel(0);
	m_algorithm.AddString(_T("Default Line"));
	m_algorithm.AddString(_T("DDA Line Algorithm"));
	m_algorithm.AddString(_T("Midpoint Line Algorithm"));
	m_algorithm.AddString(_T("Bresenham Line Algorithm"));
	m_algorithm.AddString(_T("Default Circle"));
	m_algorithm.AddString(_T("Midpoint Circle Algorithm"));
	m_algorithm.AddString(_T("Bresenham Circle Algorithm"));
	m_algorithm.AddString(_T("Bresenham Arc Algorithm"));
	m_algorithm.AddString(_T("Polygon Algorithm"));
	m_algorithm.SetCurSel(0);
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

void CPaint3Dlg::DrawLineDDA(CPoint p1, CPoint p2, CDC& dc)
{
	double dx = p2.x - p1.x;
	double dy = p2.y - p1.y;
	double steps = fabs(dx) > fabs(dy) ? fabs(dx) : fabs(dy);
	double xInc = dx / steps;
	double yInc = dy / steps;
	double x = p1.x;
	double y = p1.y;
	int dashLength = 12; // 虚线段长度
	int gapLength = 6; // 虚线间隔长度
	for (int i = 0; i <= steps; ++i)
	{
		bool drawPixel = true;
		if (LineType == 1) // 虚线
		{
			int patternLength = dashLength + gapLength;
			int pos = i % patternLength;
			if (pos >= dashLength) drawPixel = false;
		}
		if (drawPixel)
		{
			int halfW = max(1, LineWidth - 1) / 2;
			for (int wx = -halfW; wx <= halfW; ++wx)
			{
				for (int wy = -halfW; wy <= halfW; ++wy)
				{
					int px = int(x + 0.5) + wx;
					int py = int(y + 0.5) + wy;
					dc.SetPixelV(px, py, LineColor);
				}
			}
		}
		x += xInc;
		y += yInc;
	}
}

void CPaint3Dlg::DrawLineMidpoint(CPoint p1, CPoint p2, CDC& dc)
{
	int x1 = p1.x, y1 = p1.y;
	int x2 = p2.x, y2 = p2.y;
	int dx = abs(x2 - x1);
	int dy = abs(y2 - y1);
	int sx = (x1 < x2) ? 1 : -1;
	int sy = (y1 < y2) ? 1 : -1;
	bool steep = dy > dx;
	if (steep)
	{
		std::swap(x1, y1);
		std::swap(x2, y2);
		std::swap(dx, dy);
		std::swap(sx, sy);
	}
	int d = 2 * dy + dx;
	int y = y1;
	int dashLength = 12;
	int gapLength = 6;
	for (int i = 0; i <= dx; ++i)
	{
		bool drawPixel = true;
		if (LineType == 1) // 虚线
		{
			int pattern = dashLength + gapLength;
			if ((i % pattern) >= dashLength)
				drawPixel = false;
		}
		if (drawPixel)
		{
			int halfW = max(1, LineWidth) / 2;
			for (int wx = -halfW; wx <= halfW; ++wx)
			{
				for (int wy = -halfW; wy <= halfW; ++wy)
				{
					if (steep)
						dc.SetPixelV(y + wy, x1 + wx, LineColor);
					else
						dc.SetPixelV(x1 + wx, y + wy, LineColor);
				}
			}
		}
		if (d > 0)
		{
			y += sy;
			d -= 2 * dx;
		}
		d += 2 * dy;
		x1 += sx;
	}
}

void CPaint3Dlg::DrawLineBresenham(CPoint p1, CPoint p2, CDC& dc)
{
	int x1 = p1.x, y1 = p1.y;
	int x2 = p2.x, y2 = p2.y;

	int dx = abs(x2 - x1);
	int dy = abs(y2 - y1);
	int sx = (x1 < x2) ? 1 : -1;
	int sy = (y1 < y2) ? 1 : -1;

	bool steep = dy > dx;
	if (steep)
	{
		std::swap(x1, y1);
		std::swap(x2, y2);
		std::swap(dx, dy);
		std::swap(sx, sy);
	}

	int err = -2 * abs(dy - dx);
	int y = y1;

	int dashLength = 6; // 实线段长度
	int gapLength = 3;  // 虚线间隔长度

	for (int i = 0; i <= dx; ++i)
	{
		bool drawPixel = true;
		// 线型控制（虚线）
		if (LineType == 1)
		{
			int pattern = dashLength + gapLength;
			if ((i % pattern) >= dashLength)
				drawPixel = false;
		}
		if (drawPixel)
		{
			int halfW = max(1, LineWidth) / 2;
			for (int wx = -halfW; wx <= halfW; ++wx)
			{
				for (int wy = -halfW; wy <= halfW; ++wy)
				{
					if (steep)
						dc.SetPixelV(y + wy, x1 + wx, LineColor);
					else
						dc.SetPixelV(x1 + wx, y + wy, LineColor);
				}
			}
		}
		if (err > 0)
		{
			y += sy;
			err -= 2 * dx;
		}
		err += 2 * dy;
		x1 += sx;
	}
}

void CPaint3Dlg::DrawEllipseMidpoint(CDC& dc, const CRect& rect)
{
	int xc = (rect.left + rect.right) / 2;
	int yc = (rect.top + rect.bottom) / 2;
	int a = abs(rect.right - rect.left) / 2;
	int b = abs(rect.bottom - rect.top) / 2;

	double a2 = a * a;
	double b2 = b * b;
	double d1 = b2 - a2 * b + 0.25 * a2;
	int x = 0;
	int y = b;

	int halfW = max(1, LineWidth) / 2;
	int dashLength = 6;
	int gapLength = 6;
	int patternLength = dashLength + gapLength;
	int step = 0;

	auto drawPixel = [&](int px, int py) {
		if (LineType == 1) // 虚线
		{
			int pos = step % patternLength;
			if (pos >= dashLength) return;
		}

		for (int wx = -halfW; wx <= halfW; ++wx)
			for (int wy = -halfW; wy <= halfW; ++wy)
				dc.SetPixelV(px + wx, py + wy, LineColor);
	};

	long dx = 2 * b2 * x;
	long dy = 2 * a2 * y;

	while (dx < dy)
	{
		drawPixel(xc + x, yc + y);
		drawPixel(xc - x, yc + y);
		drawPixel(xc + x, yc - y);
		drawPixel(xc - x, yc - y);
		step++;

		if (d1 < 0)
		{
			x++;
			dx += 2 * b2;
			d1 += dx + b2;
		}
		else
		{
			x++;
			y--;
			dx += 2 * b2;
			dy -= 2 * a2;
			d1 += dx - dy + b2;
		}
	}
	long d2 = (long)(b2 * (x + 0.5) * (x + 0.5)
		+ a2 * (y - 1) * (y - 1)
		- a2 * b2);

	while (y >= 0)
	{
		drawPixel(xc + x, yc + y);
		drawPixel(xc - x, yc + y);
		drawPixel(xc + x, yc - y);
		drawPixel(xc - x, yc - y);
		step++;

		if (d2 > 0)
		{
			y--;
			dy -= 2 * a2;
			d2 += a2 - dy;
		}
		else
		{
			y--;
			x++;
			dx += 2 * b2;
			dy -= 2 * a2;
			d2 += dx - dy + a2;
		}
	}
}
void CPaint3Dlg::DrawEllipseBresenham(CDC& dc, const CRect& rect)
{
	// 计算椭圆的中心和半轴长度
	int xc = (rect.left + rect.right) / 2;
	int yc = (rect.top + rect.bottom) / 2;
	int a = abs(rect.right - rect.left) / 2;
	int b = abs(rect.bottom - rect.top) / 2;

	// 参数合法性检查
	if (a <= 0 || b <= 0) return;

	int x = 0, y = b;

	// 使用整数运算避免精度问题
	long long a2 = 1LL * a * a;
	long long b2 = 1LL * b * b;
	long long d1 = b2 - a2 * b + a2 / 4;
	long long d2;

	// 绘制四对称点 + 支持线宽
	auto plot = [&](int px, int py, int index) {
		// 虚线控制：每隔一定像素空一段
		if (LineType == 1 && (index / 5) % 2 == 0) return;

		// 用线宽加粗：在主方向上扩展像素
		for (int dx = -LineWidth / 2; dx <= LineWidth / 2; ++dx)
		{
			for (int dy = -LineWidth / 2; dy <= LineWidth / 2; ++dy)
			{
				dc.SetPixelV(xc + px + dx, yc + py + dy, LineColor);
				dc.SetPixelV(xc - px + dx, yc + py + dy, LineColor);
				dc.SetPixelV(xc + px + dx, yc - py + dy, LineColor);
				dc.SetPixelV(xc - px + dx, yc - py + dy, LineColor);
			}
		}
		};

	int index = 0;
	plot(x, y, index);

	// 区域1：斜率绝对值 > 1
	while (b2 * (x + 1) < a2 * (y - 1))
	{
		if (d1 < 0)
		{
			d1 += b2 * (2 * x + 3);
			x++;
		}
		else
		{
			d1 += b2 * (2 * x + 3) + a2 * (-2 * y + 2);
			x++;
			y--;
		}
		++index;
		plot(x, y, index);
	}

	// 区域2：斜率绝对值 <= 1
	d2 = b2 * (x + 1) * (x + 1) + a2 * (y - 1) * (y - 1) - a2 * b2;
	while (y > 0)
	{
		if (d2 < 0)
		{
			d2 += b2 * (2 * x + 2) + a2 * (-2 * y + 3);
			x++;
			y--;
		}
		else
		{
			d2 += a2 * (-2 * y + 3);
			y--;
		}
		++index;
		plot(x, y, index);
	}
}

void CPaint3Dlg::DrawArc(float angle, bool direction, CPoint p1, CPoint p2, CDC& dc)
{
	if (fabs(angle) < 1e-6) {
		dc.SetPixel(p1.x, p1.y, LineColor);
		return;
	}

	double x1 = p1.x, y1 = p1.y;
	double x2 = p2.x, y2 = p2.y;
	double dx = x2 - x1, dy = y2 - y1;
	double d = sqrt(dx * dx + dy * dy);
	if (d < 1e-6) return;

	double halfAngle = fabs(angle) / 2.0;
	double r = (d / 2.0) / sin(halfAngle);
	double h = sqrt(max(0.0, r * r - (d / 2.0) * (d / 2.0)));

	// 中点与垂线方向
	double mx = (x1 + x2) / 2.0;
	double my = (y1 + y2) / 2.0;
	double ux = -dy / d, uy = dx / d;

	// 候选圆心
	double cx1 = mx + ux * h;
	double cy1 = my + uy * h;
	double cx2 = mx - ux * h;
	double cy2 = my - uy * h;

	auto arcSpan = [&](double cx, double cy) {
		double a1 = atan2(y1 - cy, x1 - cx);
		double a2 = atan2(y2 - cy, x2 - cx);
		double da = a2 - a1;
		if (da < 0) da += 2 * M_PI;
		return da; // CCW span from p1 to p2 in [0, 2PI)
	};

	double span1 = arcSpan(cx1, cy1);
	double span2 = arcSpan(cx2, cy2);

	// 目标弧长（按正方向），规范到 [0, 2PI)
	double target = fmod(fabs(angle), 2 * M_PI);
	if (target < 0) target += 2 * M_PI;
	double desiredSpan;
	if (direction) {
		// 逆时针 -> 直接和 target 比
		desiredSpan = target;
	}
	else {
		// 顺时针 -> 要比较的是 2PI - target（对应的 CCW 跨度应接近 2PI - target）
		desiredSpan = fmod(2 * M_PI - target, 2 * M_PI);
	}

	double diff1 = fabs(span1 - desiredSpan);
	double diff2 = fabs(span2 - desiredSpan);

	double cx, cy;
	if (diff1 < diff2)
		cx = cx1, cy = cy1;
	else
		cx = cx2, cy = cy2;

	// 根据 direction 决定绘制方向
	double startA = atan2(y1 - cy, x1 - cx);
	// 步长计算
	double arcLen = fabs(angle * r);
	int steps = max(2, (int)ceil(arcLen / 0.8));

	/*for (int i = 0; i <= steps; ++i) {
		double t = (double)i / steps;
		double theta = direction ? (startA + t * angle) : (startA - t * angle);
		int sx = (int)round(cx + r * cos(theta));
		int sy = (int)round(cy + r * sin(theta));
		dc.SetPixel(sx, sy, LineColor);
	}

	dc.SetPixel(p1.x, p1.y, LineColor);
	dc.SetPixel(p2.x, p2.y, LineColor);*/
	for (int i = 0; i <= steps; ++i)
	{
		double t = (double)i / steps;
		double theta = direction ? (startA + t * angle) : (startA - t * angle);
		int sx = (int)round(cx + r * cos(theta));
		int sy = (int)round(cy + r * sin(theta));
		bool drawPixel = true;
		if (LineType == 1) // 虚线
		{
			const int dashLength = max(2, LineWidth * 4);
			const int gapLength = max(2, LineWidth * 2);
			int patternLength = dashLength + gapLength;
			int patternPos = i % patternLength;
			drawPixel = (patternPos < dashLength);
		}
		if (drawPixel)
		{
			int radius = max(1, LineWidth / 2);
			for (int dx = -radius; dx <= radius; ++dx)
			{
				for (int dy = -radius; dy <= radius; ++dy)
				{
					if (dx * dx + dy * dy <= radius * radius)
					{
						dc.SetPixelV(sx + dx, sy + dy, LineColor);
					}
				}
			}
		}
	}

	// 确保端点也绘制
	int radius = max(1, LineWidth / 2);
	for (int dx = -radius; dx <= radius; ++dx)
	{
		for (int dy = -radius; dy <= radius; ++dy)
		{
			if (dx * dx + dy * dy <= radius * radius)
			{
				dc.SetPixelV(p1.x + dx, p1.y + dy, LineColor);
				dc.SetPixelV(p2.x + dx, p2.y + dy, LineColor);
			}
		}
	}
}
void CPaint3Dlg::DrawArcPreview(float angle, bool direction, CPoint p1, CPoint p2, CDC& dc)
{
	if (fabs(angle) < 1e-6) {
		dc.MoveTo(p1);
		dc.LineTo(p2);
		return;
	}

	double x1 = p1.x, y1 = p1.y;
	double x2 = p2.x, y2 = p2.y;
	double dx = x2 - x1, dy = y2 - y1;
	double d = sqrt(dx * dx + dy * dy);
	if (d < 1e-6) return;

	double halfAngle = fabs(angle) / 2.0;
	double r = (d / 2.0) / sin(halfAngle);
	double h = sqrt(max(0.0, r * r - (d / 2.0) * (d / 2.0)));

	// 中点与垂线方向
	double mx = (x1 + x2) / 2.0;
	double my = (y1 + y2) / 2.0;
	double ux = -dy / d, uy = dx / d;

	// 候选圆心
	double cx1 = mx + ux * h;
	double cy1 = my + uy * h;
	double cx2 = mx - ux * h;
	double cy2 = my - uy * h;

	auto arcSpan = [&](double cx, double cy) {
		double a1 = atan2(y1 - cy, x1 - cx);
		double a2 = atan2(y2 - cy, x2 - cx);
		double da = a2 - a1;
		if (da < 0) da += 2 * M_PI;
		return da; // CCW span
		};

	// CCW跨度
	double span1 = arcSpan(cx1, cy1);
	double span2 = arcSpan(cx2, cy2);

	// 目标弧度长度
	double target = fmod(fabs(angle), 2 * M_PI);
	if (target < 0) target += 2 * M_PI;

	// 根据方向调整比较的跨度
	double desiredSpan;
	if (direction) {
		desiredSpan = target; // 逆时针
	}
	else {
		desiredSpan = fmod(2 * M_PI - target, 2 * M_PI); // 顺时针
	}

	// 选出合适圆心
	double diff1 = fabs(span1 - desiredSpan);
	double diff2 = fabs(span2 - desiredSpan);
	double cx, cy;
	if (diff1 < diff2)
		cx = cx1, cy = cy1;
	else
		cx = cx2, cy = cy2;

	// 计算实际起始角度（数学坐标系下，逆时针为正）
	double startRad = atan2(y1 - cy, x1 - cx);
	double sweepRad = direction ? angle : -angle;

	double startDeg = -startRad * 180.0 / M_PI;
	double sweepDeg = -sweepRad * 180.0 / M_PI;

	double sx = cx + r * cos(startRad);
	double sy = cy + r * sin(startRad);
	dc.MoveTo((int)round(sx), (int)round(sy));
	dc.AngleArc((int)round(cx), (int)round(cy), (int)round(r), (float)startDeg, (float)sweepDeg);
}

void CPaint3Dlg::ScanConvertPolygonOutline(CDC& dc, const std::vector<CPoint>& poly, COLORREF color)
{
	if (poly.size() < 2) return;
	for (size_t i = 0; i < poly.size(); ++i) {
		CPoint a = poly[i];
		CPoint b = poly[(i + 1) % poly.size()];
		DrawLineBresenham(a, b, dc);
	}
	if (!IsFill) return;

	// --- 3️⃣ 扫描线填充算法 ---
	// 获取多边形的 y 范围
	int ymin = poly[0].y, ymax = poly[0].y;
	for (const auto& p : poly) {
		ymin = min(ymin, p.y);
		ymax = max(ymax, p.y);
	}

	// --- 4️⃣ 对每条扫描线求交点 ---
	for (int y = ymin; y <= ymax; ++y)
	{
		std::vector<int> xIntersections;

		for (size_t i = 0; i < poly.size(); ++i)
		{
			CPoint p1 = poly[i];
			CPoint p2 = poly[(i + 1) % poly.size()];

			// 保证 p1.y <= p2.y
			if (p1.y > p2.y) std::swap(p1, p2);

			// 跳过不相交的边
			if (y < p1.y || y >= p2.y) continue;

			// 计算交点 x 坐标（线性插值）
			if (p2.y != p1.y) {
				double x = p1.x + (double)(y - p1.y) * (p2.x - p1.x) / (p2.y - p1.y);
				xIntersections.push_back((int)round(x));
			}
		}

		// --- 5️⃣ 排序交点并两两连线 ---
		std::sort(xIntersections.begin(), xIntersections.end());
		for (size_t k = 0; k + 1 < xIntersections.size(); k += 2)
		{
			int xStart = xIntersections[k];
			int xEnd = xIntersections[k + 1];
			for (int x = xStart; x <= xEnd; ++x)
				dc.SetPixel(x, y, ShapeColor);
		}
	}
}

void CPaint3Dlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	isDrawing = true;
	startPoint = lastPoint = point;
	CDialogEx::OnLButtonDown(nFlags, point);
	if (Mode == 0)
	{
		vector<CPoint> newStroke;
		newStroke.push_back(point);
		Pens.push_back(newStroke);
		PenColors.push_back(LineColor);
	}
}
void CPaint3Dlg::OnMouseMove(UINT nFlags, CPoint point)
{
	if (isDrawing)
	{
		CClientDC dc(this);
		dc.SetROP2(R2_NOTXORPEN);

		CPen pen(PS_SOLID, 1, LineColor);
		CPen* oldPen = dc.SelectObject(&pen);

		if (Mode == 0) 
		{
			Pens.back().push_back(point);
			dc.MoveTo(lastPoint);
			dc.LineTo(point);
		}

		else if (Mode == 1) // Line
		{
			dc.MoveTo(startPoint);
			dc.LineTo(lastPoint);

			dc.MoveTo(startPoint);
			dc.LineTo(point);
		}
		else if (Mode == 2)
		{
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
		}
		
		else if (Mode == 3) // Arc Preview
		{
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
		}
		else if (Mode == 4)
		{
			// 多边形模式下的预览线段
			if (currentPolygon.size() > 0)
			{
				// 擦除旧线段
				// 绘制新线段
			}
		}

		dc.SelectObject(oldPen);
		lastPoint = point;
	}
}

void CPaint3Dlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (isDrawing)
	{
		isDrawing = false;

		CClientDC dc(this);
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
			Lines.push_back(make_pair(startPoint, endPoint));
			if (Algorithm == 0) // Default line
			{
				dc.MoveTo(startPoint);
				dc.LineTo(endPoint);
			}
			else if (Algorithm == 1) // DDA line algorithm
			{
				DrawLineDDA(startPoint, endPoint, dc);
			}
			else if (Algorithm == 2) // Midpoint line algorithm
			{
				DrawLineMidpoint(startPoint, endPoint, dc);
			}
			else if (Algorithm == 3) // Bresenham line algorithm
			{
				DrawLineBresenham(startPoint, endPoint, dc);
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

			Ellipses.push_back(rect);

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
				if(Algorithm == 4) // Default Circle
				{
					dc.Ellipse(rect);
				}
				else if(Algorithm == 5) // Midpoint Circle
				{
					DrawEllipseMidpoint(dc, rect);
				}
				else if(Algorithm == 6) // Bresenham Circle
				{
					DrawEllipseBresenham(dc, rect);
				}
				dc.SelectObject(oldBrush);
			}
		}
		else if (Mode == 3) // Arc
		{
			Arcs.push_back(make_pair(startPoint, endPoint));
			hasLastDrawArc = false;
			bool direction = true; // 默认逆时针
			if (GetKeyState(VK_SHIFT) & 0x8000)
				direction = false; // 顺时针
			if (Algorithm == 7) // Bresenham Arc
			{
				arcAngle = arcAngleDeg * M_PI / 180.0;
				DrawArc(arcAngle, direction, startPoint, endPoint, dc);
			}
		}
		else if (Mode == 4) // Polygon
		{	
			currentPolygon.push_back(point);
		}
		dc.SelectObject(oldPen);
	}

	CDialogEx::OnLButtonUp(nFlags, point);
}
void CPaint3Dlg::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (Mode == 4 && nChar == VK_CONTROL) // Ctrl 键
	{
		if (currentPolygon.size() >= 3)
		{
			CClientDC dc(this);
			dc.SetROP2(R2_COPYPEN);
			int penStyle = LineType ? PS_DASH : PS_SOLID;
			LOGBRUSH logBrush = { BS_SOLID, LineColor, 0 };
			CPen pen(penStyle | PS_GEOMETRIC | PS_ENDCAP_ROUND, LineWidth, &logBrush);
			CPen* oldPen = dc.SelectObject(&pen);
			ScanConvertPolygonOutline(dc, currentPolygon, LineColor);
			Polygons.push_back(currentPolygon);
			currentPolygon.clear();
			dc.SelectObject(oldPen);
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

void CPaint3Dlg::OnEnChangeEdit1()
{
	UpdateData(TRUE); // 将控件的值更新到变量
}
void CPaint3Dlg::OnEnChangeEdit2()
{
	UpdateData(TRUE); // 将控件的值更新到变量
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
}
void CPaint3Dlg::OnCbnSelchangeCombo3()
{
	int sel = m_algorithm.GetCurSel();
	Algorithm = sel;
}